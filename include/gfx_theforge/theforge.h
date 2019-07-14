#pragma once
#ifndef GFX_THEFORGE_THEFORGE_H_
#define GFX_THEFORGE_THEFORGE_H_

#include "al2o3_platform/platform.h"
#include "gfx_theforge/enums.h"
#include "gfx_theforge/structs.h"
#include "gfx_theforge/resourceloader.h"

AL2O3_EXTERN_C TheForge_RendererHandle TheForge_RendererCreate(char const *appName,
																															 TheForge_RendererDesc const *settings);
AL2O3_EXTERN_C void TheForge_RendererDestroy(TheForge_RendererHandle handle);

AL2O3_EXTERN_C void TheForge_AddFence(TheForge_RendererHandle handle, TheForge_FenceHandle *pFence);
AL2O3_EXTERN_C void TheForge_RemoveFence(TheForge_RendererHandle handle, TheForge_FenceHandle fence);
AL2O3_EXTERN_C void TheForge_AddSemaphore(TheForge_RendererHandle handle, TheForge_SemaphoreHandle *pSemaphore);
AL2O3_EXTERN_C void TheForge_RemoveSemaphore(TheForge_RendererHandle handle, TheForge_SemaphoreHandle semaphore);

AL2O3_EXTERN_C void TheForge_AddQueue(TheForge_RendererHandle handle,
																			TheForge_QueueDesc *pQDesc,
																			TheForge_QueueHandle *pQueue);
AL2O3_EXTERN_C void TheForge_RemoveQueue(TheForge_QueueHandle queue);

AL2O3_EXTERN_C void TheForge_AddCmdPool(TheForge_RendererHandle handle,
																				TheForge_QueueHandle queue,
																				bool transient,
																				TheForge_CmdPoolHandle *pCmdPool);
AL2O3_EXTERN_C void TheForge_RemoveCmdPool(TheForge_RendererHandle handle, TheForge_CmdPoolHandle cmdPool);
AL2O3_EXTERN_C void TheForge_AddCmd(TheForge_CmdPoolHandle cmdPool, bool secondary, TheForge_CmdHandle *pCmd);
AL2O3_EXTERN_C void TheForge_RemoveCmd(TheForge_CmdPoolHandle cmdPool, TheForge_CmdHandle cmd);
AL2O3_EXTERN_C void TheForge_AddCmd_n(TheForge_CmdPoolHandle cmdPool,
																			bool secondary,
																			uint32_t cmdCount,
																			TheForge_CmdHandle **ppCmds);
AL2O3_EXTERN_C void TheForge_RemoveCmd_n(TheForge_CmdPoolHandle cmdPool, uint32_t cmdCount, TheForge_CmdHandle *pCmds);

AL2O3_EXTERN_C void TheForge_AddRenderTarget(TheForge_RendererHandle handle,
																						 const TheForge_RenderTargetDesc *pDesc,
																						 TheForge_RenderTargetHandle *pRenderTarget);
AL2O3_EXTERN_C void TheForge_RemoveRenderTarget(TheForge_RendererHandle handle,
																								TheForge_RenderTargetHandle renderTarget);

AL2O3_EXTERN_C void TheForge_AddSampler(TheForge_RendererHandle handle,
																				const TheForge_SamplerDesc *pDesc,
																				TheForge_SamplerHandle *pSampler);
AL2O3_EXTERN_C void TheForge_RemoveSampler(TheForge_RendererHandle handle, TheForge_SamplerHandle sampler);

AL2O3_EXTERN_C void TheForge_AddShader(TheForge_RendererHandle handle,
																			 const TheForge_ShaderDesc *p_desc,
																			 TheForge_ShaderHandle *pShader);
AL2O3_EXTERN_C void TheForge_AddShaderBinary(TheForge_RendererHandle handle,
																						 const TheForge_BinaryShaderDesc *p_desc,
																						 TheForge_ShaderHandle *pShader);
AL2O3_EXTERN_C void TheForge_RemoveShader(TheForge_RendererHandle handle, TheForge_ShaderHandle pShader);

AL2O3_EXTERN_C void TheForge_AddRootSignature(TheForge_RendererHandle handle,
																							const TheForge_RootSignatureDesc *pRootDesc,
																							TheForge_RootSignatureHandle *ppRootSignature);
