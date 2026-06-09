#pragma once

#include <SDL3/SDL.h>

namespace sagepatch {

void init();
void shutdown();

bool handleKeyDown(const SDL_KeyboardEvent& ev);

}
