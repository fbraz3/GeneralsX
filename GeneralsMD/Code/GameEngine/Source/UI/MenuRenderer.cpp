/**
 * @file MenuRenderer.cpp
 * @brief Phase 23: Menu Rendering - Implementation
 *
 * Vulkan-based menu UI rendering with bitmap fonts and animations.
 */

#include "MenuRenderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_MENUS           64
#define MAX_ELEMENTS_PER_MENU 256
#define MAX_FONTS           16
#define MAX_ANIMATIONS      128
#define MAX_DRAW_CALLS      4096

// Error handling
static char g_error_message[256] = {0};

// Menu system state
typedef struct {
    MenuDescriptor menus[MAX_MENUS];
    uint32_t menu_count;
    
    MenuFont fonts[MAX_FONTS];
    uint32_t font_count;
    
    MenuAnimation animations[MAX_ANIMATIONS];
    uint32_t animation_count;
    
    MenuDrawCall draw_calls[MAX_DRAW_CALLS];
    uint32_t draw_call_count;
    
    MenuRenderStats stats;
    
    int is_initialized;
} MenuRenderingSystem;

static MenuRenderingSystem g_menu_system = {0};

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
 * Initialize menu rendering system
 */
int MenuRenderer_Initialize(void) {
    if (g_menu_system.is_initialized) {
        return MENU_OK;  // Already initialized
    }
    
    memset(&g_menu_system, 0, sizeof(MenuRenderingSystem));
    g_menu_system.is_initialized = 1;
    
    return MENU_OK;
}

/**
 * Shutdown menu rendering system
 */
int MenuRenderer_Shutdown(void) {
    if (!g_menu_system.is_initialized) {
        SetError("Menu system not initialized");
        return MENU_ERROR_INVALID_PARAMS;
    }
    
    // Clean up menus
    for (uint32_t i = 0; i < g_menu_system.menu_count; i++) {
        MenuDescriptor* menu = &g_menu_system.menus[i];
        if (menu->elements) {
            free(menu->elements);
            menu->elements = NULL;
        }
    }
    
    g_menu_system.menu_count = 0;
    g_menu_system.font_count = 0;
    g_menu_system.animation_count = 0;
    g_menu_system.is_initialized = 0;
    
    return MENU_OK;
}

/**
 * Create new menu
 */
uint32_t MenuRenderer_CreateMenu(const MenuColor* bg_color) {
    if (!g_menu_system.is_initialized) {
        SetError("Menu system not initialized");
        return 0;
    }
    
    if (g_menu_system.menu_count >= MAX_MENUS) {
        SetError("Maximum menu count exceeded");
        return 0;
    }
    
    uint32_t menu_idx = g_menu_system.menu_count;
    MenuDescriptor* menu = &g_menu_system.menus[menu_idx];
    
    memset(menu, 0, sizeof(MenuDescriptor));
    menu->handle = 23000 + menu_idx;
    menu->state = MENU_STATE_HIDDEN;
    menu->element_capacity = MAX_ELEMENTS_PER_MENU;
    
    if (bg_color) {
        menu->background_color = *bg_color;
    } else {
        menu->background_color.r = 0;
        menu->background_color.g = 0;
        menu->background_color.b = 0;
        menu->background_color.a = 255;
    }
    
    // Allocate element array
    menu->elements = (MenuElement*)malloc(MAX_ELEMENTS_PER_MENU * sizeof(MenuElement));
    if (!menu->elements) {
        SetError("Failed to allocate menu elements");
        return 0;
    }
    
    memset(menu->elements, 0, MAX_ELEMENTS_PER_MENU * sizeof(MenuElement));
    menu->element_count = 0;
    
    g_menu_system.menu_count++;
    g_menu_system.stats.total_menus++;
    
    return menu->handle;
}

/**
 * Destroy menu
 */
int MenuRenderer_DestroyMenu(uint32_t menu_handle) {
    if (!g_menu_system.is_initialized) {
        SetError("Menu system not initialized");
        return MENU_ERROR_INVALID_PARAMS;
    }
    
    uint32_t menu_idx = menu_handle - 23000;
    if (menu_idx >= g_menu_system.menu_count) {
        SetError("Invalid menu handle");
        return MENU_ERROR_INVALID_HANDLE;
    }
    
    MenuDescriptor* menu = &g_menu_system.menus[menu_idx];
    if (menu->elements) {
        free(menu->elements);
        menu->elements = NULL;
    }
    
    memset(menu, 0, sizeof(MenuDescriptor));
    g_menu_system.stats.total_menus--;
    
    return MENU_OK;
}

/**
 * Add element to menu
 */
