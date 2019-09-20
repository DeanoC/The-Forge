#include "al2o3_platform/platform.h"
#include "gfx_theforge/theforge.h"
#include "Renderer/IRenderer.h"
#include "Renderer/ResourceLoader.h"
#include "Renderer/../ThirdParty/OpenSource/tinyimageformat/tinyimageformat_base.h"

static void API_CHECK();

// these are hidden but we want them
extern void addBuffer(Renderer* pRenderer, const BufferDesc* desc, Buffer** pp_buffer);
extern void removeBuffer(Renderer* pRenderer, Buffer* p_buffer);
extern void addTexture(Renderer* pRenderer, const TextureDesc* pDesc, Texture** pp_texture);
extern void removeTexture(Renderer* pRenderer, Texture* p_texture);

extern void mapBuffer(Renderer* pRenderer, Buffer* pBuffer, ReadRange* pRange);
extern void unmapBuffer(Renderer* pRenderer, Buffer* pBuffer);
extern void cmdUpdateBuffer(Cmd* pCmd, Buffer* pBuffer, uint64_t dstOffset, Buffer* pSrcBuffer, uint64_t srcOffset, uint64_t size);
extern void cmdUpdateSubresource(Cmd* pCmd, Texture* pTexture, Buffer* pSrcBuffer, SubresourceDataDesc* pSubresourceDesc);
extern const RendererShaderDefinesDesc get_renderer_shaderdefines(Renderer* pRenderer);

#ifdef METAL
// account for app directory
// I don't like this at all but for now just grin an bare it!
const char *pszBases[FSR_Count] = {
		"../binshaders/",                                // FSR_BinShaders
		"../srcshaders/",                                // FSR_SrcShaders
		"../textures/",                                  // FSR_Textures
		"../meshes/",                                    // FSR_Meshes
		"../fonts/",                                      // FSR_Builtin_Fonts
		"../gpuconfigs/",                                // FSR_GpuConfig
		"../anims/",                                     // FSR_Animation
		"../audio",                                      // FSR_Audio
		"../misc/",                                      // FSR_OtherFiles
		"",          // FSR_MIDDLEWARE_TEXT
		"",          // FSR_MIDDLEWARE_UI
};
#else
// I don't like this at all but for now just grin an bare it!
const char *pszBases[FSR_Count] = {
		"binshaders/",                                // FSR_BinShaders
		"srcshaders/",                                // FSR_SrcShaders
		"textures/",                                  // FSR_Textures
		"meshes/",                                    // FSR_Meshes
		"fonts/",                                      // FSR_Builtin_Fonts
		"gpuconfigs/",                                // FSR_GpuConfig
		"anims/",                                     // FSR_Animation
		"audio",                                      // FSR_Audio
		"misc/",                                      // FSR_OtherFiles
		"",          // FSR_MIDDLEWARE_TEXT
		"",          // FSR_MIDDLEWARE_UI
};

#endif
static void LogFunc(LogType type, const char *m0, const char *m1) {
	switch (type) {
		case LogType::LOG_TYPE_INFO: LOGINFO("%s %s", m0, m1);
		case LogType::LOG_TYPE_DEBUG: LOGDEBUG("%s %s", m0, m1);
		case LogType::LOG_TYPE_WARN: LOGWARNING("%s %s", m0, m1);
		case LogType::LOG_TYPE_ERROR: LOGERROR("%s %s", m0, m1);
	}
}

static ShaderStage TheForge_ShaderStageToShaderStage(TheForge_ShaderStage stage) {
#if defined(METAL)
	switch (stage) {
	case TheForge_SS_NONE: return SHADER_STAGE_NONE;
	case TheForge_SS_VERT: return SHADER_STAGE_VERT;
	case TheForge_SS_FRAG: return SHADER_STAGE_FRAG;
	case TheForge_SS_COMP: return SHADER_STAGE_COMP;
		default: LOGERROR("Shader stage is not supported on Metal backend");
		return SHADER_STAGE_NONE;
	}
#else
	switch(stage) {
	case TheForge_SS_NONE:				return SHADER_STAGE_NONE;
	case TheForge_SS_VERT:				return SHADER_STAGE_VERT;
	case TheForge_SS_TESC:				return SHADER_STAGE_TESC;
	case TheForge_SS_TESE:				return SHADER_STAGE_TESE;
	case TheForge_SS_GEOM:				return SHADER_STAGE_GEOM;
	case TheForge_SS_FRAG:				return SHADER_STAGE_FRAG;
	case TheForge_SS_COMP:				return SHADER_STAGE_COMP;
	case TheForge_SS_RAYTRACING:	return SHADER_STAGE_RAYTRACING;
	default:
		LOGERROR("Shader stage is not supported on Metal backend");
		return SHADER_STAGE_NONE;
	}
#endif
}

static ShaderStage TheForge_ShaderStageFlagsToShaderStage(uint32_t flags) {
	uint32_t stage = SHADER_STAGE_NONE;
#if defined(METAL)
	if (flags & TheForge_SS_VERT)
		stage |= SHADER_STAGE_VERT;
	if (flags & TheForge_SS_FRAG)
		stage |= SHADER_STAGE_FRAG;
	if (flags & TheForge_SS_COMP)
		stage |= SHADER_STAGE_COMP;
#else
	if(flags & TheForge_SS_VERT) stage |= SHADER_STAGE_VERT;
	if(flags & TheForge_SS_FRAG) stage |= SHADER_STAGE_FRAG;
	if(flags & TheForge_SS_COMP) stage |= SHADER_STAGE_COMP;
	if(flags & TheForge_SS_TESC) stage |= SHADER_STAGE_TESC;
	if(flags & TheForge_SS_TESE) stage |= SHADER_STAGE_TESE;
	if(flags & TheForge_SS_GEOM) stage |= SHADER_STAGE_GEOM;
	if(flags & TheForge_SS_RAYTRACING) stage |= SHADER_STAGE_RAYTRACING;
#endif
	return (ShaderStage) stage;
}

#ifdef METAL
static void TheForge_ShaderStageToShaderStage(TheForge_ShaderStageDesc const *src, ShaderStageDesc *dst) {
	dst->mName = src->name;
	dst->mCode = src->code;
	dst->mEntryPoint = src->entryPoint;
	dst->mMacros.resize(src->macroCount);
	for (uint32_t i = 0u; i < src->macroCount; ++i) {
		dst->mMacros[i].definition = src->macros[i].definition;
		dst->mMacros[i].value = src->macros[i].value;
	}
}
#endif

static void TheForge_BinaryShaderStageToBinaryShaderStage(TheForge_BinaryShaderStageDesc const *src,
																													BinaryShaderStageDesc *dst) {
	dst->pByteCode = src->byteCode;
	dst->mByteCodeSize = src->byteCodeSize;
	dst->mEntryPoint = src->entryPoint;
#ifdef METAL
	dst->mSource = src->source;
#endif
}

static void TheForge_ShaderLoadStageToShaderLoadStage(TheForge_ShaderLoadDesc const *src, ShaderLoadDesc *dst) {

	dst->mTarget = (ShaderTarget) src->target;
	for (int i = 0; i < SHADER_STAGE_COUNT; ++i) {
		TheForge_ShaderStageLoadDesc const *srcStage;
		ShaderStageLoadDesc *dstStage = &dst->mStages[i];

#ifdef METAL
		switch (i) {
		case 0: srcStage = &src->stages[0];
			break;
		case 1: srcStage = &src->stages[1];
			break;
		case 2: srcStage = &src->stages[5];
			break;
			default: LOGERROR("Shader stage is not supported on Metal backend");
			return;
		}
#else
		srcStage = &src->stages[i];
#endif
		dstStage->mFileName = srcStage->fileName;
		dstStage->mEntryPointName = srcStage->entryPointName;
		dstStage->mMacroCount = srcStage->macroCount;
		dstStage->mRoot = (FSRoot) srcStage->root;
		for (uint32_t i = 0u; i < srcStage->macroCount; ++i) {
			dstStage->pMacros[i].definition = srcStage->pMacros[i].definition;
			dstStage->pMacros[i].value = srcStage->pMacros[i].value;
		}
	}
}

AL2O3_EXTERN_C TheForge_RendererHandle TheForge_RendererCreate(
		char const *appName,
		TheForge_RendererDesc const *settings) {

	API_CHECK(); // windows static_assert(offsetof) is broken so do it at runtime there

	RendererDesc desc {
			&LogFunc,
			(RendererApi) 0,
			(ShaderTarget) settings->shaderTarget,
	};

	Renderer *renderer;
	initRenderer(appName, &desc, &renderer);

	return (TheForge_Renderer *) renderer;
}

AL2O3_EXTERN_C void TheForge_RendererDestroy(TheForge_RendererHandle handle) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	removeRenderer(renderer);
}

