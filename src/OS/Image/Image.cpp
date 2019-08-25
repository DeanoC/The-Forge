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

//this is needed for unix as PATH_MAX is defined instead of MAX_PATH
#ifndef _WIN32
#include <limits.h>
#define MAX_PATH PATH_MAX
#endif

#include "../../ThirdParty/OpenSource/EASTL/functional.h"
#include "../../ThirdParty/OpenSource/EASTL/unordered_map.h"

#define IMAGE_CLASS_ALLOWED
#include "Image.h"
#include "../Interfaces/ILog.h"
#include "../../ThirdParty/OpenSource/TinyEXR/tinyexr.h"
//stb_image
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC conf_malloc
#define STBI_REALLOC conf_realloc
#define STBI_FREE conf_free
#define STBI_ASSERT ASSERT
#if defined(__ANDROID__)
#define STBI_NO_SIMD
#endif
#include "../../ThirdParty/OpenSource/Nothings/stb_image.h"
//stb_image_write
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_MALLOC conf_malloc
#define STBIW_REALLOC conf_realloc
#define STBIW_FREE conf_free
#define STBIW_ASSERT ASSERT
#include "../../ThirdParty/OpenSource/Nothings/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../../ThirdParty/OpenSource/Nothings/stb_image_resize.h"

#include "../../ThirdParty/OpenSource/tinyimageformat/tinyimageformat_base.h"
#include "../../ThirdParty/OpenSource/tinyimageformat/tinyimageformat_query.h"
#include "../../ThirdParty/OpenSource/tinyimageformat/tinyimageformat_bits.h"
#include "../../ThirdParty/OpenSource/tinyimageformat/tinyimageformat_decode.h"
#include "../../ThirdParty/OpenSource/tinyimageformat/tinyimageformat_encode.h"
#define TINYDDS_IMPLEMENTATION
#include "../../ThirdParty/OpenSource/tinydds/tinydds.h"
#define TINYKTX_IMPLEMENTATION
#include "../../ThirdParty/OpenSource/tinyktx/tinyktx.h"

#define IMEMORY_FROM_HEADER
#include "../Interfaces/IMemory.h"

// Describes the header of a PVR header-texture
typedef struct PVR_Header_Texture_TAG
{
	uint32_t 	mVersion;
	uint32_t 	mFlags; //!< Various format flags.
	uint64_t 	mPixelFormat; //!< The pixel format, 8cc value storing the 4 channel identifiers and their respective sizes.
	uint32_t 	mColorSpace; //!< The Color Space of the texture, currently either linear RGB or sRGB.
	uint32_t 	mChannelType; //!< Variable type that the channel is stored in. Supports signed/uint32_t/short/char/float.
	uint32_t 	mHeight; //!< Height of the texture.
	uint32_t	mWidth; //!< Width of the texture.
	uint32_t 	mDepth; //!< Depth of the texture. (Z-slices)
	uint32_t 	mNumSurfaces; //!< Number of members in a Texture Array.
	uint32_t 	mNumFaces; //!< Number of faces in a Cube Map. Maybe be a value other than 6.
	uint32_t 	mNumMipMaps; //!< Number of MIP Maps in the texture - NB: Includes top level.
	uint32_t 	mMetaDataSize; //!< Size of the accompanying meta data.
} PVR_Texture_Header;

#ifdef TARGET_IOS
const uint32_t gPvrtexV3HeaderVersion = 0x03525650;
#endif

// --- BLOCK DECODING ---

// TODO Decode these decode block don't handle SRGB properly
void iDecodeColorBlock(
	unsigned char* dest, int w, int h, int xOff, int yOff, TinyImageFormat format, int red, int blue, unsigned char* src)
{
	unsigned char colors[4][3];

	uint16 c0 = *(uint16*)src;
	uint16 c1 = *(uint16*)(src + 2);

	colors[0][0] = ((c0 >> 11) & 0x1F) << 3;
	colors[0][1] = ((c0 >> 5) & 0x3F) << 2;
	colors[0][2] = (c0 & 0x1F) << 3;

	colors[1][0] = ((c1 >> 11) & 0x1F) << 3;
	colors[1][1] = ((c1 >> 5) & 0x3F) << 2;
	colors[1][2] = (c1 & 0x1F) << 3;

	if (c0 > c1 || (format == TinyImageFormat_DXBC3_UNORM || format == TinyImageFormat_DXBC3_SRGB))
	{
		for (int i = 0; i < 3; i++)
		{
			colors[2][i] = (2 * colors[0][i] + colors[1][i] + 1) / 3;
			colors[3][i] = (colors[0][i] + 2 * colors[1][i] + 1) / 3;
		}
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			colors[2][i] = (colors[0][i] + colors[1][i] + 1) >> 1;
			colors[3][i] = 0;
		}
	}

	src += 4;
	for (int y = 0; y < h; y++)
	{
		unsigned char* dst = dest + yOff * y;
		unsigned int   indexes = src[y];
		for (int x = 0; x < w; x++)
		{
			unsigned int index = indexes & 0x3;
			dst[red] = colors[index][0];
			dst[1] = colors[index][1];
			dst[blue] = colors[index][2];
			indexes >>= 2;

			dst += xOff;
		}
	}
}

void iDecodeDXT3Block(unsigned char* dest, int w, int h, int xOff, int yOff, unsigned char* src)
{
	for (int y = 0; y < h; y++)
	{
		unsigned char* dst = dest + yOff * y;
		unsigned int   alpha = ((unsigned short*)src)[y];
		for (int x = 0; x < w; x++)
		{
			*dst = (alpha & 0xF) * 17;
			alpha >>= 4;
			dst += xOff;
		}
	}
}

void iDecodeDXT5Block(unsigned char* dest, int w, int h, int xOff, int yOff, unsigned char* src)
{
	unsigned char a0 = src[0];
	unsigned char a1 = src[1];
	uint64_t      alpha = (*(uint64_t*)src) >> 16;

	for (int y = 0; y < h; y++)
	{
		unsigned char* dst = dest + yOff * y;
		for (int x = 0; x < w; x++)
		{
			int k = ((unsigned int)alpha) & 0x7;
			if (k == 0)
			{
				*dst = a0;
			}
			else if (k == 1)
			{
				*dst = a1;
			}
			else if (a0 > a1)
			{
				*dst = (unsigned char)(((8 - k) * a0 + (k - 1) * a1) / 7);
			}
			else if (k >= 6)
			{
				*dst = (k == 6) ? 0 : 255;
			}
			else
			{
				*dst = (unsigned char)(((6 - k) * a0 + (k - 1) * a1) / 5);
			}
			alpha >>= 3;

			dst += xOff;
		}
		if (w < 4)
			alpha >>= (3 * (4 - w));
	}
}

