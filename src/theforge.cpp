#include "al2o3_platform/platform.h"
#include "gfx_theforge/theforge.h"
#include "Renderer/IRenderer.h"

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

static void LogFunc(LogType type, const char *m0, const char *m1) {
	switch (type) {
	case LogType::LOG_TYPE_INFO: LOGINFOF("%s %s", m0, m1);
	case LogType::LOG_TYPE_DEBUG: LOGDEBUGF("%s %s", m0, m1);
	case LogType::LOG_TYPE_WARN: LOGWARNINGF("%s %s", m0, m1);
	case LogType::LOG_TYPE_ERROR: LOGERRORF("%s %s", m0, m1);
	}
}

static ShaderStage TheForge_ShaderStageToShaderStage(TheForge_ShaderStage stage) {
#if defined(METAL)
	switch (stage) {
	case TheForge_SS_NONE: return SHADER_STAGE_NONE;
	case TheForge_SS_VERT: return SHADER_STAGE_VERT;
	case TheForge_SS_FRAG: return SHADER_STAGE_FRAG;
	case TheForge_SS_COMP: return SHADER_STAGE_COMP;
	default: LOGERRORF("Shader stage is not supported on Metal backend");
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
	case TheForge_SS_RAYTRACING:	return SHADER_SS_RAYTRACING;
	default:
		LOGERRORF("Shader stage is not supported on Metal backend");
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

static void TheForge_ShaderStageToShaderStage(TheForge_ShaderStageDesc const *src, ShaderStageDesc *dst) {
	dst->mName = src->name;
	dst->mCode = src->code;
	dst->mEntryPoint = src->entryPoint;
	dst->mMacros.resize(src->macrosCount);
	for (uint32_t i = 0u; i < src->macrosCount; ++i) {
		dst->mMacros[i].definition = src->macros[i].definition;
		dst->mMacros[i].value = src->macros[i].value;
	}
}

static void TheForge_BinaryShaderStageToBinaryShaderStage(TheForge_BinaryShaderStageDesc const *src,
																													BinaryShaderStageDesc *dst) {
	dst->pByteCode = src->byteCode;
	dst->mByteCodeSize = src->byteCodeSize;
	dst->mEntryPoint = src->entryPoint;
	dst->mSource = src->source;
}

AL2O3_EXTERN_C TheForge_RendererHandle TheForge_RendererCreate(
		char const *appName,
		TheForge_RendererDesc const *settings) {
	RendererDesc desc{
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

	ImageFormat::Enum tfif = (ImageFormat::Enum) pDesc->format;
	static_assert(sizeof(TheForge_ClearValue) == sizeof(ClearValue));
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
			pDesc->sRGB,
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

	addSampler(renderer, (SamplerDesc *) &pDesc, (Sampler **) pSampler);
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

	ShaderDesc desc{};
	desc.mStages = TheForge_ShaderStageFlagsToShaderStage(pDesc->stages),
			TheForge_ShaderStageToShaderStage(&pDesc->vert, &desc.mVert);
	TheForge_ShaderStageToShaderStage(&pDesc->frag, &desc.mFrag);
	TheForge_ShaderStageToShaderStage(&pDesc->geom, &desc.mGeom);
	TheForge_ShaderStageToShaderStage(&pDesc->hull, &desc.mHull);
	TheForge_ShaderStageToShaderStage(&pDesc->domain, &desc.mDomain);
	TheForge_ShaderStageToShaderStage(&pDesc->comp, &desc.mComp);

	addShader(renderer, &desc, (Shader **) pShader);
}

AL2O3_EXTERN_C void TheForge_AddShaderBinary(TheForge_RendererHandle handle,
																						 const TheForge_BinaryShaderDesc *pDesc,
																						 TheForge_ShaderHandle *pShader) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	BinaryShaderDesc desc{};
	desc.mStages = TheForge_ShaderStageFlagsToShaderStage(pDesc->stages),
			TheForge_BinaryShaderStageToBinaryShaderStage(&pDesc->vert, &desc.mVert);
	TheForge_BinaryShaderStageToBinaryShaderStage(&pDesc->frag, &desc.mFrag);
	TheForge_BinaryShaderStageToBinaryShaderStage(&pDesc->geom, &desc.mGeom);
	TheForge_BinaryShaderStageToBinaryShaderStage(&pDesc->hull, &desc.mHull);
	TheForge_BinaryShaderStageToBinaryShaderStage(&pDesc->domain, &desc.mDomain);
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
																										TheForge_DescriptorBinderHandle *descriptorBinder) {
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
																								 uint64_t *pOffsets) {
	cmdBindVertexBuffer((Cmd *) cmd, bufferCount, (Buffer **) pBuffers, pOffsets);
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
																								TheForge_TextureBarrier *pTextureBarriers,
																								bool batch) {
	cmdResourceBarrier((Cmd *) cmd,
										 bufferBarrierCount,
										 (BufferBarrier *) pBufferBarriers,
										 textureBarrierCount,
										 (TextureBarrier *) pTextureBarriers,
										 batch);
}
AL2O3_EXTERN_C void TheForge_CmdSynchronizeResources(TheForge_CmdHandle cmd,
																										 uint32_t bufferCount,
																										 TheForge_BufferHandle *pBuffers,
																										 uint32_t textureCount,
																										 TheForge_TextureHandle *pTextures,
																										 bool batch) {
	cmdSynchronizeResources((Cmd *) cmd,
													bufferCount,
													(Buffer **) pBuffers,
													textureCount,
													(Texture **) pTextures,
													batch);
}

AL2O3_EXTERN_C void TheForge_CmdFlushBarriers(TheForge_CmdHandle cmd) {
	cmdFlushBarriers((Cmd *) cmd);
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
																					 TheForge_QueryHeapHandle queryHeap,
																					 TheForge_QueryDesc *pQuery) {
	cmdBeginQuery((Cmd *) cmd,
								(QueryHeap *) queryHeap,
								(QueryDesc *) pQuery);
}
AL2O3_EXTERN_C void TheForge_CmdEndQuery(TheForge_CmdHandle cmd,
																				 TheForge_QueryHeapHandle queryHeap,
																				 TheForge_QueryDesc *pQuery) {
	cmdEndQuery((Cmd *) cmd,
							(QueryHeap *) queryHeap,
							(QueryDesc *) pQuery);
}
AL2O3_EXTERN_C void TheForge_CmdResolveQuery(TheForge_CmdHandle cmd,
																						 TheForge_QueryHeapHandle queryHeap,
																						 TheForge_BufferHandle readbackBuffer,
																						 uint32_t startQuery,
																						 uint32_t queryCount) {
	cmdResolveQuery((Cmd *) cmd,
									(QueryHeap *) queryHeap,
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
	getTimestampFrequency((Queue*)queue, pFrequency);
}

AL2O3_EXTERN_C void TheForge_AddIndirectCommandSignature(TheForge_RendererHandle handle,
																												 const TheForge_CommandSignatureDesc *pDesc,
																												 TheForge_CommandSignatureHandle* pCommandSignature) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	addIndirectCommandSignature(renderer,
			(CommandSignatureDesc const*)pDesc,
			(CommandSignature**)pCommandSignature);
}

AL2O3_EXTERN_C void TheForge_RemoveIndirectCommandSignature(TheForge_RendererHandle handle,
																														TheForge_CommandSignatureHandle commandSignature) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	removeIndirectCommandSignature(renderer, (CommandSignature*)commandSignature);
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
AL2O3_EXTERN_C void TheForge_AddSwapChain(TheForge_RendererHandle handle, const TheForge_SwapChainDesc* pDesc, TheForge_SwapChainHandle* pSwapChain) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	addSwapChain(renderer, (SwapChainDesc*)pDesc, (SwapChain**)pSwapChain);
}


AL2O3_EXTERN_C void TheForge_RemoveSwapChain(TheForge_RendererHandle handle, TheForge_SwapChainHandle swapChain) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;
	removeSwapChain(renderer, (SwapChain*)swapChain);
}

