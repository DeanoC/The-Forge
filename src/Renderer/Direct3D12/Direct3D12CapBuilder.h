#pragma once

inline void utils_caps_builder(Renderer* pRenderer) {
	memset(pRenderer->canShaderReadFrom, 0, sizeof(pRenderer->canShaderReadFrom));
	memset(pRenderer->canShaderWriteTo, 0, sizeof(pRenderer->canShaderWriteTo));
	memset(pRenderer->canColorWriteTo, 0, sizeof(pRenderer->canColorWriteTo));

	D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport;
#define	IF_START_MACRO
#define	IF_MOD_MACRO(x) \
	formatSupport.Format = (DXGI_FORMAT) TinyImageFormat_ToDXGI_FORMAT(TinyImageFormat_##x); \
	pRenderer->pDxDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)); \
	pRenderer->canShaderReadFrom[TinyImageFormat_##x] = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) ? true : false; \
	pRenderer->canShaderWriteTo[TinyImageFormat_##x] = (formatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE) ? true : false; \
	pRenderer->canColorWriteTo[TinyImageFormat_##x] = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET) ? true : false;
#define IF_END_MACRO
#include "tiny_imageformat/formatgen.h"

}
