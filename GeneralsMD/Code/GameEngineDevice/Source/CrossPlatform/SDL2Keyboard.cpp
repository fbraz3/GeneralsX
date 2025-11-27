// ============================================================================
// SDL2 Keyboard Implementation
// Cross-platform keyboard input handling using SDL2
// ============================================================================

#include "CrossPlatform/SDL2Keyboard.h"
#include "GameClient/KeyDefs.h"
#include <SDL2/SDL.h>
#include <cstring>

// ============================================================================
// Constructor / Destructor
// ============================================================================

SDL2Keyboard::SDL2Keyboard() : Keyboard()
{
    DEBUG_LOG(("SDL2Keyboard: Created"));
    m_sdlKeyState = nullptr;
    m_numKeys = 0;
}

SDL2Keyboard::~SDL2Keyboard()
{
    DEBUG_LOG(("SDL2Keyboard: Destroyed"));
}

// ============================================================================
// Initialization / Reset
// ============================================================================

void SDL2Keyboard::init(void)
{
    DEBUG_LOG(("SDL2Keyboard::init() - Initializing SDL2 keyboard input"));
    Keyboard::init();
    
    // Get SDL2 keyboard state
    m_sdlKeyState = SDL_GetKeyboardState(&m_numKeys);
    if (!m_sdlKeyState) {
        DEBUG_LOG(("SDL2Keyboard::init() - WARNING: Could not get SDL keyboard state"));
        m_numKeys = 0;
    }
    
    initKeyNames();
}

void SDL2Keyboard::reset(void)
{
    DEBUG_LOG(("SDL2Keyboard::reset()"));
    Keyboard::reset();
}

// ============================================================================
// Update
// ============================================================================

void SDL2Keyboard::update(void)
{
    Keyboard::update();
    
    // Update SDL2 keyboard state
    m_sdlKeyState = SDL_GetKeyboardState(&m_numKeys);
}

// ============================================================================
// Caps Lock State
// ============================================================================

Bool SDL2Keyboard::getCapsState(void)
{
    // Get caps lock state from SDL
    SDL_Keymod modifiers = SDL_GetModState();
    return (modifiers & KMOD_CAPS) ? TRUE : FALSE;
}

// ============================================================================
// Key Input
// ============================================================================

void SDL2Keyboard::getKey(KeyboardIO* key)
{
    if (!key || !m_sdlKeyState) {
        key->key = KEY_NONE;
        return;
    }

    // Poll SDL2 events to get keyboard input
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
            {
                SDL_Keycode sdlKey = event.key.keysym.sym;
                unsigned char gameKey = sdlKeyToGameKey(sdlKey);
                
                if (gameKey != KEY_NONE) {
                    key->key = gameKey;
                    key->state = KEY_STATE_DOWN;
                    key->status = KeyboardIO::STATUS_UNUSED;
                    
                    // Update modifiers
                    updateModifiers();
                    return;
                }
            }
            break;

            case SDL_KEYUP:
            {
                SDL_Keycode sdlKey = event.key.keysym.sym;
                unsigned char gameKey = sdlKeyToGameKey(sdlKey);
                
                if (gameKey != KEY_NONE) {
                    key->key = gameKey;
                    key->state = KEY_STATE_UP;
                    key->status = KeyboardIO::STATUS_UNUSED;
                    
                    // Update modifiers
                    updateModifiers();
                    return;
                }
            }
            break;
        }
    }
    
    key->key = KEY_NONE;
}

// ============================================================================
// Key Mapping - SDL2 to Game Keys
// ============================================================================

