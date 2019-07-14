#pragma once
#ifndef GFX_THEFORGE_ENUMS_H_
#define GFX_THEFORGE_ENUMS_H_

typedef enum TheForge_ImageFormat
{
	TheForge_IF_NONE = 0,
	TheForge_IF_R8 = 1,
	TheForge_IF_RG8 = 2,
	TheForge_IF_RGB8 = 3,
	TheForge_IF_RGBA8 = 4,
	TheForge_IF_R16 = 5,
	TheForge_IF_RG16 = 6,
	TheForge_IF_RGB16 = 7,
	TheForge_IF_RGBA16 = 8,
	TheForge_IF_R8S = 9,
	TheForge_IF_RG8S = 10,
	TheForge_IF_RGB8S = 11,
	TheForge_IF_RGBA8S = 12,
	TheForge_IF_R16S = 13,
	TheForge_IF_RG16S = 14,
	TheForge_IF_RGB16S = 15,
	TheForge_IF_RGBA16S = 16,
	TheForge_IF_R16F = 17,
	TheForge_IF_RG16F = 18,
	TheForge_IF_RGB16F = 19,
	TheForge_IF_RGBA16F = 20,
	TheForge_IF_R32F = 21,
	TheForge_IF_RG32F = 22,
	TheForge_IF_RGB32F = 23,
	TheForge_IF_RGBA32F = 24,
	TheForge_IF_R16I = 25,
	TheForge_IF_RG16I = 26,
	TheForge_IF_RGB16I = 27,
	TheForge_IF_RGBA16I = 28,
	TheForge_IF_R32I = 29,
	TheForge_IF_RG32I = 30,
	TheForge_IF_RGB32I = 31,
	TheForge_IF_RGBA32I = 32,
	TheForge_IF_R16UI = 33,
	TheForge_IF_RG16UI = 34,
	TheForge_IF_RGB16UI = 35,
	TheForge_IF_RGBA16UI = 36,
	TheForge_IF_R32UI = 37,
	TheForge_IF_RG32UI = 38,
	TheForge_IF_RGB32UI = 39,
	TheForge_IF_RGBA32UI = 40,
	TheForge_IF_RGBE8 = 41,
	TheForge_IF_RGB9E5 = 42,
	TheForge_IF_RG11B10F = 43,
	TheForge_IF_RGB565 = 44,
	TheForge_IF_RGBA4 = 45,
	TheForge_IF_RGB10A2 = 46,
	TheForge_IF_D16 = 47,
	TheForge_IF_D24 = 48,
	TheForge_IF_D24S8 = 49,
	TheForge_IF_D32F = 50,
	TheForge_IF_DXT1 = 51,
	TheForge_IF_DXT3 = 52,
	TheForge_IF_DXT5 = 53,
	TheForge_IF_ATI1N = 54,
	TheForge_IF_ATI2N = 55,
	TheForge_IF_PVR_2BPP = 56,
	TheForge_IF_PVR_2BPPA = 57,
	TheForge_IF_PVR_4BPP = 58,
	TheForge_IF_PVR_4BPPA = 59,
	TheForge_IF_INTZ = 60,    //  NVidia hack. Supported on all DX10+ HW
	TheForge_IF_LE_XRGB8 = 61,
	TheForge_IF_LE_ARGB8 = 62,
	TheForge_IF_LE_X2RGB10 = 63,
	TheForge_IF_LE_A2RGB10 = 64,
	TheForge_IF_ETC1 = 65,    //  RGB
	TheForge_IF_ATC = 66,     //  RGB
	TheForge_IF_ATCA = 67,    //  RGBA, explicit alpha
	TheForge_IF_ATCI = 68,    //  RGBA, interpolated alpha
	TheForge_IF_RAWZ = 69,           //depth only, Nvidia (requires recombination of data) //FIX IT: PS3 as well?
	TheForge_IF_DF16 = 70,           //depth only, Intel/AMD
	TheForge_IF_STENCILONLY = 71,    // stencil ony usage
	TheForge_IF_GNF_BC1 = 72,
	TheForge_IF_GNF_BC2 = 73,
	TheForge_IF_GNF_BC3 = 74,
	TheForge_IF_GNF_BC4 = 75,
	TheForge_IF_GNF_BC5 = 76,
	TheForge_IF_GNF_BC6HUF = 77,
	TheForge_IF_GNF_BC6HSF = 78,
	TheForge_IF_GNF_BC7 = 79,
	TheForge_IF_BGRA8 = 80,
	TheForge_IF_X8D24PAX32 = 81,
	TheForge_IF_S8 = 82,
	TheForge_IF_D16S8 = 83,
	TheForge_IF_D32S8 = 84,
	TheForge_IF_ASTC_4x4 = 85,
	TheForge_IF_ASTC_5x4 = 86,
	TheForge_IF_ASTC_5x5 = 87,
	TheForge_IF_ASTC_6x5 = 88,
	TheForge_IF_ASTC_6x6 = 89,
	TheForge_IF_ASTC_8x5 = 90,
	TheForge_IF_ASTC_8x6 = 91,
	TheForge_IF_ASTC_8x8 = 92,
	TheForge_IF_ASTC_10x5 = 93,
	TheForge_IF_ASTC_10x6 = 94,
	TheForge_IF_ASTC_10x8 = 95,
	TheForge_IF_ASTC_10x10 = 96,
	TheForge_IF_ASTC_12x10 = 97,
	TheForge_IF_ASTC_12x12 = 98,
	TheForge_IF_COUNT,
	TheForge_IF_I8 = TheForge_IF_R8,
	TheForge_IF_IA8 = TheForge_IF_RG8,
	TheForge_IF_I16 = TheForge_IF_R16,
	TheForge_IF_IA16 = TheForge_IF_RG16,
	TheForge_IF_I16F = TheForge_IF_R16F,
	TheForge_IF_IA16F = TheForge_IF_RG16F,
	TheForge_IF_I32F = TheForge_IF_R32F,
	TheForge_IF_IA32F = TheForge_IF_RG32F
} TheForge_ImageFormat;