void iDecodeCompressedImage(unsigned char* dest, unsigned char* src, const int width, const int height, const TinyImageFormat format)
{
	int sx = (width < 4) ? width : 4;
	int sy = (height < 4) ? height : 4;

	int nChannels = TinyImageFormat_ChannelCount(format);

	for (int y = 0; y < height; y += 4)
	{
		for (int x = 0; x < width; x += 4)
		{
			unsigned char* dst = dest + (y * width + x) * nChannels;
			if (format == TinyImageFormat_DXBC2_UNORM || format == TinyImageFormat_DXBC2_SRGB)
			{
				iDecodeDXT3Block(dst + 3, sx, sy, nChannels, width * nChannels, src);
				src += 8;
			}
			else if (format == TinyImageFormat_DXBC3_UNORM || format == TinyImageFormat_DXBC3_SRGB)
			{
				iDecodeDXT5Block(dst + 3, sx, sy, nChannels, width * nChannels, src);
				src += 8;
			}
			if ((format == TinyImageFormat_DXBC1_RGBA_UNORM || format == TinyImageFormat_DXBC1_RGB_UNORM) ||
					(format == TinyImageFormat_DXBC1_RGBA_SRGB || format == TinyImageFormat_DXBC1_RGB_SRGB))
					{
				iDecodeColorBlock(dst, sx, sy, nChannels, width * nChannels, format, 0, 2, src);
				src += 8;
			}
			else
			{
				if (format == TinyImageFormat_DXBC4_UNORM || format == TinyImageFormat_DXBC4_SNORM)
				{
					iDecodeDXT5Block(dst, sx, sy, 1, width, src);
					src += 8;
				}
				else if (format == TinyImageFormat_DXBC5_UNORM || format == TinyImageFormat_DXBC5_SNORM)
				{
					iDecodeDXT5Block(dst, sx, sy, 2, width * 2, src + 8);
					iDecodeDXT5Block(dst + 1, sx, sy, 2, width * 2, src);
					src += 16;
				}
				else
					return;
			}
		}
	}
}

template <typename T>
inline void swapPixelChannels(T* pixels, int num_pixels, const int channels, const int ch0, const int ch1)
{
	for (int i = 0; i < num_pixels; i++)
	{
		T tmp = pixels[ch1];
		pixels[ch1] = pixels[ch0];
		pixels[ch0] = tmp;
		pixels += channels;
	}
}

Image::Image()
{
	pData = NULL;
	mLoadFileName = "";
	mWidth = 0;
	mHeight = 0;
	mDepth = 0;
	mMipMapCount = 0;
	mArrayCount = 0;
	mFormat = TinyImageFormat_UNDEFINED;
	mAdditionalDataSize = 0;
	pAdditionalData = NULL;
	mOwnsMemory = true;
	mLinearLayout = true;
}

Image::Image(const Image& img)
{
	mWidth = img.mWidth;
	mHeight = img.mHeight;
	mDepth = img.mDepth;
	mMipMapCount = img.mMipMapCount;
	mArrayCount = img.mArrayCount;
	mFormat = img.mFormat;
	mLinearLayout = img.mLinearLayout;

	int size = GetMipMappedSize(0, mMipMapCount) * mArrayCount;
	pData = (unsigned char*)conf_malloc(sizeof(unsigned char) * size);
	memcpy(pData, img.pData, size);
	mLoadFileName = img.mLoadFileName;

	mAdditionalDataSize = img.mAdditionalDataSize;
	pAdditionalData = (unsigned char*)conf_malloc(sizeof(unsigned char) * mAdditionalDataSize);
	memcpy(pAdditionalData, img.pAdditionalData, mAdditionalDataSize);
}

unsigned char* Image::Create(const TinyImageFormat fmt, const int w, const int h, const int d, const int mipMapCount, const int arraySize)
{
	mFormat = fmt;
	mWidth = w;
	mHeight = h;
	mDepth = d;
	mMipMapCount = mipMapCount;
	mArrayCount = arraySize;
	mOwnsMemory = true;
	mMipsAfterSlices = false;

	uint holder = GetMipMappedSize(0, mMipMapCount);
	pData = (unsigned char*)conf_malloc(sizeof(unsigned char) * holder * mArrayCount);
	memset(pData, 0x00, holder * mArrayCount);
	mLoadFileName = "Undefined";

	return pData;
}

unsigned char* Image::Create(const TinyImageFormat fmt, const int w, const int h, const int d, const int mipMapCount, const int arraySize, const unsigned char* rawData)
{
	mFormat = fmt;
	mWidth = w;
	mHeight = h;
	mDepth = d;
	mMipMapCount = mipMapCount;
	mArrayCount = arraySize;
	mOwnsMemory = false;
	mMipsAfterSlices = false;

	pData = (uint8_t*)rawData;
	mLoadFileName = "Undefined";

	return pData;
}

void Image::RedefineDimensions(
	const TinyImageFormat fmt, const int w, const int h, const int d, const int mipMapCount, const int arraySize)
{
	//Redefine image that was loaded in
	mFormat = fmt;
	mWidth = w;
	mHeight = h;
	mDepth = d;
	mMipMapCount = mipMapCount;
	mArrayCount = arraySize;

	switch (mFormat)
	{
	case TinyImageFormat_PVRTC1_2BPP_UNORM:
	case TinyImageFormat_PVRTC1_2BPP_SRGB:
	case TinyImageFormat_PVRTC1_4BPP_UNORM:
	case TinyImageFormat_PVRTC1_4BPP_SRGB:
		mLinearLayout = false;
		break;
	default:
		mLinearLayout = true;
	}
}

void Image::Destroy()
{
	if (pData && mOwnsMemory)
	{
		conf_free(pData);
		pData = NULL;
	}

	if (pAdditionalData)
	{
		conf_free(pAdditionalData);
		pAdditionalData = NULL;
	}
}

void Image::Clear()
{
	Destroy();

	mWidth = 0;
	mHeight = 0;
	mDepth = 0;
	mMipMapCount = 0;
	mArrayCount = 0;
	mFormat = TinyImageFormat_UNDEFINED;
	mMipsAfterSlices = false;

	mAdditionalDataSize = 0;
}

unsigned char* Image::GetPixels(unsigned char* pDstData, const uint mipMapLevel, const uint dummy)
{
	UNREF_PARAM(dummy);
	return (mipMapLevel < mMipMapCount) ? pDstData + GetMipMappedSize(0, mipMapLevel) : NULL;
}

unsigned char* Image::GetPixels(const uint mipMapLevel) const
{
	return (mipMapLevel < mMipMapCount) ? pData + GetMipMappedSize(0, mipMapLevel) : NULL;
}

unsigned char* Image::GetPixels(const uint mipMapLevel, const uint arraySlice) const
{
	if (mipMapLevel >= mMipMapCount || arraySlice >= mArrayCount)
		return NULL;

	return pData + GetMipMappedSize(0, mMipMapCount) * arraySlice + GetMipMappedSize(0, mipMapLevel);
}

uint32_t Image::GetWidth(const int mipMapLevel) const
{
	uint32_t a = mWidth >> mipMapLevel;
	return (a == 0) ? 1 : a;
}

uint32_t Image::GetHeight(const int mipMapLevel) const
{
	uint32_t a = mHeight >> mipMapLevel;
	return (a == 0) ? 1 : a;
}

uint Image::GetDepth(const int mipMapLevel) const
{
	uint32_t a = mDepth >> mipMapLevel;
	return (a == 0) ? 1 : a;
}

uint32_t Image::GetMipMapCountFromDimensions() const
{
	uint32_t m = max(mWidth, mHeight);
	m = max(m, mDepth);

	uint32_t i = 0;
	while (m > 0)
	{
		m >>= 1;
		i++;
	}

	return i;
}