AL2O3_EXTERN_C void TheForge_AddFence(TheForge_RendererHandle handle, TheForge_FenceHandle *pFence) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	addFence(renderer, (Fence **) pFence);

}

AL2O3_EXTERN_C void TheForge_RemoveFence(TheForge_RendererHandle handle, TheForge_FenceHandle fence) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;
	removeFence(renderer, (Fence *) fence);

}
AL2O3_EXTERN_C void TheForge_AddSemaphore(TheForge_RendererHandle handle, TheForge_SemaphoreHandle *pSemaphore) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	addSemaphore(renderer, (Semaphore **) pSemaphore);
}

AL2O3_EXTERN_C void TheForge_RemoveSemaphore(TheForge_RendererHandle handle, TheForge_SemaphoreHandle semaphore) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	removeSemaphore(renderer, (Semaphore *) semaphore);
}

AL2O3_EXTERN_C void TheForge_AddQueue(TheForge_RendererHandle handle,
																			TheForge_QueueDesc *pQDesc,
																			TheForge_QueueHandle *pQueue) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	addQueue(renderer, (QueueDesc *) pQDesc, (Queue **) pQueue);
}

AL2O3_EXTERN_C void TheForge_RemoveQueue(TheForge_QueueHandle queue) {
	removeQueue((Queue *) queue);
}

AL2O3_EXTERN_C void TheForge_AddCmdPool(TheForge_RendererHandle handle,
																				TheForge_QueueHandle queue,
																				bool transient,
																				TheForge_CmdPoolHandle *pCmdPool) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	addCmdPool(renderer, (Queue *) queue, transient, (CmdPool **) pCmdPool);
}

AL2O3_EXTERN_C void TheForge_RemoveCmdPool(TheForge_RendererHandle handle, TheForge_CmdPoolHandle cmdPool) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	removeCmdPool(renderer, (CmdPool *) cmdPool);
}

AL2O3_EXTERN_C void TheForge_AddCmd(TheForge_CmdPoolHandle handle, bool secondary, TheForge_CmdHandle *pCmd) {
	auto cmdPool = (CmdPool *) handle;
	if (!cmdPool)
		return;

	addCmd(cmdPool, secondary, (Cmd **) pCmd);
}

AL2O3_EXTERN_C void TheForge_RemoveCmd(TheForge_CmdPoolHandle handle, TheForge_CmdHandle cmd) {
	auto cmdPool = (CmdPool *) handle;
	if (!cmdPool)
		return;

	removeCmd(cmdPool, (Cmd *) cmd);
}

AL2O3_EXTERN_C void TheForge_AddCmd_n(TheForge_CmdPoolHandle handle,
																			bool secondary,
																			uint32_t cmdCount,
																			TheForge_CmdHandle **ppCmds) {
	auto cmdPool = (CmdPool *) handle;
	if (!cmdPool)
		return;

	addCmd_n(cmdPool, secondary, cmdCount, (Cmd ***) ppCmds);

}

AL2O3_EXTERN_C void TheForge_RemoveCmd_n(TheForge_CmdPoolHandle handle, uint32_t cmdCount, TheForge_CmdHandle *pCmds) {
	auto cmdPool = (CmdPool *) handle;
	if (!cmdPool)
		return;

	removeCmd_n(cmdPool, cmdCount, (Cmd **) pCmds);
}

AL2O3_EXTERN_C void TheForge_AddRenderTarget(TheForge_RendererHandle handle,
																						 TheForge_RenderTargetDesc const *pDesc,
																						 TheForge_RenderTargetHandle *pRenderTarget) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	TinyImageFormat tfif = pDesc->format;
	ClearValue cv;
	memcpy(&cv, &pDesc->clearValue, sizeof(ClearValue));

	RenderTargetDesc desc{
			(TextureCreationFlags) pDesc->flags,
			pDesc->width,
			pDesc->height,
			pDesc->depth,
			pDesc->arraySize,
			pDesc->mipLevels,
			(SampleCount) pDesc->sampleCount,
			tfif,
			cv,
			pDesc->sampleQuality,
			(DescriptorType) pDesc->descriptors,
			nullptr,
			nullptr, // TODO (wchar_t const*) pDesc->debugName,
			0,
			0,
			0,
	};
	addRenderTarget(renderer, &desc, (RenderTarget **) pRenderTarget);

}

AL2O3_EXTERN_C void TheForge_RemoveRenderTarget(TheForge_RendererHandle handle,
																								TheForge_RenderTargetHandle renderTarget) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	removeRenderTarget(renderer, (RenderTarget *) renderTarget);
}

AL2O3_EXTERN_C void TheForge_AddSampler(TheForge_RendererHandle handle,
																				const TheForge_SamplerDesc *pDesc,
																				TheForge_SamplerHandle *pSampler) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	addSampler(renderer, (SamplerDesc *) pDesc, (Sampler **) pSampler);
}
AL2O3_EXTERN_C void TheForge_RemoveSampler(TheForge_RendererHandle handle, TheForge_SamplerHandle sampler) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	removeSampler(renderer, (Sampler *) sampler);
}

AL2O3_EXTERN_C void TheForge_AddShader(TheForge_RendererHandle handle,
																			 const TheForge_ShaderDesc *pDesc,
																			 TheForge_ShaderHandle *pShader) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;
#ifdef METAL
	ShaderDesc desc{};
	desc.mStages = TheForge_ShaderStageFlagsToShaderStage(pDesc->stages);
	if (desc.mStages & SHADER_STAGE_VERT)
		TheForge_ShaderStageToShaderStage(&pDesc->vert, &desc.mVert);
	if (desc.mStages & SHADER_STAGE_FRAG)
		TheForge_ShaderStageToShaderStage(&pDesc->frag, &desc.mFrag);
	if (desc.mStages & SHADER_STAGE_COMP)
		TheForge_ShaderStageToShaderStage(&pDesc->comp, &desc.mComp);

	addShader(renderer, &desc, (Shader **) pShader);
#else
	LOGERROR("AddShader is only supported on Metal backends, Use AddShaderBinary");
#endif
}

AL2O3_EXTERN_C void TheForge_AddShaderBinary(TheForge_RendererHandle handle,
																						 const TheForge_BinaryShaderDesc *pDesc,
																						 TheForge_ShaderHandle *pShader) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	BinaryShaderDesc desc{};
	desc.mStages = TheForge_ShaderStageFlagsToShaderStage(pDesc->stages);
	if (desc.mStages & SHADER_STAGE_VERT)
		TheForge_BinaryShaderStageToBinaryShaderStage(&pDesc->vert, &desc.mVert);
	if (desc.mStages & SHADER_STAGE_FRAG)
		TheForge_BinaryShaderStageToBinaryShaderStage(&pDesc->frag, &desc.mFrag);
#ifndef METAL
	if(desc.mStages & SHADER_STAGE_GEOM)
		TheForge_BinaryShaderStageToBinaryShaderStage(&pDesc->geom, &desc.mGeom);
	if(desc.mStages & SHADER_STAGE_HULL)
		TheForge_BinaryShaderStageToBinaryShaderStage(&pDesc->hull, &desc.mHull);
	if(desc.mStages & SHADER_STAGE_DOMN)
		TheForge_BinaryShaderStageToBinaryShaderStage(&pDesc->domain, &desc.mDomain);
#endif
	if (desc.mStages & SHADER_STAGE_COMP)
		TheForge_BinaryShaderStageToBinaryShaderStage(&pDesc->comp, &desc.mComp);

	addShaderBinary(renderer, &desc, (Shader **) pShader);
}

AL2O3_EXTERN_C void TheForge_RemoveShader(TheForge_RendererHandle handle, TheForge_ShaderHandle pShader) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	removeShader(renderer, (Shader *) pShader);
}

AL2O3_EXTERN_C void TheForge_AddRootSignature(TheForge_RendererHandle handle,
																							const TheForge_RootSignatureDesc *pRootDesc,
																							TheForge_RootSignatureHandle *pRootSignature) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	addRootSignature(renderer, (RootSignatureDesc *) pRootDesc, (RootSignature **) pRootSignature);
}

AL2O3_EXTERN_C void TheForge_RemoveRootSignature(TheForge_RendererHandle handle,
																								 TheForge_RootSignatureHandle rootSignature) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	removeRootSignature(renderer, (RootSignature *) rootSignature);
}


AL2O3_EXTERN_C void TheForge_AddPipeline(TheForge_RendererHandle handle,
																				 const TheForge_PipelineDesc *pPipelineDesc,
																				 TheForge_PipelineHandle *pPipeline) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	addPipeline(renderer, (PipelineDesc *) pPipelineDesc, (Pipeline **) pPipeline);
}
AL2O3_EXTERN_C void TheForge_RemovePipeline(TheForge_RendererHandle handle, TheForge_PipelineHandle pipeline) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	removePipeline(renderer, (Pipeline *) pipeline);
}

