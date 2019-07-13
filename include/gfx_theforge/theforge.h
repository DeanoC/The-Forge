#pragma once
#ifndef GFX_THEFORGE_THEFORGE_HPP_
#define GFX_THEFORGE_THEFORGE_HPP_

#include "al2o3_platform/platform.h"

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

typedef struct TheForge_RendererDesc {
	TheForge_ShaderTarget shaderTarget;
	TheForge_GpuMode      gpuMode;
} TheForge_RendererDesc;

typedef struct TheForge_Renderer *TheForge_RendererHandle;

AL2O3_EXTERN_C TheForge_RendererHandle TheForge_RendererCreate(
		char const * appName,
		TheForge_RendererDesc const * settings);

AL2O3_EXTERN_C void TheForge_RendererDestroy(TheForge_RendererHandle pRenderer);

#endif // end