#include "SagePatch/Hooks.h"
#include "SagePatch/Features.h"
#include "SagePatch/Logger.h"

namespace sagepatch {

bool handleKeyDown(const SDL_KeyboardEvent& ev) {
    if (ev.repeat) return false;

    SDL_Window* window = SDL_GetWindowFromID(ev.windowID);
    if (!window) return false;

    const bool ctrl = (ev.mod & SDL_KMOD_CTRL) != 0;

    switch (ev.key) {
        case SDLK_F11:
            takeScreenshot(window);
            return true;

        case SDLK_SCROLLLOCK:
            toggleCursorLock(window);
            return true;

        case SDLK_PAGEUP:
            if (ctrl) { adjustBrightness(+8); return true; }
            break;
        case SDLK_PAGEDOWN:
            if (ctrl) { adjustBrightness(-8); return true; }
            break;

        case SDLK_1:
            if (ctrl) { moveWindow(window, WindowPosition::Center); return true; }
            break;
        case SDLK_2:
            if (ctrl) { moveWindow(window, WindowPosition::TopLeft); return true; }
            break;
        case SDLK_3:
            if (ctrl) { moveWindow(window, WindowPosition::TopRight); return true; }
            break;
        case SDLK_4:
            if (ctrl) { moveWindow(window, WindowPosition::BottomLeft); return true; }
            break;
        case SDLK_5:
            if (ctrl) { moveWindow(window, WindowPosition::BottomRight); return true; }
            break;

        default:
            break;
    }
    return false;
}

}
