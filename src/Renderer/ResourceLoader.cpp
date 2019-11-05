/*
 * Copyright (c) 2018-2019 Confetti Interactive Inc.
 *
 * This file is part of The-Forge
 * (see https://github.com/ConfettiFX/The-Forge).
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
*/

#define IMAGE_CLASS_ALLOWED

// this is needed for the CopyEngine on XBox
#ifdef _DURANGO
#include "../../Xbox/Common_3/Renderer/XBoxPrivateHeaders.h"
#endif

#include "../ThirdParty/OpenSource/EASTL/deque.h"
#include "al2o3_memory/memory.h"
#include "IRenderer.h"
#include "ResourceLoader.h"
#include "../OS/Interfaces/ILog.h"
#include "../OS/Interfaces/IThread.h"
#include "../OS/Interfaces/ITime.h"
#include "../OS/Image/Image.h"

//this is needed for unix as PATH_MAX is defined instead of MAX_PATH
#ifndef _WIN32
//linux needs limits.h for PATH_MAX
#ifdef __linux__
#include <limits.h>
#endif
#if defined(__ANDROID__)
#include <shaderc/shaderc.h>
#endif
#define MAX_PATH PATH_MAX
#endif
#include "../OS/Interfaces/IMemory.h"

extern void addBuffer(Renderer* pRenderer, const BufferDesc* desc, Buffer** pp_buffer);
extern void removeBuffer(Renderer* pRenderer, Buffer* p_buffer);
extern void mapBuffer(Renderer* pRenderer, Buffer* pBuffer, ReadRange* pRange);
extern void unmapBuffer(Renderer* pRenderer, Buffer* pBuffer);
extern void addTexture(Renderer* pRenderer, const TextureDesc* pDesc, Texture** pp_texture);
extern void removeTexture(Renderer* pRenderer, Texture* p_texture);
extern void cmdUpdateBuffer(Cmd* pCmd, Buffer* pBuffer, uint64_t dstOffset, Buffer* pSrcBuffer, uint64_t srcOffset, uint64_t size);
extern void cmdUpdateSubresource(Cmd* pCmd, Texture* pTexture, Buffer* pSrcBuffer, SubresourceDataDesc* pSubresourceDesc);
/************************************************************************/
/************************************************************************/

//////////////////////////////////////////////////////////////////////////
// Internal TextureUpdateDesc
// Used internally as to not expose Image class in the public interface
//////////////////////////////////////////////////////////////////////////
typedef struct TextureUpdateDescInternal
{
	Texture* pTexture;
	Image*   pImage;
	bool     mFreeImage;
} TextureUpdateDescInternal;

//////////////////////////////////////////////////////////////////////////
// Resource CopyEngine Structures
//////////////////////////////////////////////////////////////////////////
typedef struct MappedMemoryRange
{
	uint8_t* pData;
	Buffer*  pBuffer;
	uint64_t mOffset;
	uint64_t mSize;
} MappedMemoryRange;

typedef struct ResourceSet
{
	Fence*  pFence;
#ifdef _DURANGO
	DmaCmd* pCmd;
#else
	Cmd*    pCmd;
#endif
	Buffer* mBuffer;
} CopyResourceSet;

enum
{
	DEFAULT_BUFFER_SIZE = 16ull<<20,
	DEFAULT_BUFFER_COUNT = 2u,
	DEFAULT_TIMESLICE_MS = 4u,
	MAX_BUFFER_COUNT = 8u,
};

//Synchronization?
typedef struct CopyEngine
{
	Queue*      pQueue;
	CmdPool*    pCmdPool;
	ResourceSet* resourceSets;
	uint64_t    bufferSize;
	uint64_t    allocatedSpace;
	uint32_t    bufferCount;
	bool        isRecording;
} CopyEngine;

//////////////////////////////////////////////////////////////////////////
// Resource Loader Internal Functions
//////////////////////////////////////////////////////////////////////////
static void setupCopyEngine(Renderer* pRenderer, CopyEngine* pCopyEngine, uint32_t nodeIndex, uint64_t size, uint32_t bufferCount)
{
	QueueDesc desc = { QUEUE_FLAG_NONE, QUEUE_PRIORITY_NORMAL, CMD_POOL_COPY, nodeIndex };
	addQueue(pRenderer, &desc, &pCopyEngine->pQueue);

	addCmdPool(pRenderer, pCopyEngine->pQueue, false, &pCopyEngine->pCmdPool);

	const uint32_t maxBlockSize = 32;
	uint64_t       minUploadSize = pCopyEngine->pQueue->mUploadGranularity.mWidth * pCopyEngine->pQueue->mUploadGranularity.mHeight *
							 pCopyEngine->pQueue->mUploadGranularity.mDepth * maxBlockSize;
	size = max(size, minUploadSize);

	pCopyEngine->resourceSets = (ResourceSet*)conf_malloc(sizeof(ResourceSet)*bufferCount);
	for (uint32_t i=0;  i < bufferCount; ++i)
	{
		ResourceSet& resourceSet = pCopyEngine->resourceSets[i];
		addFence(pRenderer, &resourceSet.pFence);

		addCmd(pCopyEngine->pCmdPool, false, &resourceSet.pCmd);

		BufferDesc bufferDesc = {};
		bufferDesc.mSize = size;
		bufferDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_ONLY;
		bufferDesc.mFlags = BUFFER_CREATION_FLAG_OWN_MEMORY_BIT | BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
		bufferDesc.mNodeIndex = nodeIndex;
		addBuffer(pRenderer, &bufferDesc, &resourceSet.mBuffer);
	}

	pCopyEngine->bufferSize = size;
	pCopyEngine->bufferCount = bufferCount;
	pCopyEngine->allocatedSpace = 0;
	pCopyEngine->isRecording = false;
}

static void cleanupCopyEngine(Renderer* pRenderer, CopyEngine* pCopyEngine)
{
	for (uint32_t i = 0; i < pCopyEngine->bufferCount; ++i)
	{
		ResourceSet& resourceSet = pCopyEngine->resourceSets[i];
		removeBuffer(pRenderer, resourceSet.mBuffer);

		removeCmd(pCopyEngine->pCmdPool, resourceSet.pCmd);

		removeFence(pRenderer, resourceSet.pFence);
	}
	
	conf_free(pCopyEngine->resourceSets);

	removeCmdPool(pRenderer, pCopyEngine->pCmdPool);

	removeQueue(pCopyEngine->pQueue);
}

static void waitCopyEngineSet(Renderer* pRenderer, CopyEngine* pCopyEngine, size_t activeSet)
{
	ASSERT(!pCopyEngine->isRecording);
	ResourceSet& resourceSet = pCopyEngine->resourceSets[activeSet];
	waitForFences(pRenderer, 1, &resourceSet.pFence);
}

