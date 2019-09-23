#pragma once
#ifndef GFX_THEFORGE_STRUCTS_H_
#define GFX_THEFORGE_STRUCTS_H_

#include "gfx_theforge/enums.h"
#include "tiny_imageformat/tinyimageformat_base.h"

typedef struct TheForge_Renderer *TheForge_RendererHandle;
typedef struct TheForge_Fence *TheForge_FenceHandle;
typedef struct TheForge_Semaphore *TheForge_SemaphoreHandle;
typedef struct TheForge_Queue *TheForge_QueueHandle;
typedef struct TheForge_CmdPool *TheForge_CmdPoolHandle;
typedef struct TheForge_Cmd *TheForge_CmdHandle;
typedef struct TheForge_RenderTarget *TheForge_RenderTargetHandle;
typedef struct TheForge_Sampler *TheForge_SamplerHandle;
typedef struct TheForge_Shader *TheForge_ShaderHandle;
typedef struct TheForge_RootSignature *TheForge_RootSignatureHandle;
typedef struct TheForge_Pipeline *TheForge_PipelineHandle;
typedef struct TheForge_Raytracing *TheForge_RaytracingHandle;
typedef struct TheForge_RaytracingHitGroup *TheForge_RaytracingHitGroupHandle;
typedef struct TheForge_BlendState *TheForge_BlendStateHandle;
typedef struct TheForge_DepthState *TheForge_DepthStateHandle;
typedef struct TheForge_RasterizerState *TheForge_RasterizerStateHandle;
typedef struct TheForge_DescriptorInfo *TheForge_DescriptorInfoHandle;
typedef struct TheForge_DescriptorSet *TheForge_DescriptorSetHandle;
typedef struct TheForge_Buffer *TheForge_BufferHandle;

typedef struct TheForge_Texture *TheForge_TextureHandle;
typedef struct TheForge_AcclerationStructure *TheForge_AcclerationStructureHandle;
typedef struct TheForge_QueryPool *TheForge_QueryPoolHandle;
typedef struct TheForge_CommandSignature *TheForge_CommandSignatureHandle;
typedef struct TheForge_SwapChain *TheForge_SwapChainHandle;

typedef struct TheForge_ClearValue {
	union {
		struct {
			float r;
			float g;
			float b;
			float a;
		};
		struct {
			float depth;
			uint32_t stencil;
		};
	};
} TheForge_ClearValue;

typedef struct TheForge_RendererDesc {
	TheForge_ShaderTarget shaderTarget;
	TheForge_GpuMode gpuMode;

	union {
		TheForge_D3DFeatureLevel d3dFeatureLevel;
	};
} TheForge_RendererDesc;

typedef struct TheForge_QueueDesc {
	TheForge_QueueFlag flags;
	TheForge_QueuePriority priority;
	TheForge_CmdPoolType type;
	uint32_t nodeIndex;
} TheForge_QueueDesc;

typedef struct TheForge_RenderTargetDesc {
	/// Texture creation flags (decides memory allocation strategy, sharing access,...)
	TheForge_TextureCreationFlags flags;
	/// Width
	uint32_t width;
	/// Height
	uint32_t height;
	/// Depth (Should be 1 if not a mType is not TEXTURE_TYPE_3D)
	uint32_t depth;
	/// Texture array size (Should be 1 if texture is not a texture array or cubemap)
	uint32_t arraySize;
	/// Number of mip levels
	uint32_t mipLevels;
	/// MSAA
	TheForge_SampleCount sampleCount;
	/// image format
	TinyImageFormat format;
	/// Optimized clear value (recommended to use this same value when clearing the rendertarget)
	TheForge_ClearValue clearValue;
	/// The image quality level. The higher the quality, the lower the performance. The valid range is between zero and the value appropriate for mSampleCount
	uint32_t sampleQuality;
	/// Descriptor creation
	TheForge_DescriptorType descriptors;
	/// Debug name used in gpu profile
	char const *debugName;

} TheForge_RenderTargetDesc;

typedef struct TheForge_ShaderMacro {
	char const *definition;
	char const *value;
} TheForge_ShaderMacro;

typedef struct TheForge_RendererShaderDefinesDesc {
	TheForge_ShaderMacro *rendererShaderDefines;
	uint32_t rendererShaderDefinesCnt;
} TheForge_RendererShaderDefinesDesc;

typedef struct TheForge_ShaderStageDesc {
	char const *name;
	char const *code;
	char const *entryPoint;
	TheForge_ShaderMacro *macros;
	uint32_t macroCount;
} TheForge_ShaderStageDesc;

