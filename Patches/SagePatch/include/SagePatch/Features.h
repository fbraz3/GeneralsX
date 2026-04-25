#pragma once

#include <SDL3/SDL.h>

namespace sagepatch {

void takeScreenshot(SDL_Window* window);
void toggleCursorLock(SDL_Window* window);
void adjustBrightness(int delta);

}
