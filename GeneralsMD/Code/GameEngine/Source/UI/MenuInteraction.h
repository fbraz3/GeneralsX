/**
 * @file MenuInteraction.h
 * @brief Phase 24: Menu Interaction - Button Clicks, Keyboard Navigation, Dialogs
 *
 * Handles user input for menus including button clicks, keyboard navigation,
 * and file dialogs for save/load operations.
 */

#ifndef MENU_INTERACTION_H
#define MENU_INTERACTION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
#define INTERACTION_OK                      0
#define INTERACTION_ERROR_INVALID_PARAMS   -1
#define INTERACTION_ERROR_INVALID_HANDLE   -2
#define INTERACTION_ERROR_DIALOG_FAILED    -3
#define INTERACTION_ERROR_NO_FILE_SELECTED -4

// Input event types
typedef enum {
    INPUT_EVENT_MOUSE_MOVE,
    INPUT_EVENT_MOUSE_DOWN,
    INPUT_EVENT_MOUSE_UP,
    INPUT_EVENT_MOUSE_WHEEL,
    INPUT_EVENT_KEY_DOWN,
    INPUT_EVENT_KEY_UP,
    INPUT_EVENT_TEXT_INPUT
} InputEventType;

// Key codes
typedef enum {
    KEY_ESCAPE = 27,
    KEY_ENTER = 13,
    KEY_TAB = 9,
    KEY_UP = 256,
    KEY_DOWN = 257,
    KEY_LEFT = 258,
    KEY_RIGHT = 259,
    KEY_SPACE = 32,
    KEY_SHIFT = 1000,
    KEY_CTRL = 1001,
    KEY_ALT = 1002
} KeyCode;

// Mouse button codes
typedef enum {
    MOUSE_BUTTON_LEFT = 1,
    MOUSE_BUTTON_RIGHT = 2,
    MOUSE_BUTTON_MIDDLE = 3
} MouseButton;

// Dialog types
typedef enum {
    DIALOG_TYPE_OPEN_FILE,
    DIALOG_TYPE_SAVE_FILE,
    DIALOG_TYPE_FOLDER,
    DIALOG_TYPE_MESSAGE,
    DIALOG_TYPE_CONFIRM
} DialogType;

// Selection highlight styles
typedef enum {
    HIGHLIGHT_STYLE_BORDER,
    HIGHLIGHT_STYLE_BACKGROUND,
    HIGHLIGHT_STYLE_GLOW,
    HIGHLIGHT_STYLE_SCALE
} HighlightStyle;

// Button interaction states
typedef enum {
    BUTTON_STATE_NORMAL,
    BUTTON_STATE_HOVER,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_DISABLED,
    BUTTON_STATE_FOCUSED
} ButtonState;

// Callback function types
typedef void (*ButtonClickCallback)(uint32_t button_handle, void* user_data);
typedef void (*MenuFocusCallback)(uint32_t menu_handle, void* user_data);
typedef void (*KeyboardCallback)(KeyCode key, int is_pressed, void* user_data);

// Input event structure
typedef struct {
    InputEventType type;
    int x;
    int y;
    MouseButton button;
    KeyCode key;
    char character;
    int is_pressed;
    int shift_held;
    int ctrl_held;
    int alt_held;
} InputEvent;

// File dialog result
typedef struct {
    char file_path[512];
    char file_name[256];
    int was_cancelled;
} FileDialogResult;

// Dialog structure
typedef struct {
    uint32_t handle;
    DialogType type;
    char title[256];
    char message[512];
    char file_filter[256];
    char initial_directory[256];
    int is_open;
    FileDialogResult result;
} DialogContext;

// Button interaction state
typedef struct {
    uint32_t button_handle;
    ButtonState state;
    ButtonClickCallback callback;
    void* user_data;
    int is_enabled;
} ButtonInteractionState;

// Menu selection tracking
typedef struct {
    uint32_t menu_handle;
    uint32_t selected_element;
    uint32_t focused_element;
    int supports_keyboard_nav;
    MenuFocusCallback focus_callback;
    void* focus_user_data;
} MenuSelectionState;

// Highlight rendering info
typedef struct {
    uint32_t element_handle;
    HighlightStyle style;
    float highlight_width;
    int highlight_color_r;
    int highlight_color_g;
    int highlight_color_b;
    int highlight_color_a;
    int is_visible;
} SelectionHighlight;

// Settings menu update info
typedef struct {
    uint32_t setting_id;
    char setting_name[128];
    char setting_value[256];
    int value_type;  // 0=bool, 1=int, 2=float, 3=string
} SettingUpdate;

/**
 * Initialize menu interaction system
 * @return INTERACTION_OK on success
 */
int MenuInteraction_Initialize(void);

/**
 * Shutdown menu interaction system
 * @return INTERACTION_OK on success
 */
int MenuInteraction_Shutdown(void);

/**
 * Process input event
 * @param menu_handle Handle of menu to process input for
 * @param event Input event to process
 * @return INTERACTION_OK on success
 */