typedef struct TheForge_ShaderDesc {
	TheForge_ShaderStage stages;
	TheForge_ShaderStageDesc vert;
	TheForge_ShaderStageDesc frag;
	TheForge_ShaderStageDesc geom;
	TheForge_ShaderStageDesc hull;
	TheForge_ShaderStageDesc domain;
	TheForge_ShaderStageDesc comp;
} TheForge_ShaderDesc;

typedef struct TheForge_BinaryShaderStageDesc {
	/// Byte code array
	char const *byteCode;
	uint32_t byteCodeSize;
	char const *entryPoint;

	// Shader source is needed for reflection on Metal only
	char const *source;
} TheForge_BinaryShaderStageDesc;

typedef struct TheForge_BinaryShaderDesc {
	TheForge_ShaderStage stages;
	TheForge_BinaryShaderStageDesc vert;
	TheForge_BinaryShaderStageDesc frag;
	TheForge_BinaryShaderStageDesc geom;
	TheForge_BinaryShaderStageDesc hull;
	TheForge_BinaryShaderStageDesc domain;
	TheForge_BinaryShaderStageDesc comp;
} TheForge_BinaryShaderDesc;

typedef struct TheForge_SamplerDesc {
	TheForge_FilterType minFilter;
	TheForge_FilterType magFilter;
	TheForge_MipMapMode mipMapMode;
	TheForge_AddressMode addressU;
	TheForge_AddressMode addressV;
	TheForge_AddressMode addressW;
	float mipLodBias;
	float maxAnisotropy;
	TheForge_CompareMode compareFunc;
} TheForge_SamplerDesc;

typedef struct TheForge_RootSignatureDesc {
	TheForge_ShaderHandle *pShaders;
	uint32_t shaderCount;
	uint32_t maxBindlessTextures;
	const char **pStaticSamplerNames;
	TheForge_SamplerHandle *pStaticSamplers;
	uint32_t staticSamplerCount;
	TheForge_RootSignatureFlags flags;
} TheForge_RootSignatureDesc;

typedef struct TheForge_VertexAttrib {
	TheForge_ShaderSemantic semantic;
	uint32_t semanticNameLength;
	char semanticName[TheForge_MAX_SEMANTIC_NAME_LENGTH];
	TinyImageFormat format;
	uint32_t binding;
	uint32_t location;
	uint32_t offset;
	TheForge_VertexAttribRate rate;
} TheForge_VertexAttrib;

typedef struct TheForge_VertexLayout {
	uint32_t attribCount;
	TheForge_VertexAttrib attribs[TheForge_MAX_VERTEX_ATTRIBS];
} TheForge_VertexLayout;

typedef struct TheForge_RaytracingPipelineDesc {
	TheForge_RaytracingHandle raytracing;
	TheForge_RootSignatureHandle globalRootSignature;
	TheForge_ShaderHandle rayGenShader;
	TheForge_RootSignatureHandle rayGenRootSignature;
	TheForge_ShaderHandle *pMissShaders;
	TheForge_RootSignatureHandle *pMissRootSignatures;
	TheForge_RaytracingHitGroupHandle hitGroups;
	TheForge_RootSignatureHandle emptyRootSignature;
	unsigned missShaderCount;
	unsigned hitGroupCount;
	// #TODO : Remove this after adding shader reflection for raytracing shaders
	unsigned payloadSize;
	// #TODO : Remove this after adding shader reflection for raytracing shaders
	unsigned attributeSize;
	unsigned maxTraceRecursionDepth;
	unsigned maxRaysCount;
} TheForge_RaytracingPipelineDesc;

typedef struct TheForge_GraphicsPipelineDesc {
	TheForge_ShaderHandle shaderProgram;
	TheForge_RootSignatureHandle rootSignature;
	TheForge_VertexLayout const *pVertexLayout;
	TheForge_BlendStateHandle blendState;
	TheForge_DepthStateHandle depthState;
	TheForge_RasterizerStateHandle rasterizerState;
	TinyImageFormat const *pColorFormats;
	uint32_t renderTargetCount;
	TheForge_SampleCount sampleCount;
	uint32_t sampleQuality;
	TinyImageFormat depthStencilFormat;
	TheForge_PrimitiveTopology primitiveTopo;
} TheForge_GraphicsPipelineDesc;

typedef struct TheForge_ComputePipelineDesc {
	TheForge_ShaderHandle shaderProgram;
	TheForge_RootSignatureHandle rootSignature;
} TheForge_ComputePipelineDesc;

typedef struct TheForge_PipelineDesc {
	TheForge_PipelineType type;
	union {
		TheForge_ComputePipelineDesc computeDesc;
		TheForge_GraphicsPipelineDesc graphicsDesc;
		TheForge_RaytracingPipelineDesc raytracingDesc;
	};
} TheForge_PipelineDesc;