uint Image::GetArraySliceSize(const uint mipMapLevel, TinyImageFormat srcFormat) const
{
	uint32_t w = GetWidth(mipMapLevel);
	uint32_t h = GetHeight(mipMapLevel);

	if (srcFormat == TinyImageFormat_UNDEFINED)
		srcFormat = mFormat;

	uint32_t size;
	if (TinyImageFormat_IsCompressed(srcFormat))
	{
		size = (((w + 3U) >> 2U) * ((h + 3U) >> 2U) * TinyImageFormat_BitSizeOfBlock(srcFormat)) / 8U;
	}
	else
	{
		size = (w * h * TinyImageFormat_BitSizeOfBlock(srcFormat)) / 8U;
	}

	return size;
}

uint Image::GetNumberOfPixels(const uint firstMipMapLevel, uint nMipMapLevels) const
{
	int w = GetWidth(firstMipMapLevel);
	int h = GetHeight(firstMipMapLevel);
	int d = GetDepth(firstMipMapLevel);
	int size = 0;
	while (nMipMapLevels)
	{
		size += w * h * d;
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

		nMipMapLevels--;
	}

	return (mDepth == 0) ? 6 * size : size;
}

bool Image::GetColorRange(float& min, float& max)
{
	// TODO Deano replace with TinyImageFormat decode calls

	if (TinyImageFormat_IsFloat(mFormat) && TinyImageFormat_ChannelBitWidth(mFormat, TinyImageFormat_LC_Red) == 32)
		return false;

	uint32_t nElements = GetNumberOfPixels(0, mMipMapCount) * TinyImageFormat_ChannelCount(mFormat) * mArrayCount;

	if (nElements <= 0)
		return false;

	float minVal = FLT_MAX;
	float maxVal = -FLT_MAX;
	for (uint32_t i = 0; i < nElements; i++)
	{
		float d = ((float*)pData)[i];
		if (d > maxVal)
			maxVal = d;
		if (d < minVal)
			minVal = d;
	}
	max = maxVal;
	min = minVal;

	return true;
}
bool Image::Normalize()
{
	// TODO Deano replace with TinyImageFormat decode calls

	if (TinyImageFormat_IsFloat(mFormat) && TinyImageFormat_ChannelBitWidth(mFormat, TinyImageFormat_LC_Red) == 32)
		return false;

	float min, max;
	GetColorRange(min, max);

	uint32_t nElements = GetNumberOfPixels(0, mMipMapCount) * TinyImageFormat_ChannelCount(mFormat) * mArrayCount;

	float s = 1.0f / (max - min);
	float b = -min * s;
	for (uint32_t i = 0; i < nElements; i++)
	{
		float d = ((float*)pData)[i];
		((float*)pData)[i] = d * s + b;
	}

	return true;
}

bool Image::Uncompress()
{
	// only dxtc at the moment
	uint64_t const tifname = (TinyImageFormat_Code(mFormat) & TinyImageFormat_NAMESPACE_REQUIRED_BITS);
	if( tifname != TinyImageFormat_NAMESPACE_DXTC)
		return false;

	// only BC 1 to 5 at the moment
	if(mFormat == TinyImageFormat_DXBC6H_UFLOAT || mFormat == TinyImageFormat_DXBC6H_SFLOAT ||
			mFormat == TinyImageFormat_DXBC7_UNORM || mFormat == TinyImageFormat_DXBC7_SRGB)
		return false;

	TinyImageFormat destFormat;
	switch(TinyImageFormat_ChannelCount(mFormat)) {
	case 1: destFormat = TinyImageFormat_R8_UNORM; break;
	case 2: destFormat = TinyImageFormat_R8G8_UNORM; break;
	case 3: destFormat = TinyImageFormat_R8G8B8_UNORM; break;
	case 4: destFormat = TinyImageFormat_R8G8B8A8_UNORM; break;
	default:
		ASSERT(false);
		destFormat = TinyImageFormat_R8_UNORM;
		break;
	}

	ubyte* newPixels = (ubyte*)conf_malloc(sizeof(ubyte) * GetMipMappedSize(0, mMipMapCount, destFormat));

	int    level = 0;
	ubyte *src, *dst = newPixels;
	while ((src = GetPixels(level)) != NULL)
	{
		int w = GetWidth(level);
		int h = GetHeight(level);
		int d = (mDepth == 0) ? 6 : GetDepth(level);

		int dstSliceSize = GetArraySliceSize(level, destFormat);
		int srcSliceSize = GetArraySliceSize(level, mFormat);

		for (int slice = 0; slice < d; slice++)
		{
			iDecodeCompressedImage(dst, src, w, h, mFormat);

			dst += dstSliceSize;
			src += srcSliceSize;
		}
		level++;
	}

	mFormat = destFormat;

	Destroy();
	pData = newPixels;

	return true;
}

bool Image::Unpack()
{
	TinyImageFormat destFormat;
	if(TinyImageFormat_IsFloat(mFormat)) {
		switch (TinyImageFormat_ChannelCount(mFormat)) {
		case 1: destFormat = TinyImageFormat_R32_SFLOAT;
			break;
		case 2: destFormat = TinyImageFormat_R32G32_SFLOAT;
			break;
		case 3: destFormat = TinyImageFormat_R32G32B32_SFLOAT;
			break;
		case 4: destFormat = TinyImageFormat_R32G32B32A32_SFLOAT;
			break;
		default: ASSERT(false);
			destFormat = TinyImageFormat_R32_SFLOAT;
			break;
		}

	} else if(TinyImageFormat_IsSigned(mFormat)) {
		switch (TinyImageFormat_ChannelCount(mFormat)) {
		case 1: destFormat = TinyImageFormat_R8_SNORM;
			break;
		case 2: destFormat = TinyImageFormat_R8G8_SNORM;
			break;
		case 3: destFormat = TinyImageFormat_R8G8B8_SNORM;
			break;
		case 4: destFormat = TinyImageFormat_R8G8B8A8_SNORM;
			break;
		default: ASSERT(false);
			destFormat = TinyImageFormat_R8_SNORM;
			break;
		}

	} else if(TinyImageFormat_IsSRGB(mFormat)) {
		switch (TinyImageFormat_ChannelCount(mFormat)) {
		case 1: destFormat = TinyImageFormat_R8_SRGB;
			break;
		case 2: destFormat = TinyImageFormat_R8G8_SRGB;
			break;
		case 3: destFormat = TinyImageFormat_R8G8B8_SRGB;
			break;
		case 4: destFormat = TinyImageFormat_R8G8B8A8_SRGB;
			break;
		default: ASSERT(false);
			destFormat = TinyImageFormat_R8_SRGB;
			break;
		}
	} else {
		switch (TinyImageFormat_ChannelCount(mFormat)) {
		case 1: destFormat = TinyImageFormat_R8_UNORM;
			break;
		case 2: destFormat = TinyImageFormat_R8G8_UNORM;
			break;
		case 3: destFormat = TinyImageFormat_R8G8B8_UNORM;
			break;
		case 4: destFormat = TinyImageFormat_R8G8B8A8_UNORM;
			break;
		default: ASSERT(false);
			destFormat = TinyImageFormat_R8_UNORM;
			break;
		}
	}

	return Convert(destFormat);
}

