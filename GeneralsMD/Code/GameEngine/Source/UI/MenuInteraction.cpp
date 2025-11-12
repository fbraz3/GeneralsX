/**
 * @file MenuInteraction.cpp
 * @brief Phase 24: Menu Interaction - Implementation
 *
 * Button click detection, keyboard navigation, and file dialogs.
 */

#include "MenuInteraction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_BUTTONS           256
#define MAX_SELECTIONS        64
#define MAX_HIGHLIGHTS        256
#define MAX_DIALOGS           16

// Error handling
static char g_error_message[256] = {0};

// Interaction system state
typedef struct {
    ButtonInteractionState buttons[MAX_BUTTONS];
    uint32_t button_count;
    
    MenuSelectionState selections[MAX_SELECTIONS];
    uint32_t selection_count;
    
    SelectionHighlight highlights[MAX_HIGHLIGHTS];
    uint32_t highlight_count;
    
    DialogContext dialogs[MAX_DIALOGS];
    uint32_t dialog_count;
    
    int is_initialized;
} MenuInteractionSystem;

static MenuInteractionSystem g_interaction_system = {0};

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
 * Initialize menu interaction system
 */
int MenuInteraction_Initialize(void) {
    if (g_interaction_system.is_initialized) {
        return INTERACTION_OK;
    }
    
    memset(&g_interaction_system, 0, sizeof(MenuInteractionSystem));
    g_interaction_system.is_initialized = 1;
    
    return INTERACTION_OK;
}

/**
 * Shutdown menu interaction system
 */
int MenuInteraction_Shutdown(void) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    g_interaction_system.button_count = 0;
    g_interaction_system.selection_count = 0;
    g_interaction_system.highlight_count = 0;
    g_interaction_system.dialog_count = 0;
    g_interaction_system.is_initialized = 0;
    
    return INTERACTION_OK;
}

/**
 * Register button click callback
 */
int MenuInteraction_RegisterButtonCallback(uint32_t button_handle,
                                          ButtonClickCallback callback,
                                          void* user_data) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    if (g_interaction_system.button_count >= MAX_BUTTONS) {
        SetError("Button limit exceeded");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    // Check if button already registered
    for (uint32_t i = 0; i < g_interaction_system.button_count; i++) {
        if (g_interaction_system.buttons[i].button_handle == button_handle) {
            g_interaction_system.buttons[i].callback = callback;
            g_interaction_system.buttons[i].user_data = user_data;
            return INTERACTION_OK;
        }
    }
    
    // Add new button
    uint32_t idx = g_interaction_system.button_count;
    ButtonInteractionState* btn = &g_interaction_system.buttons[idx];
    
    btn->button_handle = button_handle;
    btn->state = BUTTON_STATE_NORMAL;
    btn->callback = callback;
    btn->user_data = user_data;
    btn->is_enabled = 1;
    
    g_interaction_system.button_count++;
    return INTERACTION_OK;
}

/**
 * Get button state
 */
ButtonState MenuInteraction_GetButtonState(uint32_t button_handle) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return BUTTON_STATE_DISABLED;
    }
    
    for (uint32_t i = 0; i < g_interaction_system.button_count; i++) {
        if (g_interaction_system.buttons[i].button_handle == button_handle) {
            return g_interaction_system.buttons[i].state;
        }
    }
    
    SetError("Button not found");
    return BUTTON_STATE_DISABLED;
}

/**
 * Set button state
 */
int MenuInteraction_SetButtonState(uint32_t button_handle, ButtonState state) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    for (uint32_t i = 0; i < g_interaction_system.button_count; i++) {
        if (g_interaction_system.buttons[i].button_handle == button_handle) {
            g_interaction_system.buttons[i].state = state;
            return INTERACTION_OK;
        }
    }
    
    SetError("Button not found");
    return INTERACTION_ERROR_INVALID_HANDLE;
}

/**
 * Process input event
 */
int MenuInteraction_ProcessInputEvent(uint32_t menu_handle, const InputEvent* event) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    if (!event) {
        SetError("Event pointer is NULL");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    switch (event->type) {
        case INPUT_EVENT_MOUSE_MOVE:
            return MenuInteraction_HandleMouseMove(menu_handle, event->x, event->y) ? INTERACTION_OK : INTERACTION_ERROR_INVALID_PARAMS;
        
        case INPUT_EVENT_MOUSE_DOWN:
        case INPUT_EVENT_MOUSE_UP:
            return MenuInteraction_HandleMouseClick(menu_handle, event->button, event->x, event->y);
        
        case INPUT_EVENT_KEY_DOWN:
        case INPUT_EVENT_KEY_UP:
            return MenuInteraction_HandleKeyboard(menu_handle, event->key, event->is_pressed);
        
        default:
            SetError("Unknown input event type");
            return INTERACTION_ERROR_INVALID_PARAMS;
    }
}

/**
 * Handle mouse movement
 */