static void resetCopyEngineSet(Renderer* pRenderer, CopyEngine* pCopyEngine, size_t activeSet)
{
	ASSERT(!pCopyEngine->isRecording);
	pCopyEngine->allocatedSpace = 0;
	pCopyEngine->isRecording = false;
}

#ifdef _DURANGO
static DmaCmd* aquireCmd(CopyEngine* pCopyEngine, size_t activeSet)
{
	ResourceSet& resourceSet = pCopyEngine->resourceSets[activeSet];
	if (!pCopyEngine->isRecording)
	{
		beginCmd(resourceSet.pCmd);
		pCopyEngine->isRecording = true;
	}
	return resourceSet.pCmd;
}
#else
static Cmd* aquireCmd(CopyEngine* pCopyEngine, size_t activeSet)
{
	ResourceSet& resourceSet = pCopyEngine->resourceSets[activeSet];
	if (!pCopyEngine->isRecording)
	{
		beginCmd(resourceSet.pCmd);
		pCopyEngine->isRecording = true;
	}
	return resourceSet.pCmd;
}
#endif

static void streamerFlush(CopyEngine* pCopyEngine, size_t activeSet)
{
	if (pCopyEngine->isRecording)
	{
		ResourceSet& resourceSet = pCopyEngine->resourceSets[activeSet];
		endCmd(resourceSet.pCmd);
		queueSubmit(pCopyEngine->pQueue, 1, &resourceSet.pCmd, resourceSet.pFence, 0, 0, 0, 0);
		pCopyEngine->isRecording = false;
	}
}

/// Return memory from pre-allocated staging buffer or create a temporary buffer if the streamer ran out of memory
static MappedMemoryRange allocateStagingMemory(Renderer* pRenderer, CopyEngine* pCopyEngine, size_t activeSet, uint64_t memoryRequirement, uint32_t alignment)
{
	uint64_t offset = pCopyEngine->allocatedSpace;
	if (alignment != 0)
	{
		offset = round_up_64(offset, alignment);
	}

	CopyResourceSet* pResourceSet = &pCopyEngine->resourceSets[activeSet];
	uint64_t size = pResourceSet->mBuffer->mDesc.mSize;
	bool memoryAvailable = (offset < size) && (memoryRequirement <= size - offset);
	if (memoryAvailable)
	{
		Buffer* buffer = pResourceSet->mBuffer;
#if defined(DIRECT3D11)
		// TODO: do done once, unmap before queue submit
	mapBuffer(pRenderer, buffer, NULL);
#endif
		uint8_t* pDstData = (uint8_t*)buffer->pCpuMappedAddress + offset;
		pCopyEngine->allocatedSpace = offset + memoryRequirement;
		return { pDstData, buffer, offset, memoryRequirement };
	}

	return { nullptr, nullptr, 0, 0};
}

static ResourceState util_determine_resource_start_state(DescriptorType usage)
{
	ResourceState state = RESOURCE_STATE_UNDEFINED;
	if (usage & DESCRIPTOR_TYPE_RW_TEXTURE)
		return RESOURCE_STATE_UNORDERED_ACCESS;
	else if (usage & DESCRIPTOR_TYPE_TEXTURE)
		return RESOURCE_STATE_SHADER_RESOURCE;
	return state;
}

static ResourceState util_determine_resource_start_state(const BufferDesc* pBuffer)
{
	// Host visible (Upload Heap)
	if (pBuffer->mMemoryUsage == RESOURCE_MEMORY_USAGE_CPU_ONLY || pBuffer->mMemoryUsage == RESOURCE_MEMORY_USAGE_CPU_TO_GPU)
	{
		return RESOURCE_STATE_GENERIC_READ;
	}
	// Device Local (Default Heap)
	else if (pBuffer->mMemoryUsage == RESOURCE_MEMORY_USAGE_GPU_ONLY)
	{
		DescriptorType usage = pBuffer->mDescriptors;

		// Try to limit number of states used overall to avoid sync complexities
		if (usage & DESCRIPTOR_TYPE_RW_BUFFER)
			return RESOURCE_STATE_UNORDERED_ACCESS;
		if ((usage & DESCRIPTOR_TYPE_VERTEX_BUFFER) || (usage & DESCRIPTOR_TYPE_UNIFORM_BUFFER))
			return RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		if (usage & DESCRIPTOR_TYPE_INDEX_BUFFER)
			return RESOURCE_STATE_INDEX_BUFFER;
		if ((usage & DESCRIPTOR_TYPE_BUFFER))
			return RESOURCE_STATE_SHADER_RESOURCE;

		return RESOURCE_STATE_COMMON;
	}
	// Host Cached (Readback Heap)
	else
	{
		return RESOURCE_STATE_COPY_DEST;
	}
}

typedef enum UpdateRequestType
{
	UPDATE_REQUEST_UPDATE_BUFFER,
	UPDATE_REQUEST_UPDATE_TEXTURE,
	UPDATE_REQUEST_UPDATE_RESOURCE_STATE,
	UPDATE_REQUEST_INVALID,
} UpdateRequestType;

typedef struct UpdateRequest
{
	UpdateRequest() : mType(UPDATE_REQUEST_INVALID) {}
	UpdateRequest(BufferUpdateDesc& buffer) : mType(UPDATE_REQUEST_UPDATE_BUFFER), bufUpdateDesc(buffer) {}
	UpdateRequest(TextureUpdateDescInternal& texture) : mType(UPDATE_REQUEST_UPDATE_TEXTURE), texUpdateDesc(texture) {}
	UpdateRequest(Buffer* buf) : mType(UPDATE_REQUEST_UPDATE_RESOURCE_STATE) { buffer = buf; texture = NULL; }
	UpdateRequest(Texture* tex) : mType(UPDATE_REQUEST_UPDATE_RESOURCE_STATE) { texture = tex; buffer = NULL; }
	UpdateRequestType mType;
	SyncToken mToken = 0;
	union
	{
		BufferUpdateDesc bufUpdateDesc;
		TextureUpdateDescInternal texUpdateDesc;
		struct { Buffer* buffer; Texture* texture; };
	};
} UpdateRequest;

typedef struct UpdateState
{
	UpdateState(): UpdateState(UpdateRequest())
	{
	}
	UpdateState(const UpdateRequest& request): mRequest(request), mMipLevel(0), mArrayLayer(0), mOffset({ 0, 0, 0 }), mSize(0)
	{
	}

	UpdateRequest mRequest;
	uint32_t      mMipLevel;
	uint32_t      mArrayLayer;
	uint3         mOffset;
	uint64_t      mSize;
} UpdateState;

class ResourceLoader
{
public:
	Renderer* pRenderer;