AL2O3_EXTERN_C void TheForge_ToggleVSync(TheForge_RendererHandle handle, TheForge_SwapChainHandle* pSwapchain) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	toggleVSync(renderer, (SwapChain**)pSwapchain);
}

AL2O3_EXTERN_C bool TheForge_IsImageFormatSupported(TheForge_ImageFormat format) {
	return isImageFormatSupported((ImageFormat::Enum)format);
}

AL2O3_EXTERN_C TheForge_ImageFormat TheForge_GetRecommendedSwapchainFormat(bool hintHDR) {
	return (TheForge_ImageFormat) getRecommendedSwapchainFormat(hintHDR);
}
AL2O3_EXTERN_C void TheForge_AcquireNextImage(TheForge_RendererHandle handle,
		TheForge_SwapChainHandle swapChain,
		TheForge_SemaphoreHandle signalSemaphore,
		TheForge_FenceHandle fence,
		uint32_t* pImageIndex) {
	auto renderer = (Renderer *) handle;
	if (!renderer)
		return;

	acquireNextImage(renderer, (SwapChain*) swapChain, (Semaphore*)signalSemaphore, (Fence*)fence, pImageIndex);
}
AL2O3_EXTERN_C void TheForge_QueuePresent(TheForge_QueueHandle queue,
		TheForge_SwapChainHandle swapChain,
		uint32_t swapChainImageIndex,
		uint32_t waitSemaphoreCount,
		TheForge_SemaphoreHandle* pWaitSemaphores) {

	queuePresent((Queue*)queue, (SwapChain*)swapChain, swapChainImageIndex, waitSemaphoreCount, (Semaphore**)pWaitSemaphores);
}