AL2O3_EXTERN_C void TheForge_AddDescriptorBinder(TheForge_RendererHandle handle,
																								 uint32_t gpuIndex,
																								 uint32_t descCount,
																								 const TheForge_DescriptorBinderDesc *pDescs,
																								 TheForge_DescriptorBinderHandle *pDescriptorBinder) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	addDescriptorBinder(renderer,
											gpuIndex,
											descCount,
											(DescriptorBinderDesc *) pDescs,
											(DescriptorBinder **) pDescriptorBinder);
}

AL2O3_EXTERN_C void TheForge_RemoveDescriptorBinder(TheForge_RendererHandle handle,
																										TheForge_DescriptorBinderHandle descriptorBinder) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	removeDescriptorBinder(renderer, (DescriptorBinder *) descriptorBinder);
}

void TheForge_AddBlendState(TheForge_RendererHandle handle,
														const TheForge_BlendStateDesc *pDesc,
														TheForge_BlendStateHandle *pBlendState) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	addBlendState(renderer, (BlendStateDesc *) pDesc, (BlendState **) pBlendState);
}
void TheForge_RemoveBlendState(TheForge_RendererHandle handle, TheForge_BlendStateHandle blendState) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	// renderer isn't currently used by TheForge?
	removeBlendState((BlendState *) blendState);
}

void TheForge_AddDepthState(TheForge_RendererHandle handle,
														const TheForge_DepthStateDesc *pDesc,
														TheForge_DepthStateHandle *pDepthState) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	addDepthState(renderer, (DepthStateDesc *) pDesc, (DepthState **) pDepthState);
}
void TheForge_RemoveDepthState(TheForge_RendererHandle handle, TheForge_DepthStateHandle depthState) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;
	// renderer isn't currently used by TheForge?
	removeDepthState((DepthState *) depthState);
}

void TheForge_AddRasterizerState(TheForge_RendererHandle handle,
																 const TheForge_RasterizerStateDesc *pDesc,
																 TheForge_RasterizerStateHandle *pRasterizerState) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;
	addRasterizerState(renderer, (RasterizerStateDesc *) pDesc, (RasterizerState **) pRasterizerState);
}

void TheForge_RemoveRasterizerState(TheForge_RendererHandle handle, TheForge_RasterizerStateHandle rasterizerState) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	// renderer isn't currently used by TheForge?
	removeRasterizerState((RasterizerState *) rasterizerState);
}
AL2O3_EXTERN_C void TheForge_BeginCmd(TheForge_CmdHandle cmd) {
	beginCmd((Cmd *) cmd);
}
AL2O3_EXTERN_C void TheForge_EndCmd(TheForge_CmdHandle cmd) {
	endCmd((Cmd *) cmd);
}
AL2O3_EXTERN_C void TheForge_CmdBindRenderTargets(TheForge_CmdHandle cmd,
																									uint32_t renderTargetCount,
																									TheForge_RenderTargetHandle *pRenderTargets,
																									TheForge_RenderTargetHandle depthStencil,
																									const TheForge_LoadActionsDesc *loadActions,
																									uint32_t *pColorArraySlices,
																									uint32_t *pColorMipSlices,
																									uint32_t depthArraySlice,
																									uint32_t depthMipSlice) {

	cmdBindRenderTargets((Cmd *) cmd,
											 renderTargetCount,
											 (RenderTarget **) pRenderTargets,
											 (RenderTarget *) depthStencil,
											 (LoadActionsDesc *) loadActions,
											 pColorArraySlices,
											 pColorMipSlices,
											 depthArraySlice,
											 depthMipSlice);

}
AL2O3_EXTERN_C void TheForge_CmdSetViewport(TheForge_CmdHandle cmd,
																						float x,
																						float y,
																						float width,
																						float height,
																						float minDepth,
																						float maxDepth) {
	cmdSetViewport((Cmd *) cmd, x, y, width, height, minDepth, maxDepth);

}
AL2O3_EXTERN_C void TheForge_CmdSetScissor(TheForge_CmdHandle cmd,
																					 uint32_t x,
																					 uint32_t y,
																					 uint32_t width,
																					 uint32_t height) {
	cmdSetScissor((Cmd *) cmd, x, y, width, height);

}
AL2O3_EXTERN_C void TheForge_CmdBindPipeline(TheForge_CmdHandle cmd, TheForge_PipelineHandle pipeline) {
	cmdBindPipeline((Cmd *) cmd, (Pipeline *) pipeline);
}

AL2O3_EXTERN_C void TheForge_CmdBindDescriptors(TheForge_CmdHandle cmd,
																								TheForge_DescriptorBinderHandle descriptorBinder,
																								TheForge_RootSignatureHandle rootSignature,
																								uint32_t numDescriptors,
																								TheForge_DescriptorData *pDescParams) {
	cmdBindDescriptors((Cmd *) cmd,
										 (DescriptorBinder *) descriptorBinder,
										 (RootSignature *) rootSignature,
										 numDescriptors,
										 (DescriptorData *) pDescParams);
}
AL2O3_EXTERN_C void TheForge_CmdBindIndexBuffer(TheForge_CmdHandle cmd, TheForge_BufferHandle buffer, uint64_t offset) {

	cmdBindIndexBuffer((Cmd *) cmd, (Buffer *) buffer, offset);
}
AL2O3_EXTERN_C void TheForge_CmdBindVertexBuffer(TheForge_CmdHandle cmd,
																								 uint32_t bufferCount,
																								 TheForge_BufferHandle *pBuffers,
																								 uint64_t const *pOffsets) {
	cmdBindVertexBuffer((Cmd *) cmd, bufferCount, (Buffer **) pBuffers, (uint64_t*)pOffsets);
}
AL2O3_EXTERN_C void TheForge_CmdDraw(TheForge_CmdHandle cmd, uint32_t vertexCount, uint32_t firstVertex) {
	cmdDraw((Cmd *) cmd, vertexCount, firstVertex);
}
AL2O3_EXTERN_C void TheForge_CmdDrawInstanced(TheForge_CmdHandle cmd,
																							uint32_t vertexCount,
																							uint32_t firstVertex,
																							uint32_t instanceCount,
																							uint32_t firstInstance) {
	cmdDrawInstanced((Cmd *) cmd, vertexCount, firstVertex, instanceCount, firstInstance);
}
AL2O3_EXTERN_C void TheForge_CmdDrawIndexed(TheForge_CmdHandle cmd,
																						uint32_t indexCount,
																						uint32_t firstIndex,
																						uint32_t firstVertex) {
	cmdDrawIndexed((Cmd *) cmd, indexCount, firstIndex, firstVertex);

}
AL2O3_EXTERN_C void TheForge_CmdDrawIndexedInstanced(TheForge_CmdHandle cmd,
																										 uint32_t indexCount,
																										 uint32_t firstIndex,
																										 uint32_t instanceCount,
																										 uint32_t firstVertex,
																										 uint32_t firstInstance) {
	cmdDrawIndexedInstanced((Cmd *) cmd, indexCount, firstIndex, instanceCount, firstVertex, firstInstance);
}
AL2O3_EXTERN_C void TheForge_CmdDispatch(TheForge_CmdHandle cmd,
																				 uint32_t groupCountX,
																				 uint32_t groupCountY,
																				 uint32_t groupCountZ) {
	cmdDispatch((Cmd *) cmd, groupCountX, groupCountY, groupCountZ);
}
AL2O3_EXTERN_C void TheForge_CmdResourceBarrier(TheForge_CmdHandle cmd,
																								uint32_t bufferBarrierCount,
																								TheForge_BufferBarrier *pBufferBarriers,
																								uint32_t textureBarrierCount,
																								TheForge_TextureBarrier *pTextureBarriers) {
	cmdResourceBarrier((Cmd *) cmd,
										 bufferBarrierCount,
										 (BufferBarrier *) pBufferBarriers,
										 textureBarrierCount,
										 (TextureBarrier *) pTextureBarriers);
}

AL2O3_EXTERN_C void TheForge_WaitQueueIdle(TheForge_QueueHandle queue) {

	waitQueueIdle((Queue *) queue);
}
AL2O3_EXTERN_C void TheForge_GetFenceStatus(TheForge_RendererHandle handle,
																						TheForge_FenceHandle fence,
																						TheForge_FenceStatus *pFenceStatus) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;
	getFenceStatus(renderer, (Fence *) fence, (FenceStatus *) pFenceStatus);
}