unsigned char SDL2Keyboard::sdlKeyToGameKey(int sdlKey)
{
    // Map SDL2 keycodes to game keycodes (DirectInput-compatible)
    // Based on KeyDefs.h enum
    
    switch (sdlKey) {
        // Function keys
        case SDLK_ESCAPE: return KEY_ESC;
        case SDLK_F1: return KEY_F1;
        case SDLK_F2: return KEY_F2;
        case SDLK_F3: return KEY_F3;
        case SDLK_F4: return KEY_F4;
        case SDLK_F5: return KEY_F5;
        case SDLK_F6: return KEY_F6;
        case SDLK_F7: return KEY_F7;
        case SDLK_F8: return KEY_F8;
        case SDLK_F9: return KEY_F9;
        case SDLK_F10: return KEY_F10;
        case SDLK_F11: return KEY_F11;
        case SDLK_F12: return KEY_F12;
        
        // Number keys
        case SDLK_1: return KEY_1;
        case SDLK_2: return KEY_2;
        case SDLK_3: return KEY_3;
        case SDLK_4: return KEY_4;
        case SDLK_5: return KEY_5;
        case SDLK_6: return KEY_6;
        case SDLK_7: return KEY_7;
        case SDLK_8: return KEY_8;
        case SDLK_9: return KEY_9;
        case SDLK_0: return KEY_0;
        
        // Letter keys
        case SDLK_a: return KEY_A;
        case SDLK_b: return KEY_B;
        case SDLK_c: return KEY_C;
        case SDLK_d: return KEY_D;
        case SDLK_e: return KEY_E;
        case SDLK_f: return KEY_F;
        case SDLK_g: return KEY_G;
        case SDLK_h: return KEY_H;
        case SDLK_i: return KEY_I;
        case SDLK_j: return KEY_J;
        case SDLK_k: return KEY_K;
        case SDLK_l: return KEY_L;
        case SDLK_m: return KEY_M;
        case SDLK_n: return KEY_N;
        case SDLK_o: return KEY_O;
        case SDLK_p: return KEY_P;
        case SDLK_q: return KEY_Q;
        case SDLK_r: return KEY_R;
        case SDLK_s: return KEY_S;
        case SDLK_t: return KEY_T;
        case SDLK_u: return KEY_U;
        case SDLK_v: return KEY_V;
        case SDLK_w: return KEY_W;
        case SDLK_x: return KEY_X;
        case SDLK_y: return KEY_Y;
        case SDLK_z: return KEY_Z;
        
        // Special keys
        case SDLK_SPACE: return KEY_SPACE;
        case SDLK_RETURN: return KEY_ENTER;
        case SDLK_BACKSPACE: return KEY_BACKSPACE;
        case SDLK_TAB: return KEY_TAB;
        case SDLK_DELETE: return KEY_DEL;
        case SDLK_INSERT: return KEY_INS;
        case SDLK_HOME: return KEY_HOME;
        case SDLK_END: return KEY_END;
        case SDLK_PAGEUP: return KEY_PGUP;
        case SDLK_PAGEDOWN: return KEY_PGDN;
        
        // Arrow keys
        case SDLK_LEFT: return KEY_LEFT;
        case SDLK_RIGHT: return KEY_RIGHT;
        case SDLK_UP: return KEY_UP;
        case SDLK_DOWN: return KEY_DOWN;
        
        // Modifiers
        case SDLK_LSHIFT: return KEY_LSHIFT;
        case SDLK_RSHIFT: return KEY_RSHIFT;
        case SDLK_LCTRL: return KEY_LCTRL;
        case SDLK_RCTRL: return KEY_RCTRL;
        case SDLK_LALT: return KEY_LALT;
        case SDLK_RALT: return KEY_RALT;
        
        // Numpad keys
        case SDLK_KP_0: return KEY_KP0;
        case SDLK_KP_1: return KEY_KP1;
        case SDLK_KP_2: return KEY_KP2;
        case SDLK_KP_3: return KEY_KP3;
        case SDLK_KP_4: return KEY_KP4;
        case SDLK_KP_5: return KEY_KP5;
        case SDLK_KP_6: return KEY_KP6;
        case SDLK_KP_7: return KEY_KP7;
        case SDLK_KP_8: return KEY_KP8;
        case SDLK_KP_9: return KEY_KP9;
        case SDLK_KP_DIVIDE: return KEY_KPSLASH;
        case SDLK_KP_MULTIPLY: return KEY_KPSTAR;
        case SDLK_KP_MINUS: return KEY_KPMINUS;
        case SDLK_KP_PLUS: return KEY_KPPLUS;
        case SDLK_KP_PERIOD: return KEY_KPDEL;
        case SDLK_KP_ENTER: return KEY_KPENTER;
        
        default: return KEY_NONE;
    }
}

// ============================================================================
// Modifier Keys Update
// ============================================================================

void SDL2Keyboard::updateModifiers(void)
{
    SDL_Keymod modifiers = SDL_GetModState();
    m_modifiers = 0;
    
    if (modifiers & KMOD_SHIFT) m_modifiers |= (1 << 0);
    if (modifiers & KMOD_CTRL) m_modifiers |= (1 << 1);
    if (modifiers & KMOD_ALT) m_modifiers |= (1 << 2);
    if (modifiers & KMOD_CAPS) m_modifiers |= (1 << 3);
}

// ============================================================================
// Factory Function
// ============================================================================

Keyboard* CreateKeyboard(void)
{
    DEBUG_LOG(("CreateKeyboard: Creating SDL2Keyboard"));
    return NEW SDL2Keyboard;
}
