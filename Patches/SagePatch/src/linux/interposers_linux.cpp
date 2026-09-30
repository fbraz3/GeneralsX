// Linux: LD_PRELOAD-style symbol override. Our SDL_PollEvent wins resolution
// (loaded first via LD_PRELOAD), so all callers — including the host
// libSDL3.so when re-entered — see our hook. We get the real one via
// dlsym(RTLD_NEXT, ...) on first call.

#include <SDL3/SDL.h>
#include <dlfcn.h>
#include "SagePatch/Hooks.h"
#include "SagePatch/Logger.h"

extern "C" bool SDL_PollEvent(SDL_Event* event) {
    using PollFn = bool (*)(SDL_Event*);
    static PollFn real = nullptr;
    if (!real) {
        real = reinterpret_cast<PollFn>(dlsym(RTLD_NEXT, "SDL_PollEvent"));
        if (!real) {
            SAGEPATCH_LOG("FATAL: dlsym(RTLD_NEXT, \"SDL_PollEvent\") failed: %s", dlerror());
            return false;
        }
    }
    while (real(event)) {
        if (event->type == SDL_EVENT_KEY_DOWN) {
            if (sagepatch::handleKeyDown(event->key)) {
                continue;
            }
        }
        return true;
    }
    return false;
}