AL2O3_EXTERN_C void TheForge_WaitForFences(TheForge_RendererHandle handle,
																					 uint32_t fenceCount,
																					 TheForge_FenceHandle *pFences) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	waitForFences(renderer, fenceCount, (Fence **) pFences);
}

AL2O3_EXTERN_C void TheForge_CmdBeginDebugMarker(TheForge_CmdHandle cmd, float r, float g, float b, const char *pName) {
	cmdBeginDebugMarker((Cmd *) cmd, r, g, b, pName);
}
AL2O3_EXTERN_C void TheForge_CmdExecuteIndirect(TheForge_CmdHandle cmd,
																								TheForge_CommandSignatureHandle commandSignature,
																								uint32_t maxCommandCount,
																								TheForge_BufferHandle indirectBuffer,
																								uint64_t bufferOffset,
																								TheForge_BufferHandle counterBuffer,
																								uint64_t counterBufferOffset) {
	cmdExecuteIndirect((Cmd *) cmd,
										 (CommandSignature *) commandSignature,
										 maxCommandCount,
										 (Buffer *) indirectBuffer,
										 bufferOffset,
										 (Buffer *) counterBuffer,
										 counterBufferOffset);
}

AL2O3_EXTERN_C void TheForge_CmdBeginQuery(TheForge_CmdHandle cmd,
																					 TheForge_QueryPoolHandle queryHeap,
																					 TheForge_QueryDesc *pQuery) {
	cmdBeginQuery((Cmd *) cmd,
								(QueryPool *) queryHeap,
								(QueryDesc *) pQuery);
}
AL2O3_EXTERN_C void TheForge_CmdEndQuery(TheForge_CmdHandle cmd,
																				 TheForge_QueryPoolHandle queryHeap,
																				 TheForge_QueryDesc *pQuery) {
	cmdEndQuery((Cmd *) cmd,
							(QueryPool *) queryHeap,
							(QueryDesc *) pQuery);
}
AL2O3_EXTERN_C void TheForge_CmdResolveQuery(TheForge_CmdHandle cmd,
																						 TheForge_QueryPoolHandle queryHeap,
																						 TheForge_BufferHandle readbackBuffer,
																						 uint32_t startQuery,
																						 uint32_t queryCount) {
	cmdResolveQuery((Cmd *) cmd,
									(QueryPool *) queryHeap,
									(Buffer *) readbackBuffer,
									startQuery,
									queryCount);

}


AL2O3_EXTERN_C void TheForge_CmdAddDebugMarker(TheForge_CmdHandle cmd, float r, float g, float b, const char *pName) {
	cmdAddDebugMarker((Cmd *) cmd, r, g, b, pName);
}

AL2O3_EXTERN_C void TheForge_CmdEndDebugMarker(TheForge_CmdHandle cmd) {
	cmdEndDebugMarker((Cmd *) cmd);
}

AL2O3_EXTERN_C void TheForge_QueueSubmit(TheForge_QueueHandle queue,
																				 uint32_t cmdCount,
																				 TheForge_CmdHandle *pCmds,
																				 TheForge_FenceHandle fence,
																				 uint32_t waitSemaphoreCount,
																				 TheForge_SemaphoreHandle *pWaitSemaphores,
																				 uint32_t signalSemaphoreCount,
																				 TheForge_SemaphoreHandle *pSignalSemaphores) {
	queueSubmit((Queue *) queue,
							cmdCount,
							(Cmd **) pCmds,
							(Fence *) fence,
							waitSemaphoreCount,
							(Semaphore **) pWaitSemaphores,
							signalSemaphoreCount,
							(Semaphore **) pSignalSemaphores
	);
}
AL2O3_EXTERN_C void TheForge_GetTimestampFrequency(TheForge_QueueHandle queue, double *pFrequency) {
	getTimestampFrequency((Queue *) queue, pFrequency);
}

AL2O3_EXTERN_C void TheForge_AddIndirectCommandSignature(TheForge_RendererHandle handle,
																												 const TheForge_CommandSignatureDesc *pDesc,
																												 TheForge_CommandSignatureHandle *pCommandSignature) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	addIndirectCommandSignature(renderer,
															(CommandSignatureDesc const *) pDesc,
															(CommandSignature **) pCommandSignature);
}

AL2O3_EXTERN_C void TheForge_RemoveIndirectCommandSignature(TheForge_RendererHandle handle,
																														TheForge_CommandSignatureHandle commandSignature) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	removeIndirectCommandSignature(renderer, (CommandSignature *) commandSignature);
}


AL2O3_EXTERN_C void TheForge_CalculateMemoryStats(TheForge_RendererHandle handle, char **stats) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	calculateMemoryStats(renderer, stats);
}

AL2O3_EXTERN_C void TheForge_FreeMemoryStats(TheForge_RendererHandle handle, char *stats) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	freeMemoryStats(renderer, stats);
}

AL2O3_EXTERN_C void TheForge_SetBufferName(TheForge_RendererHandle handle,
																					 TheForge_BufferHandle buffer,
																					 const char *pName) {

#ifndef METAL
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	setBufferName(renderer, (Buffer*)buffer, pName);
#endif
}
AL2O3_EXTERN_C void TheForge_SetTextureName(TheForge_RendererHandle handle,
																						TheForge_TextureHandle texture,
																						const char *pName) {
#ifndef METAL
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	setTextureName(renderer, (Texture*)texture, pName);
#endif
}
AL2O3_EXTERN_C void TheForge_AddSwapChain(TheForge_RendererHandle handle,
																					const TheForge_SwapChainDesc *pDesc,
																					TheForge_SwapChainHandle *pSwapChain) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	// we don't use virtually any of the the forges windows desc as we just want
	// the swap chain to attached (WindowsDesc is if you used TheForges OS to
	// allocate teh window)
	WindowsDesc windowsDesc;
	windowsDesc.handle = (WindowHandle)pDesc->pWindow->handle;
	SwapChainDesc scDesc;
	memcpy(&scDesc, pDesc, sizeof(TheForge_SwapChainDesc));
	scDesc.pWindow = &windowsDesc;

	addSwapChain(renderer, &scDesc, (SwapChain **) pSwapChain);
}


AL2O3_EXTERN_C void TheForge_RemoveSwapChain(TheForge_RendererHandle handle, TheForge_SwapChainHandle swapChain) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;
	removeSwapChain(renderer, (SwapChain *) swapChain);
}

AL2O3_EXTERN_C void TheForge_ToggleVSync(TheForge_RendererHandle handle, TheForge_SwapChainHandle *pSwapchain) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	toggleVSync(renderer, (SwapChain **) pSwapchain);
}

AL2O3_EXTERN_C TinyImageFormat TheForge_GetRecommendedSwapchainFormat(bool hintHDR) {
	return getRecommendedSwapchainFormat(hintHDR);
}
AL2O3_EXTERN_C void TheForge_AcquireNextImage(TheForge_RendererHandle handle,
																							TheForge_SwapChainHandle swapChain,
																							TheForge_SemaphoreHandle signalSemaphore,
																							TheForge_FenceHandle fence,
																							uint32_t *pImageIndex) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	acquireNextImage(renderer, (SwapChain *) swapChain, (Semaphore *) signalSemaphore, (Fence *) fence, pImageIndex);
}
AL2O3_EXTERN_C void TheForge_QueuePresent(TheForge_QueueHandle queue,
																					TheForge_SwapChainHandle swapChain,
																					uint32_t swapChainImageIndex,
																					uint32_t waitSemaphoreCount,
																					TheForge_SemaphoreHandle *pWaitSemaphores) {

	queuePresent((Queue *) queue,
							 (SwapChain *) swapChain,
							 swapChainImageIndex,
							 waitSemaphoreCount,
							 (Semaphore **) pWaitSemaphores);
}

AL2O3_EXTERN_C TheForge_RenderTargetHandle TheForge_SwapChainGetRenderTarget(TheForge_SwapChainHandle swapChain,
																																						 int index) {
	return (TheForge_RenderTargetHandle) ((SwapChain *) swapChain)->ppSwapchainRenderTargets[index];
}

AL2O3_EXTERN_C TheForge_TextureHandle TheForge_RenderTargetGetTexture(TheForge_RenderTargetHandle renderTarget) {
	return (TheForge_TextureHandle) ((RenderTarget *) renderTarget)->pTexture;
}

AL2O3_EXTERN_C TheForge_RenderTargetDesc const *TheForge_RenderTargetGetDesc(TheForge_RenderTargetHandle renderTarget) {
	return (TheForge_RenderTargetDesc const *) &((RenderTarget *) renderTarget)->mDesc;
}
AL2O3_EXTERN_C TheForge_PipelineReflection const* TheForge_ShaderGetPipelineReflection(TheForge_ShaderHandle shader) {
	return (TheForge_PipelineReflection const *) &((Shader *) shader)->mReflection;

}

