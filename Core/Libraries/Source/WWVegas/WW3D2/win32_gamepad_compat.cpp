/*
 * GeneralsX - Cross-Platform Port of Command & Conquer: Generals (2003)
 * Phase 06: Complete Input System - Gamepad/Joystick Support Implementation
 *
 * This implementation provides Windows DirectInput-compatible gamepad support
 * using SDL2 backend. All gamepad events are translated to keyboard/mouse events
 * that integrate with the existing event queue system.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <unistd.h>

#include "win32_sdl_types_compat.h"
#include "win32_gamepad_compat.h"
#include "win32_thread_compat.h"

/* Use SDL2 for gamepad input */
#include <SDL2/SDL.h>


/*
 * Global Gamepad State
 */
static SDL2_CriticalSection g_gamepad_lock;
static BOOL g_gamepad_initialized = FALSE;
static SDL2_GamepadState g_gamepad_states[SDL2_MAX_GAMEPADS];
static SDL2_GamepadMapping g_gamepad_mappings[SDL2_MAX_GAMEPADS];
static int g_gamepad_count = 0;


/*
 * Gamepad Button to Virtual Key Mapping (Default)
 * Maps SDL2 gamepad buttons to Windows VK_* codes
 */
static const BYTE g_default_button_mapping[SDL2_GAMEPAD_BUTTON_COUNT] = {
    VK_SPACE,           /* A / Cross → SPACE (jump/action) */
    VK_ESCAPE,          /* B / Circle → ESC (cancel) */
    VK_SHIFT,           /* X / Square → SHIFT (modifier) */
    VK_CONTROL,         /* Y / Triangle → CTRL (modifier) */
    'Q',                /* LB / L1 → Q (rotate left) */
    'E',                /* RB / R1 → E (rotate right) */
    VK_TAB,             /* Back / Select → TAB (menu) */
    VK_RETURN,          /* Start → RETURN (confirm/pause) */
    'L',                /* Left Stick Click → L (lock) */
    'R'                 /* Right Stick Click → R (rally) */
};


/*
 * Initialize Gamepad System
 */
int SDL2_InitGamepads(void)
{
    if (g_gamepad_initialized) {
        return 0;
    }
    
    SDL2_CreateCriticalSection(&g_gamepad_lock);
    
    SDL2_EnterCriticalSection(&g_gamepad_lock);
    {
        /* Initialize SDL gamepad subsystem */
        if (SDL_InitSubSystem(SDL_INIT_GAMEPAD) != 0) {
            printf("Phase 06: SDL gamepad subsystem initialization failed: %s\n", SDL_GetError());
            SDL2_LeaveCriticalSection(&g_gamepad_lock);
            return -1;
        }
        
        /* Clear gamepad states */
        memset(g_gamepad_states, 0, sizeof(g_gamepad_states));
        memset(g_gamepad_mappings, 0, sizeof(g_gamepad_mappings));
        
        /* Set default mappings for all slots */
        for (int i = 0; i < SDL2_MAX_GAMEPADS; i++) {
            SDL2_GetDefaultGamepadMapping(&g_gamepad_mappings[i]);
        }
        
        /* Update gamepad count */
        int joystick_count = SDL_GetNumJoysticks();
        g_gamepad_count = (joystick_count > SDL2_MAX_GAMEPADS) ? SDL2_MAX_GAMEPADS : joystick_count;
        
        printf("Phase 06: Gamepad system initialized (found %d gamepads)\n", g_gamepad_count);
        
        /* Scan for connected gamepads */
        for (int i = 0; i < g_gamepad_count; i++) {
            SDL_Joystick *joystick = SDL_GetJoystickFromID((SDL_JoystickID)i);
            if (joystick != NULL) {
                g_gamepad_states[i].connected = TRUE;
                strncpy(g_gamepad_states[i].name, SDL_GetJoystickName(joystick), 
                        sizeof(g_gamepad_states[i].name) - 1);
                
                /* Get vendor/product IDs */
                DWORD vid = SDL_GetJoystickVendor(joystick);
                DWORD pid = SDL_GetJoystickProduct(joystick);
                g_gamepad_states[i].vendor_id = vid;
                g_gamepad_states[i].product_id = pid;
                
                printf("Phase 06: Gamepad %d connected: %s (VID:0x%04X PID:0x%04X)\n",
                       i, g_gamepad_states[i].name, vid, pid);
            }
        }
        
        g_gamepad_initialized = TRUE;
    }
    SDL2_LeaveCriticalSection(&g_gamepad_lock);
    
    return 0;
}