uint Image::GetMipMappedSize(const uint firstMipMapLevel, uint nMipMapLevels, TinyImageFormat srcFormat) const
{
	uint w = GetWidth(firstMipMapLevel);
	uint h = GetHeight(firstMipMapLevel);
	uint d = GetDepth(firstMipMapLevel);

	if (srcFormat == TinyImageFormat_UNDEFINED)
		srcFormat = mFormat;
	
	// PVR formats get special case
	uint64_t const tifname = (TinyImageFormat_Code(mFormat) & TinyImageFormat_NAMESPACE_REQUIRED_BITS);
	if( tifname != TinyImageFormat_NAMESPACE_PVRTC)
	{
		uint totalSize = 0;
		uint sizeX = w;
		uint sizeY = h;
		uint sizeD = d;
		int level = nMipMapLevels;
		
		uint minWidth = 8;
		uint minHeight = 8;
		uint minDepth = 1;
		int bpp = 4;
		
		if (srcFormat == TinyImageFormat_PVRTC1_2BPP_UNORM || srcFormat == TinyImageFormat_PVRTC1_2BPP_SRGB)
		{
			minWidth = 16;
			minHeight = 8;
			bpp = 2;
		}
		
		while (level > 0)
		{
			// If pixel format is compressed, the dimensions need to be padded.
			uint paddedWidth = sizeX + ((-1 * sizeX) % minWidth);
			uint paddedHeight = sizeY + ((-1 * sizeY) % minHeight);
			uint paddedDepth = sizeD + ((-1 * sizeD) % minDepth);
			
			int mipSize = paddedWidth * paddedHeight * paddedDepth * bpp / 8;
			
			totalSize += mipSize;
			
			unsigned int MinimumSize = 1;
			sizeX = max(sizeX / 2, MinimumSize);
			sizeY = max(sizeY / 2, MinimumSize);
			sizeD = max(sizeD / 2, MinimumSize);
			level--;
		}
		
		return totalSize;
	}
	
	uint32_t size = 0;
	while (nMipMapLevels)
	{
		uint32_t bx = TinyImageFormat_WidthOfBlock(srcFormat);
		uint32_t by = TinyImageFormat_HeightOfBlock(srcFormat);
		uint32_t bz = TinyImageFormat_DepthOfBlock(srcFormat);
		size += ((w + bx - 1) / bx) * ((h + by - 1) / by) * ((d + bz - 1) / bz);

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

		nMipMapLevels--;
	}

	size *= TinyImageFormat_BitSizeOfBlock(srcFormat) / 8;

	return (mDepth == 0) ? 6 * size : size;
}

static void tinyktxddsCallbackError(void *user, char const *msg) {
	LOGERRORF("Tiny_ ERROR: %s", msg);
}
static void *tinyktxddsCallbackAlloc(void *user, size_t size) {
	return conf_malloc(size);
}
static void tinyktxddsCallbackFree(void *user, void *data) {
	conf_free(data);
}
static size_t tinyktxddsCallbackRead(void *user, void* data, size_t size) {
	auto file = (MemoryBuffer*) user;
	return file->Read(data, (unsigned int)size);
}
static bool tinyktxddsCallbackSeek(void *user, int64_t offset) {
	auto file = (MemoryBuffer*) user;
	return file->Seek((unsigned int)offset, SeekDir::SEEK_DIR_BEGIN);

}
static int64_t tinyktxddsCallbackTell(void *user) {
	auto file = (MemoryBuffer*) user;
	return file->Tell();
}

// Load Image Data form mData functions

bool iLoadDDSFromMemory(Image* pImage,
	const char* memory, uint32_t memSize, memoryAllocationFunc pAllocator, void* pUserData)
{
	if (memory == NULL || memSize == 0)
		return false;

	TinyDDS_Callbacks callbacks {
			&tinyktxddsCallbackError,
			&tinyktxddsCallbackAlloc,
			&tinyktxddsCallbackFree,
			tinyktxddsCallbackRead,
			&tinyktxddsCallbackSeek,
			&tinyktxddsCallbackTell
	};

	MemoryBuffer memoryBuffer(memory, memSize);

	auto ctx = TinyDDS_CreateContext( &callbacks, (void*)&memoryBuffer);
	bool headerOkay = TinyDDS_ReadHeader(ctx);
	if(!headerOkay) {
		TinyDDS_DestroyContext(ctx);
		return false;
	}

	uint32_t w = TinyDDS_Width(ctx);
	uint32_t h = TinyDDS_Height(ctx);
	uint32_t d = TinyDDS_Depth(ctx);
	uint32_t s = TinyDDS_ArraySlices(ctx);
	uint32_t mm = TinyDDS_NumberOfMipmaps(ctx);
	TinyImageFormat fmt = TinyImageFormat_FromTinyDDSFormat(TinyDDS_GetFormat(ctx));
	if(fmt == TinyImageFormat_UNDEFINED) {
		TinyDDS_DestroyContext(ctx);
		return false;
	}

	// TheForge Image uses d = 0 as cubemap marker
	if(TinyDDS_IsCubemap(ctx)) d = 0;

	pImage->RedefineDimensions(fmt, w, h, d, mm, s);

	int size = pImage->GetMipMappedSize();

	if (pAllocator)
	{
		pImage->SetPixels((unsigned char*)pAllocator(pImage, size, pUserData));
	}
	else
	{
		pImage->SetPixels((unsigned char*)conf_malloc(sizeof(unsigned char) * size), true);
	}

	if (pImage->IsCube())
	{
		for (int face = 0; face < 6; face++)
		{
			for (uint mipMapLevel = 0; mipMapLevel < pImage->GetMipMapCount(); mipMapLevel++)
			{
				int            faceSize = pImage->GetMipMappedSize(mipMapLevel, 1) / 6;
				unsigned char* dst = pImage->GetPixels(mipMapLevel, 0) + face * faceSize;
				memcpy( dst, TinyDDS_ImageRawData(ctx, mipMapLevel), faceSize);
			}
		}
	}
	else {
		for (uint mipMapLevel = 0; mipMapLevel < pImage->GetMipMapCount(); mipMapLevel++) {
			int mmSize = pImage->GetMipMappedSize(mipMapLevel, 1);
			unsigned char *dst = pImage->GetPixels(mipMapLevel, mipMapLevel);
			memcpy(dst, TinyDDS_ImageRawData(ctx, mipMapLevel), mmSize);
		}
	}

	return true;
}

