#include "SagePatch/Features.h"
#include "SagePatch/Logger.h"

#include <SDL3/SDL.h>

namespace sagepatch {

static bool g_cursorLocked = false;

void toggleCursorLock(SDL_Window* window) {
    if (!window) return;
    g_cursorLocked = !g_cursorLocked;
    SDL_SetWindowMouseGrab(window, g_cursorLocked);
    SDL_SetWindowRelativeMouseMode(window, g_cursorLocked);
    SAGEPATCH_LOG("Cursor lock: %s", g_cursorLocked ? "ON" : "OFF");
}

}