typedef enum TheForge_MaxEnums
{
	TheForge_MAX_INSTANCE_EXTENSIONS = 64,
	TheForge_MAX_DEVICE_EXTENSIONS = 64,
	TheForge_MAX_GPUS = 10,
	TheForge_MAX_RENDER_TARGET_ATTACHMENTS = 8,
	TheForge_MAX_SUBMIT_CMDS = 20,    // max number of command lists / command buffers
	TheForge_MAX_SUBMIT_WAIT_SEMAPHORES = 8,
	TheForge_MAX_SUBMIT_SIGNAL_SEMAPHORES = 8,
	TheForge_MAX_PRESENT_WAIT_SEMAPHORES = 8,
	TheForge_MAX_VERTEX_BINDINGS = 15,
	TheForge_MAX_VERTEX_ATTRIBS = 15,
	TheForge_MAX_SEMANTIC_NAME_LENGTH = 128,
	TheForge_MAX_MIP_LEVELS = 0xFFFFFFFF,
	TheForge_MAX_BATCH_BARRIERS = 64,
	TheForge_MAX_GPU_VENDOR_STRING_LENGTH = 64    //max size for GPUVendorPreset strings
} TheForge_MaxEnums;

typedef enum TheForge_ShaderTarget
{
	TheForge_ST_5_1,
	TheForge_ST_6_0,
	TheForge_ST_6_1,
	TheForge_ST_6_2,
	TheForge_ST_6_3, //required for Raytracing
} TheForge_ShaderTarget;

typedef enum TheForge_GpuMode
{
	TheForge_GM_SINGLE = 0,
	TheForge_GM_LINKED,
	// #TODO GPU_MODE_UNLINKED,
} TheForge_GpuMode;


typedef enum TheForge_D3DFeatureLevel {
	TheForge_D3D_FL_9_1,
	TheForge_D3D_FL_1_0_CORE,
	TheForge_D3D_FL_9_2,
	TheForge_D3D_FL_9_3,
	TheForge_D3D_FL_10_0,
	TheForge_D3D_FL_10_1,
	TheForge_D3D_FL_11_0,
	TheForge_D3D_FL_11_1,
	TheForge_D3D_FL_12_0,
	TheForge_D3D_FL_12_1
} TheForge_D3DFeatureLevel;