typedef struct TheForge_BlendStateDesc {
	TheForge_BlendConstant srcFactors[TheForge_MAX_RENDER_TARGET_ATTACHMENTS];
	TheForge_BlendConstant dstFactors[TheForge_MAX_RENDER_TARGET_ATTACHMENTS];
	TheForge_BlendConstant srcAlphaFactors[TheForge_MAX_RENDER_TARGET_ATTACHMENTS];
	TheForge_BlendConstant dstAlphaFactors[TheForge_MAX_RENDER_TARGET_ATTACHMENTS];
	TheForge_BlendMode blendModes[TheForge_MAX_RENDER_TARGET_ATTACHMENTS];
	TheForge_BlendMode blendAlphaModes[TheForge_MAX_RENDER_TARGET_ATTACHMENTS];
	uint32_t masks[TheForge_MAX_RENDER_TARGET_ATTACHMENTS];
	TheForge_BlendStateTargets renderTargetMask;
	bool alphaToCoverage;
	bool independentBlend;
} TheForge_BlendStateDesc;

typedef struct TheForge_DepthStateDesc {
	bool depthTest;
	bool depthWrite;

	TheForge_CompareMode depthFunc;

	bool stencilTest;
	uint8_t stencilReadMask;
	uint8_t stencilWriteMask;

	TheForge_CompareMode stencilFrontFunc;

	TheForge_StencilOp stencilFrontFail;
	TheForge_StencilOp depthFrontFail;
	TheForge_StencilOp stencilFrontPass;

	TheForge_CompareMode stencilBackFunc;
	TheForge_StencilOp stencilBackFail;
	TheForge_StencilOp depthBackFail;
	TheForge_StencilOp stencilBackPass;
} TheForge_DepthStateDesc;

typedef struct TheForge_RasterizerStateDesc {
	TheForge_CullMode cullMode;
	int32_t depthBias;
	float slopeScaledDepthBias;
	TheForge_FillMode fillMode;
	bool multiSample;
	bool scissor;
	TheForge_FrontFace frontFace;

} TheForge_RasterizerStateDesc;

typedef struct TheForge_LoadActionsDesc {
	TheForge_ClearValue clearColorValues[TheForge_MAX_RENDER_TARGET_ATTACHMENTS];
	TheForge_LoadActionType loadActionsColor[TheForge_MAX_RENDER_TARGET_ATTACHMENTS];
	TheForge_ClearValue clearDepth;
	TheForge_LoadActionType loadActionDepth;
	TheForge_LoadActionType loadActionStencil;
} TheForge_LoadActionsDesc;

typedef struct TheForge_DescriptorSetDesc {
	TheForge_RootSignatureHandle rootSignature;
	TheForge_DescriptorUpdateFrequency updateFrequency;
	uint32_t maxSets;
	uint32_t nodeIndex;
} TheForge_DescriptorSetDesc;

typedef struct TheForge_DescriptorData {
	const char *pName;
	union {
		struct {
			uint64_t const *pOffsets;
			uint64_t const *pSizes;
		};
		uint32_t UAVMipSlice;
		bool bindStencilResource;
	};
	union {
		TheForge_TextureHandle const *pTextures;
		TheForge_SamplerHandle const *pSamplers;
		TheForge_BufferHandle const *pBuffers;
		void const *pRootConstant;
		TheForge_AcclerationStructureHandle const *pAccelerationStructures;
	};
	uint32_t count;
	uint32_t index;
} TheForge_DescriptorData;

typedef struct TheForge_BufferBarrier {
	TheForge_BufferHandle buffer;
	TheForge_ResourceState newState;
	bool split;
} TheForge_BufferBarrier;

typedef struct TheForge_TextureBarrier {
	TheForge_TextureHandle texture;
	TheForge_ResourceState newState;
	bool split;
} TheForge_TextureBarrier;

typedef struct TheForge_BufferDesc {
	uint64_t mSize;
	TheForge_ResourceMemoryUsage mMemoryUsage;
	TheForge_BufferCreationFlags mFlags;
	TheForge_ResourceState mStartState;
	TheForge_IndexType mIndexType;
	uint32_t mVertexStride;
	uint64_t mFirstElement;
	uint64_t mElementCount;
	uint64_t mStructStride;
	TheForge_BufferHandle counterBuffer;
	TinyImageFormat mFormat;
	TheForge_DescriptorType mDescriptors;
	const char *pDebugName;
	uint32_t *pSharedNodeIndices;
	uint32_t mNodeIndex;
	uint32_t mSharedNodeIndexCount;
} TheForge_BufferDesc;