bool iLoadPVRFromMemory(Image* pImage, const char* memory, uint32_t size, memoryAllocationFunc pAllocator, void* pUserData)
{
#ifndef TARGET_IOS
	LOGERRORF("Load PVR failed: Only supported on iOS targets.");
	return false;
#else
	
	// TODO: Image
	// - no support for PVRTC2 at the moment since it isn't supported on iOS devices.
	// - only new PVR header V3 is supported at the moment.  Should we add legacy for V2 and V1?
	// - metadata is ignored for now.  Might be useful to implement it if the need for metadata arises (eg. padding, atlas coordinates, orientations, border data, etc...).
	// - flags are also ignored for now.  Currently a flag of 0x02 means that the color have been pre-multiplied byt the alpha values.
	
	// Assumptions:
	// - it's assumed that the texture is already twiddled (ie. Morton).  This should always be the case for PVRTC V3.
	
	PVR_Texture_Header* psPVRHeader = (PVR_Texture_Header*)memory;

	if (psPVRHeader->mVersion != gPvrtexV3HeaderVersion)
	{
		LOGERRORF( "Load PVR failed: Not a valid PVR V3 header.");
		return 0;
	}
	
	if (psPVRHeader->mPixelFormat > 3)
	{
		LOGERRORF( "Load PVR failed: Not a supported PVR pixel format.  Only PVRTC is supported at the moment.");
		return 0;
	}
	
	if (psPVRHeader->mNumSurfaces > 1 && psPVRHeader->mNumFaces > 1)
	{
		LOGERRORF( "Load PVR failed: Loading arrays of cubemaps isn't supported.");
		return 0;
	}

	uint32_t width = psPVRHeader->mWidth;
	uint32_t height = psPVRHeader->mHeight;
	uint32_t depth = (psPVRHeader->mNumFaces > 1) ? 0 : psPVRHeader->mDepth;
	uint32_t mipMapCount = psPVRHeader->mNumMipMaps;
	uint32_t arrayCount = psPVRHeader->mNumSurfaces;
	bool srgb = (psPVRHeader->mColorSpace == 1);
	ImageFormat::Enum imageFormat = ImageFormat::NONE;

	switch (psPVRHeader->mPixelFormat)
	{
	case 0:
		imageFormat = ImageFormat::PVR_2BPP;
		break;
	case 1:
		imageFormat = ImageFormat::PVR_2BPPA;
		break;
	case 2:
		imageFormat = ImageFormat::PVR_4BPP;
		break;
	case 3:
		imageFormat = ImageFormat::PVR_4BPPA;
		break;
	default:    // NOT SUPPORTED
		LOGERRORF( "Load PVR failed: pixel type not supported. ");
		ASSERT(0);
		return false;
	}

	if (depth != 0)
		arrayCount *= psPVRHeader->mNumFaces;

	pImage->RedefineDimensions(imageFormat, width, height, depth, mipMapCount, arrayCount, srgb);


	// Extract the pixel data
	size_t totalHeaderSizeWithMetadata = sizeof(PVR_Texture_Header) + psPVRHeader->mMetaDataSize;
	size_t pixelDataSize = pImage->GetMipMappedSize();

	if (pAllocator)
	{
		pImage->SetPixels((unsigned char*)pAllocator(pImage, sizeof(unsigned char) * pixelDataSize, pUserData));
	}
	else
	{
		pImage->SetPixels((unsigned char*)conf_malloc(sizeof(unsigned char) * pixelDataSize), true);
	}

	memcpy(pImage->GetPixels(), (unsigned char*)psPVRHeader + totalHeaderSizeWithMetadata, pixelDataSize);

	return true;
#endif
}

bool iLoadKTXFromMemory(Image* pImage, const char* memory, uint32_t memSize, memoryAllocationFunc pAllocator /*= NULL*/, void* pUserData /*= NULL*/)
{
	TinyKtx_Callbacks callbacks {
			&tinyktxddsCallbackError,
			&tinyktxddsCallbackAlloc,
			&tinyktxddsCallbackFree,
			&tinyktxddsCallbackRead,
			&tinyktxddsCallbackSeek,
			&tinyktxddsCallbackTell
	};

	MemoryBuffer memoryBuffer(memory, memSize);

	auto ctx =  TinyKtx_CreateContext( &callbacks, (void*)&memoryBuffer);
	bool headerOkay = TinyKtx_ReadHeader(ctx);
	if(!headerOkay) {
		TinyKtx_DestroyContext(ctx);
		return false;
	}

	uint32_t w = TinyKtx_Width(ctx);
	uint32_t h = TinyKtx_Height(ctx);
	uint32_t d = TinyKtx_Depth(ctx);
	uint32_t s = TinyKtx_ArraySlices(ctx);
	uint32_t mm = TinyKtx_NumberOfMipmaps(ctx);
	TinyImageFormat fmt = TinyImageFormat_FromTinyKtxFormat(TinyKtx_GetFormat(ctx));
	if(fmt == TinyImageFormat_UNDEFINED) {
		TinyKtx_DestroyContext(ctx);
		return false;
	}

	// TheForge Image uses d = 0 as cubemap marker
	if(TinyKtx_IsCubemap(ctx)) d = 0;

	pImage->RedefineDimensions(fmt, w, h, d, mm, s);

	int size = pImage->GetMipMappedSize();

	if (pAllocator)
	{
		pImage->SetPixels((uint8_t*)pAllocator(pImage, size, pUserData));
	}
	else
	{
		pImage->SetPixels((uint8_t*)conf_malloc(sizeof(uint8_t) * size), true);
	}

	if (pImage->IsCube())
	{
		for (int face = 0; face < 6; face++)
		{
			for (uint mipMapLevel = 0; mipMapLevel < pImage->GetMipMapCount(); mipMapLevel++)
			{
				uint32_t const faceSize = pImage->GetMipMappedSize(mipMapLevel, 1) / 6;
				uint8_t const* src = (uint8_t const*) TinyKtx_ImageRawData(ctx, mipMapLevel);
				uint8_t * dst = pImage->GetPixels(mipMapLevel, 0) + face * faceSize;

				if(TinyKtx_IsMipMapLevelUnpacked(ctx, mipMapLevel)) {
					uint32_t const srcStride = TinyKtx_UnpackedRowStride(ctx, mipMapLevel);
					uint32_t const dstStride = faceSize / pImage->GetHeight(mipMapLevel);

					for (uint32_t ww = 0u; ww < pImage->GetArrayCount(); ++ww) {
						for (uint32_t zz = 0; zz < pImage->GetDepth(); ++zz) {
							for (uint32_t yy = 0; yy < pImage->GetHeight(); ++yy) {
								memcpy(dst, src, dstStride);
								src += srcStride;
								dst += dstStride;
							}
						}
					}
				} else {
					memcpy(dst, src, faceSize);
				}
			}
		}
	}
	else {
		for (uint mipMapLevel = 0; mipMapLevel < pImage->GetMipMapCount(); mipMapLevel++) {
			uint32_t const mmSize = pImage->GetMipMappedSize(mipMapLevel, 1);
			uint8_t const* src = (uint8_t const*) TinyKtx_ImageRawData(ctx, mipMapLevel);
			uint8_t *dst = pImage->GetPixels(mipMapLevel, mipMapLevel);

			if(TinyKtx_IsMipMapLevelUnpacked(ctx, mipMapLevel)) {
				uint32_t const srcStride = TinyKtx_UnpackedRowStride(ctx, mipMapLevel);
				uint32_t const dstStride =  mmSize / pImage->GetHeight(mipMapLevel);

				for (uint32_t ww = 0u; ww < pImage->GetArrayCount(); ++ww) {
					for (uint32_t zz = 0; zz < pImage->GetDepth(); ++zz) {
						for (uint32_t yy = 0; yy < pImage->GetHeight(); ++yy) {
							memcpy(dst, src, dstStride);
							src += srcStride;
							dst += dstStride;
						}
					}
				}
			} else {
				memcpy(dst, src, mmSize);
			}
		}
	}
	return true;
}

#if defined(ORBIS)

// loads GNF header from memory
static GnfError iLoadGnfHeaderFromMemory(struct sce::Gnf::Header* outHeader, MemoryBuffer* mp)
{
	if (outHeader == NULL)    //  || gnfFile == NULL)
	{
		return kGnfErrorInvalidPointer;
	}
	outHeader->m_magicNumber = 0;
	outHeader->m_contentsSize = 0;

	mp->Read(outHeader, sizeof(sce::Gnf::Header));
	//MemFopen::fread(outHeader, sizeof(sce::Gnf::Header), 1, mp);

	//	fseek(gnfFile, 0, SEEK_SET);
	//	fread(outHeader, sizeof(sce::Gnf::Header), 1, gnfFile);
	if (outHeader->m_magicNumber != sce::Gnf::kMagic)
	{
		return kGnfErrorNotGnfFile;
	}
	return kGnfErrorNone;
}