uint32_t MenuInteraction_HandleMouseMove(uint32_t menu_handle, int x, int y) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return 0;
    }
    
    // Find menu selection state
    for (uint32_t i = 0; i < g_interaction_system.selection_count; i++) {
        if (g_interaction_system.selections[i].menu_handle == menu_handle) {
            // Check which element is under mouse
            // This would normally check against menu elements
            // For now, return 0 (not over any element)
            return 0;
        }
    }
    
    return 0;
}

/**
 * Handle mouse button click
 */
int MenuInteraction_HandleMouseClick(uint32_t menu_handle, MouseButton button, int x, int y) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    if (button != MOUSE_BUTTON_LEFT && button != MOUSE_BUTTON_RIGHT && button != MOUSE_BUTTON_MIDDLE) {
        SetError("Invalid mouse button");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    // Find buttons under click position and trigger callbacks
    for (uint32_t i = 0; i < g_interaction_system.button_count; i++) {
        ButtonInteractionState* btn = &g_interaction_system.buttons[i];
        
        if (!btn->is_enabled) continue;
        
        // Check if button is under click position
        // This would normally check button bounds
        // For now, just trigger callback if registered
        if (btn->callback) {
            btn->callback(btn->button_handle, btn->user_data);
            return INTERACTION_OK;
        }
    }
    
    return INTERACTION_OK;
}

/**
 * Handle keyboard input
 */
int MenuInteraction_HandleKeyboard(uint32_t menu_handle, KeyCode key, int is_pressed) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    switch (key) {
        case KEY_UP:
            if (is_pressed) {
                MenuInteraction_NavigateMenu(menu_handle, -1);
            }
            return INTERACTION_OK;
        
        case KEY_DOWN:
            if (is_pressed) {
                MenuInteraction_NavigateMenu(menu_handle, 1);
            }
            return INTERACTION_OK;
        
        case KEY_ENTER:
            if (is_pressed) {
                // Activate focused element
                // This would trigger the focused button's callback
            }
            return INTERACTION_OK;
        
        case KEY_ESCAPE:
            // Close menu or go back
            return INTERACTION_OK;
        
        default:
            return INTERACTION_OK;
    }
}

/**
 * Navigate menu with arrow keys
 */
uint32_t MenuInteraction_NavigateMenu(uint32_t menu_handle, int direction) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return 0;
    }
    
    // Find menu selection state
    for (uint32_t i = 0; i < g_interaction_system.selection_count; i++) {
        MenuSelectionState* sel = &g_interaction_system.selections[i];
        if (sel->menu_handle == menu_handle && sel->supports_keyboard_nav) {
            // Move focus in specified direction
            if (direction < 0) {
                if (sel->focused_element > 0) {
                    sel->focused_element--;
                }
            } else {
                sel->focused_element++;
            }
            
            // Trigger focus callback if registered
            if (sel->focus_callback) {
                sel->focus_callback(menu_handle, sel->focus_user_data);
            }
            
            return sel->focused_element;
        }
    }
    
    return 0;
}

/**
 * Set current focus in menu
 */
int MenuInteraction_SetFocus(uint32_t menu_handle, uint32_t element_handle) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    // Find or create menu selection state
    MenuSelectionState* sel = NULL;
    for (uint32_t i = 0; i < g_interaction_system.selection_count; i++) {
        if (g_interaction_system.selections[i].menu_handle == menu_handle) {
            sel = &g_interaction_system.selections[i];
            break;
        }
    }
    
    if (!sel) {
        if (g_interaction_system.selection_count >= MAX_SELECTIONS) {
            SetError("Selection limit exceeded");
            return INTERACTION_ERROR_INVALID_PARAMS;
        }
        sel = &g_interaction_system.selections[g_interaction_system.selection_count++];
        sel->menu_handle = menu_handle;
    }
    
    sel->focused_element = element_handle;
    sel->supports_keyboard_nav = 1;
    
    return INTERACTION_OK;
}

/**
 * Get current focus in menu
 */
uint32_t MenuInteraction_GetFocus(uint32_t menu_handle) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return 0;
    }
    
    for (uint32_t i = 0; i < g_interaction_system.selection_count; i++) {
        if (g_interaction_system.selections[i].menu_handle == menu_handle) {
            return g_interaction_system.selections[i].focused_element;
        }
    }
    
    return 0;
}

/**
 * Enable/disable element
 */
int MenuInteraction_SetElementEnabled(uint32_t element_handle, int is_enabled) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    // Find button with matching handle
    for (uint32_t i = 0; i < g_interaction_system.button_count; i++) {
        if (g_interaction_system.buttons[i].button_handle == element_handle) {
            g_interaction_system.buttons[i].is_enabled = is_enabled;
            return INTERACTION_OK;
        }
    }
    
    return INTERACTION_OK;
}

/**
 * Create selection highlight
 */
