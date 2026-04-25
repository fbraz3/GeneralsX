// Windows backend stubs.
//
// Windows has no LD_PRELOAD or __DATA,__interpose equivalent. To hook
// SDL_PollEvent on Windows we would need either:
//   - a proxy DLL pattern (rebuild the host's expected DLL load order so the
//     game opens our DLL first, like the original GenTool's d3d8.dll wrap), or
//   - in-process inline hooking via Detours / MinHook / Polyhook.
//
// Both options are larger than the macOS/Linux implementations and are
// deferred to a follow-up. On Windows users still have the original GenTool
// available, so SagePatch is opt-in QoL only on the platforms that lack one.
//
// The build system therefore disables SAGE_PATCH on Windows by default. If
// someone forces it ON, the linker will succeed (these stubs satisfy the
// symbol references) but the dylib does nothing at runtime.

#include "SagePatch/Features.h"
#include "SagePatch/Logger.h"

#include <SDL3/SDL.h>

namespace sagepatch {

void takeScreenshot(SDL_Window*) {
    SAGEPATCH_LOG("Screenshot: Windows backend not implemented yet.");
}

void adjustBrightness(int /*delta*/) {
    SAGEPATCH_LOG("Brightness: Windows backend not implemented yet.");
}

}
