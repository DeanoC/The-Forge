#pragma once

inline void utils_caps_builder(Renderer* pRenderer) {
	memset(pRenderer->canShaderReadFrom, 0, sizeof(pRenderer->canShaderReadFrom));
	memset(pRenderer->canShaderWriteTo, 0, sizeof(pRenderer->canShaderWriteTo));
	memset(pRenderer->canColorWriteTo, 0, sizeof(pRenderer->canColorWriteTo));

	VkFormatProperties formatSupport;
#define	IF_START_MACRO
#define	IF_MOD_MACRO(x) \
	vkGetPhysicalDeviceFormatProperties(pRenderer->pVkActiveGPU, (VkFormat) TinyImageFormat_ToVkFormat(TinyImageFormat_##x), &formatSupport); \
	pRenderer->canShaderReadFrom[TinyImageFormat_##x] = (formatSupport.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) ? true : false; \
	pRenderer->canShaderWriteTo[TinyImageFormat_##x] = (formatSupport.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) ? true : false; \
	pRenderer->canColorWriteTo[TinyImageFormat_##x] = (formatSupport.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) ? true : false;
#define IF_END_MACRO
#include "tiny_imageformat/format.h"

}
