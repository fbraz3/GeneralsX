/**
 * @file MenuRenderer.h
 * @brief Phase 23: Menu Rendering - Vulkan-based menu UI rendering
 *
 * Implements menu UI rendering using Vulkan primitives:
 * - Menu layouts and positioning
 * - Text rendering with bitmap fonts
 * - Button geometry and textures
 * - Transition animations
 * - Menu state visualization
 *
 * Handle range: 23000-23999
 */

#ifndef MENU_RENDERER_H
#define MENU_RENDERER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Menu renderer error codes
#define MENU_OK                         0
#define MENU_ERROR_INVALID_HANDLE       1
#define MENU_ERROR_INVALID_PARAMS       2
#define MENU_ERROR_OUT_OF_MEMORY        3
#define MENU_ERROR_VULKAN_FAILED        4

/**
 * @brief Menu element types
 */
typedef enum {
    MENU_ELEM_UNDEFINED = 0,
    MENU_ELEM_PANEL = 1,              // Container element
    MENU_ELEM_BUTTON = 2,             // Clickable button
    MENU_ELEM_TEXT = 3,               // Text label
    MENU_ELEM_IMAGE = 4,              // Image/texture
    MENU_ELEM_SLIDER = 5,             // Slider control
    MENU_ELEM_CHECKBOX = 6,           // Checkbox
    MENU_ELEM_LISTBOX = 7,            // List box
    MENU_ELEM_TEXTBOX = 8,            // Text input
} MenuElementType;

/**
 * @brief Menu state enumeration
 */
typedef enum {
    MENU_STATE_HIDDEN = 0,
    MENU_STATE_VISIBLE = 1,
    MENU_STATE_ANIMATED_IN = 2,
    MENU_STATE_ANIMATED_OUT = 3,
    MENU_STATE_DISABLED = 4,
} MenuState;

/**
 * @brief Rectangle definition (screen coordinates)
 */
typedef struct {
    int32_t x;                        // Top-left X
    int32_t y;                        // Top-left Y
    uint32_t width;                   // Width in pixels
    uint32_t height;                  // Height in pixels
} MenuRect;

/**
 * @brief Color definition (RGBA)
 */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} MenuColor;

/**
 * @brief Animation descriptor
 */
typedef struct {
    uint32_t handle;                  // Animation handle
    float duration_ms;                // Duration in milliseconds
    float elapsed_ms;                 // Elapsed time
    int is_looping;                   // Loop after completion
    int is_playing;                   // Currently playing
} MenuAnimation;

/**
 * @brief Menu element descriptor
 */
typedef struct {
    uint32_t handle;                  // Element handle (23000+)
    MenuElementType type;
    MenuState state;
    MenuRect bounds;
    MenuColor color;
    uint64_t texture_handle;          // Associated texture (0 = none)
    uint32_t font_handle;             // Font for text elements
    const char* text_content;         // Text content (MENU_ELEM_TEXT, MENU_ELEM_BUTTON)
    void* user_data;                  // User-defined data
    int is_visible;
    int is_enabled;
} MenuElement;

/**
 * @brief Menu descriptor (container)
 */
typedef struct {
    uint32_t handle;                  // Menu handle
    MenuElement* elements;            // Array of elements
    uint32_t element_count;
    uint32_t element_capacity;
    MenuState state;
    MenuAnimation* active_animation;  // Current animation (if any)
    MenuColor background_color;
    uint64_t background_texture;
    int needs_redraw;
} MenuDescriptor;

/**
 * @brief Font descriptor for text rendering
 */
typedef struct {
    uint32_t handle;                  // Font handle
    uint64_t glyph_texture;           // Texture with all glyphs
    uint32_t glyph_size;              // Size of each glyph
    uint16_t char_width[256];         // Width per character
    uint16_t char_height[256];        // Height per character
    uint16_t baseline;                // Baseline for alignment
    int is_sdf;                       // Is Signed Distance Field
} MenuFont;

/**
 * @brief Draw call for menu element
 */
typedef struct {
    uint32_t handle;
    MenuElementType element_type;
    MenuRect bounds;
    MenuColor color;
    uint64_t texture;
    float z_depth;                    // For layering
    int is_visible;
} MenuDrawCall;

/**
 * @brief Menu rendering statistics
 */
typedef struct {
    uint32_t total_menus;
    uint32_t active_menus;
    uint32_t total_elements;
    uint32_t visible_elements;
    uint32_t draw_calls;
    float frame_time_ms;
    uint64_t textures_bound;
} MenuRenderStats;

// API Functions

/**
 * @brief Initialize menu rendering system
 */
int MenuRenderer_Initialize(void);

/**
 * @brief Shutdown menu rendering system
 */
int MenuRenderer_Shutdown(void);

/**
 * @brief Create new menu
 */
uint32_t MenuRenderer_CreateMenu(const MenuColor* bg_color);

/**
 * @brief Destroy menu
 */
int MenuRenderer_DestroyMenu(uint32_t menu_handle);

/**
 * @brief Add element to menu
 */
uint32_t MenuRenderer_AddElement(uint32_t menu_handle, const MenuElement* element);

/**
 * @brief Remove element from menu
 */
int MenuRenderer_RemoveElement(uint32_t menu_handle, uint32_t element_handle);

/**
 * @brief Update element properties
 */
int MenuRenderer_UpdateElement(uint32_t menu_handle, uint32_t element_handle, 
                              const MenuElement* new_properties);

/**
 * @brief Set menu visibility
 */
int MenuRenderer_SetMenuState(uint32_t menu_handle, MenuState state);

/**
 * @brief Get menu state
 */
MenuState MenuRenderer_GetMenuState(uint32_t menu_handle);

/**
 * @brief Load font for text rendering
 */
uint32_t MenuRenderer_LoadFont(const char* font_path, uint32_t glyph_size);

/**
 * @brief Render menu to Vulkan command buffer
 */
int MenuRenderer_Render(uint32_t menu_handle);

/**
 * @brief Update animations (called per frame)
 */
int MenuRenderer_UpdateAnimations(float delta_time_ms);

/**
 * @brief Create animation
 */
uint32_t MenuRenderer_CreateAnimation(float duration_ms, int looping);

/**
 * @brief Start animation on element
 */
int MenuRenderer_StartAnimation(uint32_t element_handle, uint32_t animation_handle);

/**
 * @brief Get rendering statistics
 */
MenuRenderStats MenuRenderer_GetStats(void);

/**
 * @brief Get error message
 */
const char* MenuRenderer_GetError(void);

#ifdef __cplusplus
}
#endif

#endif // MENU_RENDERER_H
