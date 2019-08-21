#pragma once

#include "tiny_imageformat/tinyimageformat_apis.h"

inline void utils_caps_builder(Renderer* pRenderer) {
	memset(pRenderer->canShaderReadFrom, 0, sizeof(pRenderer->canShaderReadFrom));
	memset(pRenderer->canShaderWriteTo, 0, sizeof(pRenderer->canShaderWriteTo));
	memset(pRenderer->canColorWriteTo, 0, sizeof(pRenderer->canColorWriteTo));

	for (uint32_t i = 0; i < TinyImageFormat_Count;++i) {
		VkFormatProperties formatSupport;
		vkGetPhysicalDeviceFormatProperties(pRenderer->pVkActiveGPU,
				(VkFormat) TinyImageFormat_ToVkFormat((TinyImageFormat)i), &formatSupport);
		pRenderer->canShaderReadFrom[i] =
				(formatSupport.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) != 0;
		pRenderer->canShaderWriteTo[i] =
				(formatSupport.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) != 0;
		pRenderer->canColorWriteTo[i] =
				(formatSupport.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) != 0;
	}

}