uint32_t MenuRenderer_AddElement(uint32_t menu_handle, const MenuElement* element) {
    if (!g_menu_system.is_initialized) {
        SetError("Menu system not initialized");
        return 0;
    }
    
    if (!element) {
        SetError("Element pointer is NULL");
        return 0;
    }
    
    uint32_t menu_idx = menu_handle - 23000;
    if (menu_idx >= g_menu_system.menu_count) {
        SetError("Invalid menu handle");
        return 0;
    }
    
    MenuDescriptor* menu = &g_menu_system.menus[menu_idx];
    if (menu->element_count >= MAX_ELEMENTS_PER_MENU) {
        SetError("Menu element limit exceeded");
        return 0;
    }
    
    uint32_t elem_idx = menu->element_count;
    MenuElement* elem = &menu->elements[elem_idx];
    
    *elem = *element;
    elem->handle = 23000 + (menu_idx * 1000) + elem_idx;
    
    menu->element_count++;
    g_menu_system.stats.total_elements++;
    menu->needs_redraw = 1;
    
    return elem->handle;
}

/**
 * Remove element from menu
 */
int MenuRenderer_RemoveElement(uint32_t menu_handle, uint32_t element_handle) {
    if (!g_menu_system.is_initialized) {
        SetError("Menu system not initialized");
        return MENU_ERROR_INVALID_PARAMS;
    }
    
    uint32_t menu_idx = menu_handle - 23000;
    if (menu_idx >= g_menu_system.menu_count) {
        SetError("Invalid menu handle");
        return MENU_ERROR_INVALID_HANDLE;
    }
    
    MenuDescriptor* menu = &g_menu_system.menus[menu_idx];
    
    // Find and remove element
    for (uint32_t i = 0; i < menu->element_count; i++) {
        if (menu->elements[i].handle == element_handle) {
            // Swap with last element
            if (i < menu->element_count - 1) {
                menu->elements[i] = menu->elements[menu->element_count - 1];
            }
            menu->element_count--;
            g_menu_system.stats.total_elements--;
            menu->needs_redraw = 1;
            return MENU_OK;
        }
    }
    
    SetError("Element not found");
    return MENU_ERROR_INVALID_HANDLE;
}

/**
 * Update element properties
 */
int MenuRenderer_UpdateElement(uint32_t menu_handle, uint32_t element_handle,
                              const MenuElement* new_properties) {
    if (!g_menu_system.is_initialized) {
        SetError("Menu system not initialized");
        return MENU_ERROR_INVALID_PARAMS;
    }
    
    if (!new_properties) {
        SetError("Properties pointer is NULL");
        return MENU_ERROR_INVALID_PARAMS;
    }
    
    uint32_t menu_idx = menu_handle - 23000;
    if (menu_idx >= g_menu_system.menu_count) {
        SetError("Invalid menu handle");
        return MENU_ERROR_INVALID_HANDLE;
    }
    
    MenuDescriptor* menu = &g_menu_system.menus[menu_idx];
    
    // Find and update element
    for (uint32_t i = 0; i < menu->element_count; i++) {
        if (menu->elements[i].handle == element_handle) {
            menu->elements[i] = *new_properties;
            menu->elements[i].handle = element_handle;  // Preserve handle
            menu->needs_redraw = 1;
            return MENU_OK;
        }
    }
    
    SetError("Element not found");
    return MENU_ERROR_INVALID_HANDLE;
}

/**
 * Set menu visibility
 */
int MenuRenderer_SetMenuState(uint32_t menu_handle, MenuState state) {
    if (!g_menu_system.is_initialized) {
        SetError("Menu system not initialized");
        return MENU_ERROR_INVALID_PARAMS;
    }
    
    uint32_t menu_idx = menu_handle - 23000;
    if (menu_idx >= g_menu_system.menu_count) {
        SetError("Invalid menu handle");
        return MENU_ERROR_INVALID_HANDLE;
    }
    
    MenuDescriptor* menu = &g_menu_system.menus[menu_idx];
    menu->state = state;
    
    if (state == MENU_STATE_VISIBLE) {
        g_menu_system.stats.active_menus++;
    } else if (state == MENU_STATE_HIDDEN) {
        g_menu_system.stats.active_menus--;
    }
    
    menu->needs_redraw = 1;
    return MENU_OK;
}

/**
 * Get menu state
 */
MenuState MenuRenderer_GetMenuState(uint32_t menu_handle) {
    if (!g_menu_system.is_initialized) {
        SetError("Menu system not initialized");
        return MENU_STATE_HIDDEN;
    }
    
    uint32_t menu_idx = menu_handle - 23000;
    if (menu_idx >= g_menu_system.menu_count) {
        SetError("Invalid menu handle");
        return MENU_STATE_HIDDEN;
    }
    
    return g_menu_system.menus[menu_idx].state;
}

/**
 * Load font
 */
uint32_t MenuRenderer_LoadFont(const char* font_path, uint32_t glyph_size) {
    if (!g_menu_system.is_initialized) {
        SetError("Menu system not initialized");
        return 0;
    }
    
    if (!font_path) {
        SetError("Font path is NULL");
        return 0;
    }
    
    if (g_menu_system.font_count >= MAX_FONTS) {
        SetError("Font limit exceeded");
        return 0;
    }
    
    uint32_t font_idx = g_menu_system.font_count;
    MenuFont* font = &g_menu_system.fonts[font_idx];
    
    memset(font, 0, sizeof(MenuFont));
    font->handle = 24000 + font_idx;
    font->glyph_size = glyph_size;
    font->baseline = (glyph_size * 3) / 4;  // Typical baseline
    
    // Initialize character widths (monospace for now)
    for (int i = 0; i < 256; i++) {
        font->char_width[i] = glyph_size;
        font->char_height[i] = glyph_size;
    }
    
    g_menu_system.font_count++;
    return font->handle;
}

