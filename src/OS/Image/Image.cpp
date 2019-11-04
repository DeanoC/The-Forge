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
#include <cstdint>
#include "Image.h"
#include "../Interfaces/ILog.h"
#include "../../ThirdParty/OpenSource/EASTL/algorithm.h"
#include "../../ThirdParty/OpenSource/ModifiedSonyMath/vectormath.hpp"

#include "../../ThirdParty/OpenSource/tinyimageformat/tinyimageformat_base.h"
#include "../../ThirdParty/OpenSource/tinyimageformat/tinyimageformat_query.h"
#include "../../ThirdParty/OpenSource/tinyimageformat/tinyimageformat_bits.h"
#include "../../ThirdParty/OpenSource/tinyimageformat/tinyimageformat_decode.h"
#include "../../ThirdParty/OpenSource/tinyimageformat/tinyimageformat_encode.h"
#include "ImageHelper.h"

#include "../Interfaces/IMemory.h"


// --- BLOCK DECODING ---

// TODO Decode these decode block don't handle SRGB properly
void iDecodeColorBlock(
	unsigned char* dest, int w, int h, int xOff, int yOff, TinyImageFormat format, int red, int blue, unsigned char* src)
{
	unsigned char colors[4][3];

	uint16_t c0 = *(uint16_t*)src;
	uint16_t c1 = *(uint16_t*)(src + 2);

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
			}
			else if (format == TinyImageFormat_DXBC3_UNORM || format == TinyImageFormat_DXBC3_SRGB)
			{
				iDecodeDXT5Block(dst + 3, sx, sy, nChannels, width * nChannels, src);
			}
			if ((format == TinyImageFormat_DXBC1_RGBA_UNORM || format == TinyImageFormat_DXBC1_RGB_UNORM) ||
					(format == TinyImageFormat_DXBC1_RGBA_SRGB || format == TinyImageFormat_DXBC1_RGB_SRGB))
            {
				iDecodeColorBlock(dst, sx, sy, nChannels, width * nChannels, format, 0, 2, src);
			}
			else
			{
				if (format == TinyImageFormat_DXBC4_UNORM || format == TinyImageFormat_DXBC4_SNORM)
				{
					iDecodeDXT5Block(dst, sx, sy, 1, width, src);
				}
				else if (format == TinyImageFormat_DXBC5_UNORM || format == TinyImageFormat_DXBC5_SNORM)
				{
					iDecodeDXT5Block(dst, sx, sy, 2, width * 2, src + 8);
					iDecodeDXT5Block(dst + 1, sx, sy, 2, width * 2, src);
				}
				else
					return;
			}
            src += TinyImageFormat_BitSizeOfBlock(format) / 8;

		}
	}
}

Image::Image()
{
	pData = NULL;
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
//	UNREF_PARAM(dummy);
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
    
    // two ways of storing slices and mipmaps
    // 1. Old Image way. memory slices * ((w*h*d)*mipmaps)
    // 2. Mips after slices way. There are w*h*d*s*mipmaps where slices stays constant(doesn't reduce)
    if(!mMipsAfterSlices) {
        return pData + GetMipMappedSize(0, mMipMapCount) * arraySlice + GetMipMappedSize(0, mipMapLevel);
    } else {
        return pData + GetMipMappedSize(0, mipMapLevel) + arraySlice * GetArraySliceSize(mipMapLevel);
    }

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
	uint32_t m = eastl::max(mWidth, mHeight);
	m = eastl::max(m, mDepth);

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
    uint32_t d = GetDepth(mipMapLevel);
    if(d == 0) d = 1;

    if (srcFormat == TinyImageFormat_UNDEFINED)
        srcFormat = mFormat;

    uint32_t const bw = TinyImageFormat_WidthOfBlock(srcFormat);
    uint32_t const bh = TinyImageFormat_HeightOfBlock(srcFormat);
    uint32_t const bd = TinyImageFormat_DepthOfBlock(srcFormat);
    w = (w + (bw-1)) / bw;
    h = (h + (bh-1)) / bh;
    d = (d + (bd-1)) / bd;

	return (w * h * d * TinyImageFormat_BitSizeOfBlock(srcFormat)) / 8U;
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

	uint8_t * newPixels = (uint8_t*)conf_malloc(sizeof(uint8_t) * GetMipMappedSize(0, mMipMapCount, destFormat));

	int    level = 0;
	uint8_t *src, *dst = newPixels;
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

uint32_t Image::GetMipMappedSize(const uint firstMipMapLevel, uint32_t nMipMapLevels, TinyImageFormat srcFormat) const
{
	uint32_t w = GetWidth(firstMipMapLevel);
	uint32_t h = GetHeight(firstMipMapLevel);
	uint32_t d = GetDepth(firstMipMapLevel);
    d = d ? d : 1; // if a cube map treats a 2D texture for calculations
    uint32_t const s = GetArrayCount();

	if (srcFormat == TinyImageFormat_UNDEFINED)
		srcFormat = mFormat;
	
	// PVR formats get special case
	uint64_t const tifname = (TinyImageFormat_Code(mFormat) & TinyImageFormat_NAMESPACE_REQUIRED_BITS);
	if( tifname == TinyImageFormat_NAMESPACE_PVRTC)
	{
        // AFAIK pvr isn't supported for arrays
        ASSERT(s == 1);
		uint32_t totalSize = 0;
		uint32_t sizeX = w;
		uint32_t sizeY = h;
		uint32_t sizeD = d;
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
			sizeX = eastl::max(sizeX / 2, MinimumSize);
			sizeY = eastl::max(sizeY / 2, MinimumSize);
			sizeD = eastl::max(sizeD / 2, MinimumSize);
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
	// mips after slices means the slice count is included in mipsize but slices doesn't reduce
	// as slices are included, cubemaps also just fall out
	if(mMipsAfterSlices) return (mDepth == 0) ? 6 * size * s : size * s;
	else return (mDepth == 0) ? 6 * size : size;
}


bool Image::Convert(const TinyImageFormat newFormat)
{
	// TODO add RGBE8 to tiny image format
	if(TinyImageFormat_CanDecodeLogicalPixelsF(mFormat)) return false;
	if(TinyImageFormat_CanEncodeLogicalPixelsF(newFormat)) return false;

	int pixelCount = GetNumberOfPixels(0, mMipMapCount);

	uint8_t * newPixels;
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
			uint8_t * newPixels = (uint8_t *)conf_malloc(sizeof(uint8_t ) * size * mArrayCount);

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
			pData = (uint8_t *)conf_realloc(pData, size);
		}
		mMipMapCount = actualMipMaps;
	}

	int nChannels = TinyImageFormat_ChannelCount(mFormat);

	int n = IsCube() ? 6 : 1;

	for (uint arraySlice = 0; arraySlice < mArrayCount; arraySlice++)
	{
		uint8_t * src = GetPixels(0, arraySlice);
		uint8_t * dst = GetPixels(1, arraySlice);

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
					uint32_t  redChanWidth = TinyImageFormat_ChannelBitWidth(mFormat, TinyImageFormat_LC_Red);
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
