#pragma once
#ifndef GFX_THEFORGE_RESOURCELOADER_H_
#define GFX_THEFORGE_RESOURCELOADER_H_

#include "gfx_theforge/resourceloaders_structs.h"

AL2O3_EXTERN_C void TheForge_InitResourceLoaderInterface(TheForge_RendererHandle handle, TheForge_ResourceLoaderDesc* pDesc);
AL2O3_EXTERN_C void TheForge_RemoveResourceLoaderInterface(TheForge_RendererHandle handle);

AL2O3_EXTERN_C void TheForge_LoadBuffer(TheForge_BufferLoadDesc const* pBuffer, bool batch);
AL2O3_EXTERN_C void TheForge_LoadTexture(TheForge_TextureLoadDesc const* pTexture, bool batch);
AL2O3_EXTERN_C void TheForge_LoadBufferWithToken(TheForge_BufferLoadDesc const* pBufferDesc, TheForge_SyncToken* token);
AL2O3_EXTERN_C void TheForge_LoadTextureWithToken(TheForge_TextureLoadDesc const* pTextureDesc, TheForge_SyncToken* token);
AL2O3_EXTERN_C void TheForge_UpdateBuffer(TheForge_BufferUpdateDesc const* pBuffer, bool batch);
AL2O3_EXTERN_C void TheForge_UpdateTexture(TheForge_TextureUpdateDesc const* pTexture, bool batch);
AL2O3_EXTERN_C void TheForge_UpdateResources(uint32_t resourceCount, TheForge_ResourceUpdateDesc* pResources);
AL2O3_EXTERN_C void TheForge_UpdateBufferWithToken(TheForge_BufferUpdateDesc* pBuffer, TheForge_SyncToken* token);
AL2O3_EXTERN_C void TheForge_UpdateTextureWithToken(TheForge_TextureUpdateDesc* pTexture, TheForge_SyncToken* token);
AL2O3_EXTERN_C void TheForge_UpdateResourcesWithToken(uint32_t resourceCount, TheForge_ResourceUpdateDesc* pResources, TheForge_SyncToken* token);
AL2O3_EXTERN_C bool TheForge_IsBatchCompleted();
AL2O3_EXTERN_C void TheForge_WaitBatchCompleted();
AL2O3_EXTERN_C bool TheForge_IsTokenCompleted(TheForge_SyncToken token);
AL2O3_EXTERN_C void TheForge_WaitTokenCompleted(TheForge_SyncToken token);
AL2O3_EXTERN_C void TheForge_FlushResourceUpdates();
AL2O3_EXTERN_C void TheForge_FinishResourceLoading();

#endif