#include "al2o3_platform/platform.h"
#include "al2o3_memory/memory.h"
#include "gfx_theforge/theforge.h"
#include "Renderer/IRenderer.h"

struct TheForge_Renderer {
	Renderer *tfRenderer;
};

// I don't like this at all but for now just grin an bare it!
const char *pszBases[FSR_Count] = {
		"binshaders/",                                // FSR_BinShaders
		"srcshaders/",                                // FSR_SrcShaders
		"textures/",                                  // FSR_Textures
		"meshes/",                                    // FSR_Meshes
		"fonts/",                        							// FSR_Builtin_Fonts
		"gpuconfigs/",                              	// FSR_GpuConfig
		"anims/",                                     // FSR_Animation
		"audio",                                      // FSR_Audio
		"misc/",                        							// FSR_OtherFiles
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

AL2O3_EXTERN_C TheForge_RendererHandle TheForge_RendererCreate(
		char const *appName,
		TheForge_RendererDesc const *settings) {
	RendererDesc desc{
			&LogFunc,
			(RendererApi) 0,
			(ShaderTarget) settings->shaderTarget,

	};

	auto renderer = (TheForge_Renderer *) MEMORY_CALLOC(1, sizeof(TheForge_Renderer));
	if (!renderer)
		return nullptr;

	initRenderer(appName, &desc, &renderer->tfRenderer);

	return renderer;
}

AL2O3_EXTERN_C void TheForge_RendererDestroy(TheForge_RendererHandle renderer) {
	if (!renderer)
		return;
	if (renderer->tfRenderer)
		removeRenderer(renderer->tfRenderer);

	MEMORY_FREE(renderer);
}