AL2O3_EXTERN_C void TheForge_RemoveRootSignature(TheForge_RendererHandle handle,
																								 TheForge_RootSignatureHandle rootSignature);
AL2O3_EXTERN_C void TheForge_AddPipeline(TheForge_RendererHandle handle,
																				 const TheForge_PipelineDesc *pPipelineDesc,
																				 TheForge_PipelineHandle *pPipeline);

AL2O3_EXTERN_C void TheForge_RemovePipeline(TheForge_RendererHandle handle, TheForge_PipelineHandle pipeline);
AL2O3_EXTERN_C void TheForge_AddDescriptorBinder(TheForge_RendererHandle handle,
																								 uint32_t gpuIndex,
																								 uint32_t descCount,
																								 const TheForge_DescriptorBinderDesc *pDescs,
																								 TheForge_DescriptorBinderHandle *pDescriptorBinder);
AL2O3_EXTERN_C void TheForge_RemoveDescriptorBinder(TheForge_RendererHandle handle,
																										TheForge_DescriptorBinderHandle *descriptorBinder);
AL2O3_EXTERN_C void TheForge_AddBlendState(TheForge_RendererHandle handle,
																					 const TheForge_BlendStateDesc *pDesc,
																					 TheForge_BlendStateHandle *pBlendState);
AL2O3_EXTERN_C void TheForge_RemoveBlendState(TheForge_RendererHandle handle, TheForge_BlendStateHandle pBlendState);

AL2O3_EXTERN_C void TheForge_AddDepthState(TheForge_RendererHandle handle,
																					 const TheForge_DepthStateDesc *pDesc,
																					 TheForge_DepthStateHandle *pDepthState);
AL2O3_EXTERN_C void TheForge_RemoveDepthState(TheForge_RendererHandle handle, TheForge_DepthStateHandle pDepthState);

AL2O3_EXTERN_C void TheForge_AddRasterizerState(TheForge_RendererHandle handle,
																								const TheForge_RasterizerStateDesc *pDesc,
																								TheForge_RasterizerStateHandle *pRasterizerState);
AL2O3_EXTERN_C void TheForge_RemoveRasterizerState(TheForge_RendererHandle handle,
																									 TheForge_RasterizerStateHandle pRasterizerState);

AL2O3_EXTERN_C void TheForge_AddQueryHeap(TheForge_RendererHandle handle,
																					const TheForge_QueryHeapDesc *pDesc,
																					TheForge_QueryHeapHandle *pQueryHeap);
AL2O3_EXTERN_C void TheForge_RemoveQueryHeap(TheForge_RendererHandle handle, TheForge_QueryHeapHandle queryHeap);


AL2O3_EXTERN_C void TheForge_BeginCmd(TheForge_CmdHandle cmd);
AL2O3_EXTERN_C void TheForge_EndCmd(TheForge_CmdHandle cmd);
AL2O3_EXTERN_C void TheForge_CmdBindRenderTargets(TheForge_CmdHandle cmd,
																									uint32_t renderTargetCount,
																									TheForge_RenderTargetHandle *pRenderTargets,
																									TheForge_RenderTargetHandle depthStencil,
																									const TheForge_LoadActionsDesc *loadActions,
																									uint32_t *pColorArraySlices,
																									uint32_t *pColorMipSlices,
																									uint32_t depthArraySlice,
																									uint32_t depthMipSlice);
AL2O3_EXTERN_C void TheForge_CmdSetViewport(TheForge_CmdHandle cmd,
																						float x,
																						float y,
																						float width,
																						float height,
																						float minDepth,
																						float maxDepth);
AL2O3_EXTERN_C void TheForge_CmdSetScissor(TheForge_CmdHandle cmd,
																					 uint32_t x,
																					 uint32_t y,
																					 uint32_t width,
																					 uint32_t height);
AL2O3_EXTERN_C void TheForge_CmdBindPipeline(TheForge_CmdHandle cmd, TheForge_PipelineHandle pipeline);
AL2O3_EXTERN_C void TheForge_CmdBindDescriptors(TheForge_CmdHandle cmd,
																								TheForge_DescriptorBinderHandle descriptorBinder,
																								TheForge_RootSignatureHandle rootSignature,
																								uint32_t numDescriptors,
																								TheForge_DescriptorData *pDescParams);
