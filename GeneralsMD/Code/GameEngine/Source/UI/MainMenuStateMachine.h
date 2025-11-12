/**
 * @file MainMenuStateMachine.h
 * @brief Phase 25: Main Menu State Machine
 *
 * Handles main menu state transitions, mode selection (Campaign/Skirmish/Multiplayer),
 * and menu navigation.
 */

#ifndef MAIN_MENU_STATE_MACHINE_H
#define MAIN_MENU_STATE_MACHINE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
#define STATEMACHINE_OK                    0
#define STATEMACHINE_ERROR_INVALID_PARAMS -1
#define STATEMACHINE_ERROR_INVALID_STATE  -2
#define STATEMACHINE_ERROR_TRANSITION_FAILED -3

// Main menu states
typedef enum {
    MENU_STATE_NONE = 0,
    MENU_STATE_SPLASH,           // Initial splash screen
    MENU_STATE_MAIN,             // Main menu
    MENU_STATE_CAMPAIGN,         // Campaign mode selection
    MENU_STATE_SKIRMISH,         // Skirmish mode setup
    MENU_STATE_MULTIPLAYER,      // Multiplayer mode
    MENU_STATE_SETTINGS,         // Settings menu
    MENU_STATE_DIFFICULTY,       // Difficulty selection
    MENU_STATE_MAP_SELECTION,    // Map selection
    MENU_STATE_LOADING,          // Game loading
    MENU_STATE_PLAYING,          // Game in progress
    MENU_STATE_PAUSED,           // Game paused
    MENU_STATE_RESULTS,          // Game results/score screen
    MENU_STATE_DEMO,             // Demo/intro video playback
    MENU_STATE_EXITING           // Exit to desktop
} MainMenuState;

// Game mode types
typedef enum {
    GAME_MODE_NONE = 0,
    GAME_MODE_CAMPAIGN,
    GAME_MODE_SKIRMISH,
    GAME_MODE_MULTIPLAYER,
    GAME_MODE_CUSTOM
} GameMode;

// Difficulty levels
typedef enum {
    DIFFICULTY_EASY = 0,
    DIFFICULTY_NORMAL = 1,
    DIFFICULTY_HARD = 2,
    DIFFICULTY_BRUTAL = 3
} DifficultyLevel;

// State transition callback
typedef void (*StateTransitionCallback)(MainMenuState from_state, MainMenuState to_state, void* user_data);

// Menu option structure
typedef struct {
    uint32_t option_id;
    char label[128];
    MainMenuState target_state;
    int is_enabled;
    int is_visible;
} MenuOption;

// Game configuration structure
typedef struct {
    GameMode mode;
    DifficultyLevel difficulty;
    uint32_t map_id;
    char map_name[256];
    int player_count;
    int ai_count;
    char faction_name[128];
} GameConfiguration;

// Menu state context
typedef struct {
    MainMenuState current_state;
    MainMenuState previous_state;
    GameConfiguration game_config;
    uint32_t menu_handle;
    uint32_t active_option;
    int is_loading;
    int should_exit;
    float state_time_ms;
} MenuStateContext;

// Demo video info
typedef struct {
    uint32_t video_id;
    char video_path[512];
    float duration_ms;
    int is_playing;
    float current_time_ms;
    int loop_count;
} DemoVideoInfo;

/**
 * Initialize main menu state machine
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_Initialize(void);

/**
 * Shutdown main menu state machine
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_Shutdown(void);

/**
 * Get current menu state
 * @return Current menu state
 */
MainMenuState MainMenuStateMachine_GetCurrentState(void);

/**
 * Get previous menu state
 * @return Previous menu state
 */
MainMenuState MainMenuStateMachine_GetPreviousState(void);

/**
 * Request state transition
 * @param target_state Target menu state
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_RequestStateTransition(MainMenuState target_state);

/**
 * Update state machine
 * @param delta_time_ms Time delta in milliseconds
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_Update(float delta_time_ms);

/**
 * Set game mode
 * @param mode Game mode to set
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_SetGameMode(GameMode mode);

/**
 * Get game mode
 * @return Current game mode
 */
GameMode MainMenuStateMachine_GetGameMode(void);

/**
 * Set difficulty level
 * @param difficulty Difficulty level
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_SetDifficulty(DifficultyLevel difficulty);

/**
 * Get difficulty level
 * @return Current difficulty level
 */
DifficultyLevel MainMenuStateMachine_GetDifficulty(void);

/**
 * Select campaign
 * @param campaign_id Campaign identifier
 * @param mission_index Mission index (0-based)
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_SelectCampaign(uint32_t campaign_id, uint32_t mission_index);

/**
 * Select map for skirmish/custom game
 * @param map_id Map identifier
 * @param map_name Map name
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_SelectMap(uint32_t map_id, const char* map_name);

/**
 * Set faction selection
 * @param faction_name Faction name (e.g., "USA", "GLA", "China")
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_SetFaction(const char* faction_name);

/**
 * Set player count (for multiplayer/skirmish)
 * @param player_count Total number of players
 * @param ai_count Number of AI players
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_SetPlayerCount(uint32_t player_count, uint32_t ai_count);

/**
 * Start game with current configuration
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_StartGame(void);

/**
 * Pause game (from playing state)
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_PauseGame(void);

/**
 * Resume game (from paused state)
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_ResumeGame(void);

/**
 * Return to main menu
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_ReturnToMainMenu(void);

/**
 * Show settings menu
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_ShowSettings(void);

/**
 * Play demo/intro video
 * @param video_id Video identifier
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_PlayDemo(uint32_t video_id);

/**
 * Stop demo video playback
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_StopDemo(void);

/**
 * Request exit to desktop
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_RequestExit(void);

/**
 * Check if should exit
 * @return 1 if should exit, 0 otherwise
 */
int MainMenuStateMachine_ShouldExit(void);

/**
 * Register state transition callback
 * @param callback Function to call on state transition
 * @param user_data User data to pass to callback
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_RegisterTransitionCallback(StateTransitionCallback callback,
                                                   void* user_data);

/**
 * Get current game configuration
 * @return Pointer to game configuration
 */
const GameConfiguration* MainMenuStateMachine_GetGameConfiguration(void);

/**
 * Render current menu state
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_Render(void);

/**
 * Handle menu input
 * @param menu_option Selected menu option
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_HandleMenuInput(uint32_t menu_option);

/**
 * Get menu options for current state
 * @param options Pointer to array to receive menu options
 * @param max_options Maximum number of options to return
 * @return Number of options returned
 */
uint32_t MainMenuStateMachine_GetMenuOptions(MenuOption* options, uint32_t max_options);

/**
 * Load campaign data
 * @param campaign_id Campaign identifier
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_LoadCampaignData(uint32_t campaign_id);

/**
 * Load map data
 * @param map_id Map identifier
 * @return STATEMACHINE_OK on success
 */
int MainMenuStateMachine_LoadMapData(uint32_t map_id);

/**
 * Validate game configuration
 * @param config Configuration to validate
 * @return 1 if valid, 0 if invalid
 */
int MainMenuStateMachine_ValidateConfiguration(const GameConfiguration* config);

/**
 * Get error message from last operation
 * @return Error message string
 */
const char* MainMenuStateMachine_GetError(void);

#ifdef __cplusplus
}
#endif

#endif // MAIN_MENU_STATE_MACHINE_H
