/*
**  Command & Conquer Generals Zero Hour(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//  SDL2 IME (Input Method Editor) Implementation
//  Cross-platform text input and composition handling using SDL2
////////////////////////////////////////////////////////////////////////////////

#include "SDL2Device/GameClient/SDL2IMEManager.h"
#include "GameClient/GameWindow.h"
#include "GameClient/WindowManager.h"
#include "Common/UnicodeString.h"
#include <SDL2/SDL.h>
#include <cstring>
#include <cwchar>

// Global instance
SDL2IMEManager *TheSDL2IMEManager = nullptr;

// ============================================================================
// Constructor / Destructor
// ============================================================================

SDL2IMEManager::SDL2IMEManager()
    : m_window(nullptr),
      m_enabled(TRUE),
      m_composing(FALSE),
      m_disabled(0),
      m_compositionCursorPos(0),
      m_compositionStringLength(0),
      m_candidateCount(0),
      m_selectedIndex(0),
      m_pageStart(0),
      m_pageSize(10),
      m_indexBase(0),
      m_result(0)
{
    // Initialize strings to empty
    m_compositionString = L"";
    m_resultsString = L"";
}

SDL2IMEManager::~SDL2IMEManager()
{
    detatch();
}

// ============================================================================
// SubsystemInterface Implementation
// ============================================================================

void SDL2IMEManager::init(void)
{
    // No platform-specific initialization needed for SDL2
    // SDL2 IME is handled automatically by SDL2 subsystem
}

void SDL2IMEManager::reset(void)
{
    clearComposition();
}

void SDL2IMEManager::update(void)
{
    // No per-frame updates needed for SDL2 IME
    // Events are processed in SDL2GameEngine event loop
}

// ============================================================================
// IMEManagerInterface Implementation
// ============================================================================

void SDL2IMEManager::attach(GameWindow *window)
{
    if (window) {
        m_window = window;
        m_enabled = TRUE;
        m_disabled = 0;
        clearComposition();

        // Enable text input for this window
        SDL_StartTextInput();
    }
}

void SDL2IMEManager::detatch(void)
{
    if (m_window) {
        SDL_StopTextInput();
        clearComposition();
        m_window = nullptr;
    }
}

void SDL2IMEManager::enable(void)
{
    m_disabled--;
    if (m_disabled < 0) {
        m_disabled = 0;
    }
    m_enabled = (m_disabled == 0);

    if (m_enabled && m_window) {
        SDL_StartTextInput();
    }
}

void SDL2IMEManager::disable(void)
{
    m_disabled++;
    m_enabled = FALSE;

    if (m_window) {
        SDL_StopTextInput();
    }
}

Bool SDL2IMEManager::isEnabled(void)
{
    return m_enabled;
}

Bool SDL2IMEManager::isAttachedTo(GameWindow *window)
{
    return (m_window == window) && (m_window != nullptr);
}

GameWindow* SDL2IMEManager::getWindow(void)
{
    return m_window;
}

Bool SDL2IMEManager::isComposing(void)
{
    return m_composing;
}

void SDL2IMEManager::getCompositionString(UnicodeString &string)
{
    string = m_compositionString;
}

Int SDL2IMEManager::getCompositionCursorPosition(void)
{
    return m_compositionCursorPos;
}

Int SDL2IMEManager::getIndexBase(void)
{
    return m_indexBase;
}

Int SDL2IMEManager::getCandidateCount(void)
{
    return m_candidateCount;
}

const UnicodeString* SDL2IMEManager::getCandidate(Int index)
{
    // SDL2 doesn't provide candidate list directly
    // This would need to be populated by the IME system
    // For now, return nullptr (not implemented)
    return nullptr;
}

Int SDL2IMEManager::getSelectedCandidateIndex(void)
{
    return m_selectedIndex;
}

Int SDL2IMEManager::getCandidatePageSize(void)
{
    return m_pageSize;
}

Int SDL2IMEManager::getCandidatePageStart(void)
{
    return m_pageStart;
}

Bool SDL2IMEManager::serviceIMEMessage(void *windowsHandle,
                                       UnsignedInt message,
                                       Int wParam,
                                       Int lParam)
{
    // SDL2 IME messages are handled by onTextEditing/onTextInput
    // This method exists for Win32 compatibility but is not used for SDL2
    m_result = 0;
    return FALSE;
}

Int SDL2IMEManager::result(void)
{
    return m_result;
}

// ============================================================================
// Event Handlers
// ============================================================================

void SDL2IMEManager::onTextEditing(const SDL_TextEditingEvent &event)
{
    // This event is fired when IME is composing text
    // event.text: composition string (UTF-8)
    // event.start: start of selection
    // event.length: length of selection

    if (!m_window || !m_enabled) {
        return;
    }

    m_composing = TRUE;

    // Convert UTF-8 composition string to UnicodeString
    m_compositionString = utf8ToUnicode(event.text);
    m_compositionCursorPos = event.start;
    m_compositionStringLength = event.length;

    // Send composition update to window
    sendCompositionToWindow();
}

void SDL2IMEManager::onTextInput(const SDL_TextInputEvent &event)
{
    // This event is fired when text input is confirmed
    // event.text: committed text (UTF-8)

    if (!m_window || !m_enabled) {
        return;
    }

    // If we were composing, the composition is now complete
    if (m_composing) {
        clearComposition();
    }

    // Convert UTF-8 input to UnicodeString
    m_resultsString = utf8ToUnicode(event.text);

    // Send results to window as GWM_IME_CHAR messages
    sendResultsToWindow();

    // Clear results
    m_resultsString = L"";
}

// ============================================================================
// Helper Methods
// ============================================================================

UnicodeString SDL2IMEManager::utf8ToUnicode(const char *utf8) const
{
    if (!utf8 || utf8[0] == '\0') {
        return L"";
    }

    // Simple UTF-8 to UnicodeString conversion
    // This is a basic implementation that works for most cases
    // More sophisticated implementations would handle surrogate pairs, etc.

    UnicodeString result;

    const unsigned char *ptr = reinterpret_cast<const unsigned char *>(utf8);
    while (*ptr != '\0') {
        unsigned int codepoint = 0;

        if ((*ptr & 0x80) == 0) {
            // ASCII character (0xxxxxxx)
            codepoint = *ptr;
            ptr++;
        } else if ((*ptr & 0xE0) == 0xC0) {
            // 2-byte sequence (110xxxxx 10xxxxxx)
            if (ptr[1] == '\0' || (ptr[1] & 0xC0) != 0x80) {
                ptr++;  // Skip invalid sequence
                continue;
            }
            codepoint = ((ptr[0] & 0x1F) << 6) | (ptr[1] & 0x3F);
            ptr += 2;
        } else if ((*ptr & 0xF0) == 0xE0) {
            // 3-byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
            if (ptr[1] == '\0' || ptr[2] == '\0' || 
                (ptr[1] & 0xC0) != 0x80 || (ptr[2] & 0xC0) != 0x80) {
                ptr++;  // Skip invalid sequence
                continue;
            }
            codepoint = ((ptr[0] & 0x0F) << 12) | 
                       ((ptr[1] & 0x3F) << 6) | 
                       (ptr[2] & 0x3F);
            ptr += 3;
        } else if ((*ptr & 0xF8) == 0xF0) {
            // 4-byte sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
            if (ptr[1] == '\0' || ptr[2] == '\0' || ptr[3] == '\0' ||
                (ptr[1] & 0xC0) != 0x80 || (ptr[2] & 0xC0) != 0x80 || (ptr[3] & 0xC0) != 0x80) {
                ptr++;  // Skip invalid sequence
                continue;
            }
            codepoint = ((ptr[0] & 0x07) << 18) |
                       ((ptr[1] & 0x3F) << 12) |
                       ((ptr[2] & 0x3F) << 6) |
                       (ptr[3] & 0x3F);
            ptr += 4;
        } else {
            // Invalid UTF-8 sequence
            ptr++;
            continue;
        }

        // Add codepoint to result
        // For simplicity, truncate to BMP (codepoints > 0xFFFF)
        if (codepoint <= 0xFFFF) {
            result += static_cast<WideChar>(codepoint);
        }
    }

    return result;
}

void SDL2IMEManager::sendCompositionToWindow(void)
{
    if (!m_window || !TheWindowManager) {
        return;
    }

    // Send each character of composition string to window
    // This allows the window to display the composition string as it's being edited
    const WideChar *str = m_compositionString.str();
    if (!str) {
        return;
    }

    for (int i = 0; str[i] != L'\0'; i++) {
        TheWindowManager->winSendInputMsg(m_window, GWM_IME_CHAR, 
                                          static_cast<WindowMsgData>(str[i]), 0);
    }
}

void SDL2IMEManager::sendResultsToWindow(void)
{
    if (!m_window || !TheWindowManager) {
        return;
    }

    // Send each character of results string to window
    // This is the final committed text
    const WideChar *str = m_resultsString.str();
    if (!str) {
        return;
    }

    for (int i = 0; str[i] != L'\0'; i++) {
        TheWindowManager->winSendInputMsg(m_window, GWM_IME_CHAR,
                                          static_cast<WindowMsgData>(str[i]), 0);
    }
}

void SDL2IMEManager::clearComposition(void)
{
    m_composing = FALSE;
    m_compositionString = L"";
    m_resultsString = L"";
    m_compositionCursorPos = 0;
    m_compositionStringLength = 0;
    m_candidateCount = 0;
    m_selectedIndex = 0;
    m_pageStart = 0;
}