typedef enum TheForge_QueueFlag
{
	TheForge_QF_NONE = 0,
	TheForge_QF_DISABLE_GPU_TIMEOUT = 0x1,
	TheForge_QF_INIT_MICROPROFILE		= 0x2,
} TheForge_QueueFlag;

typedef enum TheForge_QueuePriority
{
	TheForge_QP_NORMAL,
	TheForge_QP_HIGH,
	TheForge_QP_GLOBAL_REALTIME,
} TheForge_QueuePriority;

typedef enum TheForge_CmdPoolType
{
	TheForge_CP_DIRECT,
	TheForge_CP_BUNDLE,
	TheForge_CP_COPY,
	TheForge_CP_COMPUTE,
} TheForge_CmdPoolType;

typedef enum TheForge_BufferCreationFlags
{
	TheForge_BCF_NONE = 0x01,
	TheForge_BCF_OWN_MEMORY_BIT = 0x02,
	TheForge_BCF_PERSISTENT_MAP_BIT = 0x04,
	TheForge_BCF_ESRAM = 0x08,
	TheForge_BCF_NO_DESCRIPTOR_VIEW_CREATION = 0x10,
} TheForge_BufferCreationFlags;

typedef enum TheForge_TextureCreationFlags
{
	TheForge_TCF_NONE = 0,
	TheForge_TCF_OWN_MEMORY_BIT = 0x01,
	TheForge_TCF_EXPORT_BIT = 0x02,
	TheForge_TCF_EXPORT_ADAPTER_BIT = 0x04,
	TheForge_TCF_IMPORT_BIT = 0x08,
	TheForge_TCF_ESRAM = 0x10,
	TheForge_TCF_ON_TILE = 0x20,
	TheForge_TCF_NO_COMPRESSION = 0x40,
} TheForge_TextureCreationFlags;

typedef enum TheForge_SampleCount
{
	TheForge_SC_1 = 1,
	TheForge_SC_2 = 2,
	TheForge_SC_4 = 4,
	TheForge_SC_8 = 8,
	TheForge_SC_16 = 16,
} TheForge_SampleCount;