int MenuInteraction_ProcessInputEvent(uint32_t menu_handle, const InputEvent* event);

/**
 * Register button click callback
 * @param button_handle Handle of button
 * @param callback Function to call on button click
 * @param user_data User data to pass to callback
 * @return INTERACTION_OK on success
 */
int MenuInteraction_RegisterButtonCallback(uint32_t button_handle,
                                          ButtonClickCallback callback,
                                          void* user_data);

/**
 * Handle mouse movement
 * @param menu_handle Handle of menu
 * @param x Mouse X coordinate
 * @param y Mouse Y coordinate
 * @return Handle of element under mouse, or 0 if none
 */
uint32_t MenuInteraction_HandleMouseMove(uint32_t menu_handle, int x, int y);

/**
 * Handle mouse button click
 * @param menu_handle Handle of menu
 * @param button Mouse button code
 * @param x Mouse X coordinate
 * @param y Mouse Y coordinate
 * @return INTERACTION_OK on success
 */
int MenuInteraction_HandleMouseClick(uint32_t menu_handle, MouseButton button, int x, int y);

/**
 * Handle keyboard input
 * @param menu_handle Handle of menu
 * @param key Key code
 * @param is_pressed 1 if key pressed, 0 if released
 * @return INTERACTION_OK on success
 */
int MenuInteraction_HandleKeyboard(uint32_t menu_handle, KeyCode key, int is_pressed);

/**
 * Navigate menu with arrow keys
 * @param menu_handle Handle of menu
 * @param direction -1 for up/left, 1 for down/right
 * @return Handle of newly focused element
 */
uint32_t MenuInteraction_NavigateMenu(uint32_t menu_handle, int direction);

/**
 * Set current focus in menu
 * @param menu_handle Handle of menu
 * @param element_handle Handle of element to focus
 * @return INTERACTION_OK on success
 */
int MenuInteraction_SetFocus(uint32_t menu_handle, uint32_t element_handle);

/**
 * Get current focus in menu
 * @param menu_handle Handle of menu
 * @return Handle of focused element, or 0 if none
 */
uint32_t MenuInteraction_GetFocus(uint32_t menu_handle);

/**
 * Enable/disable element
 * @param element_handle Handle of element
 * @param is_enabled 1 to enable, 0 to disable
 * @return INTERACTION_OK on success
 */
int MenuInteraction_SetElementEnabled(uint32_t element_handle, int is_enabled);

/**
 * Create selection highlight
 * @param element_handle Handle of element to highlight
 * @param style Highlight style
 * @return Handle of highlight object
 */
uint32_t MenuInteraction_CreateSelectionHighlight(uint32_t element_handle,
                                                 HighlightStyle style);

/**
 * Render all selection highlights for a menu
 * @param menu_handle Handle of menu
 * @return INTERACTION_OK on success
 */
int MenuInteraction_RenderSelectionHighlights(uint32_t menu_handle);

/**
 * Open file dialog
 * @param title Dialog title
 * @param initial_directory Initial directory path
 * @param file_filter File filter (e.g. "*.map;*.txt")
 * @param result Pointer to FileDialogResult structure
 * @return INTERACTION_OK on success
 */
int MenuInteraction_OpenFileDialog(const char* title,
                                  const char* initial_directory,
                                  const char* file_filter,
                                  FileDialogResult* result);

/**
 * Open save file dialog
 * @param title Dialog title
 * @param initial_directory Initial directory path
 * @param file_filter File filter
 * @param default_name Default file name
 * @param result Pointer to FileDialogResult structure
 * @return INTERACTION_OK on success
 */
int MenuInteraction_SaveFileDialog(const char* title,
                                  const char* initial_directory,
                                  const char* file_filter,
                                  const char* default_name,
                                  FileDialogResult* result);

/**
 * Update settings menu
 * @param menu_handle Handle of settings menu
 * @param settings Array of settings to update
 * @param count Number of settings
 * @return INTERACTION_OK on success
 */
int MenuInteraction_UpdateSettingsMenu(uint32_t menu_handle,
                                      const SettingUpdate* settings,
                                      uint32_t count);

/**
 * Get button state
 * @param button_handle Handle of button
 * @return Current button state
 */
ButtonState MenuInteraction_GetButtonState(uint32_t button_handle);

/**
 * Set button state
 * @param button_handle Handle of button
 * @param state New state
 * @return INTERACTION_OK on success
 */
int MenuInteraction_SetButtonState(uint32_t button_handle, ButtonState state);

/**
 * Check if point is over element
 * @param element_handle Handle of element
 * @param x X coordinate
 * @param y Y coordinate
 * @return 1 if over element, 0 if not
 */
int MenuInteraction_IsPointOverElement(uint32_t element_handle, int x, int y);

/**
 * Get error message from last operation
 * @return Error message string
 */
const char* MenuInteraction_GetError(void);

#ifdef __cplusplus
}
#endif

#endif // MENU_INTERACTION_H
