#pragma once

#include "tiny_imageformat/tinyimageformat_base.h"
#include "tiny_imageformat/tinyimageformat_apis.h"

inline void utils_caps_builder(Renderer* pRenderer) {
	memset(pRenderer->capBits.canShaderReadFrom, 0, sizeof(pRenderer->capBits.canShaderReadFrom));
	memset(pRenderer->capBits.canShaderWriteTo, 0, sizeof(pRenderer->capBits.canShaderWriteTo));
	memset(pRenderer->capBits.canColorWriteTo, 0, sizeof(pRenderer->capBits.canColorWriteTo));

	for (uint32_t i = 0; i < TinyImageFormat_Count;++i) {
		D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport;
		DXGI_FORMAT fmt = (DXGI_FORMAT) TinyImageFormat_ToDXGI_FORMAT((TinyImageFormat)i);
		if(fmt == DXGI_FORMAT_UNKNOWN) continue;

		pRenderer->pDxDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport));
		pRenderer->capBits.canShaderReadFrom[i] = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) != 0;
		pRenderer->capBits.canShaderWriteTo[i] = (formatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE) != 0;
		pRenderer->capBits.canColorWriteTo[i] = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET)  != 0;
	}

}