AL2O3_EXTERN_C void TheForge_AddBuffer(TheForge_RendererHandle handle, TheForge_BufferDesc const* pDesc, TheForge_BufferHandle* pBuffer) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	addBuffer(renderer, (BufferDesc const*)pDesc, (Buffer**)pBuffer);
}
AL2O3_EXTERN_C void TheForge_AddTexture(TheForge_RendererHandle handle, TheForge_TextureDesc const* pDesc, TheForge_TextureHandle* pTexture) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	addTexture(renderer, (TextureDesc const*)pDesc, (Texture**)pTexture);
}
AL2O3_EXTERN_C void TheForge_RemoveBuffer(TheForge_RendererHandle handle, TheForge_BufferHandle buffer) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;
	removeBuffer(renderer, (Buffer*)buffer);
}
AL2O3_EXTERN_C void TheForge_RemoveTexture(TheForge_RendererHandle handle, TheForge_TextureHandle texture) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	removeTexture(renderer, (Texture*)texture);
}

AL2O3_EXTERN_C void TheForge_MapBuffer(TheForge_RendererHandle handle, TheForge_BufferHandle buffer, TheForge_ReadRange* pRange) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;
	mapBuffer(renderer, (Buffer*)buffer, (ReadRange*)pRange);
}

AL2O3_EXTERN_C void TheForge_UnmapBuffer(TheForge_RendererHandle handle, TheForge_BufferHandle buffer) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;
	unmapBuffer(renderer, (Buffer*)buffer);

}

AL2O3_EXTERN_C void TheForge_InitResourceLoaderInterface(TheForge_RendererHandle handle,
																												 TheForge_ResourceLoaderDesc *pDesc) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	initResourceLoaderInterface(renderer, (ResourceLoaderDesc *) pDesc);
}
AL2O3_EXTERN_C void TheForge_RemoveResourceLoaderInterface(TheForge_RendererHandle handle) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;
	removeResourceLoaderInterface(renderer);
}
AL2O3_EXTERN_C void TheForge_LoadShader(TheForge_RendererHandle handle,
																				const TheForge_ShaderLoadDesc *pDesc,
																				TheForge_ShaderHandle *pShader) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;
	ShaderLoadDesc desc;
	TheForge_ShaderLoadStageToShaderLoadStage(pDesc, &desc);
	addShader(renderer, &desc, (Shader **) pShader);
}

AL2O3_EXTERN_C void TheForge_LoadBuffer(TheForge_BufferLoadDesc const *pBufferLoadDesc, bool batch) {
	addResource((BufferLoadDesc *) pBufferLoadDesc, batch);
}
AL2O3_EXTERN_C void TheForge_LoadTexture(TheForge_TextureLoadDesc const *pTextureLoadDesc, bool batch) {
	addResource((TextureLoadDesc *) pTextureLoadDesc, batch);
}
AL2O3_EXTERN_C void TheForge_LoadBufferWithToken(TheForge_BufferLoadDesc const *pBufferLoadDesc, TheForge_SyncToken *token) {
	addResource((BufferLoadDesc *) pBufferLoadDesc, (SyncToken *) token);
}
AL2O3_EXTERN_C void TheForge_LoadTextureWithToken(TheForge_TextureLoadDesc const *pTextureLoadDesc,
																								 TheForge_SyncToken *token) {
	addResource((TextureLoadDesc *) pTextureLoadDesc, (SyncToken *) token);
}
AL2O3_EXTERN_C void TheForge_UpdateBuffer(TheForge_BufferUpdateDesc const *pBuffer, bool batch) {
	updateResource((BufferUpdateDesc *) pBuffer, batch);
}
AL2O3_EXTERN_C void TheForge_UpdateTexture(TheForge_TextureUpdateDesc const *pTexture, bool batch) {
	updateResource((TextureUpdateDesc *) pTexture, batch);
}
AL2O3_EXTERN_C void TheForge_UpdateResources(uint32_t resourceCount, TheForge_ResourceUpdateDesc *pResources) {
	updateResources(resourceCount, (ResourceUpdateDesc *) pResources);
}
AL2O3_EXTERN_C void TheForge_UpdateBufferWithToken(TheForge_BufferUpdateDesc *pBuffer, TheForge_SyncToken *token) {
	updateResource((BufferUpdateDesc *) pBuffer, (SyncToken *) token);
}
AL2O3_EXTERN_C void TheForge_UpdateTextureWithToken(TheForge_TextureUpdateDesc *pTexture, TheForge_SyncToken *token) {
	updateResource((TextureUpdateDesc *) pTexture, (SyncToken *) token);
}
AL2O3_EXTERN_C void TheForge_UpdateResourcesWithToken(uint32_t resourceCount,
																											TheForge_ResourceUpdateDesc *pResources,
																											TheForge_SyncToken *token) {
	updateResources(resourceCount, (ResourceUpdateDesc *) pResources, (SyncToken *) token);
}
AL2O3_EXTERN_C bool TheForge_IsBatchCompleted() {
	return isBatchCompleted();
}
AL2O3_EXTERN_C void TheForge_WaitBatchCompleted() {
	waitBatchCompleted();
}
AL2O3_EXTERN_C bool TheForge_IsTokenCompleted(TheForge_SyncToken token) {
	return isTokenCompleted(token);
}
AL2O3_EXTERN_C void TheForge_WaitTokenCompleted(TheForge_SyncToken token) {
	waitTokenCompleted(token);
}

AL2O3_EXTERN_C void TheForge_FlushResourceUpdates() {
	flushResourceUpdates();
}
AL2O3_EXTERN_C void TheForge_FinishResourceLoading() {
	finishResourceLoading();
}

AL2O3_EXTERN_C TheForge_RendererApi TheForge_GetRendererApi(TheForge_RendererHandle handle) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return TheForge_API_D3D11;

	return (TheForge_RendererApi)renderer->mSettings.mApi;
}

AL2O3_EXTERN_C bool TheForge_CanShaderReadFrom(TheForge_RendererHandle handle, TinyImageFormat format) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return false;

	return renderer->capBits.canShaderReadFrom[format];
}
AL2O3_EXTERN_C bool TheForge_CanColorWriteTo(TheForge_RendererHandle handle, TinyImageFormat format) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return false;
	return renderer->capBits.canShaderWriteTo[format];
}

AL2O3_EXTERN_C bool TheForge_CanShaderWriteTo(TheForge_RendererHandle handle, TinyImageFormat format) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return false;
	return renderer->capBits.canColorWriteTo[format];
}

#if AL2O3_PLATFORM == AL2O3_PLATFORM_WINDOWS
#define API_CHK(x) ASSERT(x)
#else
#define API_CHK(x) static_assert(x)
#endif