// content size is sizeof(sce::Gnf::Contents)+gnfContents->m_numTextures*sizeof(sce::Gnm::Texture)+ paddings which is a variable of: gnfContents->alignment
static uint32_t iComputeContentSize(const sce::Gnf::Contents* gnfContents)
{
	// compute the size of used bytes
	uint32_t headerSize = sizeof(sce::Gnf::Header) + sizeof(sce::Gnf::Contents) + gnfContents->m_numTextures * sizeof(sce::Gnm::Texture);
	// add the paddings
	uint32_t align = 1 << gnfContents->m_alignment;    // actual alignment
	size_t   mask = align - 1;
	uint32_t missAligned = (headerSize & mask);    // number of bytes after the alignemnet point
	if (missAligned)                               // if none zero we have to add paddings
	{
		headerSize += align - missAligned;
	}
	return headerSize - sizeof(sce::Gnf::Header);
}

// loads GNF content from memory
static GnfError iReadGnfContentsFromMemory(sce::Gnf::Contents* outContents, uint32_t contentsSizeInBytes, MemoryBuffer* memstart)
{
	// now read the content data ...
	memstart->Read(outContents, contentsSizeInBytes);
	//MemFopen::fread(outContents, contentsSizeInBytes, 1, memstart);

	if (outContents->m_alignment > 31)
	{
		return kGnfErrorAlignmentOutOfRange;
	}

	if (outContents->m_version == 1)
	{
		if ((outContents->m_numTextures * sizeof(sce::Gnm::Texture) + sizeof(sce::Gnf::Contents)) != contentsSizeInBytes)
		{
			return kGnfErrorContentsSizeMismatch;
		}
	}
	else
	{
		if (outContents->m_version != sce::Gnf::kVersion)
		{
			return kGnfErrorVersionMismatch;
		}
		else
		{
			if (iComputeContentSize(outContents) != contentsSizeInBytes)
				return kGnfErrorContentsSizeMismatch;
		}
	}

	return kGnfErrorNone;
}

//------------------------------------------------------------------------------
//  Loads a GNF file from memory.
//
bool Image::iLoadGNFFromMemory(const char* memory, size_t memSize, const bool useMipMaps)
{
	GnfError result = kGnfErrorNone;

	MemoryBuffer m1(memory, memSize);

	sce::Gnf::Header header;
	result = iLoadGnfHeaderFromMemory(&header, m1);
	if (result != 0)
	{
		return false;
	}

	char*               memoryArray = (char*)conf_calloc(header.m_contentsSize, sizeof(char));
	sce::Gnf::Contents* gnfContents = NULL;
	gnfContents = (sce::Gnf::Contents*)memoryArray;

	// move the pointer behind the header
	const char*  mp = memory + sizeof(sce::Gnf::Header);
	MemoryBuffer m2(mp, memSize - sizeof(sce::Gnf::Header));

	result = iReadGnfContentsFromMemory(gnfContents, header.m_contentsSize, m2);

	mWidth = gnfContents->m_textures[0].getWidth();
	mHeight = gnfContents->m_textures[0].getHeight();
	mDepth = gnfContents->m_textures[0].getDepth();

	mMipMapCount =
		((!useMipMaps) || (gnfContents->m_textures[0].getLastMipLevel() == 0)) ? 1 : gnfContents->m_textures[0].getLastMipLevel();
	mArrayCount = (gnfContents->m_textures[0].getLastArraySliceIndex() > 1) ? gnfContents->m_textures[0].getLastArraySliceIndex() : 1;

	uint32 dataFormat = gnfContents->m_textures[0].getDataFormat().m_asInt;

	if (dataFormat == sce::Gnm::kDataFormatBc1Unorm.m_asInt || dataFormat == sce::Gnm::kDataFormatBc1UnormSrgb.m_asInt)
		mFormat = ImageFormat::GNF_BC1;
	//	else if(dataFormat == sce::Gnm::kDataFormatBc2Unorm.m_asInt || dataFormat == sce::Gnm::kDataFormatBc2UnormSrgb.m_asInt)
	//		format = ImageFormat::GNF_BC2;
	else if (dataFormat == sce::Gnm::kDataFormatBc3Unorm.m_asInt || dataFormat == sce::Gnm::kDataFormatBc3UnormSrgb.m_asInt)
		mFormat = ImageFormat::GNF_BC3;
	//	else if(dataFormat == sce::Gnm::kDataFormatBc4Unorm.m_asInt || dataFormat == sce::Gnm::kDataFormatBc4UnormSrgb.m_asInt)
	//		format = ImageFormat::GNF_BC4;
	// it seems in the moment there is no kDataFormatBc5UnormSrgb .. so I just check for the SRGB flag
	else if (
		dataFormat == sce::Gnm::kDataFormatBc5Unorm.m_asInt ||
		((dataFormat == sce::Gnm::kDataFormatBc5Unorm.m_asInt) &&
		 (gnfContents->m_textures[0].getDataFormat().getTextureChannelType() == sce::Gnm::kTextureChannelTypeSrgb)))
		mFormat = ImageFormat::GNF_BC5;
	//	else if(dataFormat == sce::Gnm::kDataFormatBc6Unorm.m_asInt || dataFormat == sce::Gnm::kDataFormatBc6UnormSrgb.m_asInt)
	//		format = ImageFormat::GNF_BC6;
	else if (dataFormat == sce::Gnm::kDataFormatBc7Unorm.m_asInt || dataFormat == sce::Gnm::kDataFormatBc7UnormSrgb.m_asInt)
		mFormat = ImageFormat::GNF_BC7;
	else
	{
		LOGERRORF( "Couldn't find the data format of the texture");
		return false;
	}

	//
	// storing the GNF header in the extra data
	//
	// we do this because on the addTexture level, we would like to have all this data to allocate and load the data
	//
	pAdditionalData = (unsigned char*)conf_calloc(header.m_contentsSize, sizeof(unsigned char));
	memcpy(pAdditionalData, gnfContents, header.m_contentsSize);

	// storing all the pixel data in pixels
	//
	// unfortunately that means we have the data twice in pixels and then in VRAM ...
	//
	sce::Gnm::SizeAlign pixelsSa = getTexturePixelsSize(gnfContents, 0);

	// move pointer forward
	const char*  memPoint = mp + header.m_contentsSize + getTexturePixelsByteOffset(gnfContents, 0);
	MemoryBuffer m3(memPoint, memSize - (sizeof(sce::Gnf::Header) + header.m_contentsSize + getTexturePixelsByteOffset(gnfContents, 0)));

	// dealing with mip-map stuff ... ???
	int size = pixelsSa.m_size;    //getMipMappedSize(0, nMipMaps);
	pData = (unsigned char*)conf_malloc(sizeof(unsigned char) * size);

	m3.Read(pData, size);
	//MemFopen::fread(pData, 1, size, m3);

	/*
  if (isCube()){
  for (int face = 0; face < 6; face++)
  {
  for (uint mipMapLevel = 0; mipMapLevel < nMipMaps; mipMapLevel++)
  {
  int faceSize = getMipMappedSize(mipMapLevel, 1) / 6;
  unsigned char *src = getPixels(mipMapLevel) + face * faceSize;

  memread(src, 1, faceSize, mp);
  }
  if ((useMipMaps ) && header.dwMipMapCount > 1)
  {
  memseek(mp, memory, getMipMappedSize(1, header.dwMipMapCount - 1) / 6, SEEK_CUR);
  }
  }
  }
  else
  {
  memread(pixels, 1, size, mpoint);
  }
  */
	conf_free(gnfContents);

	return !result;
}
#endif