AL2O3_EXTERN_C void TheForge_CmdBindIndexBuffer(TheForge_CmdHandle cmd, TheForge_BufferHandle buffer, uint64_t offset);
AL2O3_EXTERN_C void TheForge_CmdBindVertexBuffer(TheForge_CmdHandle cmd,
																								 uint32_t bufferCount,
																								 TheForge_BufferHandle *pBuffers,
																								 uint64_t *pOffsets);
AL2O3_EXTERN_C void TheForge_CmdDraw(TheForge_CmdHandle cmd, uint32_t vertexCount, uint32_t firstVertex);
AL2O3_EXTERN_C void TheForge_CmdDrawInstanced(TheForge_CmdHandle cmd,
																							uint32_t vertexCount,
																							uint32_t firstVertex,
																							uint32_t instanceCount,
																							uint32_t firstInstance);
AL2O3_EXTERN_C void TheForge_CmdDrawIndexed(TheForge_CmdHandle cmd,
																						uint32_t indexCount,
																						uint32_t firstIndex,
																						uint32_t firstVertex);
AL2O3_EXTERN_C void TheForge_CmdDrawIndexedInstanced(TheForge_CmdHandle cmd,
																										 uint32_t indexCount,
																										 uint32_t firstIndex,
																										 uint32_t instanceCount,
																										 uint32_t firstVertex,
																										 uint32_t firstInstance);
AL2O3_EXTERN_C void TheForge_CmdDispatch(TheForge_CmdHandle cmd,
																				 uint32_t groupCountX,
																				 uint32_t groupCountY,
																				 uint32_t groupCountZ);
AL2O3_EXTERN_C void TheForge_CmdResourceBarrier(TheForge_CmdHandle cmd,
																								uint32_t bufferBarrierCount,
																								TheForge_BufferBarrier *pBufferBarriers,
																								uint32_t textureBarrierCount,
																								TheForge_TextureBarrier *pTextureBarriers,
																								bool batch);
AL2O3_EXTERN_C void TheForge_CmdSynchronizeResources(TheForge_CmdHandle cmd,
																										 uint32_t bufferCount,
																										 TheForge_BufferHandle *pBuffers, uint32_t textureCount,
																										 TheForge_TextureHandle *pTextures,
																										 bool batch);
AL2O3_EXTERN_C void TheForge_CmdFlushBarriers(TheForge_CmdHandle cmd);

AL2O3_EXTERN_C void TheForge_CmdBeginDebugMarker(TheForge_CmdHandle cmd, float r, float g, float b, const char *pName);
AL2O3_EXTERN_C void TheForge_CmdExecuteIndirect(TheForge_CmdHandle cmd,
																								TheForge_CommandSignatureHandle commandSignature,
																								uint32_t maxCommandCount,
																								TheForge_BufferHandle indirectBuffer,
																								uint64_t bufferOffset,
																								TheForge_BufferHandle counterBuffer,
																								uint64_t counterBufferOffset);
AL2O3_EXTERN_C void TheForge_CmdBeginQuery(TheForge_CmdHandle cmd,
																					 TheForge_QueryHeapHandle queryHeap,
																					 TheForge_QueryDesc *pQuery);
AL2O3_EXTERN_C void TheForge_CmdEndQuery(TheForge_CmdHandle cmd,
																				 TheForge_QueryHeapHandle queryHeap,
																				 TheForge_QueryDesc *pQuery);
AL2O3_EXTERN_C void TheForge_CmdResolveQuery(TheForge_CmdHandle cmd,
																						 TheForge_QueryHeapHandle queryHeap,
																						 TheForge_BufferHandle readbackBuffer,
																						 uint32_t startQuery,
																						 uint32_t queryCount);
AL2O3_EXTERN_C void TheForge_CmdEndDebugMarker(TheForge_CmdHandle cmd);
AL2O3_EXTERN_C void TheForge_CmdAddDebugMarker(TheForge_CmdHandle cmd, float r, float g, float b, const char *pName);