	ResourceLoaderDesc mDesc;

	volatile int mRun;
	ThreadDesc   mThreadDesc;
	ThreadHandle mThread;

	Mutex mQueueMutex;
	ConditionVariable mQueueCond;
	Mutex mTokenMutex;
	ConditionVariable mTokenCond;
	eastl::deque <UpdateRequest> mRequestQueue[MAX_GPUS];

	tfrg_atomic64_t mTokenCompleted;
	tfrg_atomic64_t mTokenCounter;

	static Image* AllocImage()
	{
		return conf_new(Image);
	}

	static Image* CreateImage(const TinyImageFormat fmt, const int w, const int h, const int d, const int mipMapCount, const int arraySize, const unsigned char* rawData)
	{
		Image* pImage = AllocImage();
		pImage->Create(fmt, w, h, d, mipMapCount, arraySize, rawData);
		return pImage;
	}

	static void DestroyImage(Image* pImage)
	{
		pImage->Destroy();
		conf_delete(pImage);
	}
};

uint32 SplitBitsWith0(uint32 x)
{
	x &= 0x0000ffff;
	x = (x ^ (x << 8)) & 0x00ff00ff;
	x = (x ^ (x << 4)) & 0x0f0f0f0f;
	x = (x ^ (x << 2)) & 0x33333333;
	x = (x ^ (x << 1)) & 0x55555555;
	return x;
}

uint32 EncodeMorton(uint32 x, uint32 y)
{
	return (SplitBitsWith0(y) << 1) + SplitBitsWith0(x);
}

// TODO: test and fix
void copyUploadRectZCurve(uint8_t* pDstData, uint8_t* pSrcData, Region3D uploadRegion, uint3 srcPitches, uint3 dstPitches)
{
	uint32_t offset = 0;
	for (uint32_t z = uploadRegion.mZOffset; z < uploadRegion.mZOffset+uploadRegion.mDepth; ++z)
	{
		for (uint32_t y = uploadRegion.mYOffset; y < uploadRegion.mYOffset+uploadRegion.mHeight; ++y)
		{
			for (uint32_t x = uploadRegion.mXOffset; x < uploadRegion.mXOffset+uploadRegion.mWidth; ++x)
			{
				uint32_t blockOffset = EncodeMorton(y, x);
				memcpy(pDstData + offset, pSrcData + blockOffset * srcPitches.x, srcPitches.x);
				offset += dstPitches.x;
			}
			offset = round_up(offset, dstPitches.y);
		}
		pSrcData += srcPitches.z;
	}
}

void copyUploadRect(uint8_t* pDstData, uint8_t* pSrcData, Region3D uploadRegion, uint3 srcPitches, uint3 dstPitches)
{
	uint32_t srcOffset = uploadRegion.mZOffset * srcPitches.z + uploadRegion.mYOffset * srcPitches.y +
						 uploadRegion.mXOffset * srcPitches.x;
	uint32_t numSlices = uploadRegion.mHeight * uploadRegion.mDepth;
	uint32_t pitch = uploadRegion.mWidth * srcPitches.x;
	pSrcData += srcOffset;
	for (uint32_t s = 0; s < numSlices; ++s)
	{
		memcpy(pDstData, pSrcData, pitch);
		pSrcData += srcPitches.y;
		pDstData += dstPitches.y;
	}
}

uint3 calculateUploadRect(uint64_t mem, uint3 pitches, uint3 offset, uint3 extent, uint3 granularity)
{
	uint3 scaler{granularity.x*granularity.y*granularity.z, granularity.y*granularity.z, granularity.z};
	pitches *= scaler;
	uint3 leftover = extent-offset;
	uint32_t numSlices = (uint32_t)min<uint64_t>((mem / pitches.z) * granularity.z, leftover.z);
	// advance by slices
	if (offset.x == 0 && offset.y == 0 && numSlices > 0)
	{
		return { extent.x, extent.y, numSlices };
	}

	// advance by strides
	numSlices = min(leftover.z, granularity.z);
	uint32_t numStrides = (uint32_t)min<uint64_t>((mem / pitches.y) * granularity.y, leftover.y);
	if (offset.x == 0 && numStrides > 0)
	{
		return { extent.x, numStrides, numSlices };
	}

	numStrides = min(leftover.y, granularity.y);
	// advance by blocks
	uint32_t numBlocks = (uint32_t)min<uint64_t>((mem / pitches.x) * granularity.x, leftover.x);
	return { numBlocks, numStrides, numSlices };
}

Region3D calculateUploadRegion(uint3 offset, uint3 extent, uint3 uploadBlock, uint3 pxImageDim)
{
	uint3 regionOffset = offset * uploadBlock;
	uint3 regionSize = min<uint3>(extent * uploadBlock, pxImageDim);
	return { regionOffset.x, regionOffset.y, regionOffset.z, regionSize.x, regionSize.y, regionSize.z };
}

uint64_t GetMipMappedSizeUpto( uint3 dims, uint32_t nMipMapLevels, int32_t slices, TinyImageFormat format)
{
	uint32_t w = dims.x;
	uint32_t h = dims.y;
	uint32_t d = dims.z;

	uint64_t size = 0;
	for(uint32_t i = 0; i < nMipMapLevels;++i)
	{
		uint64_t bx = TinyImageFormat_WidthOfBlock(format);
		uint64_t by = TinyImageFormat_HeightOfBlock(format);
		uint64_t bz = TinyImageFormat_DepthOfBlock(format);

		uint64_t tmpsize = ((w + bx - 1) / bx) * ((h + by - 1) / by) * ((d + bz - 1) / bz);
		tmpsize *= slices;
		size += tmpsize;

		w >>= 1;
		h >>= 1;
		d >>= 1;
		if (w + h + d == 0)
			break;
		if (w == 0)
			w = 1;
		if (h == 0)
			h = 1;
		if (d == 0)
			d = 1;
	}
	size = size * TinyImageFormat_BitSizeOfBlock(format) / 8;
	return size;
}

