/**
 * @file GameEngine.cpp
 * @brief Game engine implementation - coordinates all subsystems
 */

#include "GameEngine.h"
#include "../Audio/AudioManager.h"
#include "../GameWorld/GameWorld.h"
#include "../GameLoop/GameLoop.h"
#include "../Graphics/GameRenderer.h"
#include "../GameObject/GameObject.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define ERROR_BUFFER_SIZE 256
#define DEFAULT_WORLD_WIDTH 1000.0f
#define DEFAULT_WORLD_HEIGHT 1000.0f

// Engine state
typedef struct {
    int is_initialized;
    uint32_t max_objects;
    uint32_t target_fps;
    
    // Subsystem states
    int audio_initialized;
    int world_initialized;
    int loop_initialized;
    int renderer_initialized;
    int gameobject_initialized;
    
    char error_message[ERROR_BUFFER_SIZE];
} EngineState;

static EngineState g_engine_state = {0};

// Set error message
static void SetError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(g_engine_state.error_message, ERROR_BUFFER_SIZE, format, args);
    va_end(args);
}

// Audio update callback from game loop
static int EngineAudioUpdateCallback(float delta_time, void* user_data) {
    // Update audio system with delta time
    // Audio events would be processed here
    (void)delta_time;
    (void)user_data;
    return 0;
}

// Game world update callback from game loop
static int EngineWorldUpdateCallback(float delta_time, void* user_data) {
    // Process deferred deletions
    GameWorld_ProcessDeferredDeletions();
    
    // Update world spatial index
    GameWorld_Update();
    
    (void)delta_time;
    (void)user_data;
    return 0;
}

// Render callback from game loop
static int RenderCallback(void* user_data) {
    // Begin render frame
    if (GameRenderer_BeginFrame() != GAMERENDERER_OK) {
        return -1;
    }
    
    // Get all visible objects from world based on camera frustum
    // (Simplified: render all objects for now)
    uint32_t visible_objects[1000];
    uint32_t visible_count = GameWorld_GetObjectCount();
    
    for (uint32_t i = 0; i < visible_count && i < 1000; i++) {
        // In a real implementation, we'd iterate through GameWorld objects
        // and render each one through GameRenderer
    }
    
    // End render frame (sorting, batching, statistics)
    if (GameRenderer_EndFrame() != GAMERENDERER_OK) {
        return -1;
    }
    
    (void)user_data;
    return 0;
}

int GameEngine_Initialize(uint32_t max_objects, uint32_t target_fps) {
    if (g_engine_state.is_initialized) {
        SetError("Game engine already initialized");
        return GAMEENGINE_ERROR_INVALID;
    }
    
    memset(&g_engine_state, 0, sizeof(EngineState));
    g_engine_state.max_objects = max_objects;
    g_engine_state.target_fps = target_fps > 0 ? target_fps : 60;
    
    // Phase 26: Initialize audio system
    if (AudioManager_Initialize() != AUDIO_OK) {
        SetError("Failed to initialize audio system");
        return GAMEENGINE_ERROR_SUBSYSTEM;
    }
    g_engine_state.audio_initialized = 1;
    
    // Phase 28: Initialize world management with spatial indexing
    if (GameWorld_Initialize(DEFAULT_WORLD_WIDTH, DEFAULT_WORLD_HEIGHT, max_objects) != GAMEWORLD_OK) {
        SetError("Failed to initialize game world");
        AudioManager_Shutdown();
        return GAMEENGINE_ERROR_SUBSYSTEM;
    }
    g_engine_state.world_initialized = 1;
    
    // Phase 29: Initialize game loop with frame timing
    if (GameLoop_Initialize(g_engine_state.target_fps) != GAMELOOP_OK) {
        SetError("Failed to initialize game loop");
        GameWorld_Shutdown();
        AudioManager_Shutdown();
        return GAMEENGINE_ERROR_SUBSYSTEM;
    }
    g_engine_state.loop_initialized = 1;
    
    // Phase 30: Initialize renderer
    if (GameRenderer_Initialize(max_objects) != GAMERENDERER_OK) {
        SetError("Failed to initialize renderer");
        GameLoop_Shutdown();
        GameWorld_Shutdown();
        AudioManager_Shutdown();
        return GAMEENGINE_ERROR_SUBSYSTEM;
    }
    g_engine_state.renderer_initialized = 1;
    
    // Phase 27: Initialize game object system
    if (GameObject_Initialize(max_objects) != GAMEOBJECT_OK) {
        SetError("Failed to initialize game objects");
        GameRenderer_Shutdown();
        GameLoop_Shutdown();
        GameWorld_Shutdown();
        AudioManager_Shutdown();
        return GAMEENGINE_ERROR_SUBSYSTEM;
    }
    g_engine_state.gameobject_initialized = 1;
    
    // Register callbacks to game loop for subsystem updates
    GameLoop_RegisterUpdateCallback(EngineAudioUpdateCallback, NULL);
    GameLoop_RegisterUpdateCallback(EngineWorldUpdateCallback, NULL);
    GameLoop_RegisterRenderCallback(RenderCallback, NULL);
    
    g_engine_state.is_initialized = 1;
    
    return GAMEENGINE_OK;
}

int GameEngine_Shutdown(void) {
    if (!g_engine_state.is_initialized) {
        SetError("Game engine not initialized");
        return GAMEENGINE_ERROR_NOT_INIT;
    }
    
    if (GameEngine_IsRunning()) {
        GameEngine_Stop();
    }
    
    // Shutdown in reverse order
    if (g_engine_state.gameobject_initialized) {
        GameObject_Shutdown();
    }
    
    if (g_engine_state.renderer_initialized) {
        GameRenderer_Shutdown();
    }
    
    if (g_engine_state.loop_initialized) {
        GameLoop_Shutdown();
    }
    
    if (g_engine_state.world_initialized) {
        GameWorld_Shutdown();
    }
    
    if (g_engine_state.audio_initialized) {
        AudioManager_Shutdown();
    }
    
    memset(&g_engine_state, 0, sizeof(EngineState));
    
    return GAMEENGINE_OK;
}

int GameEngine_Start(void) {
    if (!g_engine_state.is_initialized) {
        SetError("Game engine not initialized");
        return GAMEENGINE_ERROR_NOT_INIT;
    }
    
    if (GameLoop_Start() != GAMELOOP_OK) {
        SetError("Failed to start game loop");
        return GAMEENGINE_ERROR_SUBSYSTEM;
    }
    
    return GAMEENGINE_OK;
}

int GameEngine_Stop(void) {
    if (!g_engine_state.is_initialized) {
        SetError("Game engine not initialized");
        return GAMEENGINE_ERROR_NOT_INIT;
    }
    
    if (GameLoop_Stop() != GAMELOOP_OK) {
        SetError("Failed to stop game loop");
        return GAMEENGINE_ERROR_SUBSYSTEM;
    }
    
    return GAMEENGINE_OK;
}

int GameEngine_IsRunning(void) {
    if (!g_engine_state.is_initialized) return 0;
    return GameLoop_IsRunning();
}

int GameEngine_Update(void) {
    if (!g_engine_state.is_initialized) {
        SetError("Game engine not initialized");
        return GAMEENGINE_ERROR_NOT_INIT;
    }
    
    if (GameLoop_ExecuteFrame() != GAMELOOP_OK) {
        SetError("Failed to execute frame");
        return GAMEENGINE_ERROR_SUBSYSTEM;
    }
    
    return GAMEENGINE_OK;
}

const char* GameEngine_GetError(void) {
    return g_engine_state.error_message;
}