AL2O3_EXTERN_C TheForge_RenderTargetHandle TheForge_SwapChainGetRenderTarget(TheForge_SwapChainHandle swapChain, int index) {
	return (TheForge_RenderTargetHandle) ((SwapChain*)swapChain)->ppSwapchainRenderTargets[index];
}

AL2O3_EXTERN_C TheForge_TextureHandle TheForge_RenderTargetGetTexture(TheForge_RenderTargetHandle renderTarget) {
	return (TheForge_TextureHandle) ((RenderTarget*)renderTarget)->pTexture;
}

AL2O3_EXTERN_C TheForge_RenderTargetDesc const* TheForge_RenderTargetGetDesc(TheForge_RenderTargetHandle renderTarget) {
	return (TheForge_RenderTargetDesc const*) &((RenderTarget*)renderTarget)->mDesc;
}


static void API_CHECK() {
	static_assert(sizeof(TheForge_ComputePipelineDesc) == sizeof(ComputePipelineDesc));
	static_assert(sizeof(TheForge_GraphicsPipelineDesc) == sizeof(GraphicsPipelineDesc));
	static_assert(sizeof(TheForge_RaytracingPipelineDesc) == sizeof(RaytracingPipelineDesc));

	static_assert(offsetof(TheForge_ComputePipelineDesc, shaderProgram) == offsetof(ComputePipelineDesc, pShaderProgram));
	static_assert(offsetof(TheForge_ComputePipelineDesc, rootSignature) == offsetof(ComputePipelineDesc, pRootSignature));

	static_assert(
			offsetof(TheForge_GraphicsPipelineDesc, shaderProgram) == offsetof(GraphicsPipelineDesc, pShaderProgram));
	static_assert(
			offsetof(TheForge_GraphicsPipelineDesc, rootSignature) == offsetof(GraphicsPipelineDesc, pRootSignature));
	static_assert(
			offsetof(TheForge_GraphicsPipelineDesc, pVertexLayout) == offsetof(GraphicsPipelineDesc, pVertexLayout));
	static_assert(offsetof(TheForge_GraphicsPipelineDesc, blendState) == offsetof(GraphicsPipelineDesc, pBlendState));
	static_assert(offsetof(TheForge_GraphicsPipelineDesc, depthState) == offsetof(GraphicsPipelineDesc, pDepthState));
	static_assert(
			offsetof(TheForge_GraphicsPipelineDesc, rasterizerState) == offsetof(GraphicsPipelineDesc, pRasterizerState));
	static_assert(
			offsetof(TheForge_GraphicsPipelineDesc, pColorFormats) == offsetof(GraphicsPipelineDesc, pColorFormats));
	static_assert(offsetof(TheForge_GraphicsPipelineDesc, pSrgbValues) == offsetof(GraphicsPipelineDesc, pSrgbValues));
	static_assert(
			offsetof(TheForge_GraphicsPipelineDesc, renderTargetCount) == offsetof(GraphicsPipelineDesc, mRenderTargetCount));
	static_assert(offsetof(TheForge_GraphicsPipelineDesc, sampleCount) == offsetof(GraphicsPipelineDesc, mSampleCount));
	static_assert(
			offsetof(TheForge_GraphicsPipelineDesc, sampleQuality) == offsetof(GraphicsPipelineDesc, mSampleQuality));
	static_assert(offsetof(TheForge_GraphicsPipelineDesc, depthStencilFormat)
										== offsetof(GraphicsPipelineDesc, mDepthStencilFormat));
	static_assert(
			offsetof(TheForge_GraphicsPipelineDesc, primitiveTopo) == offsetof(GraphicsPipelineDesc, mPrimitiveTopo));

	static_assert(offsetof(TheForge_RaytracingPipelineDesc, raytracing) == offsetof(RaytracingPipelineDesc, pRaytracing));
	static_assert(offsetof(TheForge_RaytracingPipelineDesc, globalRootSignature)
										== offsetof(RaytracingPipelineDesc, pGlobalRootSignature));
	static_assert(
			offsetof(TheForge_RaytracingPipelineDesc, rayGenShader) == offsetof(RaytracingPipelineDesc, pRayGenShader));
	static_assert(offsetof(TheForge_RaytracingPipelineDesc, rayGenRootSignature)
										== offsetof(RaytracingPipelineDesc, pRayGenRootSignature));
	static_assert(
			offsetof(TheForge_RaytracingPipelineDesc, pMissShaders) == offsetof(RaytracingPipelineDesc, ppMissShaders));
	static_assert(offsetof(TheForge_RaytracingPipelineDesc, pMissRootSignatures)
										== offsetof(RaytracingPipelineDesc, ppMissRootSignatures));
	static_assert(offsetof(TheForge_RaytracingPipelineDesc, hitGroups) == offsetof(RaytracingPipelineDesc, pHitGroups));
	static_assert(
			offsetof(TheForge_RaytracingPipelineDesc, missShaderCount) == offsetof(RaytracingPipelineDesc, mMissShaderCount));
	static_assert(
			offsetof(TheForge_RaytracingPipelineDesc, hitGroupCount) == offsetof(RaytracingPipelineDesc, mHitGroupCount));
	static_assert(
			offsetof(TheForge_RaytracingPipelineDesc, payloadSize) == offsetof(RaytracingPipelineDesc, mPayloadSize));
	static_assert(
			offsetof(TheForge_RaytracingPipelineDesc, attributeSize) == offsetof(RaytracingPipelineDesc, mAttributeSize));
	static_assert(offsetof(TheForge_RaytracingPipelineDesc, maxTraceRecursionDepth)
										== offsetof(RaytracingPipelineDesc, mMaxTraceRecursionDepth));
	static_assert(
			offsetof(TheForge_RaytracingPipelineDesc, maxRaysCount) == offsetof(RaytracingPipelineDesc, mMaxRaysCount));

	static_assert(sizeof(TheForge_VertexLayout) == sizeof(VertexLayout));
	static_assert(offsetof(TheForge_VertexLayout, attribCount) == offsetof(VertexLayout, mAttribCount));
	static_assert(offsetof(TheForge_VertexLayout, attribs) == offsetof(VertexLayout, mAttribs));
	static_assert(offsetof(TheForge_VertexAttrib, semantic) == offsetof(VertexAttrib, mSemantic));
	static_assert(offsetof(TheForge_VertexAttrib, semanticNameLength) == offsetof(VertexAttrib, mSemanticNameLength));
	static_assert(offsetof(TheForge_VertexAttrib, semanticName) == offsetof(VertexAttrib, mSemanticName));
	static_assert(offsetof(TheForge_VertexAttrib, format) == offsetof(VertexAttrib, mFormat));
	static_assert(offsetof(TheForge_VertexAttrib, location) == offsetof(VertexAttrib, mLocation));
	static_assert(offsetof(TheForge_VertexAttrib, offset) == offsetof(VertexAttrib, mOffset));
	static_assert(offsetof(TheForge_VertexAttrib, rate) == offsetof(VertexAttrib, mRate));

	static_assert(sizeof(TheForge_RootSignatureDesc) == sizeof(RootSignatureDesc));
	static_assert(offsetof(TheForge_RootSignatureDesc, pShaders) == offsetof(RootSignatureDesc, ppShaders));
	static_assert(offsetof(TheForge_RootSignatureDesc, shaderCount) == offsetof(RootSignatureDesc, mShaderCount));
	static_assert(
			offsetof(TheForge_RootSignatureDesc, maxBindlessTextures) == offsetof(RootSignatureDesc, mMaxBindlessTextures));
	static_assert(
			offsetof(TheForge_RootSignatureDesc, pStaticSamplerNames) == offsetof(RootSignatureDesc, ppStaticSamplerNames));
	static_assert(offsetof(TheForge_RootSignatureDesc, pStaticSamplers) == offsetof(RootSignatureDesc, ppStaticSamplers));
	static_assert(
			offsetof(TheForge_RootSignatureDesc, staticSamplerCount) == offsetof(RootSignatureDesc, mStaticSamplerCount));
	static_assert(offsetof(TheForge_RootSignatureDesc, flags) == offsetof(RootSignatureDesc, mFlags));

	static_assert(sizeof(TheForge_SamplerDesc) == sizeof(SamplerDesc));
	static_assert(offsetof(TheForge_SamplerDesc, minFilter) == offsetof(SamplerDesc, mMinFilter));
	static_assert(offsetof(TheForge_SamplerDesc, magFilter) == offsetof(SamplerDesc, mMagFilter));
	static_assert(offsetof(TheForge_SamplerDesc, mipMapMode) == offsetof(SamplerDesc, mMipMapMode));
	static_assert(offsetof(TheForge_SamplerDesc, addressU) == offsetof(SamplerDesc, mAddressU));
	static_assert(offsetof(TheForge_SamplerDesc, addressV) == offsetof(SamplerDesc, mAddressV));
	static_assert(offsetof(TheForge_SamplerDesc, addressW) == offsetof(SamplerDesc, mAddressW));
	static_assert(offsetof(TheForge_SamplerDesc, mipLosBias) == offsetof(SamplerDesc, mMipLosBias));
	static_assert(offsetof(TheForge_SamplerDesc, maxAnisotropy) == offsetof(SamplerDesc, mMaxAnisotropy));
	static_assert(offsetof(TheForge_SamplerDesc, compareFunc) == offsetof(SamplerDesc, mCompareFunc));

	static_assert(sizeof(TheForge_QueueDesc) == sizeof(QueueDesc));
	static_assert(offsetof(TheForge_QueueDesc, flags) == offsetof(QueueDesc, mFlag));
	static_assert(offsetof(TheForge_QueueDesc, priority) == offsetof(QueueDesc, mPriority));
	static_assert(offsetof(TheForge_QueueDesc, type) == offsetof(QueueDesc, mType));
	static_assert(offsetof(TheForge_QueueDesc, nodeIndex) == offsetof(QueueDesc, mNodeIndex));

	static_assert(sizeof(TheForge_BlendStateDesc) == sizeof(BlendStateDesc));
	static_assert(offsetof(TheForge_BlendStateDesc, srcFactors) == offsetof(BlendStateDesc, mSrcFactors));
	static_assert(offsetof(TheForge_BlendStateDesc, dstFactors) == offsetof(BlendStateDesc, mDstFactors));
	static_assert(offsetof(TheForge_BlendStateDesc, srcAlphaFactors) == offsetof(BlendStateDesc, mSrcAlphaFactors));
	static_assert(offsetof(TheForge_BlendStateDesc, dstAlphaFactors) == offsetof(BlendStateDesc, mDstAlphaFactors));
	static_assert(offsetof(TheForge_BlendStateDesc, blendModes) == offsetof(BlendStateDesc, mBlendModes));
	static_assert(offsetof(TheForge_BlendStateDesc, blendAlphaModes) == offsetof(BlendStateDesc, mBlendAlphaModes));
	static_assert(offsetof(TheForge_BlendStateDesc, masks) == offsetof(BlendStateDesc, mMasks));
	static_assert(offsetof(TheForge_BlendStateDesc, renderTargetMask) == offsetof(BlendStateDesc, mRenderTargetMask));
	static_assert(offsetof(TheForge_BlendStateDesc, alphaToCoverage) == offsetof(BlendStateDesc, mAlphaToCoverage));
	static_assert(offsetof(TheForge_BlendStateDesc, independentBlend) == offsetof(BlendStateDesc, mIndependentBlend));

	static_assert(sizeof(TheForge_DepthStateDesc) == sizeof(DepthStateDesc));
	static_assert(offsetof(TheForge_DepthStateDesc, depthTest) == offsetof(DepthStateDesc, mDepthTest));
	static_assert(offsetof(TheForge_DepthStateDesc, depthWrite) == offsetof(DepthStateDesc, mDepthWrite));
	static_assert(offsetof(TheForge_DepthStateDesc, depthFunc) == offsetof(DepthStateDesc, mDepthFunc));
	static_assert(offsetof(TheForge_DepthStateDesc, stencilTest) == offsetof(DepthStateDesc, mStencilTest));
	static_assert(offsetof(TheForge_DepthStateDesc, stencilReadMask) == offsetof(DepthStateDesc, mStencilReadMask));
	static_assert(offsetof(TheForge_DepthStateDesc, stencilWriteMask) == offsetof(DepthStateDesc, mStencilWriteMask));
	static_assert(offsetof(TheForge_DepthStateDesc, stencilFrontFunc) == offsetof(DepthStateDesc, mStencilFrontFunc));
	static_assert(offsetof(TheForge_DepthStateDesc, stencilFrontFail) == offsetof(DepthStateDesc, mStencilFrontFail));
	static_assert(offsetof(TheForge_DepthStateDesc, depthFrontFail) == offsetof(DepthStateDesc, mDepthFrontFail));
	static_assert(offsetof(TheForge_DepthStateDesc, stencilFrontPass) == offsetof(DepthStateDesc, mStencilFrontPass));
	static_assert(offsetof(TheForge_DepthStateDesc, stencilBackFunc) == offsetof(DepthStateDesc, mStencilBackFunc));
	static_assert(offsetof(TheForge_DepthStateDesc, stencilBackFail) == offsetof(DepthStateDesc, mStencilBackFail));
	static_assert(offsetof(TheForge_DepthStateDesc, depthBackFail) == offsetof(DepthStateDesc, mDepthBackFail));
	static_assert(offsetof(TheForge_DepthStateDesc, stencilBackPass) == offsetof(DepthStateDesc, mStencilBackPass));


	static_assert(sizeof(TheForge_RasterizerStateDesc) == sizeof(RasterizerStateDesc));
	static_assert(offsetof(TheForge_RasterizerStateDesc, cullMode) == offsetof(RasterizerStateDesc, mCullMode));
	static_assert(offsetof(TheForge_RasterizerStateDesc, depthBias) == offsetof(RasterizerStateDesc, mDepthBias));
	static_assert(offsetof(TheForge_RasterizerStateDesc, slopeScaledDepthBias)
										== offsetof(RasterizerStateDesc, mSlopeScaledDepthBias));
	static_assert(offsetof(TheForge_RasterizerStateDesc, fillMode) == offsetof(RasterizerStateDesc, mFillMode));
	static_assert(offsetof(TheForge_RasterizerStateDesc, multiSample) == offsetof(RasterizerStateDesc, mMultiSample));
	static_assert(offsetof(TheForge_RasterizerStateDesc, scissor) == offsetof(RasterizerStateDesc, mScissor));
	static_assert(offsetof(TheForge_RasterizerStateDesc, frontFace) == offsetof(RasterizerStateDesc, mFrontFace));

	static_assert(sizeof(TheForge_DescriptorBinderDesc) == sizeof(DescriptorBinderDesc));
	static_assert(
			offsetof(TheForge_DescriptorBinderDesc, rootSignature) == offsetof(DescriptorBinderDesc, pRootSignature));
	static_assert(offsetof(TheForge_DescriptorBinderDesc, maxDynamicUpdatesPerBatch)
										== offsetof(DescriptorBinderDesc, mMaxDynamicUpdatesPerBatch));
	static_assert(offsetof(TheForge_DescriptorBinderDesc, maxDynamicUpdatesPerDraw)
										== offsetof(DescriptorBinderDesc, mMaxDynamicUpdatesPerDraw));

	static_assert(sizeof(TheForge_LoadActionsDesc) == sizeof(LoadActionsDesc));
	static_assert(offsetof(TheForge_LoadActionsDesc, clearColorValues) == offsetof(LoadActionsDesc, mClearColorValues));
	static_assert(offsetof(TheForge_LoadActionsDesc, loadActionsColor) == offsetof(LoadActionsDesc, mLoadActionsColor));
	static_assert(offsetof(TheForge_LoadActionsDesc, clearDepth) == offsetof(LoadActionsDesc, mClearDepth));
	static_assert(offsetof(TheForge_LoadActionsDesc, loadActionDepth) == offsetof(LoadActionsDesc, mLoadActionDepth));
	static_assert(offsetof(TheForge_LoadActionsDesc, loadActionStencil) == offsetof(LoadActionsDesc, mLoadActionStencil));

	static_assert(sizeof(TheForge_DescriptorData) == sizeof(DescriptorData));
	static_assert(offsetof(TheForge_DescriptorData, pName) == offsetof(DescriptorData, pName));
	static_assert(offsetof(TheForge_DescriptorData, pOffsets) == offsetof(DescriptorData, pOffsets));
	static_assert(offsetof(TheForge_DescriptorData, pSizes) == offsetof(DescriptorData, pSizes));
	static_assert(offsetof(TheForge_DescriptorData, UAVMipSlice) == offsetof(DescriptorData, mUAVMipSlice));
	static_assert(
			offsetof(TheForge_DescriptorData, bindStencilResource) == offsetof(DescriptorData, mBindStencilResource));
	static_assert(offsetof(TheForge_DescriptorData, pTextures) == offsetof(DescriptorData, ppTextures));
	static_assert(offsetof(TheForge_DescriptorData, pSamplers) == offsetof(DescriptorData, ppSamplers));
	static_assert(offsetof(TheForge_DescriptorData, pBuffers) == offsetof(DescriptorData, ppBuffers));
	static_assert(offsetof(TheForge_DescriptorData, pRootConstant) == offsetof(DescriptorData, pRootConstant));
	static_assert(
			offsetof(TheForge_DescriptorData, pAccelerationStructures) == offsetof(DescriptorData, ppAccelerationStructures));
	static_assert(offsetof(TheForge_DescriptorData, count) == offsetof(DescriptorData, mCount));


	static_assert(sizeof(TheForge_BufferBarrier) == sizeof(BufferBarrier));
	static_assert(offsetof(TheForge_BufferBarrier, buffer) == offsetof(BufferBarrier, pBuffer));
	static_assert(offsetof(TheForge_BufferBarrier, newState) == offsetof(BufferBarrier, mNewState));
	static_assert(offsetof(TheForge_BufferBarrier, split) == offsetof(BufferBarrier, mSplit));


	static_assert(sizeof(TheForge_TextureBarrier) == sizeof(TextureBarrier));
	static_assert(offsetof(TheForge_TextureBarrier, texture) == offsetof(TextureBarrier, pTexture));
	static_assert(offsetof(TheForge_TextureBarrier, newState) == offsetof(TextureBarrier, mNewState));
	static_assert(offsetof(TheForge_TextureBarrier, split) == offsetof(TextureBarrier, mSplit));

	static_assert(sizeof(TheForge_QueryHeapDesc) == sizeof(QueryHeapDesc));
	static_assert(offsetof(TheForge_QueryHeapDesc, type) == offsetof(QueryHeapDesc, mType));
	static_assert(offsetof(TheForge_QueryHeapDesc, queryCount) == offsetof(QueryHeapDesc, mQueryCount));
	static_assert(offsetof(TheForge_QueryHeapDesc, nodeIndex) == offsetof(QueryHeapDesc, mNodeIndex));

	static_assert(sizeof(TheForge_QueryDesc) == sizeof(QueryDesc));
	static_assert(offsetof(TheForge_QueryDesc, index) == offsetof(QueryDesc, mIndex));

	static_assert(sizeof(TheForge_CommandSignatureDesc) == sizeof(CommandSignatureDesc));
	static_assert(offsetof(TheForge_CommandSignatureDesc, cmdPool) == offsetof(CommandSignatureDesc, pCmdPool));
	static_assert(offsetof(TheForge_CommandSignatureDesc, rootSignature) == offsetof(CommandSignatureDesc, pRootSignature));
	static_assert(offsetof(TheForge_CommandSignatureDesc, indirectArgCount) == offsetof(CommandSignatureDesc, mIndirectArgCount));
	static_assert(offsetof(TheForge_CommandSignatureDesc, pArgDescs) == offsetof(CommandSignatureDesc, pArgDescs));


	static_assert(sizeof(TheForge_RectDesc) == sizeof(RectDesc));
	static_assert(offsetof(TheForge_RectDesc, left) == offsetof(RectDesc, left));
	static_assert(offsetof(TheForge_RectDesc, top) == offsetof(RectDesc, top));
	static_assert(offsetof(TheForge_RectDesc, right) == offsetof(RectDesc, right));
	static_assert(offsetof(TheForge_RectDesc, bottom) == offsetof(RectDesc, bottom));

	static_assert(sizeof(TheForge_WindowsDesc) == sizeof(WindowsDesc));
	static_assert(offsetof(TheForge_WindowsDesc, handle) == offsetof(WindowsDesc, handle));
	static_assert(offsetof(TheForge_WindowsDesc, windowedRect) == offsetof(WindowsDesc, windowedRect));
	static_assert(offsetof(TheForge_WindowsDesc, fullscreenRect) == offsetof(WindowsDesc, fullscreenRect));
	static_assert(offsetof(TheForge_WindowsDesc, clientRect) == offsetof(WindowsDesc, clientRect));
	static_assert(offsetof(TheForge_WindowsDesc, fullScreen) == offsetof(WindowsDesc, fullScreen));
	static_assert(offsetof(TheForge_WindowsDesc, windowsFlags) == offsetof(WindowsDesc, windowsFlags));
	static_assert(offsetof(TheForge_WindowsDesc, bigIcon) == offsetof(WindowsDesc, bigIcon));
	static_assert(offsetof(TheForge_WindowsDesc, smallIcon) == offsetof(WindowsDesc, smallIcon));
	static_assert(offsetof(TheForge_WindowsDesc, cursorTracked) == offsetof(WindowsDesc, cursorTracked));
	static_assert(offsetof(TheForge_WindowsDesc, iconified) == offsetof(WindowsDesc, iconified));
	static_assert(offsetof(TheForge_WindowsDesc, maximized) == offsetof(WindowsDesc, maximized));
	static_assert(offsetof(TheForge_WindowsDesc, minimized) == offsetof(WindowsDesc, minimized));
	static_assert(offsetof(TheForge_WindowsDesc, visible) == offsetof(WindowsDesc, visible));
	static_assert(offsetof(TheForge_WindowsDesc, lastCursorPosX) == offsetof(WindowsDesc, lastCursorPosX));
	static_assert(offsetof(TheForge_WindowsDesc, lastCursorPosY) == offsetof(WindowsDesc, lastCursorPosY));

	static_assert(sizeof(TheForge_SwapChainDesc) == sizeof(SwapChainDesc));
	static_assert(offsetof(TheForge_SwapChainDesc, pWindow) == offsetof(SwapChainDesc, pWindow));
	static_assert(offsetof(TheForge_SwapChainDesc, pPresentQueues) == offsetof(SwapChainDesc, ppPresentQueues));
	static_assert(offsetof(TheForge_SwapChainDesc, presentQueueCount) == offsetof(SwapChainDesc, mPresentQueueCount));
	static_assert(offsetof(TheForge_SwapChainDesc, imageCount) == offsetof(SwapChainDesc, mImageCount));
	static_assert(offsetof(TheForge_SwapChainDesc, width) == offsetof(SwapChainDesc, mWidth));
	static_assert(offsetof(TheForge_SwapChainDesc, height) == offsetof(SwapChainDesc, mHeight));
	static_assert(offsetof(TheForge_SwapChainDesc, sampleCount) == offsetof(SwapChainDesc, mSampleCount));
	static_assert(offsetof(TheForge_SwapChainDesc, sampleQuality) == offsetof(SwapChainDesc, mSampleQuality));
	static_assert(offsetof(TheForge_SwapChainDesc, colorFormat) == offsetof(SwapChainDesc, mColorFormat));
	static_assert(offsetof(TheForge_SwapChainDesc, colorClearValue) == offsetof(SwapChainDesc, mColorClearValue));
	static_assert(offsetof(TheForge_SwapChainDesc, srgb) == offsetof(SwapChainDesc, mSrgb));
	static_assert(offsetof(TheForge_SwapChainDesc, enableVsync) == offsetof(SwapChainDesc, mEnableVsync));

}