typedef struct TheForge_TextureDesc {
	TheForge_TextureCreationFlags mFlags;
	uint32_t mWidth;
	uint32_t mHeight;
	uint32_t mDepth;
	uint32_t mArraySize;
	uint32_t mMipLevels;
	TheForge_SampleCount mSampleCount;
	uint32_t mSampleQuality;
	TinyImageFormat mFormat;
	TheForge_ClearValue mClearValue;
	TheForge_ResourceState mStartState;
	TheForge_DescriptorType mDescriptors;
	const void *pNativeHandle;
	const char *pDebugName;
	uint32_t *pSharedNodeIndices;
	uint32_t mSharedNodeIndexCount;
	uint32_t mNodeIndex;
	bool mHostVisible;
} TheForge_TextureDesc;

typedef struct TheForge_QueryPoolDesc {
	TheForge_QueryType type;
	uint32_t queryCount;
	uint32_t nodeIndex;
} TheForge_QueryPoolDesc;

typedef struct TheForge_QueryDesc {
	uint32_t index;
} TheForge_QueryDesc;

typedef struct TheForge_IndirectDrawArguments {
	uint32_t mVertexCount;
	uint32_t mInstanceCount;
	uint32_t mStartVertex;
	uint32_t mStartInstance;
} TheForge_IndirectDrawArguments;

typedef struct TheForge_IndirectDrawIndexArguments {
	uint32_t mIndexCount;
	uint32_t mInstanceCount;
	uint32_t mStartIndex;
	uint32_t mVertexOffset;
	uint32_t mStartInstance;
} TheForge_IndirectDrawIndexArguments;

typedef struct TheForge_IndirectDispatchArguments {
	uint32_t mGroupCountX;
	uint32_t mGroupCountY;
	uint32_t mGroupCountZ;
} TheForge_IndirectDispatchArguments;

typedef struct TheForge_IndirectArgumentDescriptor {
	TheForge_IndirectArgumentType mType;
	uint32_t mRootParameterIndex;
	uint32_t mCount;
	uint32_t mDivisor;

} TheForge_IndirectArgumentDescriptor;

typedef struct TheForge_CommandSignatureDesc {
	TheForge_CmdPoolHandle cmdPool;
	TheForge_RootSignatureHandle rootSignature;
	uint32_t indirectArgCount;
	TheForge_IndirectArgumentDescriptor *pArgDescs;
} TheForge_CommandSignatureDesc;

typedef struct TheForge_ReadRange {
	uint64_t offset;
	uint64_t size;
} TheForge_ReadRange;

typedef struct TheForge_Region3D {
	uint32_t xOffset;
	uint32_t yOffset;
	uint32_t zOffset;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
} TheForge_Region3D;

typedef struct TheForge_Extent3D {
	uint32_t width;
	uint32_t height;
	uint32_t depth;
} TheForge_Extent3D;

typedef void *TheForge_IconHandle;

// for windows this should be HWND
// for OSX this shoulbe NSWindow*
typedef void *TheForge_WindowHandle;

typedef struct TheForge_RectDesc {
	int left;
	int top;
	int right;
	int bottom;
} TheForge_RectDesc;

// I don't like the defines in the structure but for now will leave until
// I tackle linux build
typedef struct TheForge_WindowsDesc {
#if defined(VK_USE_PLATFORM_XLIB_KHR)
	Display* display;
	Window   xlib_window;
	Atom     xlib_wm_delete_window;
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	Display*                 display;
	xcb_connection_t*        connection;
	xcb_screen_t*            screen;
	xcb_window_t             xcb_window;
	xcb_intern_atom_reply_t* atom_wm_delete_window;
#else
	TheForge_WindowHandle handle = NULL;    //hWnd
#endif
} TheForge_WindowsDesc;

typedef struct TheForge_SwapChainDesc {
	/// Window handle
	TheForge_WindowsDesc *pWindow;
	/// Queues which should be allowed to present
	TheForge_QueueHandle *pPresentQueues;
	/// Number of present queues
	uint32_t presentQueueCount;
	/// Number of backbuffers in this swapchain
	uint32_t imageCount;
	/// Width of the swapchain
	uint32_t width;
	/// Height of the swapchain
	uint32_t height;
	/// Sample count
	TheForge_SampleCount sampleCount;
	/// Sample quality (DirectX12 only)
	uint32_t sampleQuality;
	/// Color format of the swapchain
	TinyImageFormat colorFormat;
	/// Clear value
	TheForge_ClearValue colorClearValue;
	/// Set whether swap chain will be presented using vsync
	bool enableVsync;
} TheForge_SwapChainDesc;

#endif
