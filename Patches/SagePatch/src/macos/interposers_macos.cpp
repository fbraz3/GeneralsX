// macOS DYLD interpose table — replaces SDL3 functions at load time without
// modifying the host binary or the real libSDL3.0.dylib.
//
// Reference: dyld(1) man page, section "Interposing" + <mach-o/dyld-interposing.h>.
//
// We hook SDL_PollEvent so we see every input event the game processes. When
// our hot-keys (F11, Scroll Lock, Page Up/Down) appear, we eat the event so the
// game does not also receive it.

#include <SDL3/SDL.h>
#include "SagePatch/Hooks.h"
#include "SagePatch/Logger.h"

namespace sagepatch {

extern bool handleKeyDown(const SDL_KeyboardEvent& ev);

static bool sage_SDL_PollEvent(SDL_Event* event) {
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_EVENT_KEY_DOWN) {
            if (handleKeyDown(event->key)) {
                continue;
            }
        }
        return true;
    }
    return false;
}

}

extern "C" {

__attribute__((used)) static struct interpose_t {
    const void* replacement;
    const void* original;
} _sage_interposers[] __attribute__((section("__DATA,__interpose"))) = {
    { (const void*)&sagepatch::sage_SDL_PollEvent, (const void*)&SDL_PollEvent },
};

}
