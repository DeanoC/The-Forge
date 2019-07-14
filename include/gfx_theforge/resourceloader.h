#pragma once
#ifndef GFX_THEFORGE_RESOURCELOADER_H_
#define GFX_THEFORGE_RESOURCELOADER_H_

#include "gfx_theforge/resourceloaders_structs.h"

AL2O3_EXTERN_C void TheForge_InitResourceLoaderInterface(TheForge_RendererHandle handle, TheForge_ResourceLoaderDesc* pDesc);
AL2O3_EXTERN_C void TheForge_RemoveResourceLoaderInterface(TheForge_RendererHandle handle);
AL2O3_EXTERN_C void TheForge_LoadShader(TheForge_RendererHandle handle, const TheForge_ShaderLoadDesc* pDesc, TheForge_ShaderHandle* pShader);

AL2O3_EXTERN_C void TheForge_AddBuffer(TheForge_BufferLoadDesc* pBuffer, bool batch);
AL2O3_EXTERN_C void TheForge_AddTexture(TheForge_TextureLoadDesc* pTexture, bool batch);
AL2O3_EXTERN_C void TheForge_AddBufferWithToken(TheForge_BufferLoadDesc* pBufferDesc, TheForge_SyncToken* token);
AL2O3_EXTERN_C void TheForge_AddTextureWithToken(TheForge_TextureLoadDesc* pTextureDesc, TheForge_SyncToken* token);
AL2O3_EXTERN_C void TheForge_UpdateBuffer(TheForge_BufferUpdateDesc* pBuffer, bool batch);
AL2O3_EXTERN_C void TheForge_UpdateTexture(TheForge_TextureUpdateDesc* pTexture, bool batch);
AL2O3_EXTERN_C void TheForge_UpdateResources(uint32_t resourceCount, TheForge_ResourceUpdateDesc* pResources);
AL2O3_EXTERN_C void TheForge_UpdateBufferWithToken(TheForge_BufferUpdateDesc* pBuffer, TheForge_SyncToken* token);
AL2O3_EXTERN_C void TheForge_UpdateTextureWithToken(TheForge_TextureUpdateDesc* pTexture, TheForge_SyncToken* token);
AL2O3_EXTERN_C void TheForge_UpdateResourcesWithToken(uint32_t resourceCount, TheForge_ResourceUpdateDesc* pResources, TheForge_SyncToken* token);
AL2O3_EXTERN_C bool TheForge_IsBatchCompleted();
AL2O3_EXTERN_C void TheForge_WaitBatchCompleted();
AL2O3_EXTERN_C bool TheForge_IsTokenCompleted(TheForge_SyncToken token);
AL2O3_EXTERN_C void TheForge_WaitTokenCompleted(TheForge_SyncToken token);
AL2O3_EXTERN_C void TheForge_RemoveBuffer(TheForge_BufferHandle buffer);
AL2O3_EXTERN_C void TheForge_RemoveTexture(TheForge_TextureHandle texture);
AL2O3_EXTERN_C void TheForge_FlushResourceUpdates();
AL2O3_EXTERN_C void TheForge_FinishResourceLoading();

#endif