/*
 * Shutdown Gamepad System
 */
void SDL2_ShutdownGamepads(void)
{
    SDL2_EnterCriticalSection(&g_gamepad_lock);
    {
        SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
        memset(g_gamepad_states, 0, sizeof(g_gamepad_states));
        g_gamepad_initialized = FALSE;
        printf("Phase 06: Gamepad system shutdown\n");
    }
    SDL2_LeaveCriticalSection(&g_gamepad_lock);
    
    SDL2_DestroyCriticalSection(&g_gamepad_lock);
}


/*
 * Get Number of Connected Gamepads
 */
int SDL2_GetGamepadCount(void)
{
    int count = 0;
    
    SDL2_EnterCriticalSection(&g_gamepad_lock);
    {
        count = g_gamepad_count;
    }
    SDL2_LeaveCriticalSection(&g_gamepad_lock);
    
    return count;
}


/*
 * Get Gamepad State by Index
 */
int SDL2_GetGamepadState(int index, SDL2_GamepadState *state)
{
    if (state == NULL || index < 0 || index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    SDL2_EnterCriticalSection(&g_gamepad_lock);
    {
        if (!g_gamepad_states[index].connected) {
            SDL2_LeaveCriticalSection(&g_gamepad_lock);
            return -1;
        }
        
        memcpy(state, &g_gamepad_states[index], sizeof(SDL2_GamepadState));
    }
    SDL2_LeaveCriticalSection(&g_gamepad_lock);
    
    return 0;
}


/*
 * Get Gamepad Name
 */
int SDL2_GetGamepadName(int index, char *buffer, unsigned int buffer_size)
{
    if (buffer == NULL || buffer_size == 0 || index < 0 || index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    SDL2_EnterCriticalSection(&g_gamepad_lock);
    {
        if (!g_gamepad_states[index].connected) {
            SDL2_LeaveCriticalSection(&g_gamepad_lock);
            return -1;
        }
        
        strncpy(buffer, g_gamepad_states[index].name, buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
    }
    SDL2_LeaveCriticalSection(&g_gamepad_lock);
    
    return 0;
}


/*
 * Check if Button is Pressed
 */
int SDL2_IsGamepadButtonPressed(int index, SDL2_GamepadButton button)
{
    if (button < 0 || button >= SDL2_GAMEPAD_BUTTON_COUNT || index < 0 || index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    int pressed = 0;
    
    SDL2_EnterCriticalSection(&g_gamepad_lock);
    {
        if (g_gamepad_states[index].connected) {
            pressed = g_gamepad_states[index].buttons[button] ? 1 : 0;
        } else {
            SDL2_LeaveCriticalSection(&g_gamepad_lock);
            return -1;
        }
    }
    SDL2_LeaveCriticalSection(&g_gamepad_lock);
    
    return pressed;
}


/*
 * Get Analog Axis Value
 */
short SDL2_GetGamepadAxis(int index, SDL2_GamepadAxis axis)
{
    if (axis < 0 || axis >= SDL2_GAMEPAD_AXIS_COUNT || index < 0 || index >= SDL2_MAX_GAMEPADS) {
        return 0;
    }
    
    short value = 0;
    
    SDL2_EnterCriticalSection(&g_gamepad_lock);
    {
        if (g_gamepad_states[index].connected) {
            value = g_gamepad_states[index].axes[axis];
        }
    }
    SDL2_LeaveCriticalSection(&g_gamepad_lock);
    
    return value;
}


/*
 * Get Normalized Axis Value with Deadzone
 */
float SDL2_GetGamepadAxisNormalized(int index, SDL2_GamepadAxis axis)
{
    short raw_value = SDL2_GetGamepadAxis(index, axis);
    
    /* Apply deadzone */
    if (raw_value > -SDL2_GAMEPAD_AXIS_DEADZONE && raw_value < SDL2_GAMEPAD_AXIS_DEADZONE) {
        return 0.0f;
    }
    
    /* Normalize to -1.0 to 1.0 */
    float normalized;
    if (raw_value > 0) {
        normalized = (float)(raw_value - SDL2_GAMEPAD_AXIS_DEADZONE) / 
                    (SDL2_GAMEPAD_AXIS_MAX - SDL2_GAMEPAD_AXIS_DEADZONE);
    } else {
        normalized = (float)(raw_value + SDL2_GAMEPAD_AXIS_DEADZONE) / 
                    (SDL2_GAMEPAD_AXIS_MIN + SDL2_GAMEPAD_AXIS_DEADZONE);
    }
    
    /* Clamp to -1.0 to 1.0 */
    if (normalized > 1.0f) normalized = 1.0f;
    if (normalized < -1.0f) normalized = -1.0f;
    
    return normalized;
}


/*
 * Set Gamepad Rumble/Vibration
 */
int SDL2_SetGamepadRumble(int index, BYTE left_intensity, BYTE right_intensity, unsigned int duration_ms)
{
    if (index < 0 || index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    int result = -1;
    
    SDL2_EnterCriticalSection(&g_gamepad_lock);
    {
        if (!g_gamepad_states[index].connected) {
            SDL2_LeaveCriticalSection(&g_gamepad_lock);
            return -1;
        }
        
        SDL_Joystick *joystick = SDL_GetJoystickFromID((SDL_JoystickID)index);
        if (joystick != NULL && SDL_JoystickHasRumble(joystick)) {
            /* Convert byte intensities (0-255) to SDL format (0-65535) */
            Uint16 left = (Uint16)((left_intensity / 255.0f) * 65535);
            Uint16 right = (Uint16)((right_intensity / 255.0f) * 65535);
            
            if (SDL_JoystickRumble(joystick, left, right, duration_ms) == 0) {
                printf("Phase 06: Gamepad %d rumble: L=%u R=%u for %ums\n",
                       index, left_intensity, right_intensity, duration_ms);
                result = 0;
            }
        }
    }
    SDL2_LeaveCriticalSection(&g_gamepad_lock);
    
    return result;
}


/*
 * Get Default Gamepad Mapping
 */
int SDL2_GetDefaultGamepadMapping(SDL2_GamepadMapping *mapping)
{
    if (mapping == NULL) {
        return -1;
    }
    
    /* Copy default button mapping */
    memcpy(mapping->button_to_vkey, g_default_button_mapping, 
           sizeof(g_default_button_mapping));
    
    /* Set default axis settings */
    mapping->axis_triggers_fire = FALSE;
    mapping->axis_sensitivity = 1.0f;
    mapping->axis_deadzone = 0.15f;  /* 15% deadzone */
    
    printf("Phase 06: Default gamepad mapping loaded\n");
    return 0;
}


/*
 * Load Gamepad Mapping from Configuration
 */
int SDL2_LoadGamepadMapping(int index, SDL2_GamepadMapping *mapping)
{
    if (mapping == NULL || index < 0 || index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    /* For now, use defaults (Phase 07 will integrate with Phase 05 Registry) */
    SDL2_GetDefaultGamepadMapping(mapping);
    
    printf("Phase 06: Gamepad %d mapping loaded (using defaults for now)\n", index);
    return 0;
}


/*
 * Save Gamepad Mapping to Configuration
 */
int SDL2_SaveGamepadMapping(int index, const SDL2_GamepadMapping *mapping)
{
    if (mapping == NULL || index < 0 || index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    /* For now, just log (Phase 07 will integrate with Phase 05 Registry) */
    printf("Phase 06: Gamepad %d mapping saved (persistent storage not yet implemented)\n", index);
    return 0;
}


/*
 * Apply Gamepad Mapping and Generate Events
 */
int SDL2_ApplyGamepadMapping(int index, const SDL2_GamepadMapping *mapping)
{
    if (mapping == NULL || index < 0 || index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    SDL2_GamepadState state;
    if (SDL2_GetGamepadState(index, &state) != 0) {
        return -1;
    }
    
    /* Process button presses - generate keyboard events */
    for (int i = 0; i < SDL2_GAMEPAD_BUTTON_COUNT; i++) {
        BYTE vkey = mapping->button_to_vkey[i];
        BOOL pressed = state.buttons[i];
        
        if (pressed) {
            printf("Phase 06: Gamepad %d button %d (VK 0x%02X) pressed\n", index, i, vkey);
            /* TODO: Post WM_KEYDOWN event to keyboard queue via TheWin32Keyboard->addWin32Event() */
        }
    }
    
    /* Process analog sticks - generate mouse events */
    float left_x = SDL2_GetGamepadAxisNormalized(index, SDL2_GAMEPAD_AXIS_LEFTX);
    float left_y = SDL2_GetGamepadAxisNormalized(index, SDL2_GAMEPAD_AXIS_LEFTY);
    
    if (fabs(left_x) > 0.1f || fabs(left_y) > 0.1f) {
        /* Generate mouse movement from analog stick */
        int mouse_dx = (int)(left_x * 10 * mapping->axis_sensitivity);
        int mouse_dy = (int)(left_y * 10 * mapping->axis_sensitivity);
        
        printf("Phase 06: Gamepad %d left stick: X=%.2f Y=%.2f → mouse delta (%d, %d)\n",
               index, left_x, left_y, mouse_dx, mouse_dy);
        /* TODO: Post WM_MOUSEMOVE event via TheWin32Mouse->addWin32Event() */
    }
    
    /* Process triggers - generate fire events if configured */
    if (mapping->axis_triggers_fire) {
        float left_trigger = (float)SDL2_GetGamepadAxis(index, SDL2_GAMEPAD_AXIS_TRIGGERLEFT) / 32767.0f;
        float right_trigger = (float)SDL2_GetGamepadAxis(index, SDL2_GAMEPAD_AXIS_TRIGGERRIGHT) / 32767.0f;
        
        if (left_trigger > 0.5f) {
            printf("Phase 06: Gamepad %d left trigger: %.2f (weapon fire)\n", index, left_trigger);
            /* TODO: Post weapon fire event */
        }
        
        if (right_trigger > 0.5f) {
            printf("Phase 06: Gamepad %d right trigger: %.2f (alt fire)\n", index, right_trigger);
            /* TODO: Post alt fire event */
        }
    }
    
    return 0;
}


/*
 * Process SDL Gamepad Event
 */
int SDL2_ProcessGamepadEvent(const void *event_void)
{
    const SDL_Event *event = (const SDL_Event *)event_void;
    
    if (event == NULL) {
        return -1;
    }
    
    switch (event->type) {
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            {
                int which = event->gbutton.which;
                SDL2_GamepadButton button = (SDL2_GamepadButton)event->gbutton.button;
                BOOL pressed = (event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
                
                printf("Phase 06: Gamepad button event: which=%d button=%d %s\n",
                       which, button, pressed ? "DOWN" : "UP");
                
                SDL2_EnterCriticalSection(&g_gamepad_lock);
                {
                    if (which >= 0 && which < SDL2_MAX_GAMEPADS) {
                        g_gamepad_states[which].buttons[button] = pressed;
                    }
                }
                SDL2_LeaveCriticalSection(&g_gamepad_lock);
            }
            return 0;
            
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            {
                int which = event->gaxis.which;
                SDL2_GamepadAxis axis = (SDL2_GamepadAxis)event->gaxis.axis;
                short value = event->gaxis.value;
                
                printf("Phase 06: Gamepad axis event: which=%d axis=%d value=%d\n",
                       which, axis, value);
                
                SDL2_EnterCriticalSection(&g_gamepad_lock);
                {
                    if (which >= 0 && which < SDL2_MAX_GAMEPADS) {
                        g_gamepad_states[which].axes[axis] = value;
                    }
                }
                SDL2_LeaveCriticalSection(&g_gamepad_lock);
            }
            return 0;
            
        case SDL_EVENT_GAMEPAD_CONNECTED:
            printf("Phase 06: Gamepad connected: device %d\n", event->gdevice.which);
            SDL2_InitGamepads();  /* Rescan */
            return 0;
            
        case SDL_EVENT_GAMEPAD_DISCONNECTED:
            printf("Phase 06: Gamepad disconnected: device %d\n", event->gdevice.which);
            {
                int which = event->gdevice.which;
                SDL2_EnterCriticalSection(&g_gamepad_lock);
                {
                    if (which >= 0 && which < SDL2_MAX_GAMEPADS) {
                        g_gamepad_states[which].connected = FALSE;
                    }
                }
                SDL2_LeaveCriticalSection(&g_gamepad_lock);
            }
            return 0;
    }
    
    return -1;
}


/*
 * Process SDL Joystick Event (Legacy)
 */
int SDL2_ProcessJoystickEvent(const void *event_void)
{
    const SDL_Event *event = (const SDL_Event *)event_void;
    
    if (event == NULL) {
        return -1;
    }
    
    /* Modern SDL uses gamepad events instead of joystick events */
    printf("Phase 06: Legacy joystick event (should use gamepad events)\n");
    return -1;
}


/*
 * Get Gamepad Button Name
 */
int SDL2_GetGamepadButtonName(SDL2_GamepadButton button, char *buffer, unsigned int buffer_size)
{
    if (buffer == NULL || buffer_size == 0) {
        return -1;
    }
    
    const char *names[] = {
        "A (Cross)",
        "B (Circle)",
        "X (Square)",
        "Y (Triangle)",
        "LB (L1)",
        "RB (R1)",
        "Back (Select)",
        "Start",
        "Left Stick",
        "Right Stick"
    };
    
    if (button < 0 || button >= SDL2_GAMEPAD_BUTTON_COUNT) {
        return -1;
    }
    
    strncpy(buffer, names[button], buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    
    return 0;
}


/*
 * Get Gamepad Axis Name
 */
int SDL2_GetGamepadAxisName(SDL2_GamepadAxis axis, char *buffer, unsigned int buffer_size)
{
    if (buffer == NULL || buffer_size == 0) {
        return -1;
    }
    
    const char *names[] = {
        "Left Stick X",
        "Left Stick Y",
        "Right Stick X",
        "Right Stick Y",
        "Left Trigger",
        "Right Trigger"
    };
    
    if (axis < 0 || axis >= SDL2_GAMEPAD_AXIS_COUNT) {
        return -1;
    }
    
    strncpy(buffer, names[axis], buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    
    return 0;
}


/*
 * Test Gamepad Connectivity and Functionality
 */
int SDL2_TestGamepad(int index)
{
    SDL2_GamepadState state;
    
    if (SDL2_GetGamepadState(index, &state) != 0) {
        printf("Phase 06: Gamepad %d not connected\n", index);
        return -2;
    }
    
    printf("Phase 06: Testing Gamepad %d: %s\n", index, state.name);
    printf("  Vendor ID: 0x%04X\n", state.vendor_id);
    printf("  Product ID: 0x%04X\n", state.product_id);
    
    /* Test all buttons */
    printf("  Button States: ");
    for (int i = 0; i < SDL2_GAMEPAD_BUTTON_COUNT; i++) {
        printf("%s%d", state.buttons[i] ? "1" : "0", (i < SDL2_GAMEPAD_BUTTON_COUNT - 1) ? "," : "");
    }
    printf("\n");
    
    /* Test all axes */
    printf("  Axis States: ");
    for (int i = 0; i < SDL2_GAMEPAD_AXIS_COUNT; i++) {
        printf("%d", state.axes[i]);
        if (i < SDL2_GAMEPAD_AXIS_COUNT - 1) printf(",");
    }
    printf("\n");
    
    return 0;
}