// Image loading
// struct of table for file format to loading function
struct ImageLoaderDefinition
{
	eastl::string              mExtension;
	Image::ImageLoaderFunction pLoader;
};

static eastl::vector<ImageLoaderDefinition> gImageLoaders;

struct StaticImageLoader
{
	StaticImageLoader()
	{
		gImageLoaders.push_back({ ".dds", iLoadDDSFromMemory });
		gImageLoaders.push_back({ ".pvr", iLoadPVRFromMemory });
		gImageLoaders.push_back({ ".ktx", iLoadKTXFromMemory });
#if defined(ORBIS)
		gImageLoaders.push_back({ ".gnf", iLoadGNFFromMemory });
#endif

	}
} gImageLoaderInst;

void Image::AddImageLoader(const char* pExtension, ImageLoaderFunction pFunc) { gImageLoaders.push_back({ pExtension, pFunc }); }

bool Image::LoadFromMemory(
	void const* mem, uint32_t size, char const* extension, memoryAllocationFunc pAllocator, void* pUserData)
{
	// try loading the format
	bool loaded = false;
	for (uint32_t i = 0; i < (uint32_t)gImageLoaders.size(); ++i)
	{
		ImageLoaderDefinition const& def = gImageLoaders[i];
		if (stricmp(extension, def.mExtension.c_str()) == 0)
		{
			loaded = def.pLoader(this, (char const*)mem, size, pAllocator, pUserData);
			break;
		}
	}
	return loaded;
}

bool Image::LoadFromFile(const char* origFileName, memoryAllocationFunc pAllocator, void* pUserData, FSRoot root)
{
	// clear current image
	Clear();

	eastl::string extension = FileSystem::GetExtension(origFileName);
	uint32_t loaderIndex = -1;

	if (extension.size())
	{
		for (int i = 0; i < (int)gImageLoaders.size(); i++)
		{
			if (stricmp(extension.c_str(), gImageLoaders[i].mExtension.c_str()) == 0)
			{
				loaderIndex = i;
				break;
			}
		}

		if (loaderIndex == -1)
			extension = "";
	}

	char fileName[MAX_PATH] = {};
	strcpy(fileName, origFileName);
	if (!extension.size())
	{
#if defined(__ANDROID__)
		extension = ".ktx";
#elif defined(TARGET_IOS)
		extension = ".ktx";
#elif defined(__linux__)
		extension = ".dds";
#elif defined(__APPLE__)
		extension = ".dds";
#else
		extension = ".dds";
#endif

		strcpy(fileName + strlen(origFileName), extension.c_str());
	}

	// open file
	File file = {};
	file.Open(fileName, FM_ReadBinary, root);
	if (!file.IsOpen())
	{
		LOGERRORF( "\"%s\": Image file not found.", fileName);
		return false;
	}

	// load file into memory
	uint32_t length = file.GetSize();
	if (length == 0)
	{
		//char output[256];
		//sprintf(output, "\"%s\": Image file is empty.", fileName);
		LOGERRORF( "\"%s\": Image is an empty file.", fileName);
		file.Close();
		return false;
	}

	// read and close file.
	char* data = (char*)conf_malloc(length * sizeof(char));
	file.Read(data, (unsigned)length);
	file.Close();

	// try loading the format
	bool loaded = false;
	bool support = false;
	for (int i = 0; i < (int)gImageLoaders.size(); i++)
	{
		if (stricmp(extension.c_str(), gImageLoaders[i].mExtension.c_str()) == 0)
		{
			support = true;
			loaded = gImageLoaders[i].pLoader(this, data, length, pAllocator, pUserData);
			if (loaded)
			{
				break;
			}
		}
	}
	if (!support)
	{
		LOGERRORF( "Can't load this file format for image  :  %s", fileName);
	}
	else
	{
		mLoadFileName = fileName;
	}
	// cleanup the compressed data
	conf_free(data);

	return loaded;
}

bool Image::Convert(const TinyImageFormat newFormat)
{
	// TODO add RGBE8 to tiny image format
	if(TinyImageFormat_CanDecodeLogicalPixelsF(mFormat)) return false;
	if(TinyImageFormat_CanEncodeLogicalPixelsF(newFormat)) return false;

	int pixelCount = GetNumberOfPixels(0, mMipMapCount);

	ubyte* newPixels;
	newPixels = (unsigned char*)conf_malloc(sizeof(unsigned char) * GetMipMappedSize(0, mMipMapCount));

	TinyImageFormat_DecodeInput input{};
	input.pixel = pData;
	TinyImageFormat_EncodeOutput output{};
	input.pixel = newPixels;

	float* tmp = (float*)conf_malloc(sizeof(float) * 4 * pixelCount);

	TinyImageFormat_DecodeLogicalPixelsF(mFormat, &input, pixelCount, tmp);
	TinyImageFormat_EncodeLogicalPixelsF(newFormat, tmp, pixelCount, &output);

	conf_free(tmp);

	conf_free(pData);
	pData = newPixels;
	mFormat = newFormat;

	return true;
}

template <typename T>
void buildMipMap(T* dst, const T* src, const uint w, const uint h, const uint d, const uint c)
{
	uint xOff = (w < 2) ? 0 : c;
	uint yOff = (h < 2) ? 0 : c * w;
	uint zOff = (d < 2) ? 0 : c * w * h;

	for (uint z = 0; z < d; z += 2)
	{
		for (uint y = 0; y < h; y += 2)
		{
			for (uint x = 0; x < w; x += 2)
			{
				for (uint i = 0; i < c; i++)
				{
					*dst++ = (src[0] + src[xOff] + src[yOff] + src[yOff + xOff] + src[zOff] + src[zOff + xOff] + src[zOff + yOff] +
							  src[zOff + yOff + xOff]) /
							 8;
					src++;
				}
				src += xOff;
			}
			src += yOff;
		}
		src += zOff;
	}
}

