#include "al2o3_platform/platform.h"
#include "al2o3_memory/memory.h"
#include "gfx_theforge/theforge.h"
#include "tiny_imageformat/formatcracker.h"

AL2O3_EXTERN_C TheForge_ImageFormat TinyImageFormatToTheForge_ImageFormat(TinyImageFormat fmt) {
	// TheForge uses a SRGB flag rather than format. so you will have to
	// manually set based on the original TinyImageFormat_IsSRGB(fmt)
	switch(fmt) {
	case TinyImageFormat_R8_SRGB:
	case TinyImageFormat_R8_UNORM: 								return TheForge_IF_R8;
	case TinyImageFormat_R8G8_SRGB:
	case TinyImageFormat_R8G8_UNORM: 							return TheForge_IF_RG8;
	case TinyImageFormat_R8G8B8_SRGB:
	case TinyImageFormat_R8G8B8_UNORM: 						return TheForge_IF_RGB8;
	case TinyImageFormat_R8G8B8A8_SRGB:
	case TinyImageFormat_R8G8B8A8_UNORM: 					return TheForge_IF_RGBA8;
	case TinyImageFormat_R16_UNORM: 					 		return TheForge_IF_R16;
	case TinyImageFormat_R16G16_UNORM: 				 		return TheForge_IF_RG16;
	case TinyImageFormat_R16G16B16_UNORM: 			 	return TheForge_IF_RGB16;
	case TinyImageFormat_R16G16B16A16_UNORM:		 	return TheForge_IF_RGBA16;
	case TinyImageFormat_R8_SNORM: 								return TheForge_IF_R8S;
	case TinyImageFormat_R8G8_SNORM: 							return TheForge_IF_RG8S;
	case TinyImageFormat_R8G8B8_SNORM: 						return TheForge_IF_RGB8S;
	case TinyImageFormat_R8G8B8A8_SNORM: 					return TheForge_IF_RGBA8S;
	case TinyImageFormat_R16_SNORM: 							return TheForge_IF_R16S;
	case TinyImageFormat_R16G16_SNORM: 						return TheForge_IF_RG16S;
	case TinyImageFormat_R16G16B16_SNORM: 				return TheForge_IF_RGB16S;
	case TinyImageFormat_R16G16B16A16_SNORM: 			return TheForge_IF_RGBA16S;
	case TinyImageFormat_R16_SFLOAT: 							return TheForge_IF_R16F;
	case TinyImageFormat_R16G16_SFLOAT: 					return TheForge_IF_RG16F;
	case TinyImageFormat_R16G16B16_SFLOAT: 				return TheForge_IF_RGB16F;
	case TinyImageFormat_R16G16B16A16_SFLOAT: 		return TheForge_IF_RGBA16F;
	case TinyImageFormat_R32_SFLOAT: 							return TheForge_IF_R32F;
	case TinyImageFormat_R32G32_SFLOAT: 					return TheForge_IF_RG32F;
	case TinyImageFormat_R32G32B32_SFLOAT: 				return TheForge_IF_RGB32F;
	case TinyImageFormat_R32G32B32A32_SFLOAT: 		return TheForge_IF_RGBA32F;
	case TinyImageFormat_R16_SINT: 								return TheForge_IF_R16I;
	case TinyImageFormat_R16G16_SINT: 						return TheForge_IF_RG16I;
	case TinyImageFormat_R16G16B16_SINT: 					return TheForge_IF_RGB16I;
	case TinyImageFormat_R16G16B16A16_SINT: 			return TheForge_IF_RGBA16I;
	case TinyImageFormat_R32_SINT: 								return TheForge_IF_R32I;
	case TinyImageFormat_R32G32_SINT: 						return TheForge_IF_RG32I;
	case TinyImageFormat_R32G32B32_SINT: 					return TheForge_IF_RGB32I;
	case TinyImageFormat_R32G32B32A32_SINT: 			return TheForge_IF_RGBA32I;
	case TinyImageFormat_R16_UINT: 								return TheForge_IF_R16UI;
	case TinyImageFormat_R16G16_UINT: 						return TheForge_IF_RG16UI;
	case TinyImageFormat_R16G16B16_UINT: 					return TheForge_IF_RGB16UI;
	case TinyImageFormat_R16G16B16A16_UINT: 			return TheForge_IF_RGBA16UI;
	case TinyImageFormat_R32_UINT: 								return TheForge_IF_R32UI;
	case TinyImageFormat_R32G32_UINT: 						return TheForge_IF_RG32UI;
	case TinyImageFormat_R32G32B32_UINT: 					return TheForge_IF_RGB32UI;
	case TinyImageFormat_R32G32B32A32_UINT: 			return TheForge_IF_RGBA32UI;
	case TinyImageFormat_E5B9G9R9_UFLOAT_PACK32: 	return TheForge_IF_RGB9E5;
	case TinyImageFormat_B10G11R11_UFLOAT_PACK32: return TheForge_IF_RG11B10F;
	case TinyImageFormat_R5G6B5_UNORM_PACK16: 		return TheForge_IF_RGB565;
	case TinyImageFormat_R4G4B4A4_UNORM_PACK16: 	return TheForge_IF_RGBA4;
	case TinyImageFormat_A2R10G10B10_UINT_PACK32: return TheForge_IF_RGB10A2;
	case TinyImageFormat_D16_UNORM: 							return TheForge_IF_D16;
	case TinyImageFormat_X8_D24_UNORM_PACK32: 		return TheForge_IF_D24;
	case TinyImageFormat_D24_UNORM_S8_UINT: 			return TheForge_IF_D24S8;
	case TinyImageFormat_D32_SFLOAT: 							return TheForge_IF_D32F;
	case TinyImageFormat_PVR_2BPP_SRGB_BLOCK:
	case TinyImageFormat_PVR_2BPP_UNORM_BLOCK: return TheForge_IF_PVR_2BPP;
	case TinyImageFormat_PVR_2BPPA_SRGB_BLOCK:
	case TinyImageFormat_PVR_2BPPA_UNORM_BLOCK: return TheForge_IF_PVR_2BPPA;
	case TinyImageFormat_PVR_4BPP_SRGB_BLOCK:
	case TinyImageFormat_PVR_4BPP_UNORM_BLOCK: return TheForge_IF_PVR_4BPP;
	case TinyImageFormat_PVR_4BPPA_SRGB_BLOCK:
	case TinyImageFormat_PVR_4BPPA_UNORM_BLOCK: return TheForge_IF_PVR_4BPPA;
	case TinyImageFormat_BC1_RGBA_UNORM_BLOCK:
	case TinyImageFormat_BC1_RGBA_SRGB_BLOCK:
	case TinyImageFormat_BC1_RGB_SRGB_BLOCK:
	case TinyImageFormat_BC1_RGB_UNORM_BLOCK: return TheForge_IF_GNF_BC1;
	case TinyImageFormat_BC2_SRGB_BLOCK:
	case TinyImageFormat_BC2_UNORM_BLOCK: return TheForge_IF_GNF_BC2;
	case TinyImageFormat_BC3_SRGB_BLOCK:
	case TinyImageFormat_BC3_UNORM_BLOCK: return TheForge_IF_GNF_BC3;
	case TinyImageFormat_BC4_UNORM_BLOCK: return TheForge_IF_GNF_BC4;
	case TinyImageFormat_BC5_UNORM_BLOCK: return TheForge_IF_GNF_BC5;
	case TinyImageFormat_BC6H_UFLOAT_BLOCK: return TheForge_IF_GNF_BC6HUF;
	case TinyImageFormat_BC6H_SFLOAT_BLOCK: return TheForge_IF_GNF_BC6HSF;
	case TinyImageFormat_BC7_SRGB_BLOCK:
	case TinyImageFormat_BC7_UNORM_BLOCK: return TheForge_IF_GNF_BC7;
	case TinyImageFormat_B8G8R8A8_SRGB:
	case TinyImageFormat_B8G8R8A8_UNORM: return TheForge_IF_BGRA8;
	case TinyImageFormat_S8_UINT: return TheForge_IF_S8;
	case TinyImageFormat_D16_UNORM_S8_UINT: return TheForge_IF_D16S8;
	case TinyImageFormat_D32_SFLOAT_S8_UINT: return TheForge_IF_D32S8;
	case TinyImageFormat_ASTC_4x4_UNORM_BLOCK: return TheForge_IF_ASTC_4x4;
	case TinyImageFormat_ASTC_5x4_UNORM_BLOCK: return TheForge_IF_ASTC_5x4;
	case TinyImageFormat_ASTC_5x5_UNORM_BLOCK: return TheForge_IF_ASTC_5x5;
	case TinyImageFormat_ASTC_6x5_UNORM_BLOCK: return TheForge_IF_ASTC_6x5;
	case TinyImageFormat_ASTC_6x6_UNORM_BLOCK: return TheForge_IF_ASTC_6x6;
	case TinyImageFormat_ASTC_8x5_UNORM_BLOCK: return TheForge_IF_ASTC_8x5;
	case TinyImageFormat_ASTC_8x6_UNORM_BLOCK: return TheForge_IF_ASTC_8x6;
	case TinyImageFormat_ASTC_8x8_UNORM_BLOCK: return TheForge_IF_ASTC_8x8;
	case TinyImageFormat_ASTC_10x5_UNORM_BLOCK: return TheForge_IF_ASTC_10x5;
	case TinyImageFormat_ASTC_10x6_UNORM_BLOCK: return TheForge_IF_ASTC_10x6;
	case TinyImageFormat_ASTC_10x8_UNORM_BLOCK: return TheForge_IF_ASTC_10x8;
	case TinyImageFormat_ASTC_10x10_UNORM_BLOCK: return TheForge_IF_ASTC_10x10;
	case TinyImageFormat_ASTC_12x10_UNORM_BLOCK: return TheForge_IF_ASTC_12x10;
	case TinyImageFormat_ASTC_12x12_UNORM_BLOCK: return TheForge_IF_ASTC_12x12;
	case TinyImageFormat_ASTC_4x4_SRGB_BLOCK: return TheForge_IF_ASTC_4x4;
	case TinyImageFormat_ASTC_5x4_SRGB_BLOCK: return TheForge_IF_ASTC_5x4;
	case TinyImageFormat_ASTC_5x5_SRGB_BLOCK: return TheForge_IF_ASTC_5x5;
	case TinyImageFormat_ASTC_6x5_SRGB_BLOCK: return TheForge_IF_ASTC_6x5;
	case TinyImageFormat_ASTC_6x6_SRGB_BLOCK: return TheForge_IF_ASTC_6x6;
	case TinyImageFormat_ASTC_8x5_SRGB_BLOCK: return TheForge_IF_ASTC_8x5;
	case TinyImageFormat_ASTC_8x6_SRGB_BLOCK: return TheForge_IF_ASTC_8x6;
	case TinyImageFormat_ASTC_8x8_SRGB_BLOCK: return TheForge_IF_ASTC_8x8;
	case TinyImageFormat_ASTC_10x5_SRGB_BLOCK: return TheForge_IF_ASTC_10x5;
	case TinyImageFormat_ASTC_10x6_SRGB_BLOCK: return TheForge_IF_ASTC_10x6;
	case TinyImageFormat_ASTC_10x8_SRGB_BLOCK: return TheForge_IF_ASTC_10x8;
	case TinyImageFormat_ASTC_10x10_SRGB_BLOCK: return TheForge_IF_ASTC_10x10;
	case TinyImageFormat_ASTC_12x10_SRGB_BLOCK: return TheForge_IF_ASTC_12x10;
	case TinyImageFormat_ASTC_12x12_SRGB_BLOCK: return TheForge_IF_ASTC_12x12;

	default:
		return TheForge_IF_NONE;
	}
}
