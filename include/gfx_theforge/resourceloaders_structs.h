#pragma once
#ifndef GFX_THEFORGE_RESOURCELOADER_STRUCTS_H_
#define GFX_THEFORGE_RESOURCELOADER_STRUCTS_H_

#include "gfx_theforge/enums.h"
#include "gfx_theforge/structs.h"

typedef struct TheForge_BufferLoadDesc
{
	TheForge_BufferHandle*    pBuffer;
	const void* pData;
	TheForge_BufferDesc  mDesc;
	/// Force Reset buffer to NULL
	bool mForceReset;
} TheForge_BufferLoadDesc;

typedef struct TheForge_RawImageData
{
	unsigned char* pRawData;
	TheForge_ImageFormat mFormat;
	uint32_t mWidth, mHeight, mDepth, mArraySize, mMipLevels;
} TheForge_RawImageData;

typedef struct TheForge_BinaryImageData
{
	unsigned char* pBinaryData;
	uint32_t mSize;
	const char* pExtension;
} TheForge_BinaryImageData;

typedef struct TheForge_TextureLoadDesc
{
	TheForge_TextureHandle* pTexture;

	/// Load empty texture
	TheForge_TextureDesc* pDesc;
	/// Load texture from disk
	const char* pFilename;
	TheForge_ResourceFolders      mRoot;
	uint32_t    mNodeIndex;
	bool        mSrgb;
	/// Load texture from raw data
	TheForge_RawImageData* pRawImageData = NULL;
	/// Load texture from binary data (with header)
	TheForge_BinaryImageData* pBinaryImageData = NULL;

	// Following is ignored if pDesc != NULL.  pDesc->mFlags will be considered instead.
	TheForge_TextureCreationFlags mCreationFlag;
} TheForge_TextureLoadDesc;

typedef struct TheForge_BufferUpdateDesc
{
	TheForge_BufferHandle     buffer;
	const void* pData;
	uint64_t    mSrcOffset;
	uint64_t    mDstOffset;
	uint64_t    mSize;    // If 0, uses size of pBuffer
} TheForge_BufferUpdateDesc;

typedef struct TheForge_TextureUpdateDesc
{
	TheForge_TextureHandle texture;
	TheForge_RawImageData* pRawImageData;
} TheForge_TextureUpdateDesc;

typedef struct TheForge_ResourceUpdateDesc
{
	TheForge_ResourceType mType;
	union
	{
		TheForge_BufferUpdateDesc  buf;
		TheForge_TextureUpdateDesc tex;
	};
} TheForge_ResourceUpdateDesc;

typedef struct TheForge_ShaderStageLoadDesc
{
	const char* 							mFileName;
	TheForge_ShaderMacro*    pMacros;
	uint32_t        					mMacroCount;
	TheForge_ResourceFolders          					mRoot;
	const char*     					mEntryPointName;
} TheForge_ShaderStageLoadDesc;

typedef struct TheForge_ShaderLoadDesc
{
	TheForge_ShaderStageLoadDesc mStages[TheForge_SS_COUNT];
	TheForge_ShaderTarget        mTarget;
} TheForge_ShaderLoadDesc;

// access atomically!
typedef uint64_t TheForge_SyncToken;

typedef struct TheForge_ResourceLoaderDesc
{
	uint64_t mBufferSize;
	uint32_t mBufferCount;
	uint32_t mTimesliceMs;
} TheForge_ResourceLoaderDesc;

#endif