static bool updateTexture(Renderer* pRenderer, CopyEngine* pCopyEngine, size_t activeSet, UpdateState& pTextureUpdate)
{
	TextureUpdateDescInternal& texUpdateDesc = pTextureUpdate.mRequest.texUpdateDesc;
	ASSERT(pCopyEngine->pQueue->mQueueDesc.mNodeIndex == texUpdateDesc.pTexture->mDesc.mNodeIndex);
	bool         applyBarrieers = pRenderer->mSettings.mApi == RENDERER_API_VULKAN || pRenderer->mSettings.mApi == RENDERER_API_XBOX_D3D12 || pRenderer->mSettings.mApi == RENDERER_API_METAL;
	const Image& img = *texUpdateDesc.pImage;
	Texture*     pTexture = texUpdateDesc.pTexture;

#ifdef _DURANGO
	DmaCmd*      pCmd = aquireCmd(pCopyEngine, activeSet);
#else
	Cmd*         pCmd = aquireCmd(pCopyEngine, activeSet);
#endif

	ASSERT(pTexture);

	uint32_t  textureAlignment = pRenderer->pActiveGpuSettings->mUploadBufferTextureAlignment;
	uint32_t  textureRowAlignment = pRenderer->pActiveGpuSettings->mUploadBufferTextureRowAlignment;


	// TODO: move to Image
	bool isSwizzledZCurve = !img.IsLinearLayout();

	uint32_t i = pTextureUpdate.mMipLevel;
	uint32_t j = pTextureUpdate.mArrayLayer;
	uint3 uploadOffset = pTextureUpdate.mOffset;

	// Only need transition for vulkan and durango since resource will auto promote to copy dest on copy queue in PC dx12
	if (applyBarrieers && (uploadOffset.x == 0) && (uploadOffset.y == 0) && (uploadOffset.z == 0))
	{
		TextureBarrier preCopyBarrier = { pTexture, RESOURCE_STATE_COPY_DEST };
		cmdResourceBarrier(pCmd, 0, NULL, 1, &preCopyBarrier);
	}
	Extent3D          uploadGran = pCopyEngine->pQueue->mUploadGranularity;

	TinyImageFormat fmt = img.GetFormat();
	uint32_t blockSize;
	uint3 pxBlockDim;
	uint32_t	nSlices;
	uint32_t	arrayCount;

	blockSize = TinyImageFormat_BitSizeOfBlock(fmt) / 8;
	pxBlockDim = { TinyImageFormat_WidthOfBlock(fmt),
								 TinyImageFormat_HeightOfBlock(fmt),
								 TinyImageFormat_DepthOfBlock(fmt) };
	nSlices = img.IsCube() ? 6 : 1;
	arrayCount = img.GetArrayCount() * nSlices;

	const uint32 pxPerRow = max<uint32_t>(round_down(textureRowAlignment / blockSize, uploadGran.mWidth), uploadGran.mWidth);
	const uint3 queueGranularity = {pxPerRow, uploadGran.mHeight, uploadGran.mDepth};
	const uint3 fullSizeDim = {img.GetWidth(), img.GetHeight(), img.GetDepth()};

	for (; i < pTexture->mDesc.mMipLevels; ++i)
	{
		uint3 const pxImageDim{ img.GetWidth(i), img.GetHeight(i), img.GetDepth(i) };
		uint3    uploadExtent{ (pxImageDim + pxBlockDim - uint3(1)) / pxBlockDim };
		uint3    granularity{ min<uint3>(queueGranularity, uploadExtent) };
		uint32_t srcPitchY{ blockSize * uploadExtent.x };
		uint32_t dstPitchY{ round_up(srcPitchY, textureRowAlignment) };
		uint3    srcPitches{ blockSize, srcPitchY, srcPitchY * uploadExtent.y };
		uint3    dstPitches{ blockSize, dstPitchY, dstPitchY * uploadExtent.y };

		ASSERT(uploadOffset.x < uploadExtent.x || uploadOffset.y < uploadExtent.y || uploadOffset.z < uploadExtent.z);

		for (; j < arrayCount; ++j)
		{
			uint64_t spaceAvailable{ round_down_64(pCopyEngine->bufferSize - pCopyEngine->allocatedSpace, textureRowAlignment) };
			uint3    uploadRectExtent{ calculateUploadRect(spaceAvailable, dstPitches, uploadOffset, uploadExtent, granularity) };
			uint32_t uploadPitchY{ round_up(uploadRectExtent.x * dstPitches.x, textureRowAlignment) };
			uint3    uploadPitches{ blockSize, uploadPitchY, uploadPitchY * uploadRectExtent.y };

			ASSERT(
				uploadOffset.x + uploadRectExtent.x <= uploadExtent.x || uploadOffset.y + uploadRectExtent.y <= uploadExtent.y ||
				uploadOffset.z + uploadRectExtent.z <= uploadExtent.z);

			if (uploadRectExtent.x == 0)
			{
				pTextureUpdate.mMipLevel = i;
				pTextureUpdate.mArrayLayer = j;
				pTextureUpdate.mOffset = uploadOffset;
				return false;
			}

			MappedMemoryRange range =
				allocateStagingMemory(pRenderer, pCopyEngine, activeSet, uploadRectExtent.z * uploadPitches.z, textureAlignment);
			// TODO: should not happed, resolve, simplify
			//ASSERT(range.pData);
			if (!range.pData)
			{
				pTextureUpdate.mMipLevel = i;
				pTextureUpdate.mArrayLayer = j;
				pTextureUpdate.mOffset = uploadOffset;
				return false;
			}

			SubresourceDataDesc  texData;
			texData.mArrayLayer = j /*n * nSlices + k*/;
			texData.mMipLevel = i;
			texData.mBufferOffset = range.mOffset;
			texData.mRegion = calculateUploadRegion(uploadOffset, uploadRectExtent, pxBlockDim, pxImageDim);
			texData.mRowPitch = uploadPitches.y;
			texData.mSlicePitch = uploadPitches.z;

			uint8_t* pSrcData;
			// there are two common formats for how slices and mipmaps are laid out in
			// either a slice is just another dimension (the 4th) that doesn't undergo
			// mip map reduction
			// so images the top level is just w * h * d * s in size
			// a mipmap level is w >> mml * h >> mml * d >> mml * s in size
			// or DDS style where each slice is mipmapped as a seperate image
			if(img.AreMipsAfterSlices()) {
				pSrcData = (uint8_t *) img.GetPixels() +
						GetMipMappedSizeUpto(fullSizeDim, i, arrayCount, fmt) +
						j * srcPitches.z;
			} else {
				uint32_t n = j / nSlices;
				uint32_t k = j - n * nSlices;
				pSrcData = (uint8_t *) img.GetPixels(i, n) + k * srcPitches.z;
			}

			Region3D uploadRegion{ uploadOffset.x,     uploadOffset.y,     uploadOffset.z,
								   uploadRectExtent.x, uploadRectExtent.y, uploadRectExtent.z };

			if (isSwizzledZCurve)
				copyUploadRectZCurve(range.pData, pSrcData, uploadRegion, srcPitches, uploadPitches);
			else
				copyUploadRect(range.pData, pSrcData, uploadRegion, srcPitches, uploadPitches);

#if defined(DIRECT3D11)
			unmapBuffer(pRenderer, range.pBuffer);
#endif

			cmdUpdateSubresource(pCmd, pTexture, pCopyEngine->resourceSets[activeSet].mBuffer, &texData);

			uploadOffset.x += uploadRectExtent.x;
			uploadOffset.y += (uploadOffset.x < uploadExtent.x) ? 0 : uploadRectExtent.y;
			uploadOffset.z += (uploadOffset.y < uploadExtent.y) ? 0 : uploadRectExtent.z;

			uploadOffset.x = uploadOffset.x % uploadExtent.x;
			uploadOffset.y = uploadOffset.y % uploadExtent.y;
			uploadOffset.z = uploadOffset.z % uploadExtent.z;

			if (uploadOffset.x != 0 || uploadOffset.y != 0 || uploadOffset.z != 0)
			{
				pTextureUpdate.mMipLevel = i;
				pTextureUpdate.mArrayLayer = j;
				pTextureUpdate.mOffset = uploadOffset;
				return false;
			}
		}
		j = 0;
		ASSERT(uploadOffset.x == 0 && uploadOffset.y == 0 && uploadOffset.z == 0);
	}

	// Only need transition for vulkan and durango since resource will decay to srv on graphics queue in PC dx12
	if (applyBarrieers)
	{
		TextureBarrier postCopyBarrier = { pTexture, util_determine_resource_start_state(pTexture->mDesc.mDescriptors) };
		cmdResourceBarrier(pCmd, 0, NULL, 1, &postCopyBarrier);
	}
	else
	{
		pTexture->mCurrentState = util_determine_resource_start_state(pTexture->mDesc.mDescriptors);
	}
	
	if (texUpdateDesc.mFreeImage)
	{
		ResourceLoader::DestroyImage(texUpdateDesc.pImage);
	}

	return true;
}