typedef enum TheForge_DescriptorType
{
	TheForge_DESCRIPTOR_TYPE_UNDEFINED = 0,
	TheForge_DESCRIPTOR_TYPE_SAMPLER = 0x01,
	TheForge_DESCRIPTOR_TYPE_TEXTURE = (TheForge_DESCRIPTOR_TYPE_SAMPLER * 2),
	TheForge_DESCRIPTOR_TYPE_RW_TEXTURE = (TheForge_DESCRIPTOR_TYPE_TEXTURE * 2),
	TheForge_DESCRIPTOR_TYPE_BUFFER = (TheForge_DESCRIPTOR_TYPE_RW_TEXTURE * 2),
	TheForge_DESCRIPTOR_TYPE_BUFFER_RAW = (TheForge_DESCRIPTOR_TYPE_BUFFER | (TheForge_DESCRIPTOR_TYPE_BUFFER * 2)),
	TheForge_DESCRIPTOR_TYPE_RW_BUFFER = (TheForge_DESCRIPTOR_TYPE_BUFFER * 4),
	TheForge_DESCRIPTOR_TYPE_RW_BUFFER_RAW = (TheForge_DESCRIPTOR_TYPE_RW_BUFFER | (TheForge_DESCRIPTOR_TYPE_RW_BUFFER * 2)),
	TheForge_DESCRIPTOR_TYPE_UNIFORM_BUFFER = (TheForge_DESCRIPTOR_TYPE_RW_BUFFER * 4),
	TheForge_DESCRIPTOR_TYPE_VERTEX_BUFFER = (TheForge_DESCRIPTOR_TYPE_UNIFORM_BUFFER * 2),
	TheForge_DESCRIPTOR_TYPE_INDEX_BUFFER = (TheForge_DESCRIPTOR_TYPE_VERTEX_BUFFER * 2),
	TheForge_DESCRIPTOR_TYPE_INDIRECT_BUFFER = (TheForge_DESCRIPTOR_TYPE_INDEX_BUFFER * 2),
	TheForge_DESCRIPTOR_TYPE_ROOT_CONSTANT = (TheForge_DESCRIPTOR_TYPE_INDIRECT_BUFFER * 2),
	TheForge_DESCRIPTOR_TYPE_TEXTURE_CUBE = (TheForge_DESCRIPTOR_TYPE_TEXTURE | (TheForge_DESCRIPTOR_TYPE_ROOT_CONSTANT * 2)),
	TheForge_DESCRIPTOR_TYPE_RENDER_TARGET_ARRAY_SLICES = (TheForge_DESCRIPTOR_TYPE_ROOT_CONSTANT * 4),
	TheForge_DESCRIPTOR_TYPE_RENDER_TARGET_DEPTH_SLICES = (TheForge_DESCRIPTOR_TYPE_RENDER_TARGET_ARRAY_SLICES * 2),
	TheForge_DESCRIPTOR_TYPE_RAY_TRACING = (TheForge_DESCRIPTOR_TYPE_RENDER_TARGET_DEPTH_SLICES * 2),

	/// Subpass input (descriptor type only available in Vulkan)
	TheForge_DESCRIPTOR_TYPE_INPUT_ATTACHMENT = (TheForge_DESCRIPTOR_TYPE_RAY_TRACING * 2),
	TheForge_DESCRIPTOR_TYPE_TEXEL_BUFFER = (TheForge_DESCRIPTOR_TYPE_INPUT_ATTACHMENT * 2),
	TheForge_DESCRIPTOR_TYPE_RW_TEXEL_BUFFER = (TheForge_DESCRIPTOR_TYPE_TEXEL_BUFFER * 2),
} TheForge_DescriptorType;

typedef enum TheForge_BlendConstant
{
	TheForge_BC_ZERO = 0,
	TheForge_BC_ONE,
	TheForge_BC_SRC_COLOR,
	TheForge_BC_ONE_MINUS_SRC_COLOR,
	TheForge_BC_DST_COLOR,
	TheForge_BC_ONE_MINUS_DST_COLOR,
	TheForge_BC_SRC_ALPHA,
	TheForge_BC_ONE_MINUS_SRC_ALPHA,
	TheForge_BC_DST_ALPHA,
	TheForge_BC_ONE_MINUS_DST_ALPHA,
	TheForge_BC_SRC_ALPHA_SATURATE,
	TheForge_BC_BLEND_FACTOR,
	TheForge_BC_INV_BLEND_FACTOR,
} TheForge_BlendConstant;

typedef enum TheForge_BlendMode
{
	TheForge_BM_ADD,
	TheForge_BM_SUBTRACT,
	TheForge_BM_REVERSE_SUBTRACT,
	TheForge_BM_MIN,
	TheForge_BM_MAX,
} TheForge_BlendMode;

typedef enum TheForge_CompareMode
{
	TheForge_CMP_NEVER,
	TheForge_CMP_LESS,
	TheForge_CMP_EQUAL,
	TheForge_CMP_LEQUAL,
	TheForge_CMP_GREATER,
	TheForge_CMP_NOTEQUAL,
	TheForge_CMP_GEQUAL,
	TheForge_CMP_ALWAYS,
} TheForge_CompareMode;

typedef enum TheForge_StencilOp
{
	TheForge_SOP_KEEP,
	TheForge_SOP_SET_ZERO,
	TheForge_SOP_REPLACE,
	TheForge_SOP_INVERT,
	TheForge_SOP_INCR,
	TheForge_SOP_DECR,
	TheForge_SOP_INCR_SAT,
	TheForge_SOP_DECR_SAT,
} TheForge_StencilOp;

