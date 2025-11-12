/**
 * @file MainMenuStateMachine.cpp
 * @brief Phase 25: Main Menu State Machine - Implementation
 *
 * Menu state transitions, game mode selection, and menu navigation.
 */

#include "MainMenuStateMachine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_TRANSITIONS    32
#define MAX_CAMPAIGNS      8
#define MAX_MAPS           64
#define MAX_FACTIONS       3
#define MAX_VIDEOS         16

// Error handling
static char g_error_message[256] = {0};

// State transition callback
typedef struct {
    StateTransitionCallback callback;
    void* user_data;
} TransitionCallbackEntry;

// State machine instance
typedef struct {
    MenuStateContext context;
    
    TransitionCallbackEntry transition_callbacks[MAX_TRANSITIONS];
    uint32_t callback_count;
    
    DemoVideoInfo demo_videos[MAX_VIDEOS];
    uint32_t video_count;
    
    int is_initialized;
} MainMenuStateMachineInstance;

static MainMenuStateMachineInstance g_state_machine;

/**
 * Set error message
 */
static void SetError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(g_error_message, sizeof(g_error_message), format, args);
    va_end(args);
}

/**
 * Trigger transition callbacks
 */
static void TriggerTransitionCallbacks(MainMenuState from_state, MainMenuState to_state) {
    for (uint32_t i = 0; i < g_state_machine.callback_count; i++) {
        TransitionCallbackEntry* entry = &g_state_machine.transition_callbacks[i];
        if (entry->callback) {
            entry->callback(from_state, to_state, entry->user_data);
        }
    }
}

/**
 * Initialize main menu state machine
 */
int MainMenuStateMachine_Initialize(void) {
    if (g_state_machine.is_initialized) {
        return STATEMACHINE_OK;
    }
    
    memset(&g_state_machine, 0, sizeof(MainMenuStateMachineInstance));
    
    // Initialize to splash screen
    g_state_machine.context.current_state = MENU_STATE_SPLASH;
    g_state_machine.context.previous_state = MENU_STATE_NONE;
    g_state_machine.context.should_exit = 0;
    g_state_machine.context.is_loading = 0;
    g_state_machine.context.state_time_ms = 0.0f;
    
    // Initialize game config
    g_state_machine.context.game_config.mode = GAME_MODE_NONE;
    g_state_machine.context.game_config.difficulty = DIFFICULTY_NORMAL;
    g_state_machine.context.game_config.player_count = 1;
    g_state_machine.context.game_config.ai_count = 1;
    
    g_state_machine.is_initialized = 1;
    
    return STATEMACHINE_OK;
}

/**
 * Shutdown main menu state machine
 */
int MainMenuStateMachine_Shutdown(void) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    g_state_machine.callback_count = 0;
    g_state_machine.video_count = 0;
    g_state_machine.is_initialized = 0;
    
    return STATEMACHINE_OK;
}

/**
 * Get current menu state
 */
MainMenuState MainMenuStateMachine_GetCurrentState(void) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return MENU_STATE_NONE;
    }
    
    return g_state_machine.context.current_state;
}

/**
 * Get previous menu state
 */
MainMenuState MainMenuStateMachine_GetPreviousState(void) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return MENU_STATE_NONE;
    }
    
    return g_state_machine.context.previous_state;
}

/**
 * Request state transition
 */