static bool updateBuffer(Renderer* pRenderer, CopyEngine* pCopyEngine, size_t activeSet, UpdateState& pBufferUpdate)
{
	BufferUpdateDesc& bufUpdateDesc = pBufferUpdate.mRequest.bufUpdateDesc;
	ASSERT(pCopyEngine->pQueue->mQueueDesc.mNodeIndex == bufUpdateDesc.pBuffer->mDesc.mNodeIndex);
	Buffer* pBuffer = bufUpdateDesc.pBuffer;
	ASSERT(pBuffer->mDesc.mMemoryUsage == RESOURCE_MEMORY_USAGE_GPU_ONLY || pBuffer->mDesc.mMemoryUsage == RESOURCE_MEMORY_USAGE_GPU_TO_CPU);

	// TODO: remove uniform buffer alignment?
	const uint64_t bufferSize = (bufUpdateDesc.mSize > 0) ? bufUpdateDesc.mSize : pBuffer->mDesc.mSize;
	const uint64_t alignment = pBuffer->mDesc.mDescriptors & DESCRIPTOR_TYPE_UNIFORM_BUFFER ? pRenderer->pActiveGpuSettings->mUniformBufferAlignment : 1;
	const uint64_t offset = round_up_64(bufUpdateDesc.mDstOffset, alignment) + pBufferUpdate.mSize;
	uint64_t       spaceAvailable = round_down_64(pCopyEngine->bufferSize - pCopyEngine->allocatedSpace, RESOURCE_BUFFER_ALIGNMENT);

	if (spaceAvailable < RESOURCE_BUFFER_ALIGNMENT)
		return false;

	uint64_t dataToCopy = min(spaceAvailable, bufferSize - pBufferUpdate.mSize);

#ifdef _DURANGO
	DmaCmd* pCmd = aquireCmd(pCopyEngine, activeSet);
#else
	Cmd* pCmd = aquireCmd(pCopyEngine, activeSet);
#endif
	
	MappedMemoryRange range = allocateStagingMemory(pRenderer, pCopyEngine, activeSet, dataToCopy, RESOURCE_BUFFER_ALIGNMENT);

	// TODO: should not happed, resolve, simplify
	//ASSERT(range.pData);
	if (!range.pData)
		return false;

	void* pSrcBufferAddress = NULL;
	if (bufUpdateDesc.pData)
		pSrcBufferAddress = (uint8_t*)(bufUpdateDesc.pData) + (bufUpdateDesc.mSrcOffset + pBufferUpdate.mSize);

	if (pSrcBufferAddress)
		memcpy(range.pData, pSrcBufferAddress, dataToCopy);
	else
		memset(range.pData, 0, dataToCopy);

	cmdUpdateBuffer(pCmd, pBuffer, offset, range.pBuffer, range.mOffset, dataToCopy);
#if defined(DIRECT3D11)
	unmapBuffer(pRenderer, range.pBuffer);
#endif

	pBufferUpdate.mSize += dataToCopy;

	if (pBufferUpdate.mSize != bufferSize)
	{
		return false;
	}

	ResourceState state = util_determine_resource_start_state(&pBuffer->mDesc);
#ifdef _DURANGO
	// XBox One needs explicit resource transitions
	BufferBarrier bufferBarriers[] = { { pBuffer, state } };
	cmdResourceBarrier(pCmd, 1, bufferBarriers, 0, NULL);
#else
	// Resource will automatically transition so just set the next state without a barrier
	pBuffer->mCurrentState = state;
#endif

	return true;
}

