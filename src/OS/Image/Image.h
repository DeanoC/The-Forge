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

#ifndef IMAGE_CLASS_ALLOWED
static_assert(false, "Image.h can only be included by ResourceLoader.cpp and Image.cpp");
#endif

#ifndef COMMON_3_OS_IMAGE_IMAGE_H_
#define COMMON_3_OS_IMAGE_IMAGE_H_

#include "../../ThirdParty/OpenSource/tinyimageformat/tinyimageformat_base.h"
#include "../../ThirdParty/OpenSource/tinyimageformat/tinyimageformat_query.h"

#define ALL_MIPLEVELS 127

/************************************************************************************/
// Define some useful macros
#define MCHAR2(a, b) (a | (b << 8))
#define MAKE_CHAR4(a, b, c, d) (a | (b << 8) | (c << 16) | (d << 24))

/*************************************************************************************/

typedef void* (*memoryAllocationFunc)(class Image* pImage, uint64_t memoryRequirement, void* pUserData);

class Image
{
protected:
	Image();
	Image(const Image& img);
	void Destroy();
private:

	friend class ResourceLoader;
	friend bool convertAndSaveImage(const Image& image, bool (Image::*saverFunction)(const char*), const char* fileName);

	unsigned char* Create(const TinyImageFormat fmt, const int w, const int h, const int d, const int mipMapCount, const int arraySize = 1);
	// The following Create function will use passed in data as reference without allocating memory for internal pData (meaning the Image object will not own the data)
	unsigned char* Create(const TinyImageFormat fmt, const int w, const int h, const int d, const int mipMapCount, const int arraySize, const unsigned char* rawData);

	void Clear();

public:

    void RedefineDimensions(const TinyImageFormat fmt, const int w, const int h, const int d, const int mipMapCount, const int arraySize = 1);

	unsigned char* GetPixels() const { return pData; }
	unsigned char* GetPixels(const uint32_t mipMapLevel) const;
	unsigned char* GetPixels(unsigned char* pDstData, const uint32_t mipMapLevel, const uint32_t dummy);
	unsigned char* GetPixels(const uint32_t mipMapLevel, const uint32_t arraySlice) const;

	void SetPixels(unsigned char* pixelData, bool own = false)
	{
		mOwnsMemory = own;
		pData = pixelData;
	}

	uint32_t                 GetWidth() const { return mWidth; }
	uint32_t                 GetHeight() const { return mHeight; }
	uint32_t                 GetDepth() const { return mDepth; }
	uint32_t                 GetWidth(const int mipMapLevel) const;
	uint32_t                 GetHeight(const int mipMapLevel) const;
	uint32_t                 GetDepth(const int mipMapLevel) const;
	uint32_t                 GetMipMapCount() const { return mMipMapCount; }
	uint32_t                 GetMipMapCountFromDimensions() const;
	uint32_t                 GetArraySliceSize(const uint32_t mipMapLevel = 0, TinyImageFormat srcFormat = TinyImageFormat_UNDEFINED) const;
	uint32_t                 GetNumberOfPixels(const uint32_t firstMipLevel = 0, uint32_t numMipLevels = ALL_MIPLEVELS) const;
	bool                 GetColorRange(float& min, float& max);
	TinyImageFormat    	 GetFormat() const { return mFormat; }
	uint32_t                 GetArrayCount() const { return mArrayCount; }
	uint32_t                 GetMipMappedSize(
		const uint32_t firstMipLevel = 0, uint32_t numMipLevels = ALL_MIPLEVELS, TinyImageFormat srcFormat = TinyImageFormat_UNDEFINED) const;

	bool                 Is1D() const { return (mDepth == 1 && mHeight == 1); }
	bool                 Is2D() const { return (mDepth == 1 && mHeight > 1); }
	bool                 Is3D() const { return (mDepth > 1); }
	bool                 IsArray() const { return (mArrayCount > 1); }
	bool                 IsCube() const { return (mDepth == 0); }
	bool                 IsSrgb() const { return TinyImageFormat_IsSRGB(mFormat); }
	bool                 IsLinearLayout() const { return mLinearLayout; }
	bool                 AreMipsAfterSlices() const { return mMipsAfterSlices; }

	void                 SetMipsAfterSlices(bool onoff) { mMipsAfterSlices = onoff; }

	bool                 Normalize();
	bool                 Uncompress();
	bool                 Unpack();

	bool                 Convert(const TinyImageFormat newFormat);
	bool                 GenerateMipMaps(const uint32_t mipMaps = ALL_MIPLEVELS);

	bool                 iSwap(const int c0, const int c1);

protected:
	unsigned char*       pData;
	uint32_t             mWidth, mHeight, mDepth;
	uint32_t             mMipMapCount;
	uint32_t             mArrayCount;
	TinyImageFormat    	 mFormat;
	int                  mAdditionalDataSize;
	unsigned char*       pAdditionalData;
	bool                 mLinearLayout;
	bool                 mOwnsMemory;
	// is memory (mipmaps*w*h*d)*s or
	// mipmaps * (w*h*d*s) with s being constant for all mipmaps
	bool				 mMipsAfterSlices;

public:
};

static inline uint32_t calculateMipMapLevels(uint32_t width, uint32_t height)
{
	if (width == 0 || height == 0)
		return 0;

	uint32_t result = 1;
	while (width > 1 || height > 1)
	{
		width >>= 1;
		height >>= 1;
		result++;
	}
	return result;
}

#endif    // COMMON_3_OS_IMAGE_IMAGE_H_