/**
 * Render menu
 */
int MenuRenderer_Render(uint32_t menu_handle) {
    if (!g_menu_system.is_initialized) {
        SetError("Menu system not initialized");
        return MENU_ERROR_INVALID_PARAMS;
    }
    
    uint32_t menu_idx = menu_handle - 23000;
    if (menu_idx >= g_menu_system.menu_count) {
        SetError("Invalid menu handle");
        return MENU_ERROR_INVALID_HANDLE;
    }
    
    MenuDescriptor* menu = &g_menu_system.menus[menu_idx];
    
    if (menu->state != MENU_STATE_VISIBLE && menu->state != MENU_STATE_ANIMATED_IN) {
        return MENU_OK;  // Menu not visible, skip rendering
    }
    
    // Generate draw calls for menu elements
    g_menu_system.draw_call_count = 0;
    
    for (uint32_t i = 0; i < menu->element_count; i++) {
        MenuElement* elem = &menu->elements[i];
        
        if (!elem->is_visible) {
            continue;
        }
        
        if (g_menu_system.draw_call_count >= MAX_DRAW_CALLS) {
            SetError("Draw call limit exceeded");
            return MENU_ERROR_VULKAN_FAILED;
        }
        
        MenuDrawCall* draw = &g_menu_system.draw_calls[g_menu_system.draw_call_count];
        draw->handle = elem->handle;
        draw->element_type = elem->type;
        draw->bounds = elem->bounds;
        draw->color = elem->color;
        draw->texture = elem->texture_handle;
        draw->z_depth = (float)i / menu->element_count;
        draw->is_visible = elem->is_visible;
        
        g_menu_system.draw_call_count++;
    }
    
    g_menu_system.stats.draw_calls = g_menu_system.draw_call_count;
    menu->needs_redraw = 0;
    
    return MENU_OK;
}

/**
 * Update animations
 */
int MenuRenderer_UpdateAnimations(float delta_time_ms) {
    if (!g_menu_system.is_initialized) {
        SetError("Menu system not initialized");
        return MENU_ERROR_INVALID_PARAMS;
    }
    
    for (uint32_t i = 0; i < g_menu_system.animation_count; i++) {
        MenuAnimation* anim = &g_menu_system.animations[i];
        
        if (!anim->is_playing) {
            continue;
        }
        
        anim->elapsed_ms += delta_time_ms;
        
        if (anim->elapsed_ms >= anim->duration_ms) {
            if (anim->is_looping) {
                anim->elapsed_ms = 0.0f;
            } else {
                anim->is_playing = 0;
            }
        }
    }
    
    return MENU_OK;
}

/**
 * Create animation
 */
uint32_t MenuRenderer_CreateAnimation(float duration_ms, int looping) {
    if (!g_menu_system.is_initialized) {
        SetError("Menu system not initialized");
        return 0;
    }
    
    if (g_menu_system.animation_count >= MAX_ANIMATIONS) {
        SetError("Animation limit exceeded");
        return 0;
    }
    
    uint32_t anim_idx = g_menu_system.animation_count;
    MenuAnimation* anim = &g_menu_system.animations[anim_idx];
    
    anim->handle = 25000 + anim_idx;
    anim->duration_ms = duration_ms;
    anim->elapsed_ms = 0.0f;
    anim->is_looping = looping;
    anim->is_playing = 0;
    
    g_menu_system.animation_count++;
    return anim->handle;
}

/**
 * Start animation on element
 */
int MenuRenderer_StartAnimation(uint32_t element_handle, uint32_t animation_handle) {
    if (!g_menu_system.is_initialized) {
        SetError("Menu system not initialized");
        return MENU_ERROR_INVALID_PARAMS;
    }
    
    uint32_t anim_idx = animation_handle - 25000;
    if (anim_idx >= g_menu_system.animation_count) {
        SetError("Invalid animation handle");
        return MENU_ERROR_INVALID_HANDLE;
    }
    
    MenuAnimation* anim = &g_menu_system.animations[anim_idx];
    anim->is_playing = 1;
    anim->elapsed_ms = 0.0f;
    
    return MENU_OK;
}

/**
 * Get statistics
 */
MenuRenderStats MenuRenderer_GetStats(void) {
    if (!g_menu_system.is_initialized) {
        SetError("Menu system not initialized");
        memset(&g_menu_system.stats, 0, sizeof(MenuRenderStats));
    }
    
    return g_menu_system.stats;
}

/**
 * Get error message
 */
const char* MenuRenderer_GetError(void) {
    if (g_error_message[0] == '\0') {
        return "No error";
    }
    return g_error_message;
}
