#include "SagePatch/Hooks.h"
#include "SagePatch/Features.h"
#include "SagePatch/Logger.h"

namespace sagepatch {

bool handleKeyDown(const SDL_KeyboardEvent& ev) {
    if (ev.repeat) return false;

    SDL_Window* window = SDL_GetWindowFromID(ev.windowID);
    if (!window) return false;

    switch (ev.key) {
        case SDLK_F11:
            takeScreenshot(window);
            return true;
        case SDLK_SCROLLLOCK:
            toggleCursorLock(window);
            return true;
        case SDLK_PAGEUP:
            if (ev.mod & SDL_KMOD_CTRL) {
                adjustBrightness(+8);
                return true;
            }
            break;
        case SDLK_PAGEDOWN:
            if (ev.mod & SDL_KMOD_CTRL) {
                adjustBrightness(-8);
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

}
