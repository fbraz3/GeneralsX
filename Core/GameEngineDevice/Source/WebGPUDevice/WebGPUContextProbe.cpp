#include "WebGPUDevice/WebGPUContext.h"

#include <cstdio>
#include <emscripten/html5.h>

namespace
{
WebGPUContext context;

EM_BOOL OnFrame(double, void *userData)
{
	WebGPUContext *readyContext = static_cast<WebGPUContext *>(userData);
	if (!readyContext->renderClear(0.035, 0.055, 0.075, 1.0)) {
		fprintf(stderr, "GENERALSX_WEBGPU_CLEAR_FAILED\n");
		return EM_FALSE;
	}
	puts("GENERALSX_WEBGPU_CONTEXT_READY");
	return EM_FALSE;
}

void OnReady(WebGPUContext &readyContext, void *)
{
	emscripten_request_animation_frame_loop(OnFrame, &readyContext);
}
}

// GeneralsX @feature Codex 04/08/2026 Provide deterministic browser backend acceptance.
int main()
{
	if (!context.initialize("#canvas", 640, 360, OnReady, nullptr)) {
		fprintf(stderr, "GENERALSX_WEBGPU_CONTEXT_FAILED\n");
		return 1;
	}
	return 0;
}