int MainMenuStateMachine_RequestStateTransition(MainMenuState target_state) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    if (target_state == MENU_STATE_NONE) {
        SetError("Invalid target state");
        return STATEMACHINE_ERROR_INVALID_STATE;
    }
    
    MainMenuState from_state = g_state_machine.context.current_state;
    
    // Validate transition based on current state
    switch (from_state) {
        case MENU_STATE_SPLASH:
            // Can only go to MAIN or DEMO
            if (target_state != MENU_STATE_MAIN && target_state != MENU_STATE_DEMO) {
                SetError("Invalid transition from SPLASH");
                return STATEMACHINE_ERROR_TRANSITION_FAILED;
            }
            break;
        
        case MENU_STATE_MAIN:
            // Can go to CAMPAIGN, SKIRMISH, MULTIPLAYER, SETTINGS, DEMO, or EXIT
            if (target_state != MENU_STATE_CAMPAIGN && 
                target_state != MENU_STATE_SKIRMISH &&
                target_state != MENU_STATE_MULTIPLAYER &&
                target_state != MENU_STATE_SETTINGS &&
                target_state != MENU_STATE_DEMO &&
                target_state != MENU_STATE_EXITING) {
                SetError("Invalid transition from MAIN");
                return STATEMACHINE_ERROR_TRANSITION_FAILED;
            }
            break;
        
        case MENU_STATE_PLAYING:
        case MENU_STATE_PAUSED:
            // Can go to PAUSED, RESUME, or MAIN
            if (target_state != MENU_STATE_PAUSED && 
                target_state != MENU_STATE_PLAYING &&
                target_state != MENU_STATE_MAIN &&
                target_state != MENU_STATE_RESULTS) {
                SetError("Invalid transition from PLAYING/PAUSED");
                return STATEMACHINE_ERROR_TRANSITION_FAILED;
            }
            break;
        
        default:
            break;
    }
    
    g_state_machine.context.previous_state = from_state;
    g_state_machine.context.current_state = target_state;
    g_state_machine.context.state_time_ms = 0.0f;
    
    TriggerTransitionCallbacks(from_state, target_state);
    
    return STATEMACHINE_OK;
}

/**
 * Update state machine
 */
int MainMenuStateMachine_Update(float delta_time_ms) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    g_state_machine.context.state_time_ms += delta_time_ms;
    
    // Update based on current state
    switch (g_state_machine.context.current_state) {
        case MENU_STATE_SPLASH:
            // Auto-transition to main after splash timeout (3 seconds)
            if (g_state_machine.context.state_time_ms > 3000.0f) {
                MainMenuStateMachine_RequestStateTransition(MENU_STATE_MAIN);
            }
            break;
        
        case MENU_STATE_LOADING:
            // Simulate loading progress
            if (g_state_machine.context.state_time_ms > 2000.0f) {
                MainMenuStateMachine_RequestStateTransition(MENU_STATE_PLAYING);
            }
            break;
        
        case MENU_STATE_DEMO:
            // Update demo video
            for (uint32_t i = 0; i < g_state_machine.video_count; i++) {
                DemoVideoInfo* video = &g_state_machine.demo_videos[i];
                if (video->is_playing) {
                    video->current_time_ms += delta_time_ms;
                    
                    if (video->current_time_ms >= video->duration_ms) {
                        if (video->loop_count > 0 || video->loop_count == -1) {
                            video->current_time_ms = 0.0f;
                            if (video->loop_count > 0) {
                                video->loop_count--;
                            }
                        } else {
                            video->is_playing = 0;
                        }
                    }
                }
            }
            break;
        
        default:
            break;
    }
    
    return STATEMACHINE_OK;
}

/**
 * Set game mode
 */
int MainMenuStateMachine_SetGameMode(GameMode mode) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    if (mode == GAME_MODE_NONE) {
        SetError("Invalid game mode");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    g_state_machine.context.game_config.mode = mode;
    return STATEMACHINE_OK;
}

/**
 * Get game mode
 */
GameMode MainMenuStateMachine_GetGameMode(void) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return GAME_MODE_NONE;
    }
    
    return g_state_machine.context.game_config.mode;
}

/**
 * Set difficulty level
 */
int MainMenuStateMachine_SetDifficulty(DifficultyLevel difficulty) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    g_state_machine.context.game_config.difficulty = difficulty;
    return STATEMACHINE_OK;
}

/**
 * Get difficulty level
 */
DifficultyLevel MainMenuStateMachine_GetDifficulty(void) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return DIFFICULTY_NORMAL;
    }
    
    return g_state_machine.context.game_config.difficulty;
}

/**
 * Select campaign
 */
int MainMenuStateMachine_SelectCampaign(uint32_t campaign_id, uint32_t mission_index) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    if (campaign_id >= MAX_CAMPAIGNS) {
        SetError("Invalid campaign ID");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    g_state_machine.context.game_config.mode = GAME_MODE_CAMPAIGN;
    g_state_machine.context.game_config.map_id = campaign_id * 100 + mission_index;
    
    snprintf(g_state_machine.context.game_config.map_name,
             sizeof(g_state_machine.context.game_config.map_name),
             "Campaign %u Mission %u", campaign_id, mission_index);
    
    return STATEMACHINE_OK;
}