bool Image::GenerateMipMaps(const uint32_t mipMaps)
{
	if (TinyImageFormat_IsCompressed(mFormat))
		return false;
	if (!(mWidth) || !isPowerOf2(mHeight) || !isPowerOf2(mDepth))
		return false;

	uint actualMipMaps = min(mipMaps, GetMipMapCountFromDimensions());

	if (mMipMapCount != actualMipMaps)
	{
		int size = GetMipMappedSize(0, actualMipMaps);
		if (mArrayCount > 1)
		{
			ubyte* newPixels = (ubyte*)conf_malloc(sizeof(ubyte) * size * mArrayCount);

			// Copy top mipmap of all array slices to new location
			int firstMipSize = GetMipMappedSize(0, 1);
			int oldSize = GetMipMappedSize(0, mMipMapCount);

			for (uint i = 0; i < mArrayCount; i++)
			{
				memcpy(newPixels + i * size, pData + i * oldSize, firstMipSize);
			}

			conf_free(pData);
			pData = newPixels;
		}
		else
		{
			pData = (ubyte*)conf_realloc(pData, size);
		}
		mMipMapCount = actualMipMaps;
	}

	int nChannels = TinyImageFormat_ChannelCount(mFormat);

	int n = IsCube() ? 6 : 1;

	for (uint arraySlice = 0; arraySlice < mArrayCount; arraySlice++)
	{
		ubyte* src = GetPixels(0, arraySlice);
		ubyte* dst = GetPixels(1, arraySlice);

		for (uint level = 1; level < mMipMapCount; level++)
		{
			int w = GetWidth(level - 1);
			int h = GetHeight(level - 1);
			int d = GetDepth(level - 1);

			uint32_t srcSize = GetMipMappedSize(level - 1, 1) / n;
			uint32_t dstSize = GetMipMappedSize(level, 1) / n;

			for (int i = 0; i < n; i++)
			{
				// only homogoenous is supported via this method
				// TODO use decode/encode for the others
				// TODO check these methods work for SNORM
				if(TinyImageFormat_IsHomogenous(mFormat)) {
					uint32 redChanWidth = TinyImageFormat_ChannelBitWidth(mFormat, TinyImageFormat_LC_Red);
					if (redChanWidth == 32 && TinyImageFormat_IsFloat(mFormat))
					{
						buildMipMap((float*)dst, (float*)src, w, h, d, nChannels);
					}
					else if (redChanWidth == 32)
					{
						buildMipMap((uint32_t*)dst, (uint32_t*)src, w, h, d, nChannels);
					}
					else if (redChanWidth == 16)
					{
						buildMipMap((uint16_t*)dst, (uint16_t*)src, w, h, d, nChannels);
					}
					else if (redChanWidth == 8)
					{
						buildMipMap((uint8_t*)dst, (uint8_t*)src, w, h, d, nChannels);
					}
					// TODO fall back to to be written generic downsizer
				}
				src += srcSize;
				dst += dstSize;
			}
		}
	}

	return true;
}

// -- IMAGE SAVING --

bool Image::iSaveDDS(const char* fileName)
{
	// TODO replace with TInyDDS

	return false;
}

bool convertAndSaveImage(const Image& image, bool (Image::*saverFunction)(const char*), const char* fileName)
{
	bool  bSaveImageSuccess = false;
	Image imgCopy(image);
	imgCopy.Uncompress();
	if (imgCopy.Convert(TinyImageFormat_R8G8B8A8_UNORM))
	{
		bSaveImageSuccess = (imgCopy.*saverFunction)(fileName);
	}

	imgCopy.Destroy();
	return bSaveImageSuccess;
}

bool Image::iSaveTGA(const char* fileName)
{
	switch (mFormat)
	{
		case TinyImageFormat_R8_UNORM: return 0 != stbi_write_tga(fileName, mWidth, mHeight, 1, pData); break;
		case TinyImageFormat_R8G8_UNORM: return 0 != stbi_write_tga(fileName, mWidth, mHeight, 2, pData); break;
		case TinyImageFormat_R8G8B8_UNORM: return 0 != stbi_write_tga(fileName, mWidth, mHeight, 3, pData); break;
		case TinyImageFormat_R8G8B8A8_UNORM: return 0 != stbi_write_tga(fileName, mWidth, mHeight, 4, pData); break;
		default:
		{
			// uncompress/convert and try again
			return convertAndSaveImage(*this, &Image::iSaveTGA, fileName);
		}
	}

	//return false; //Unreachable
}

bool Image::iSaveBMP(const char* fileName)
{
	switch (mFormat)
	{
		case TinyImageFormat_R8_UNORM: stbi_write_bmp(fileName, mWidth, mHeight, 1, pData); break;
		case TinyImageFormat_R8G8_UNORM: stbi_write_bmp(fileName, mWidth, mHeight, 2, pData); break;
		case TinyImageFormat_R8G8B8_UNORM: stbi_write_bmp(fileName, mWidth, mHeight, 3, pData); break;
		case TinyImageFormat_R8G8B8A8_UNORM: stbi_write_bmp(fileName, mWidth, mHeight, 4, pData); break;
		default:
		{
			// uncompress/convert and try again
			return convertAndSaveImage(*this, &Image::iSaveBMP, fileName);
		}
	}
	return true;
}

bool Image::iSavePNG(const char* fileName)
{
	switch (mFormat)
	{
		case TinyImageFormat_R8_UNORM: stbi_write_png(fileName, mWidth, mHeight, 1, pData, 0); break;
		case TinyImageFormat_R8G8_UNORM: stbi_write_png(fileName, mWidth, mHeight, 2, pData, 0); break;
		case TinyImageFormat_R8G8B8_UNORM: stbi_write_png(fileName, mWidth, mHeight, 3, pData, 0); break;
		case TinyImageFormat_R8G8B8A8_UNORM: stbi_write_png(fileName, mWidth, mHeight, 4, pData, 0); break;
		default:
		{
			// uncompress/convert and try again
			return convertAndSaveImage(*this, &Image::iSavePNG, fileName);
		}
	}

	return true;
}

bool Image::iSaveHDR(const char* fileName)
{
	switch (mFormat)
	{
		case TinyImageFormat_R32_SFLOAT: stbi_write_hdr(fileName, mWidth, mHeight, 1, (float*)pData); break;
		case TinyImageFormat_R32G32_SFLOAT: stbi_write_hdr(fileName, mWidth, mHeight, 2, (float*)pData); break;
		case TinyImageFormat_R32G32B32_SFLOAT: stbi_write_hdr(fileName, mWidth, mHeight, 3, (float*)pData); break;
		case TinyImageFormat_R32G32B32A32_SFLOAT: stbi_write_hdr(fileName, mWidth, mHeight, 4, (float*)pData); break;
		default:
		{
			// uncompress/convert and try again
			return convertAndSaveImage(*this, &Image::iSaveHDR, fileName);
		}
	}

	return true;
}

bool Image::iSaveJPG(const char* fileName)
{
	switch (mFormat)
	{
		case TinyImageFormat_R8_UNORM: stbi_write_jpg(fileName, mWidth, mHeight, 1, pData, 0); break;
		case TinyImageFormat_R8G8_UNORM: stbi_write_jpg(fileName, mWidth, mHeight, 2, pData, 0); break;
		case TinyImageFormat_R8G8B8_UNORM: stbi_write_jpg(fileName, mWidth, mHeight, 3, pData, 0); break;
		case TinyImageFormat_R8G8B8A8_UNORM: stbi_write_jpg(fileName, mWidth, mHeight, 4, pData, 0); break;
		default:
		{
			// uncompress/convert and try again
			return convertAndSaveImage(*this, &Image::iSaveJPG, fileName);
		}
	}

	return true;
}

struct ImageSaverDefinition
{
	typedef bool (Image::*ImageSaverFunction)(const char*);
	const char*        Extension;
	ImageSaverFunction Loader;
};

static ImageSaverDefinition gImageSavers[] = {
#if !defined(NO_STBI)
	{ ".bmp", &Image::iSaveBMP }, { ".hdr", &Image::iSaveHDR }, { ".png", &Image::iSavePNG },
	{ ".tga", &Image::iSaveTGA }, { ".jpg", &Image::iSaveJPG },
#endif
	{ ".dds", &Image::iSaveDDS }
};

bool Image::Save(const char* fileName)
{
	const char* extension = strrchr(fileName, '.');
	bool        support = false;
	;
	for (int i = 0; i < sizeof(gImageSavers) / sizeof(gImageSavers[0]); i++)
	{
		if (stricmp(extension, gImageSavers[i].Extension) == 0)
		{
			support = true;
			return (this->*gImageSavers[i].Loader)(fileName);
		}
	}
	if (!support)
	{
		LOGERRORF( "Can't save this file format for image  :  %s", fileName);
	}

	return false;
}