static bool updateResourceState(Renderer* pRenderer, CopyEngine* pCopyEngine, size_t activeSet, UpdateState& pUpdate)
{
	bool applyBarriers = pRenderer->mSettings.mApi == RENDERER_API_VULKAN;
	if (applyBarriers)
	{
#ifdef _DURANGO
		DmaCmd* pCmd = aquireCmd(pCopyEngine, activeSet);
#else
		Cmd* pCmd = aquireCmd(pCopyEngine, activeSet);
#endif
		if (pUpdate.mRequest.buffer)
		{
			BufferBarrier barrier = { pUpdate.mRequest.buffer, pUpdate.mRequest.buffer->mDesc.mStartState };
			cmdResourceBarrier(pCmd, 1, &barrier, 0, NULL);
		}
		else if (pUpdate.mRequest.texture)
		{
			TextureBarrier barrier = { pUpdate.mRequest.texture, pUpdate.mRequest.texture->mDesc.mStartState };
			cmdResourceBarrier(pCmd, 0, NULL, 1, &barrier);
		}
		else
		{
			ASSERT(0 && "Invalid params");
			return false;
		}
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////
// Resource Loader Implementation
//////////////////////////////////////////////////////////////////////////
static bool allQueuesEmpty(ResourceLoader* pLoader)
{
	for (size_t i = 0; i < MAX_GPUS; ++i)
	{
		if (!pLoader->mRequestQueue[i].empty())
		{
			return false;
		}
	}
	return true;
}

static void streamerThreadFunc(void* pThreadData)
{
	ResourceLoader* pLoader = (ResourceLoader*)pThreadData;
	ASSERT(pLoader);

	uint32_t linkedGPUCount = pLoader->pRenderer->mLinkedNodeCount;
	CopyEngine pCopyEngines[MAX_GPUS];
	for (uint32_t i = 0; i < linkedGPUCount; ++i)
	{
		setupCopyEngine(pLoader->pRenderer, &pCopyEngines[i], i, pLoader->mDesc.mBufferSize, pLoader->mDesc.mBufferCount);
	}

	const uint32_t allUploadsCompleted = (1 << linkedGPUCount) - 1;
	uint32_t       completionMask = allUploadsCompleted;
	UpdateState updateState[MAX_GPUS];

	unsigned nextTimeslot = getSystemTime() + pLoader->mDesc.mTimesliceMs;
	SyncToken maxToken[MAX_BUFFER_COUNT] = { 0 };
	size_t activeSet = 0;
	while (pLoader->mRun)
	{
		pLoader->mQueueMutex.Acquire();
		while (pLoader->mRun && (completionMask == allUploadsCompleted) && allQueuesEmpty(pLoader) && getSystemTime() < nextTimeslot)
		{
			unsigned time = getSystemTime();
			unsigned nextSlot = min(nextTimeslot - time, pLoader->mDesc.mTimesliceMs);
			pLoader->mQueueCond.Wait(pLoader->mQueueMutex, nextSlot);
		}
		pLoader->mQueueMutex.Release();

		for (uint32_t i = 0; i < linkedGPUCount; ++i)
		{
			pLoader->mQueueMutex.Acquire();
			const uint32_t mask = 1 << i;
			if (completionMask & mask)
			{
				if (!pLoader->mRequestQueue[i].empty())
				{
					updateState[i] = pLoader->mRequestQueue[i].front();
					pLoader->mRequestQueue[i].pop_front();
					completionMask &= ~mask;
				}
				else
				{
					updateState[i] = UpdateRequest();
				}
			}

			pLoader->mQueueMutex.Release();

			bool completed = true;
			switch (updateState[i].mRequest.mType)
			{
				case UPDATE_REQUEST_UPDATE_BUFFER:
					completed = updateBuffer(pLoader->pRenderer, &pCopyEngines[i], activeSet, updateState[i]);
					break;
				case UPDATE_REQUEST_UPDATE_TEXTURE:
					completed = updateTexture(pLoader->pRenderer, &pCopyEngines[i], activeSet, updateState[i]);
					break;
				case UPDATE_REQUEST_UPDATE_RESOURCE_STATE:
					completed = updateResourceState(pLoader->pRenderer, &pCopyEngines[i], activeSet, updateState[i]);
					break;
				default: break;
			}
			completionMask |= completed << i;
			if (updateState[i].mRequest.mToken && completed)
			{
				ASSERT(maxToken[activeSet] < updateState[i].mRequest.mToken);
				maxToken[activeSet] = updateState[i].mRequest.mToken;
			}
		}
		
		if (getSystemTime() > nextTimeslot || completionMask == 0)
		{
			for (uint32_t i = 0; i < linkedGPUCount; ++i)
			{
				streamerFlush(&pCopyEngines[i], activeSet);
			}
			
			activeSet = (activeSet + 1) % pLoader->mDesc.mBufferCount;
			for (uint32_t i = 0; i < linkedGPUCount; ++i)
			{
				waitCopyEngineSet(pLoader->pRenderer, &pCopyEngines[i], activeSet);
				resetCopyEngineSet(pLoader->pRenderer, &pCopyEngines[i], activeSet);
			}
			
			SyncToken nextToken = maxToken[activeSet];
			SyncToken prevToken = tfrg_atomic64_load_relaxed(&pLoader->mTokenCompleted);
			// As the only writer atomicity is preserved
			pLoader->mTokenMutex.Acquire();
			tfrg_atomic64_store_release(&pLoader->mTokenCompleted, nextToken > prevToken ? nextToken : prevToken);
			pLoader->mTokenMutex.Release();
			pLoader->mTokenCond.WakeAll();
			nextTimeslot = getSystemTime() + pLoader->mDesc.mTimesliceMs;
		}

	}

	for (uint32_t i = 0; i < linkedGPUCount; ++i)
	{
		streamerFlush(&pCopyEngines[i], activeSet);
		waitQueueIdle(pCopyEngines[i].pQueue);
		cleanupCopyEngine(pLoader->pRenderer, &pCopyEngines[i]);
	}
}

static void addResourceLoader(Renderer* pRenderer, ResourceLoaderDesc* pDesc, ResourceLoader** ppLoader)
{
	ResourceLoader* pLoader = conf_new(ResourceLoader);
	pLoader->pRenderer = pRenderer;

	pLoader->mRun = true;
	pLoader->mDesc = pDesc ? *pDesc : ResourceLoaderDesc{ DEFAULT_BUFFER_SIZE, DEFAULT_BUFFER_COUNT, DEFAULT_TIMESLICE_MS };

	pLoader->mQueueMutex.Init();
	pLoader->mTokenMutex.Init();
	pLoader->mQueueCond.Init();
	pLoader->mTokenCond.Init();
	
	pLoader->mThreadDesc.pFunc = streamerThreadFunc;
	pLoader->mThreadDesc.pData = pLoader;

	pLoader->mThread = create_thread(&pLoader->mThreadDesc);

	*ppLoader = pLoader;
}

static void removeResourceLoader(ResourceLoader* pLoader)
{
	pLoader->mRun = false;
	pLoader->mQueueCond.WakeOne();
	destroy_thread(pLoader->mThread);
	pLoader->mQueueCond.Destroy();
	pLoader->mTokenCond.Destroy();
	pLoader->mQueueMutex.Destroy();
	pLoader->mTokenMutex.Destroy();
	conf_delete(pLoader);
}

static void updateCPUbuffer(Renderer* pRenderer, BufferUpdateDesc* pBufferUpdate)
{
	Buffer* pBuffer = pBufferUpdate->pBuffer;

	ASSERT(
		pBuffer->mDesc.mMemoryUsage == RESOURCE_MEMORY_USAGE_CPU_ONLY || pBuffer->mDesc.mMemoryUsage == RESOURCE_MEMORY_USAGE_CPU_TO_GPU);

	bool map = !pBuffer->pCpuMappedAddress;
	if (map)
	{
		mapBuffer(pRenderer, pBuffer, NULL);
	}

	const uint64_t bufferSize = (pBufferUpdate->mSize > 0) ? pBufferUpdate->mSize : pBuffer->mDesc.mSize;
	// TODO: remove???
	const uint64_t alignment =
		pBuffer->mDesc.mDescriptors & DESCRIPTOR_TYPE_UNIFORM_BUFFER ? pRenderer->pActiveGpuSettings->mUniformBufferAlignment : 1;
	const uint64_t offset = round_up_64(pBufferUpdate->mDstOffset, alignment);
	void*          pDstBufferAddress = (uint8_t*)(pBuffer->pCpuMappedAddress) + offset;

	if (pBufferUpdate->pData)
	{
		uint8_t* pSrcBufferAddress = (uint8_t*)(pBufferUpdate->pData) + pBufferUpdate->mSrcOffset;
		memcpy(pDstBufferAddress, pSrcBufferAddress, bufferSize);
	}
	else
	{
		memset(pDstBufferAddress, 0, bufferSize);
	}

	if (map)
	{
		unmapBuffer(pRenderer, pBuffer);
	}
}

static void queueResourceUpdate(ResourceLoader* pLoader, BufferUpdateDesc* pBufferUpdate, SyncToken* token)
{
	uint32_t nodeIndex = pBufferUpdate->pBuffer->mDesc.mNodeIndex;
	pLoader->mQueueMutex.Acquire();
	SyncToken t = tfrg_atomic64_add_relaxed(&pLoader->mTokenCounter, 1) + 1;
	pLoader->mRequestQueue[nodeIndex].emplace_back(UpdateRequest(*pBufferUpdate));
	pLoader->mRequestQueue[nodeIndex].back().mToken = t;
	pLoader->mQueueMutex.Release();
	pLoader->mQueueCond.WakeOne();
	if (token) *token = t;
}

static void queueResourceUpdate(ResourceLoader* pLoader, TextureUpdateDescInternal* pTextureUpdate, SyncToken* token)
{
	uint32_t nodeIndex = pTextureUpdate->pTexture->mDesc.mNodeIndex;
	pLoader->mQueueMutex.Acquire();
	SyncToken t = tfrg_atomic64_add_relaxed(&pLoader->mTokenCounter, 1) + 1;
	pLoader->mRequestQueue[nodeIndex].emplace_back(UpdateRequest(*pTextureUpdate));
	pLoader->mRequestQueue[nodeIndex].back().mToken = t;
	pLoader->mQueueMutex.Release();
	pLoader->mQueueCond.WakeOne();
	if (token) *token = t;
}

static void queueResourceUpdate(ResourceLoader* pLoader, Buffer* pBuffer, SyncToken* token)
{
	uint32_t nodeIndex = pBuffer->mDesc.mNodeIndex;
	pLoader->mQueueMutex.Acquire();
	SyncToken t = tfrg_atomic64_add_relaxed(&pLoader->mTokenCounter, 1) + 1;
	pLoader->mRequestQueue[nodeIndex].emplace_back(UpdateRequest(pBuffer));
	pLoader->mRequestQueue[nodeIndex].back().mToken = t;
	pLoader->mQueueMutex.Release();
	pLoader->mQueueCond.WakeOne();
	if (token) *token = t;
}

static void queueResourceUpdate(ResourceLoader* pLoader, Texture* pTexture, SyncToken* token)
{
	uint32_t nodeIndex = pTexture->mDesc.mNodeIndex;
	pLoader->mQueueMutex.Acquire();
	SyncToken t = tfrg_atomic64_add_relaxed(&pLoader->mTokenCounter, 1) + 1;
	pLoader->mRequestQueue[nodeIndex].emplace_back(UpdateRequest(pTexture));
	pLoader->mRequestQueue[nodeIndex].back().mToken = t;
	pLoader->mQueueMutex.Release();
	pLoader->mQueueCond.WakeOne();
	if (token) *token = t;
}

static bool isTokenCompleted(ResourceLoader* pLoader, SyncToken token)
{
	bool completed = tfrg_atomic64_load_acquire(&pLoader->mTokenCompleted) >= token;
	return completed;
}

static void waitTokenCompleted(ResourceLoader* pLoader, SyncToken token)
{
	pLoader->mTokenMutex.Acquire();
	while (!isTokenCompleted(token))
	{
		pLoader->mTokenCond.Wait(pLoader->mTokenMutex);
	}
	pLoader->mTokenMutex.Release();
}

static ResourceLoader* pResourceLoader = NULL;

void initResourceLoaderInterface(Renderer* pRenderer, ResourceLoaderDesc* pDesc)
{
	addResourceLoader(pRenderer, pDesc, &pResourceLoader);

}

void removeResourceLoaderInterface(Renderer* pRenderer)
{
	removeResourceLoader(pResourceLoader);
}

void addResource(BufferLoadDesc* pBufferDesc, bool batch)
{
	SyncToken token = 0;
	addResource(pBufferDesc, &token);
	if (!batch) waitTokenCompleted(token);
}

void addResource(TextureLoadDesc* pTextureDesc, bool batch)
{
	SyncToken token = 0;
	addResource(pTextureDesc, &token);
	if (!batch) waitTokenCompleted(token);
}

void addResource(BufferLoadDesc* pBufferDesc, SyncToken* token)
{
	ASSERT(pBufferDesc->ppBuffer);

	bool update = pBufferDesc->pData || pBufferDesc->mForceReset;

	pBufferDesc->mDesc.mStartState = update ? RESOURCE_STATE_COMMON : pBufferDesc->mDesc.mStartState;
	addBuffer(pResourceLoader->pRenderer, &pBufferDesc->mDesc, pBufferDesc->ppBuffer);

	if (update)
	{
		BufferUpdateDesc bufferUpdate(*pBufferDesc->ppBuffer, pBufferDesc->pData);
        bufferUpdate.mSize = pBufferDesc->mDesc.mSize;
		updateResource(&bufferUpdate, token);
	}
	else
	{
		// Transition GPU buffer to desired state for Vulkan since all Vulkan resources are created in undefined state
		if (pResourceLoader->pRenderer->mSettings.mApi == RENDERER_API_VULKAN &&
			pBufferDesc->mDesc.mMemoryUsage == RESOURCE_MEMORY_USAGE_GPU_ONLY &&
			// Check whether this is required (user specified a state other than undefined / common)
			(pBufferDesc->mDesc.mStartState != RESOURCE_STATE_UNDEFINED && pBufferDesc->mDesc.mStartState != RESOURCE_STATE_COMMON))
			queueResourceUpdate(pResourceLoader, *pBufferDesc->ppBuffer, token);
	}
}

void addResource(TextureLoadDesc* pTextureDesc, SyncToken* token)
{
	ASSERT(pTextureDesc->ppTexture);

	bool freeImage = false;
	Image* pImage = NULL;

 	if (!pTextureDesc->pRawImageData && !pTextureDesc->pBinaryImageData && pTextureDesc->pDesc)
	{
		// If texture is supposed to be filled later (UAV / Update later / ...) proceed with the mStartState provided by the user in the texture description
		addTexture(pResourceLoader->pRenderer, pTextureDesc->pDesc, pTextureDesc->ppTexture);

		// Transition texture to desired state for Vulkan since all Vulkan resources are created in undefined state
		if (pResourceLoader->pRenderer->mSettings.mApi == RENDERER_API_VULKAN &&
			// Check whether this is required (user specified a state other than undefined / common)
			(pTextureDesc->pDesc->mStartState != RESOURCE_STATE_UNDEFINED && pTextureDesc->pDesc->mStartState != RESOURCE_STATE_COMMON))
			queueResourceUpdate(pResourceLoader, *pTextureDesc->ppTexture, token);
		return;
	}
	else if (pTextureDesc->pRawImageData && !pTextureDesc->pBinaryImageData)
	{
		pImage = ResourceLoader::CreateImage(pTextureDesc->pRawImageData->mFormat, pTextureDesc->pRawImageData->mWidth, pTextureDesc->pRawImageData->mHeight, pTextureDesc->pRawImageData->mDepth, pTextureDesc->pRawImageData->mMipLevels, pTextureDesc->pRawImageData->mArraySize, pTextureDesc->pRawImageData->pRawData);
		pImage->SetMipsAfterSlices(pTextureDesc->pRawImageData->mMipsAfterSlices);
		freeImage = true;
	}
	else
		ASSERT(0 && "Invalid params");

	TextureDesc desc = {};
	desc.mFlags = pTextureDesc->mCreationFlag;
	desc.mWidth = pImage->GetWidth();
	desc.mHeight = pImage->GetHeight();
	desc.mDepth = max(1U, pImage->GetDepth());
	desc.mArraySize = pImage->GetArrayCount();

	desc.mMipLevels = pImage->GetMipMapCount();
	desc.mSampleCount = SAMPLE_COUNT_1;
	desc.mSampleQuality = 0;
	desc.mFormat = pImage->GetFormat();

	desc.mClearValue = ClearValue();
	desc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
	desc.mStartState = RESOURCE_STATE_COMMON;
	desc.pNativeHandle = NULL;
	desc.mHostVisible = false;
	desc.mNodeIndex = pTextureDesc->mNodeIndex;

	if (pImage->IsCube())
	{
		desc.mDescriptors |= DESCRIPTOR_TYPE_TEXTURE_CUBE;
		desc.mArraySize *= 6;
	}

	wchar_t         debugName[256] = {};
	mbstowcs(debugName, "TODO", 4);
	desc.pDebugName = debugName;

	addTexture(pResourceLoader->pRenderer, &desc, pTextureDesc->ppTexture);

	TextureUpdateDescInternal updateDesc = { *pTextureDesc->ppTexture, pImage, freeImage };
	queueResourceUpdate(pResourceLoader, &updateDesc, token);
}

void updateResource(BufferUpdateDesc* pBufferUpdate, bool batch)
{
	SyncToken token = 0;
	updateResource(pBufferUpdate, &token);
#if defined(DIRECT3D11)
	batch = false;
#endif
	if (!batch) waitTokenCompleted(token);
}

void updateResource(TextureUpdateDesc* pTextureUpdate, bool batch)
{
	SyncToken token = 0;
	updateResource(pTextureUpdate, &token);
#if defined(DIRECT3D11)
	batch = false;
#endif
	if (!batch) waitTokenCompleted(token);
}

void updateResources(uint32_t resourceCount, ResourceUpdateDesc* pResources)
{
	SyncToken token = 0;
	updateResources(resourceCount, pResources, &token);
	waitTokenCompleted(token);
}

void updateResource(BufferUpdateDesc* pBufferUpdate, SyncToken* token)
{
	if (pBufferUpdate->pBuffer->mDesc.mMemoryUsage == RESOURCE_MEMORY_USAGE_GPU_ONLY)
	{
		SyncToken updateToken;
		queueResourceUpdate(pResourceLoader, pBufferUpdate, &updateToken);
#if defined(DIRECT3D11)
		waitTokenCompleted(updateToken);
#endif
		if (token) *token = updateToken;
	}
	else
	{
		updateCPUbuffer(pResourceLoader->pRenderer, pBufferUpdate);
	}
}

void updateResource(TextureUpdateDesc* pTextureUpdate, SyncToken* token)
{	
	TextureUpdateDescInternal desc;
	desc.pTexture = pTextureUpdate->pTexture;
	if (pTextureUpdate->pRawImageData)
	{
		Image* pImage = ResourceLoader::CreateImage(pTextureUpdate->pRawImageData->mFormat, pTextureUpdate->pRawImageData->mWidth, pTextureUpdate->pRawImageData->mHeight,
			pTextureUpdate->pRawImageData->mDepth, pTextureUpdate->pRawImageData->mMipLevels, pTextureUpdate->pRawImageData->mArraySize,
			pTextureUpdate->pRawImageData->pRawData);
		pImage->SetMipsAfterSlices(pTextureUpdate->pRawImageData->mMipsAfterSlices);			
		desc.mFreeImage = true;
		desc.pImage = pImage;
	}
	else
	{
		ASSERT(false && "TextureUpdateDesc::pRawImageData cannot be NULL");
		return;
	}

	SyncToken updateToken;
	queueResourceUpdate(pResourceLoader, &desc, &updateToken);
#if defined(DIRECT3D11)
	waitTokenCompleted(updateToken);
#endif
	if (token) *token = updateToken;
}

void updateResources(uint32_t resourceCount, ResourceUpdateDesc* pResources, SyncToken* token)
{
	for (uint32_t i = 0; i < resourceCount; ++i)
	{
		if (pResources[i].mType == RESOURCE_TYPE_BUFFER)
		{
			updateResource(&pResources[i].buf, token);
		}
		else
		{
			updateResource(&pResources[i].tex, token);
		}
	}
}

void removeResource(Texture* pTexture)
{
	removeTexture(pResourceLoader->pRenderer, pTexture);
}

void removeResource(Buffer* pBuffer)
{
	removeBuffer(pResourceLoader->pRenderer, pBuffer);
}

bool isTokenCompleted(SyncToken token)
{
	return isTokenCompleted(pResourceLoader, token);
}

void waitTokenCompleted(SyncToken token)
{
	waitTokenCompleted(pResourceLoader, token);
}

bool isBatchCompleted()
{
	SyncToken token = tfrg_atomic64_load_relaxed(&pResourceLoader->mTokenCounter);
	return isTokenCompleted(pResourceLoader, token);
}

void waitBatchCompleted()
{
	SyncToken token = tfrg_atomic64_load_relaxed(&pResourceLoader->mTokenCounter);
	waitTokenCompleted(pResourceLoader, token);
}

void flushResourceUpdates()
{
	waitBatchCompleted();
}

void finishResourceLoading()
{
	waitBatchCompleted();
}