#pragma once

#include <SDL3/SDL.h>

namespace sagepatch {

enum class WindowPosition {
    Center,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
};

void takeScreenshot(SDL_Window* window);
void toggleCursorLock(SDL_Window* window);
void adjustBrightness(int delta);
void moveWindow(SDL_Window* window, WindowPosition where);

}