typedef enum TheForge_ShaderStage
{
	TheForge_SS_NONE = 0,
	TheForge_SS_VERT = 0X00000001,
	TheForge_SS_TESC = 0X00000002,
	TheForge_SS_TESE = 0X00000004,
	TheForge_SS_GEOM = 0X00000008,
	TheForge_SS_FRAG = 0X00000010,
	TheForge_SS_COMP = 0X00000020,
	TheForge_SS_RAYTRACING  = 0X00000040,
	TheForge_SS_COUNT = 7,
} TheForge_ShaderStage;

typedef enum TheForge_BlendStateTargets
{
	TheForge_BST_0 = 0x1,
	TheForge_BST_1 = 0x2,
	TheForge_BST_2 = 0x4,
	TheForge_BST_3 = 0x8,
	TheForge_BST_4 = 0x10,
	TheForge_BST_5 = 0x20,
	TheForge_BST_6 = 0x40,
	TheForge_BST_7 = 0x80,
	TheForge_BST_ALL = 0xFF,
} TheForge_BlendStateTargets;

typedef enum TheForge_CullMode
{
	TheForge_CM_NONE = 0,
	TheForge_CM_BACK,
	TheForge_CM_FRONT,
	TheForge_CM_BOTH,
} TheForge_CullMode;

typedef enum TheForge_FrontFace
{
	TheForge_FF_CCW = 0,
	TheForge_FF_CW
} TheForge_FrontFace;

typedef enum TheForge_FillMode
{
	TheForge_FM_SOLID,
	TheForge_FM_WIREFRAME,
} TheForge_FillMode;

typedef enum TheForge_PipelineType
{
	TheForge_PT_UNDEFINED = 0,
	TheForge_PT_COMPUTE,
	TheForge_PT_GRAPHICS,
	TheForge_PT_RAYTRACING,
} TheForge_PipelineType;

typedef enum TheForge_FilterType
{
	TheForge_FT_NEAREST = 0,
	TheForge_FT_LINEAR,
} TheForge_FilterType;

typedef enum TheForge_AddressMode
{
	TheForge_AM_MIRROR,
	TheForge_AM_REPEAT,
	TheForge_AM_CLAMP_TO_EDGE,
	TheForge_AM_CLAMP_TO_BORDER
} TheForge_AddressMode;

typedef enum TheForge_MipMapMode
{
	TheForge_MM_NEAREST = 0,
	TheForge_MM_LINEAR
} TheForge_MipMapMode;

typedef enum TheForge_DepthStencilClearFlags
{
	TheForge_DSCF_DEPTH = 0x01,
	TheForge_DSCF_STENCIL = 0x02
} TheForge_DepthStencilClearFlags;

typedef enum TheForge_RootSignatureFlags
{
	TheForge_RSF_NONE = 0,
	TheForge_RSF_LOCAL_BIT = 0x1,
} TheForge_RootSignatureFlags;

typedef enum TheForge_VertexAttribRate
{
	TheForge_VAR_VERTEX = 0,
	TheForge_VAR_INSTANCE = 1,
} TheForge_VertexAttribRate;

typedef enum TheForge_ShaderSemantic
{
	TheForge_SS_UNDEFINED = 0,
	TheForge_SS_POSITION,
	TheForge_SS_NORMAL,
	TheForge_SS_COLOR,
	TheForge_SS_TANGENT,
	TheForge_SS_BITANGENT,
	TheForge_SS_TEXCOORD0,
	TheForge_SS_TEXCOORD1,
	TheForge_SS_TEXCOORD2,
	TheForge_SS_TEXCOORD3,
	TheForge_SS_TEXCOORD4,
	TheForge_SS_TEXCOORD5,
	TheForge_SS_TEXCOORD6,
	TheForge_SS_TEXCOORD7,
	TheForge_SS_TEXCOORD8,
	TheForge_SS_TEXCOORD9,
} TheForge_ShaderSemantic;

typedef enum TheForge_IndexType
{
	TheForge_IT_UINT32 = 0,
	TheForge_IT_UINT16,
} TheForge_IndexType;

