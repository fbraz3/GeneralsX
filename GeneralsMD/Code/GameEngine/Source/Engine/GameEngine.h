/**
 * @file GameEngine.h
 * @brief Main game engine orchestrator - coordinates all subsystems (Phases 26-30)
 *
 * Central integration point for:
 * - Phase 26: Audio system
 * - Phase 27: Game object system
 * - Phase 28: World management (spatial indexing)
 * - Phase 29: Game loop (frame timing, update/render)
 * - Phase 30: Rendering system
 */

#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
#define GAMEENGINE_OK              0
#define GAMEENGINE_ERROR_INVALID   -1
#define GAMEENGINE_ERROR_NOT_INIT  -2
#define GAMEENGINE_ERROR_SUBSYSTEM -3

/**
 * Initialize game engine with all subsystems
 * Initializes in order: Audio, GameWorld, GameLoop, Renderer, GameObject
 * @param max_objects Maximum game objects
 * @param target_fps Target frames per second
 * @return GAMEENGINE_OK on success
 */
int GameEngine_Initialize(uint32_t max_objects, uint32_t target_fps);

/**
 * Shutdown game engine with all subsystems
 * Shuts down in reverse order
 * @return GAMEENGINE_OK on success
 */
int GameEngine_Shutdown(void);

/**
 * Start game engine main loop
 * @return GAMEENGINE_OK on success
 */
int GameEngine_Start(void);

/**
 * Stop game engine main loop
 * @return GAMEENGINE_OK on success
 */
int GameEngine_Stop(void);

/**
 * Check if engine is running
 * @return 1 if running, 0 if not
 */
int GameEngine_IsRunning(void);

/**
 * Execute single frame
 * Sequence: Update -> Render -> Present
 * - Update phase: audio events, game logic, physics
 * - Render phase: culling, rendering, UI
 * @return GAMEENGINE_OK on success
 */
int GameEngine_Update(void);

/**
 * Get error message from last operation
 * @return Error message string
 */
const char* GameEngine_GetError(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_ENGINE_H