/**
 * Select map
 */
int MainMenuStateMachine_SelectMap(uint32_t map_id, const char* map_name) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    if (!map_name || map_id >= MAX_MAPS) {
        SetError("Invalid map parameters");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    g_state_machine.context.game_config.map_id = map_id;
    strncpy(g_state_machine.context.game_config.map_name, map_name,
            sizeof(g_state_machine.context.game_config.map_name) - 1);
    
    return STATEMACHINE_OK;
}

/**
 * Set faction
 */
int MainMenuStateMachine_SetFaction(const char* faction_name) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    if (!faction_name) {
        SetError("Faction name is NULL");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    strncpy(g_state_machine.context.game_config.faction_name, faction_name,
            sizeof(g_state_machine.context.game_config.faction_name) - 1);
    
    return STATEMACHINE_OK;
}

/**
 * Set player count
 */
int MainMenuStateMachine_SetPlayerCount(uint32_t player_count, uint32_t ai_count) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    if (player_count == 0) {
        SetError("Invalid player count");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    g_state_machine.context.game_config.player_count = player_count;
    g_state_machine.context.game_config.ai_count = ai_count;
    
    return STATEMACHINE_OK;
}

/**
 * Start game
 */
int MainMenuStateMachine_StartGame(void) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    if (!MainMenuStateMachine_ValidateConfiguration(&g_state_machine.context.game_config)) {
        SetError("Invalid game configuration");
        return STATEMACHINE_ERROR_TRANSITION_FAILED;
    }
    
    g_state_machine.context.is_loading = 1;
    return MainMenuStateMachine_RequestStateTransition(MENU_STATE_LOADING);
}

/**
 * Pause game
 */
int MainMenuStateMachine_PauseGame(void) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    if (g_state_machine.context.current_state != MENU_STATE_PLAYING) {
        SetError("Game is not playing");
        return STATEMACHINE_ERROR_INVALID_STATE;
    }
    
    return MainMenuStateMachine_RequestStateTransition(MENU_STATE_PAUSED);
}

/**
 * Resume game
 */
int MainMenuStateMachine_ResumeGame(void) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    if (g_state_machine.context.current_state != MENU_STATE_PAUSED) {
        SetError("Game is not paused");
        return STATEMACHINE_ERROR_INVALID_STATE;
    }
    
    return MainMenuStateMachine_RequestStateTransition(MENU_STATE_PLAYING);
}

/**
 * Return to main menu
 */
int MainMenuStateMachine_ReturnToMainMenu(void) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    return MainMenuStateMachine_RequestStateTransition(MENU_STATE_MAIN);
}

/**
 * Show settings
 */
int MainMenuStateMachine_ShowSettings(void) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    return MainMenuStateMachine_RequestStateTransition(MENU_STATE_SETTINGS);
}

/**
 * Play demo
 */
int MainMenuStateMachine_PlayDemo(uint32_t video_id) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    if (video_id >= g_state_machine.video_count) {
        SetError("Invalid video ID");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    g_state_machine.demo_videos[video_id].is_playing = 1;
    g_state_machine.demo_videos[video_id].current_time_ms = 0.0f;
    
    return MainMenuStateMachine_RequestStateTransition(MENU_STATE_DEMO);
}

/**
 * Stop demo
 */
int MainMenuStateMachine_StopDemo(void) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    for (uint32_t i = 0; i < g_state_machine.video_count; i++) {
        g_state_machine.demo_videos[i].is_playing = 0;
    }
    
    return MainMenuStateMachine_RequestStateTransition(MENU_STATE_MAIN);
}

/**
 * Request exit
 */
int MainMenuStateMachine_RequestExit(void) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    g_state_machine.context.should_exit = 1;
    return MainMenuStateMachine_RequestStateTransition(MENU_STATE_EXITING);
}

/**
 * Should exit check
 */
int MainMenuStateMachine_ShouldExit(void) {
    if (!g_state_machine.is_initialized) {
        return 0;
    }
    
    return g_state_machine.context.should_exit;
}

/**
 * Register transition callback
 */