static void API_CHECK() {
	API_CHK(sizeof(TheForge_ComputePipelineDesc) == sizeof(ComputePipelineDesc));
	API_CHK(sizeof(TheForge_GraphicsPipelineDesc) == sizeof(GraphicsPipelineDesc));
	API_CHK(sizeof(TheForge_RaytracingPipelineDesc) == sizeof(RaytracingPipelineDesc));

	API_CHK(offsetof(TheForge_ComputePipelineDesc, shaderProgram) == offsetof(ComputePipelineDesc, pShaderProgram));
	API_CHK(offsetof(TheForge_ComputePipelineDesc, rootSignature) == offsetof(ComputePipelineDesc, pRootSignature));

	API_CHK(
			offsetof(TheForge_GraphicsPipelineDesc, shaderProgram) == offsetof(GraphicsPipelineDesc, pShaderProgram));
	API_CHK(
			offsetof(TheForge_GraphicsPipelineDesc, rootSignature) == offsetof(GraphicsPipelineDesc, pRootSignature));
	API_CHK(
			offsetof(TheForge_GraphicsPipelineDesc, pVertexLayout) == offsetof(GraphicsPipelineDesc, pVertexLayout));
	API_CHK(offsetof(TheForge_GraphicsPipelineDesc, blendState) == offsetof(GraphicsPipelineDesc, pBlendState));
	API_CHK(offsetof(TheForge_GraphicsPipelineDesc, depthState) == offsetof(GraphicsPipelineDesc, pDepthState));
	API_CHK(
			offsetof(TheForge_GraphicsPipelineDesc, rasterizerState) == offsetof(GraphicsPipelineDesc, pRasterizerState));
	API_CHK(
			offsetof(TheForge_GraphicsPipelineDesc, pColorFormats) == offsetof(GraphicsPipelineDesc, pColorFormats));
	API_CHK(
			offsetof(TheForge_GraphicsPipelineDesc, renderTargetCount) == offsetof(GraphicsPipelineDesc, mRenderTargetCount));
	API_CHK(offsetof(TheForge_GraphicsPipelineDesc, sampleCount) == offsetof(GraphicsPipelineDesc, mSampleCount));
	API_CHK(
			offsetof(TheForge_GraphicsPipelineDesc, sampleQuality) == offsetof(GraphicsPipelineDesc, mSampleQuality));
	API_CHK(offsetof(TheForge_GraphicsPipelineDesc, depthStencilFormat)
										== offsetof(GraphicsPipelineDesc, mDepthStencilFormat));
	API_CHK(
			offsetof(TheForge_GraphicsPipelineDesc, primitiveTopo) == offsetof(GraphicsPipelineDesc, mPrimitiveTopo));

	API_CHK(offsetof(TheForge_RaytracingPipelineDesc, raytracing) == offsetof(RaytracingPipelineDesc, pRaytracing));
	API_CHK(offsetof(TheForge_RaytracingPipelineDesc, globalRootSignature)
										== offsetof(RaytracingPipelineDesc, pGlobalRootSignature));
	API_CHK(
			offsetof(TheForge_RaytracingPipelineDesc, rayGenShader) == offsetof(RaytracingPipelineDesc, pRayGenShader));
	API_CHK(offsetof(TheForge_RaytracingPipelineDesc, rayGenRootSignature)
										== offsetof(RaytracingPipelineDesc, pRayGenRootSignature));
	API_CHK(
			offsetof(TheForge_RaytracingPipelineDesc, pMissShaders) == offsetof(RaytracingPipelineDesc, ppMissShaders));
	API_CHK(offsetof(TheForge_RaytracingPipelineDesc, pMissRootSignatures)
										== offsetof(RaytracingPipelineDesc, ppMissRootSignatures));
	API_CHK(offsetof(TheForge_RaytracingPipelineDesc, hitGroups) == offsetof(RaytracingPipelineDesc, pHitGroups));
	API_CHK(
			offsetof(TheForge_RaytracingPipelineDesc, missShaderCount) == offsetof(RaytracingPipelineDesc, mMissShaderCount));
	API_CHK(
			offsetof(TheForge_RaytracingPipelineDesc, hitGroupCount) == offsetof(RaytracingPipelineDesc, mHitGroupCount));
	API_CHK(
			offsetof(TheForge_RaytracingPipelineDesc, payloadSize) == offsetof(RaytracingPipelineDesc, mPayloadSize));
	API_CHK(
			offsetof(TheForge_RaytracingPipelineDesc, attributeSize) == offsetof(RaytracingPipelineDesc, mAttributeSize));
	API_CHK(offsetof(TheForge_RaytracingPipelineDesc, maxTraceRecursionDepth)
										== offsetof(RaytracingPipelineDesc, mMaxTraceRecursionDepth));
	API_CHK(
			offsetof(TheForge_RaytracingPipelineDesc, maxRaysCount) == offsetof(RaytracingPipelineDesc, mMaxRaysCount));

	API_CHK(sizeof(TheForge_VertexLayout) == sizeof(VertexLayout));
	API_CHK(offsetof(TheForge_VertexLayout, attribCount) == offsetof(VertexLayout, mAttribCount));
	API_CHK(offsetof(TheForge_VertexLayout, attribs) == offsetof(VertexLayout, mAttribs));
	API_CHK(offsetof(TheForge_VertexAttrib, semantic) == offsetof(VertexAttrib, mSemantic));
	API_CHK(offsetof(TheForge_VertexAttrib, semanticNameLength) == offsetof(VertexAttrib, mSemanticNameLength));
	API_CHK(offsetof(TheForge_VertexAttrib, semanticName) == offsetof(VertexAttrib, mSemanticName));
	API_CHK(offsetof(TheForge_VertexAttrib, format) == offsetof(VertexAttrib, mFormat));
	API_CHK(offsetof(TheForge_VertexAttrib, location) == offsetof(VertexAttrib, mLocation));
	API_CHK(offsetof(TheForge_VertexAttrib, offset) == offsetof(VertexAttrib, mOffset));
	API_CHK(offsetof(TheForge_VertexAttrib, rate) == offsetof(VertexAttrib, mRate));

	API_CHK(sizeof(TheForge_RootSignatureDesc) == sizeof(RootSignatureDesc));
	API_CHK(offsetof(TheForge_RootSignatureDesc, pShaders) == offsetof(RootSignatureDesc, ppShaders));
	API_CHK(offsetof(TheForge_RootSignatureDesc, shaderCount) == offsetof(RootSignatureDesc, mShaderCount));
	API_CHK(
			offsetof(TheForge_RootSignatureDesc, maxBindlessTextures) == offsetof(RootSignatureDesc, mMaxBindlessTextures));
	API_CHK(
			offsetof(TheForge_RootSignatureDesc, pStaticSamplerNames) == offsetof(RootSignatureDesc, ppStaticSamplerNames));
	API_CHK(offsetof(TheForge_RootSignatureDesc, pStaticSamplers) == offsetof(RootSignatureDesc, ppStaticSamplers));
	API_CHK(
			offsetof(TheForge_RootSignatureDesc, staticSamplerCount) == offsetof(RootSignatureDesc, mStaticSamplerCount));
	API_CHK(offsetof(TheForge_RootSignatureDesc, flags) == offsetof(RootSignatureDesc, mFlags));

	API_CHK(sizeof(TheForge_SamplerDesc) == sizeof(SamplerDesc));
	API_CHK(offsetof(TheForge_SamplerDesc, minFilter) == offsetof(SamplerDesc, mMinFilter));
	API_CHK(offsetof(TheForge_SamplerDesc, magFilter) == offsetof(SamplerDesc, mMagFilter));
	API_CHK(offsetof(TheForge_SamplerDesc, mipMapMode) == offsetof(SamplerDesc, mMipMapMode));
	API_CHK(offsetof(TheForge_SamplerDesc, addressU) == offsetof(SamplerDesc, mAddressU));
	API_CHK(offsetof(TheForge_SamplerDesc, addressV) == offsetof(SamplerDesc, mAddressV));
	API_CHK(offsetof(TheForge_SamplerDesc, addressW) == offsetof(SamplerDesc, mAddressW));
	API_CHK(offsetof(TheForge_SamplerDesc, mipLodBias) == offsetof(SamplerDesc, mMipLodBias));
	API_CHK(offsetof(TheForge_SamplerDesc, maxAnisotropy) == offsetof(SamplerDesc, mMaxAnisotropy));
	API_CHK(offsetof(TheForge_SamplerDesc, compareFunc) == offsetof(SamplerDesc, mCompareFunc));

	API_CHK(sizeof(TheForge_QueueDesc) == sizeof(QueueDesc));
	API_CHK(offsetof(TheForge_QueueDesc, flags) == offsetof(QueueDesc, mFlag));
	API_CHK(offsetof(TheForge_QueueDesc, priority) == offsetof(QueueDesc, mPriority));
	API_CHK(offsetof(TheForge_QueueDesc, type) == offsetof(QueueDesc, mType));
	API_CHK(offsetof(TheForge_QueueDesc, nodeIndex) == offsetof(QueueDesc, mNodeIndex));

	API_CHK(sizeof(TheForge_BlendStateDesc) == sizeof(BlendStateDesc));
	API_CHK(offsetof(TheForge_BlendStateDesc, srcFactors) == offsetof(BlendStateDesc, mSrcFactors));
	API_CHK(offsetof(TheForge_BlendStateDesc, dstFactors) == offsetof(BlendStateDesc, mDstFactors));
	API_CHK(offsetof(TheForge_BlendStateDesc, srcAlphaFactors) == offsetof(BlendStateDesc, mSrcAlphaFactors));
	API_CHK(offsetof(TheForge_BlendStateDesc, dstAlphaFactors) == offsetof(BlendStateDesc, mDstAlphaFactors));
	API_CHK(offsetof(TheForge_BlendStateDesc, blendModes) == offsetof(BlendStateDesc, mBlendModes));
	API_CHK(offsetof(TheForge_BlendStateDesc, blendAlphaModes) == offsetof(BlendStateDesc, mBlendAlphaModes));
	API_CHK(offsetof(TheForge_BlendStateDesc, masks) == offsetof(BlendStateDesc, mMasks));
	API_CHK(offsetof(TheForge_BlendStateDesc, renderTargetMask) == offsetof(BlendStateDesc, mRenderTargetMask));
	API_CHK(offsetof(TheForge_BlendStateDesc, alphaToCoverage) == offsetof(BlendStateDesc, mAlphaToCoverage));
	API_CHK(offsetof(TheForge_BlendStateDesc, independentBlend) == offsetof(BlendStateDesc, mIndependentBlend));

	API_CHK(sizeof(TheForge_DepthStateDesc) == sizeof(DepthStateDesc));
	API_CHK(offsetof(TheForge_DepthStateDesc, depthTest) == offsetof(DepthStateDesc, mDepthTest));
	API_CHK(offsetof(TheForge_DepthStateDesc, depthWrite) == offsetof(DepthStateDesc, mDepthWrite));
	API_CHK(offsetof(TheForge_DepthStateDesc, depthFunc) == offsetof(DepthStateDesc, mDepthFunc));
	API_CHK(offsetof(TheForge_DepthStateDesc, stencilTest) == offsetof(DepthStateDesc, mStencilTest));
	API_CHK(offsetof(TheForge_DepthStateDesc, stencilReadMask) == offsetof(DepthStateDesc, mStencilReadMask));
	API_CHK(offsetof(TheForge_DepthStateDesc, stencilWriteMask) == offsetof(DepthStateDesc, mStencilWriteMask));
	API_CHK(offsetof(TheForge_DepthStateDesc, stencilFrontFunc) == offsetof(DepthStateDesc, mStencilFrontFunc));
	API_CHK(offsetof(TheForge_DepthStateDesc, stencilFrontFail) == offsetof(DepthStateDesc, mStencilFrontFail));
	API_CHK(offsetof(TheForge_DepthStateDesc, depthFrontFail) == offsetof(DepthStateDesc, mDepthFrontFail));
	API_CHK(offsetof(TheForge_DepthStateDesc, stencilFrontPass) == offsetof(DepthStateDesc, mStencilFrontPass));
	API_CHK(offsetof(TheForge_DepthStateDesc, stencilBackFunc) == offsetof(DepthStateDesc, mStencilBackFunc));
	API_CHK(offsetof(TheForge_DepthStateDesc, stencilBackFail) == offsetof(DepthStateDesc, mStencilBackFail));
	API_CHK(offsetof(TheForge_DepthStateDesc, depthBackFail) == offsetof(DepthStateDesc, mDepthBackFail));
	API_CHK(offsetof(TheForge_DepthStateDesc, stencilBackPass) == offsetof(DepthStateDesc, mStencilBackPass));


	API_CHK(sizeof(TheForge_RasterizerStateDesc) == sizeof(RasterizerStateDesc));
	API_CHK(offsetof(TheForge_RasterizerStateDesc, cullMode) == offsetof(RasterizerStateDesc, mCullMode));
	API_CHK(offsetof(TheForge_RasterizerStateDesc, depthBias) == offsetof(RasterizerStateDesc, mDepthBias));
	API_CHK(offsetof(TheForge_RasterizerStateDesc, slopeScaledDepthBias)
										== offsetof(RasterizerStateDesc, mSlopeScaledDepthBias));
	API_CHK(offsetof(TheForge_RasterizerStateDesc, fillMode) == offsetof(RasterizerStateDesc, mFillMode));
	API_CHK(offsetof(TheForge_RasterizerStateDesc, multiSample) == offsetof(RasterizerStateDesc, mMultiSample));
	API_CHK(offsetof(TheForge_RasterizerStateDesc, scissor) == offsetof(RasterizerStateDesc, mScissor));
	API_CHK(offsetof(TheForge_RasterizerStateDesc, frontFace) == offsetof(RasterizerStateDesc, mFrontFace));

	API_CHK(sizeof(TheForge_DescriptorBinderDesc) == sizeof(DescriptorBinderDesc));
	API_CHK(
			offsetof(TheForge_DescriptorBinderDesc, rootSignature) == offsetof(DescriptorBinderDesc, pRootSignature));
	API_CHK(offsetof(TheForge_DescriptorBinderDesc, maxDynamicUpdatesPerBatch)
										== offsetof(DescriptorBinderDesc, mMaxDynamicUpdatesPerBatch));
	API_CHK(offsetof(TheForge_DescriptorBinderDesc, maxDynamicUpdatesPerDraw)
										== offsetof(DescriptorBinderDesc, mMaxDynamicUpdatesPerDraw));

	API_CHK(sizeof(TheForge_LoadActionsDesc) == sizeof(LoadActionsDesc));
	API_CHK(offsetof(TheForge_LoadActionsDesc, clearColorValues) == offsetof(LoadActionsDesc, mClearColorValues));
	API_CHK(offsetof(TheForge_LoadActionsDesc, loadActionsColor) == offsetof(LoadActionsDesc, mLoadActionsColor));
	API_CHK(offsetof(TheForge_LoadActionsDesc, clearDepth) == offsetof(LoadActionsDesc, mClearDepth));
	API_CHK(offsetof(TheForge_LoadActionsDesc, loadActionDepth) == offsetof(LoadActionsDesc, mLoadActionDepth));
	API_CHK(offsetof(TheForge_LoadActionsDesc, loadActionStencil) == offsetof(LoadActionsDesc, mLoadActionStencil));

	API_CHK(sizeof(TheForge_DescriptorData) == sizeof(DescriptorData));
	API_CHK(offsetof(TheForge_DescriptorData, pName) == offsetof(DescriptorData, pName));
	API_CHK(offsetof(TheForge_DescriptorData, pOffsets) == offsetof(DescriptorData, pOffsets));
	API_CHK(offsetof(TheForge_DescriptorData, pSizes) == offsetof(DescriptorData, pSizes));
	API_CHK(offsetof(TheForge_DescriptorData, UAVMipSlice) == offsetof(DescriptorData, mUAVMipSlice));
	API_CHK(
			offsetof(TheForge_DescriptorData, bindStencilResource) == offsetof(DescriptorData, mBindStencilResource));
	API_CHK(offsetof(TheForge_DescriptorData, pTextures) == offsetof(DescriptorData, ppTextures));
	API_CHK(offsetof(TheForge_DescriptorData, pSamplers) == offsetof(DescriptorData, ppSamplers));
	API_CHK(offsetof(TheForge_DescriptorData, pBuffers) == offsetof(DescriptorData, ppBuffers));
	API_CHK(offsetof(TheForge_DescriptorData, pRootConstant) == offsetof(DescriptorData, pRootConstant));
	API_CHK(
			offsetof(TheForge_DescriptorData, pAccelerationStructures) == offsetof(DescriptorData, ppAccelerationStructures));
	API_CHK(offsetof(TheForge_DescriptorData, count) == offsetof(DescriptorData, mCount));


	API_CHK(sizeof(TheForge_BufferBarrier) == sizeof(BufferBarrier));
	API_CHK(offsetof(TheForge_BufferBarrier, buffer) == offsetof(BufferBarrier, pBuffer));
	API_CHK(offsetof(TheForge_BufferBarrier, newState) == offsetof(BufferBarrier, mNewState));
	API_CHK(offsetof(TheForge_BufferBarrier, split) == offsetof(BufferBarrier, mSplit));


	API_CHK(sizeof(TheForge_TextureBarrier) == sizeof(TextureBarrier));
	API_CHK(offsetof(TheForge_TextureBarrier, texture) == offsetof(TextureBarrier, pTexture));
	API_CHK(offsetof(TheForge_TextureBarrier, newState) == offsetof(TextureBarrier, mNewState));
	API_CHK(offsetof(TheForge_TextureBarrier, split) == offsetof(TextureBarrier, mSplit));

	API_CHK(sizeof(TheForge_QueryPoolDesc) == sizeof(QueryPoolDesc));
	API_CHK(offsetof(TheForge_QueryPoolDesc, type) == offsetof(QueryPoolDesc, mType));
	API_CHK(offsetof(TheForge_QueryPoolDesc, queryCount) == offsetof(QueryPoolDesc, mQueryCount));
	API_CHK(offsetof(TheForge_QueryPoolDesc, nodeIndex) == offsetof(QueryPoolDesc, mNodeIndex));

	API_CHK(sizeof(TheForge_QueryDesc) == sizeof(QueryDesc));
	API_CHK(offsetof(TheForge_QueryDesc, index) == offsetof(QueryDesc, mIndex));

	API_CHK(sizeof(TheForge_CommandSignatureDesc) == sizeof(CommandSignatureDesc));
	API_CHK(offsetof(TheForge_CommandSignatureDesc, cmdPool) == offsetof(CommandSignatureDesc, pCmdPool));
	API_CHK(
			offsetof(TheForge_CommandSignatureDesc, rootSignature) == offsetof(CommandSignatureDesc, pRootSignature));
	API_CHK(
			offsetof(TheForge_CommandSignatureDesc, indirectArgCount) == offsetof(CommandSignatureDesc, mIndirectArgCount));
	API_CHK(offsetof(TheForge_CommandSignatureDesc, pArgDescs) == offsetof(CommandSignatureDesc, pArgDescs));


	API_CHK(sizeof(TheForge_RectDesc) == sizeof(RectDesc));
	API_CHK(offsetof(TheForge_RectDesc, left) == offsetof(RectDesc, left));
	API_CHK(offsetof(TheForge_RectDesc, top) == offsetof(RectDesc, top));
	API_CHK(offsetof(TheForge_RectDesc, right) == offsetof(RectDesc, right));
	API_CHK(offsetof(TheForge_RectDesc, bottom) == offsetof(RectDesc, bottom));

	API_CHK(sizeof(TheForge_SwapChainDesc) == sizeof(SwapChainDesc));
	API_CHK(offsetof(TheForge_SwapChainDesc, pWindow) == offsetof(SwapChainDesc, pWindow));
	API_CHK(offsetof(TheForge_SwapChainDesc, pPresentQueues) == offsetof(SwapChainDesc, ppPresentQueues));
	API_CHK(offsetof(TheForge_SwapChainDesc, presentQueueCount) == offsetof(SwapChainDesc, mPresentQueueCount));
	API_CHK(offsetof(TheForge_SwapChainDesc, imageCount) == offsetof(SwapChainDesc, mImageCount));
	API_CHK(offsetof(TheForge_SwapChainDesc, width) == offsetof(SwapChainDesc, mWidth));
	API_CHK(offsetof(TheForge_SwapChainDesc, height) == offsetof(SwapChainDesc, mHeight));
	API_CHK(offsetof(TheForge_SwapChainDesc, sampleCount) == offsetof(SwapChainDesc, mSampleCount));
	API_CHK(offsetof(TheForge_SwapChainDesc, sampleQuality) == offsetof(SwapChainDesc, mSampleQuality));
	API_CHK(offsetof(TheForge_SwapChainDesc, colorFormat) == offsetof(SwapChainDesc, mColorFormat));
	API_CHK(offsetof(TheForge_SwapChainDesc, colorClearValue) == offsetof(SwapChainDesc, mColorClearValue));
	API_CHK(offsetof(TheForge_SwapChainDesc, enableVsync) == offsetof(SwapChainDesc, mEnableVsync));

	API_CHK(sizeof(TheForge_RawImageData) == sizeof(RawImageData));
	API_CHK(offsetof(TheForge_RawImageData, pRawData) == offsetof(RawImageData, pRawData));
	API_CHK(offsetof(TheForge_RawImageData, format) == offsetof(RawImageData, mFormat));
	API_CHK(offsetof(TheForge_RawImageData, width) == offsetof(RawImageData, mWidth));
	API_CHK(offsetof(TheForge_RawImageData, height) == offsetof(RawImageData, mHeight));
	API_CHK(offsetof(TheForge_RawImageData, depth) == offsetof(RawImageData, mDepth));
	API_CHK(offsetof(TheForge_RawImageData, arraySize) == offsetof(RawImageData, mArraySize));
	API_CHK(offsetof(TheForge_RawImageData, mipLevels) == offsetof(RawImageData, mMipLevels));
	API_CHK(offsetof(TheForge_RawImageData, mipsAfterSlices) == offsetof(RawImageData, mMipsAfterSlices));

	API_CHK(sizeof(TheForge_ShaderReflection) == sizeof(ShaderReflection));
	API_CHK(sizeof(TheForge_ShaderVariable) == sizeof(ShaderVariable));
	API_CHK(sizeof(TheForge_ClearValue) == sizeof(ClearValue));

	API_CHK(sizeof(TheForge_PipelineReflection) == sizeof(PipelineReflection));
	API_CHK(offsetof(TheForge_PipelineReflection, mShaderStages) == offsetof(PipelineReflection,  mShaderStages));
	API_CHK(offsetof(TheForge_PipelineReflection, mStageReflections) == offsetof(PipelineReflection,  mStageReflections));
	API_CHK(offsetof(TheForge_PipelineReflection, mStageReflectionCount) == offsetof(PipelineReflection,  mStageReflectionCount));
	API_CHK(offsetof(TheForge_PipelineReflection, mVertexStageIndex) == offsetof(PipelineReflection,  mVertexStageIndex));
	API_CHK(offsetof(TheForge_PipelineReflection, mHullStageIndex) == offsetof(PipelineReflection,  mHullStageIndex));
	API_CHK(offsetof(TheForge_PipelineReflection, mDomainStageIndex) == offsetof(PipelineReflection,  mDomainStageIndex));
	API_CHK(offsetof(TheForge_PipelineReflection, mGeometryStageIndex) == offsetof(PipelineReflection,  mGeometryStageIndex));
	API_CHK(offsetof(TheForge_PipelineReflection, mPixelStageIndex) == offsetof(PipelineReflection,  mPixelStageIndex));
	API_CHK(offsetof(TheForge_PipelineReflection, pShaderResources) == offsetof(PipelineReflection,  pShaderResources));
	API_CHK(offsetof(TheForge_PipelineReflection, mShaderResourceCount) == offsetof(PipelineReflection,  mShaderResourceCount));
	API_CHK(offsetof(TheForge_PipelineReflection, pVariables) == offsetof(PipelineReflection,  pVariables));
	API_CHK(offsetof(TheForge_PipelineReflection, mVariableCount) == offsetof(PipelineReflection,  mVariableCount));


	API_CHK(sizeof(TheForge_TextureLoadDesc) == sizeof(TextureLoadDesc));
	API_CHK(offsetof(TheForge_TextureLoadDesc,pTexture) == offsetof(TextureLoadDesc,ppTexture));
	API_CHK(offsetof(TheForge_TextureLoadDesc, pDesc) == offsetof(TextureLoadDesc, pDesc));
	API_CHK(offsetof(TheForge_TextureLoadDesc, pFilename) == offsetof(TextureLoadDesc, pFilename) );
	API_CHK(offsetof(TheForge_TextureLoadDesc, mRoot) == offsetof(TextureLoadDesc, mRoot));
	API_CHK(offsetof(TheForge_TextureLoadDesc, mNodeIndex) == offsetof(TextureLoadDesc, mNodeIndex));
	API_CHK(offsetof(TheForge_TextureLoadDesc, pRawImageData) == offsetof(TextureLoadDesc, pRawImageData));
	API_CHK(offsetof(TheForge_TextureLoadDesc, pBinaryImageData) == offsetof(TextureLoadDesc, pBinaryImageData));
	API_CHK(offsetof(TheForge_TextureLoadDesc, mCreationFlag) == offsetof(TextureLoadDesc, mCreationFlag));

	API_CHK(sizeof(TheForge_TextureDesc) == sizeof(TextureDesc));
	API_CHK(offsetof(TheForge_TextureDesc, mFlags) == offsetof(TextureDesc, mFlags));
	API_CHK(offsetof(TheForge_TextureDesc, mWidth) == offsetof(TextureDesc, mWidth));
	API_CHK(offsetof(TheForge_TextureDesc, mHeight) == offsetof(TextureDesc, mHeight));
	API_CHK(offsetof(TheForge_TextureDesc, mDepth) == offsetof(TextureDesc, mDepth));
	API_CHK(offsetof(TheForge_TextureDesc, mArraySize) == offsetof(TextureDesc, mArraySize));
	API_CHK(offsetof(TheForge_TextureDesc, mMipLevels) == offsetof(TextureDesc, mMipLevels));
	API_CHK(offsetof(TheForge_TextureDesc, mSampleCount) == offsetof(TextureDesc, mSampleCount));
	API_CHK(offsetof(TheForge_TextureDesc, mSampleQuality) == offsetof(TextureDesc, mSampleQuality));
	API_CHK(offsetof(TheForge_TextureDesc, mFormat) == offsetof(TextureDesc, mFormat));
	API_CHK(offsetof(TheForge_TextureDesc, mClearValue) == offsetof(TextureDesc, mClearValue));
	API_CHK(offsetof(TheForge_TextureDesc, mStartState) == offsetof(TextureDesc, mStartState));
	API_CHK(offsetof(TheForge_TextureDesc, mDescriptors) == offsetof(TextureDesc, mDescriptors));
	API_CHK(offsetof(TheForge_TextureDesc, pNativeHandle) == offsetof(TextureDesc, pNativeHandle));
	API_CHK(offsetof(TheForge_TextureDesc, pDebugName) == offsetof(TextureDesc, pDebugName));
	API_CHK(offsetof(TheForge_TextureDesc, pSharedNodeIndices) == offsetof(TextureDesc, pSharedNodeIndices));
	API_CHK(offsetof(TheForge_TextureDesc, mSharedNodeIndexCount) == offsetof(TextureDesc, mSharedNodeIndexCount));
	API_CHK(offsetof(TheForge_TextureDesc, mNodeIndex) == offsetof(TextureDesc, mNodeIndex));
	API_CHK(offsetof(TheForge_TextureDesc, mHostVisible) == offsetof(TextureDesc, mHostVisible));

}
#undef API_CHK
