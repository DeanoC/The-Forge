#pragma once
#ifndef GFX_THEFORGE_SHADERREFLECTION_H_
#define GFX_THEFORGE_SHADERREFLECTION_H_


typedef enum TheForge_TextureDimension
{
	TheForge_TEXTURE_DIM_UNDEFINED = 0,
	TheForge_TEXTURE_DIM_1D,
	TheForge_TEXTURE_DIM_2D,
	TheForge_TEXTURE_DIM_2DMS,
	TheForge_TEXTURE_DIM_3D,
	TheForge_TEXTURE_DIM_CUBE,
	TheForge_TEXTURE_DIM_1D_ARRAY,
	TheForge_TEXTURE_DIM_2D_ARRAY,
	TheForge_TEXTURE_DIM_2DMS_ARRAY,
	TheForge_TEXTURE_DIM_CUBE_ARRAY,
	TheForge_TEXTURE_DIM_COUNT
} TheForge_TextureDimension;

typedef struct TheForge_VertexInput
{
	// The size of the attribute
	uint32_t size;

	// resource name
	const char* name;

	// name size
	uint32_t name_size;
} TheForge_VertexInput;

typedef struct TheForge_ShaderResource
{
	// resource Type
	TheForge_DescriptorType type;

	// The resource set for binding frequency
	uint32_t set;

	// The resource binding location
	uint32_t reg;

	// The size of the resource. This will be the DescriptorInfo array size for textures
	uint32_t size;

	// what stages use this resource
	TheForge_ShaderStage used_stages;

	// resource name
	const char* name;

	// name size
	uint32_t name_size;

	// 1D / 2D / Array / MSAA / ...
	TheForge_TextureDimension dim;

	union {
		struct {
			uint32_t mtlTextureType;           // Needed to bind different types of textures as default resources on Metal.
			uint32_t mtlArgumentBufferType;    // Needed to bind multiple resources under a same descriptor on Metal.
		};
		uint32_t dx11Constant_size; 					 // dx11
	};

} TheForge_ShaderResource;

typedef struct TheForge_ShaderVariable
{
	// parents resource index
	uint32_t parent_index;

	// The offset of the Variable.
	uint32_t offset;

	// The size of the Variable.
	uint32_t size;

	// Variable name
	const char* name;

	// name size
	uint32_t name_size;
} TheForge_ShaderVariable;

typedef struct TheForge_ShaderReflection
{
	TheForge_ShaderStage mShaderStage;

	// single large allocation for names to reduce number of allocations
	char*    pNamePool;
	uint32_t mNamePoolSize;

	TheForge_VertexInput* pVertexInputs;
	uint32_t     mVertexInputsCount;

	TheForge_ShaderResource* pShaderResources;
	uint32_t        mShaderResourceCount;

	TheForge_ShaderVariable* pVariables;
	uint32_t        mVariableCount;

	// Thread group size for compute shader
	uint32_t mNumThreadsPerGroup[3];

	//number of tessellation control point
	uint32_t mNumControlPoint;

} TheForge_ShaderReflection;

typedef struct TheForge_PipelineReflection
{
	TheForge_ShaderStage mShaderStages;
	// the individual stages reflection data.
	TheForge_ShaderReflection mStageReflections[TheForge_MAX_SHADER_STAGE_COUNT];
	uint32_t mStageReflectionCount;

	uint32_t mVertexStageIndex;
	uint32_t mHullStageIndex;
	uint32_t mDomainStageIndex;
	uint32_t mGeometryStageIndex;
	uint32_t mPixelStageIndex;

	TheForge_ShaderResource* pShaderResources;
	uint32_t mShaderResourceCount;

	TheForge_ShaderVariable* pVariables;
	uint32_t mVariableCount;
} TheForge_PipelineReflection;

#endif