typedef enum TheForge_PrimitiveTopology
{
	TheForge_PT_POINT_LIST = 0,
	TheForge_PT_LINE_LIST,
	TheForge_PT_LINE_STRIP,
	TheForge_PT_TRI_LIST,
	TheForge_PT_TRI_STRIP,
	TheForge_PT_PATCH_LIST,
} TheForge_PrimitiveTopology;

typedef enum TheForge_LoadActionType
{
	TheForge_LA_DONTCARE,
	TheForge_LA_LOAD,
	TheForge_LA_CLEAR,
} TheForge_LoadActionType;

typedef enum TheForge_ResourceState
{
	TheForge_RS_UNDEFINED = 0,
	TheForge_RS_VERTEX_AND_CONSTANT_BUFFER = 0x1,
	TheForge_RS_INDEX_BUFFER = 0x2,
	TheForge_RS_RENDER_TARGET = 0x4,
	TheForge_RS_UNORDERED_ACCESS = 0x8,
	TheForge_RS_DEPTH_WRITE = 0x10,
	TheForge_RS_DEPTH_READ = 0x20,
	TheForge_RS_NON_PIXEL_SHADER_RESOURCE = 0x40,
	TheForge_RS_SHADER_RESOURCE = 0x40 | 0x80,
	TheForge_RS_STREAM_OUT = 0x100,
	TheForge_RS_INDIRECT_ARGUMENT = 0x200,
	TheForge_RS_COPY_DEST = 0x400,
	TheForge_RS_COPY_SOURCE = 0x800,
	TheForge_RS_GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
	TheForge_RS_PRESENT = 0x4000,
	TheForge_RS_COMMON = 0x8000,
} TheForge_TheForge_ResourceState;

typedef enum TheForge_ResourceMemoryUsage
{
	TheForge_RMU_UNKNOWN = 0,
	TheForge_RMU_GPU_ONLY = 1,
	TheForge_RMU_CPU_ONLY = 2,
	TheForge_RMU_CPU_TO_GPU = 3,
	TheForge_RMU_GPU_TO_CPU = 4,
} TheForge_ResourceMemoryUsage;

typedef enum TheForge_FenceStatus
{
	TheForge_FS_COMPLETE = 0,
	TheForge_FS_INCOMPLETE,
	TheForge_FS_NOTSUBMITTED,
} TheForge_FenceStatus;

typedef enum TheForge_IndirectArgumentType
{
	TheForge_IAT_DRAW,
	TheForge_IAT_DRAW_INDEX,
	TheForge_IAT_DISPATCH,
	TheForge_IAT_VERTEX_BUFFER,
	TheForge_IAT_INDEX_BUFFER,
	TheForge_IAT_CONSTANT,
	TheForge_IAT_DESCRIPTOR_TABLE,        // only for vulkan
	TheForge_IAT_PIPELINE,                // only for vulkan now, probally will add to dx when it comes to xbox
	TheForge_IAT_CONSTANT_BUFFER_VIEW,    // only for dx
	TheForge_IAT_SHADER_RESOURCE_VIEW,    // only for dx
	TheForge_IAT_UNORDERED_ACCESS_VIEW    // only for dx
} TheForge_IndirectArgumentType;

typedef enum TheForge_QueryType
{
	TheForge_QT_TIMESTAMP = 0,
	TheForge_QT_PIPELINE_STATISTICS,
	TheForge_QT_OCCLUSION,
	TheForge_QT_COUNT,
} TheForge_QueryType;

typedef enum TheForge_ResourceFolders
{
	TheForge_FSR_BinShaders = 0,
	TheForge_FSR_SrcShaders,
	TheForge_FSR_Textures,
	TheForge_FSR_Meshes,
	TheForge_FSR_Builtin_Fonts,
	TheForge_FSR_GpuConfig,
	TheForge_FSR_Animation,    // NOT USED
	TheForge_FSR_Audio,				// NOT USED
	TheForge_FSR_OtherFiles,
} TheForge_ResourceFolders;

typedef enum TheForge_ResourceType
{
	TheForge_RESOURCE_TYPE_BUFFER = 0,
	TheForge_RESOURCE_TYPE_TEXTURE,
} TheForge_ResourceType;

#endif