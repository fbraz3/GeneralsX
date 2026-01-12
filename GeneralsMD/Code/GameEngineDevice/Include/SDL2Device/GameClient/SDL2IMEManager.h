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

#pragma once

#include "GameClient/IMEManager.h"
#include "Common/UnicodeString.h"
#include <SDL2/SDL.h>

// Forward declarations
class GameWindow;

/**
 * SDL2IMEManager - Text input and IME composition manager for SDL2
 *
 * Handles text input including IME composition events for CJK and other
 * writing systems. Manages composition string, candidates, and text commit.
 *
 * Key Features:
 * - Text composition support via SDL_TEXTEDITING events
 * - Committed text support via SDL_TEXTINPUT events
 * - UTF-8 to UnicodeString conversion
 * - Composition state tracking
 * - Integration with TheWindowManager for message dispatch
 */
class SDL2IMEManager : public IMEManagerInterface
{
public:
    SDL2IMEManager();
    virtual ~SDL2IMEManager();

    // SubsystemInterface methods (virtual overrides)
    virtual void init(void) override;
    virtual void reset(void) override;
    virtual void update(void) override;

    // IMEManagerInterface methods (virtual overrides)
    virtual void attach(GameWindow *window) override;
    virtual void detatch(void) override;
    virtual void enable(void) override;
    virtual void disable(void) override;
    virtual Bool isEnabled(void) override;
    virtual Bool isAttachedTo(GameWindow *window) override;
    virtual GameWindow* getWindow(void) override;
    virtual Bool isComposing(void) override;
    virtual void getCompositionString(UnicodeString &string) override;
    virtual Int getCompositionCursorPosition(void) override;
    virtual Int getIndexBase(void) override;
    virtual Int getCandidateCount(void) override;
    virtual const UnicodeString* getCandidate(Int index) override;
    virtual Int getSelectedCandidateIndex(void) override;
    virtual Int getCandidatePageSize(void) override;
    virtual Int getCandidatePageStart(void) override;
    virtual Bool serviceIMEMessage(void *windowsHandle,
                                   UnsignedInt message,
                                   Int wParam,
                                   Int lParam) override;
    virtual Int result(void) override;

    /**
     * Handle SDL2 text editing event (composition)
     * Called when IME is composing text (CJK, Cyrillic, etc.)
     * @param event SDL_TextEditingEvent
     */
    void onTextEditing(const SDL_TextEditingEvent &event);

    /**
     * Handle SDL2 text input event (committed text)
     * Called when user confirms text input (after composition or direct typing)
     * @param event SDL_TextInputEvent
     */
    void onTextInput(const SDL_TextInputEvent &event);

private:
    // Manager state
    GameWindow *m_window;        ///< Window we're attached to (nullptr if detached)
    Bool m_enabled;              ///< Is IME enabled for this window
    Bool m_composing;            ///< Are we currently composing text
    Int m_disabled;              ///< Disable counter (can be nested)

    // Composition state
    UnicodeString m_compositionString;     ///< Current composition string (being edited)
    UnicodeString m_resultsString;         ///< Results string (committed text)
    Int m_compositionCursorPos;            ///< Cursor position in composition string
    Int m_compositionStringLength;         ///< Length of composition string

    // Candidate list (IME suggestions)
    Int m_candidateCount;       ///< Total number of candidates
    Int m_selectedIndex;        ///< Currently selected candidate
    Int m_pageStart;            ///< First visible candidate index
    Int m_pageSize;             ///< Max candidates per page
    Int m_indexBase;            ///< Index base (0 or 1) for candidate numbering

    // Last message result
    Int m_result;               ///< Return value of last serviceIMEMessage call

    /**
     * Convert UTF-8 string to UnicodeString
     * @param utf8 UTF-8 encoded C string
     * @return Converted UnicodeString
     */
    UnicodeString utf8ToUnicode(const char *utf8) const;

    /**
     * Send composition text to attached window
     * Sends GWM_IME_CHAR messages for each character in composition string
     */
    void sendCompositionToWindow(void);

    /**
     * Send committed text to attached window
     * Sends GWM_IME_CHAR messages for each character in results string
     */
    void sendResultsToWindow(void);

    /**
     * Clear all composition and candidate data
     */
    void clearComposition(void);
};

// Global instance
extern SDL2IMEManager *TheSDL2IMEManager;

#endif  // SDL2IMEMANAGER_H