uint32_t MenuInteraction_CreateSelectionHighlight(uint32_t element_handle,
                                                 HighlightStyle style) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return 0;
    }
    
    if (g_interaction_system.highlight_count >= MAX_HIGHLIGHTS) {
        SetError("Highlight limit exceeded");
        return 0;
    }
    
    uint32_t hl_idx = g_interaction_system.highlight_count;
    SelectionHighlight* hl = &g_interaction_system.highlights[hl_idx];
    
    hl->element_handle = element_handle;
    hl->style = style;
    hl->highlight_width = 3.0f;
    hl->highlight_color_r = 255;
    hl->highlight_color_g = 200;
    hl->highlight_color_b = 0;
    hl->highlight_color_a = 255;
    hl->is_visible = 1;
    
    g_interaction_system.highlight_count++;
    return element_handle;  // Use element handle as highlight ID
}

/**
 * Render all selection highlights
 */
int MenuInteraction_RenderSelectionHighlights(uint32_t menu_handle) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    // Get focused element for this menu
    uint32_t focused = MenuInteraction_GetFocus(menu_handle);
    
    // Render highlights for focused element
    for (uint32_t i = 0; i < g_interaction_system.highlight_count; i++) {
        SelectionHighlight* hl = &g_interaction_system.highlights[i];
        
        if (hl->element_handle == focused && hl->is_visible) {
            // Render highlight based on style
            // This would normally call rendering functions
        }
    }
    
    return INTERACTION_OK;
}

/**
 * Open file dialog
 */
int MenuInteraction_OpenFileDialog(const char* title,
                                  const char* initial_directory,
                                  const char* file_filter,
                                  FileDialogResult* result) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    if (!title || !result) {
        SetError("Invalid parameters");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    // Initialize result
    memset(result, 0, sizeof(FileDialogResult));
    result->was_cancelled = 1;
    
    // On macOS/Linux, would use platform-specific dialogs
    // For now, just set up the dialog context
    if (g_interaction_system.dialog_count >= MAX_DIALOGS) {
        SetError("Dialog limit exceeded");
        return INTERACTION_ERROR_DIALOG_FAILED;
    }
    
    DialogContext* dlg = &g_interaction_system.dialogs[g_interaction_system.dialog_count++];
    dlg->handle = 26000 + g_interaction_system.dialog_count;
    dlg->type = DIALOG_TYPE_OPEN_FILE;
    dlg->is_open = 1;
    
    strncpy(dlg->title, title, sizeof(dlg->title) - 1);
    if (initial_directory) {
        strncpy(dlg->initial_directory, initial_directory, sizeof(dlg->initial_directory) - 1);
    }
    if (file_filter) {
        strncpy(dlg->file_filter, file_filter, sizeof(dlg->file_filter) - 1);
    }
    
    return INTERACTION_OK;
}

/**
 * Open save file dialog
 */
int MenuInteraction_SaveFileDialog(const char* title,
                                  const char* initial_directory,
                                  const char* file_filter,
                                  const char* default_name,
                                  FileDialogResult* result) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    if (!title || !result) {
        SetError("Invalid parameters");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    // Initialize result
    memset(result, 0, sizeof(FileDialogResult));
    result->was_cancelled = 1;
    
    if (g_interaction_system.dialog_count >= MAX_DIALOGS) {
        SetError("Dialog limit exceeded");
        return INTERACTION_ERROR_DIALOG_FAILED;
    }
    
    DialogContext* dlg = &g_interaction_system.dialogs[g_interaction_system.dialog_count++];
    dlg->handle = 26000 + g_interaction_system.dialog_count;
    dlg->type = DIALOG_TYPE_SAVE_FILE;
    dlg->is_open = 1;
    
    strncpy(dlg->title, title, sizeof(dlg->title) - 1);
    if (initial_directory) {
        strncpy(dlg->initial_directory, initial_directory, sizeof(dlg->initial_directory) - 1);
    }
    if (file_filter) {
        strncpy(dlg->file_filter, file_filter, sizeof(dlg->file_filter) - 1);
    }
    
    return INTERACTION_OK;
}

/**
 * Update settings menu
 */
int MenuInteraction_UpdateSettingsMenu(uint32_t menu_handle,
                                      const SettingUpdate* settings,
                                      uint32_t count) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    if (!settings || count == 0) {
        SetError("Invalid settings");
        return INTERACTION_ERROR_INVALID_PARAMS;
    }
    
    // Update settings in the UI
    for (uint32_t i = 0; i < count; i++) {
        const SettingUpdate* setting = &settings[i];
        // Update UI element for this setting
        // This would normally update text, checkboxes, sliders, etc.
    }
    
    return INTERACTION_OK;
}

/**
 * Check if point is over element
 */
int MenuInteraction_IsPointOverElement(uint32_t element_handle, int x, int y) {
    if (!g_interaction_system.is_initialized) {
        SetError("Interaction system not initialized");
        return 0;
    }
    
    // This would normally check against element bounds
    // For now, return 0 (not over)
    return 0;
}

/**
 * Get error message
 */
const char* MenuInteraction_GetError(void) {
    if (g_error_message[0] == '\0') {
        return "No error";
    }
    return g_error_message;
}