int MainMenuStateMachine_RegisterTransitionCallback(StateTransitionCallback callback,
                                                   void* user_data) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    if (g_state_machine.callback_count >= MAX_TRANSITIONS) {
        SetError("Callback limit exceeded");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    TransitionCallbackEntry* entry = &g_state_machine.transition_callbacks[g_state_machine.callback_count++];
    entry->callback = callback;
    entry->user_data = user_data;
    
    return STATEMACHINE_OK;
}

/**
 * Get game configuration
 */
const GameConfiguration* MainMenuStateMachine_GetGameConfiguration(void) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return NULL;
    }
    
    return &g_state_machine.context.game_config;
}

/**
 * Render current menu state
 */
int MainMenuStateMachine_Render(void) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    // Rendering would be delegated to the menu renderer
    // This is a hook for future rendering logic
    
    return STATEMACHINE_OK;
}

/**
 * Handle menu input
 */
int MainMenuStateMachine_HandleMenuInput(uint32_t menu_option) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    // Handle input based on current state
    // This would be expanded based on specific menu options
    
    return STATEMACHINE_OK;
}

/**
 * Get menu options for current state
 */
uint32_t MainMenuStateMachine_GetMenuOptions(MenuOption* options, uint32_t max_options) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return 0;
    }
    
    if (!options || max_options == 0) {
        return 0;
    }
    
    uint32_t option_count = 0;
    
    switch (g_state_machine.context.current_state) {
        case MENU_STATE_MAIN:
            if (option_count < max_options) {
                options[option_count].option_id = 1;
                strcpy(options[option_count].label, "Campaign");
                options[option_count].target_state = MENU_STATE_CAMPAIGN;
                options[option_count].is_enabled = 1;
                options[option_count].is_visible = 1;
                option_count++;
            }
            if (option_count < max_options) {
                options[option_count].option_id = 2;
                strcpy(options[option_count].label, "Skirmish");
                options[option_count].target_state = MENU_STATE_SKIRMISH;
                options[option_count].is_enabled = 1;
                options[option_count].is_visible = 1;
                option_count++;
            }
            if (option_count < max_options) {
                options[option_count].option_id = 3;
                strcpy(options[option_count].label, "Multiplayer");
                options[option_count].target_state = MENU_STATE_MULTIPLAYER;
                options[option_count].is_enabled = 1;
                options[option_count].is_visible = 1;
                option_count++;
            }
            if (option_count < max_options) {
                options[option_count].option_id = 4;
                strcpy(options[option_count].label, "Settings");
                options[option_count].target_state = MENU_STATE_SETTINGS;
                options[option_count].is_enabled = 1;
                options[option_count].is_visible = 1;
                option_count++;
            }
            if (option_count < max_options) {
                options[option_count].option_id = 5;
                strcpy(options[option_count].label, "Exit");
                options[option_count].target_state = MENU_STATE_EXITING;
                options[option_count].is_enabled = 1;
                options[option_count].is_visible = 1;
                option_count++;
            }
            break;
        
        default:
            break;
    }
    
    return option_count;
}

/**
 * Load campaign data
 */
int MainMenuStateMachine_LoadCampaignData(uint32_t campaign_id) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    if (campaign_id >= MAX_CAMPAIGNS) {
        SetError("Invalid campaign ID");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    // Load campaign data from files
    // This would load mission info, images, etc.
    
    return STATEMACHINE_OK;
}

/**
 * Load map data
 */
int MainMenuStateMachine_LoadMapData(uint32_t map_id) {
    if (!g_state_machine.is_initialized) {
        SetError("State machine not initialized");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    if (map_id >= MAX_MAPS) {
        SetError("Invalid map ID");
        return STATEMACHINE_ERROR_INVALID_PARAMS;
    }
    
    // Load map data from files
    
    return STATEMACHINE_OK;
}

/**
 * Validate game configuration
 */
int MainMenuStateMachine_ValidateConfiguration(const GameConfiguration* config) {
    if (!config) {
        SetError("Configuration pointer is NULL");
        return 0;
    }
    
    if (config->mode == GAME_MODE_NONE) {
        SetError("Game mode not set");
        return 0;
    }
    
    if (config->player_count == 0) {
        SetError("Invalid player count");
        return 0;
    }
    
    return 1;
}

/**
 * Get error message
 */
const char* MainMenuStateMachine_GetError(void) {
    if (g_error_message[0] == '\0') {
        return "No error";
    }
    return g_error_message;
}