AL2O3_EXTERN_C void TheForge_QueueSubmit(TheForge_QueueHandle queue,
																				 uint32_t cmdCount,
																				 TheForge_CmdHandle *pCmds,
																				 TheForge_FenceHandle fence,
																				 uint32_t wait_semaphore_count,
																				 TheForge_SemaphoreHandle *pWaitSemaphores,
																				 uint32_t signal_semaphore_count,
																				 TheForge_SemaphoreHandle *pSignalSemaphores);
AL2O3_EXTERN_C void TheForge_GetTimestampFrequency(TheForge_QueueHandle queue, double *pFrequency);
AL2O3_EXTERN_C void TheForge_WaitQueueIdle(TheForge_QueueHandle queue);

AL2O3_EXTERN_C void TheForge_GetFenceStatus(TheForge_RendererHandle handle,
																						TheForge_FenceHandle fence,
																						TheForge_FenceStatus *p_fence_status);
AL2O3_EXTERN_C void TheForge_WaitForFences(TheForge_RendererHandle handle,
																					 uint32_t fenceCount,
																					 TheForge_FenceHandle *pFences);

AL2O3_EXTERN_C void TheForge_AddIndirectCommandSignature(TheForge_RendererHandle handle,
																												 const TheForge_CommandSignatureDesc *p_desc,
																												 TheForge_CommandSignatureHandle* pCommandSignature);
AL2O3_EXTERN_C void TheForge_RemoveIndirectCommandSignature(TheForge_RendererHandle handle,
																														TheForge_CommandSignatureHandle commandSignature);


AL2O3_EXTERN_C void TheForge_CalculateMemoryStats(TheForge_RendererHandle handle, char **stats);
AL2O3_EXTERN_C void TheForge_FreeMemoryStats(TheForge_RendererHandle handle, char *stats);
AL2O3_EXTERN_C void TheForge_SetBufferName(TheForge_RendererHandle handle,
																					 TheForge_BufferHandle buffer,
																					 const char *pName);
AL2O3_EXTERN_C void TheForge_SetTextureName(TheForge_RendererHandle handle,
																						TheForge_TextureHandle texture,
																						const char *pName);

AL2O3_EXTERN_C void TheForge_AddSwapChain(TheForge_RendererHandle handle, const TheForge_SwapChainDesc* pDesc, TheForge_SwapChainHandle* pSwapChain);
AL2O3_EXTERN_C void TheForge_RemoveSwapChain(TheForge_RendererHandle handle, TheForge_SwapChainHandle swapChain);
AL2O3_EXTERN_C void TheForge_ToggleVSync(TheForge_RendererHandle handle, TheForge_SwapChainHandle* pSwapchain);
AL2O3_EXTERN_C bool TheForge_IsImageFormatSupported(TheForge_ImageFormat format);
AL2O3_EXTERN_C TheForge_ImageFormat TheForge_GetRecommendedSwapchainFormat(bool hintHDR);
AL2O3_EXTERN_C void TheForge_AcquireNextImage(TheForge_RendererHandle handle, TheForge_SwapChainHandle swapChain, TheForge_SemaphoreHandle signalSemaphore, TheForge_FenceHandle fence, uint32_t* pImageIndex);
AL2O3_EXTERN_C void TheForge_QueuePresent(TheForge_QueueHandle queue, TheForge_SwapChainHandle swapChain, uint32_t swapChainImageIndex, uint32_t waitSemaphoreCount, TheForge_SemaphoreHandle* pWaitSemaphores);


// accessors TheForge C API has opaque handles, this is largely okay as its mostly a push API
// however a few things do need passing back (mostly for rendertarget and swapchain)
// these calls implment the required accessors.
AL2O3_EXTERN_C TheForge_RenderTargetHandle TheForge_SwapChainGetRenderTarget(TheForge_SwapChainHandle swapChain, int index);

AL2O3_EXTERN_C TheForge_TextureHandle TheForge_RenderTargetGetTexture(TheForge_RenderTargetHandle renderTarget);

AL2O3_EXTERN_C TheForge_RenderTargetDesc const* TheForge_RenderTargetGetDesc(TheForge_RenderTargetHandle renderTarget);


#endif // end