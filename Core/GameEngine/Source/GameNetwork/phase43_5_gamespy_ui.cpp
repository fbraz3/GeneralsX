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

/**
 * Phase 43.5: GameSpy Integration - UI & Overlay Implementation
 *
 * Overlay windows and UI functions for GameSpy features.
 * Real implementations copied from GameSpyOverlay.cpp production code.
 *
 * Architecture:
 * - Cross-platform: WindowLayout system used for both Win32 and POSIX
 * - Vulkan: Render integration via existing UI system
 * - Message boxes: GameWindow system with callback support
 */

#include "PreRTS.h"

#include "GameNetwork/GameSpyOverlay.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/MessageBox.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Gadget.h"

 // ============================================================================
 // Static Message Box Management
 // ============================================================================

 /**
  * Static message box window and callback tracking
  * Maintains pointer to active message box and callback functions
  */
static GameWindow* g_messageBoxWindow = NULL;
static GameWinMsgBoxFunc g_okFunc = NULL;
static GameWinMsgBoxFunc g_cancelFunc = NULL;

/**
 * Static overlay layout storage
 * Maintains WindowLayout pointers for each overlay type
 */
static WindowLayout* g_overlayLayouts[GSOVERLAY_MAX] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

/**
 * Overlay window file paths
 * Must be in same order as GSOverlayType enum
 */
static const char* g_gsOverlayFiles[GSOVERLAY_MAX] =
{
	"Menus\\PopupPlayerInfo.wnd",		// GSOVERLAY_PLAYERINFO
	"Menus\\WOLMapSelectMenu.wnd",		// GSOVERLAY_MAPSELECT
	"Menus\\WOLBuddyOverlay.wnd",		// GSOVERLAY_BUDDY
	"Menus\\WOLPageOverlay.wnd",			// GSOVERLAY_PAGE
	"Menus\\PopupHostGame.wnd",			// GSOVERLAY_GAMEOPTIONS
	"Menus\\PopupJoinGame.wnd",			// GSOVERLAY_GAMEPASSWORD
	"Menus\\PopupLadderSelect.wnd",		// GSOVERLAY_LADDERSELECT
	"Menus\\PopupLocaleSelect.wnd",		// GSOVERLAY_LOCALESELECT
	"Menus\\OptionsMenu.wnd",				// GSOVERLAY_OPTIONS
};

// ============================================================================
// Static Message Box Callbacks
// ============================================================================

/**
 * messageBoxOk
 * Internal callback when Ok button is clicked in message box
 */
static void messageBoxOk(void)
{
	g_messageBoxWindow = NULL;
	if (g_okFunc) {
		g_okFunc();
		g_okFunc = NULL;
	}
}

/**
 * messageBoxCancel
 * Internal callback when Cancel button is clicked in message box
 */
static void messageBoxCancel(void)
{
	g_messageBoxWindow = NULL;
	if (g_cancelFunc) {
		g_cancelFunc();
		g_cancelFunc = NULL;
	}
}

// ============================================================================
// Message Box Functions (Real Implementation)
// ============================================================================

/**
 * ClearGSMessageBoxes
 * Tear down any active GameSpy message boxes
 *
 * Implementation:
 * - Destroys active message box window via WindowManager
 * - Clears all callback pointers
 * - Called before opening new message boxes (ensures only one at a time)
 * ✅ COMPLETE: Real implementation
 */
void ClearGSMessageBoxes(void)
{
	if (g_messageBoxWindow) {
		TheWindowManager->winDestroy(g_messageBoxWindow);
		g_messageBoxWindow = NULL;
	}

	if (g_okFunc) {
		g_okFunc = NULL;
	}

	if (g_cancelFunc) {
		g_cancelFunc = NULL;
	}
}

/**
 * GSMessageBoxOk
 * Display a message box with single "Ok" button
 *
 * Parameters:
 * - titleString: Window title
 * - bodyString: Message text
 * - okFunc: Callback function when Ok pressed (optional, may be NULL)
 *
 * Implementation:
 * - Clears any previous message box first
 * - Creates new message box via MessageBoxOk()
 * - Saves callback for execution when user clicks Ok
 * ✅ COMPLETE: Real implementation
 */
void GSMessageBoxOk(UnicodeString titleString, UnicodeString bodyString, GameWinMsgBoxFunc okFunc)
{
	ClearGSMessageBoxes();
	g_messageBoxWindow = MessageBoxOk(titleString, bodyString, messageBoxOk);
	g_okFunc = okFunc;
}

/**
 * GSMessageBoxOkCancel
 * Display a message box with "Ok" and "Cancel" buttons
 *
 * Parameters:
 * - title: Window title
 * - message: Message text
 * - okFunc: Callback for Ok button
 * - cancelFunc: Callback for Cancel button
 *
 * Implementation:
 * - Clears any previous message box first
 * - Creates two-button message box via MessageBoxOkCancel()
 * - Saves both callbacks for execution on button click
 * ✅ COMPLETE: Real implementation
 */
void GSMessageBoxOkCancel(UnicodeString title, UnicodeString message, GameWinMsgBoxFunc okFunc, GameWinMsgBoxFunc cancelFunc)
{
	ClearGSMessageBoxes();
	g_messageBoxWindow = MessageBoxOkCancel(title, message, messageBoxOk, messageBoxCancel);
	g_okFunc = okFunc;
	g_cancelFunc = cancelFunc;
}

/**
 * GSMessageBoxYesNo
 * Display a message box with "Yes" and "No" buttons
 *
 * Parameters:
 * - title: Window title
 * - message: Message text
 * - yesFunc: Callback for Yes button
 * - noFunc: Callback for No button
 *
 * Implementation:
 * - Clears any previous message box first
 * - Creates two-button message box via MessageBoxYesNo()
 * - Saves both callbacks (yes->okFunc, no->cancelFunc) for execution
 * ✅ COMPLETE: Real implementation
 */
void GSMessageBoxYesNo(UnicodeString title, UnicodeString message, GameWinMsgBoxFunc yesFunc, GameWinMsgBoxFunc noFunc)
{
	ClearGSMessageBoxes();
	g_messageBoxWindow = MessageBoxYesNo(title, message, messageBoxOk, messageBoxCancel);
	g_okFunc = yesFunc;
	g_cancelFunc = noFunc;
}

/**
 * RaiseGSMessageBox
 * Bring GameSpy message box to foreground
 *
 * Implementation:
 * - Called when transitioning screens while message box is active
 * - Ensures message box remains visible above new screen
 * - Calls winBringToTop() on message box window
 * ✅ COMPLETE: Real implementation
 */
void RaiseGSMessageBox(void)
{
	if (!g_messageBoxWindow) {
		return;
	}

	g_messageBoxWindow->winBringToTop();
}

// ============================================================================
// Overlay Functions (Real Implementation)
// ============================================================================

/**
 * GameSpyOpenOverlay
 * Open a GameSpy overlay window (player info, buddy list, map select, etc)
 *
 * Parameters:
 * - overlayType: One of GSOverlayType enum values
 *
 * Implementation:
 * - Loads overlay .wnd file template
 * - Creates WindowLayout instance from template
 * - Runs initialization callbacks
 * - Makes overlay visible and brings to foreground
 * ✅ COMPLETE: Real implementation
 */
void GameSpyOpenOverlay(GSOverlayType overlayType)
{
	if (overlayType < 0 || overlayType >= GSOVERLAY_MAX) {
		return;  // Invalid overlay type
	}

	// Check if already open
	if (g_overlayLayouts[overlayType] != NULL) {
		// Already open - just bring to front
		g_overlayLayouts[overlayType]->bringForward();
		return;
	}

	// Load and initialize overlay layout from template
	g_overlayLayouts[overlayType] = TheWindowManager->winCreateLayout(AsciiString(g_gsOverlayFiles[overlayType]));

	if (g_overlayLayouts[overlayType]) {
		// Run initialization callbacks
		g_overlayLayouts[overlayType]->runInit();
		// Show overlay window
		g_overlayLayouts[overlayType]->hide(FALSE);
		// Bring to foreground
		g_overlayLayouts[overlayType]->bringForward();
	}
}

/**
 * GameSpyCloseOverlay
 * Close a specific GameSpy overlay window
 *
 * Parameters:
 * - overlayType: Overlay to close
 *
 * Implementation:
 * - Runs shutdown callbacks on layout
 * - Destroys all windows in layout tree
 * - Deletes layout instance
 * - Clears pointer to release resources
 * ✅ COMPLETE: Real implementation
 */
void GameSpyCloseOverlay(GSOverlayType overlayType)
{
	if (overlayType < 0 || overlayType >= GSOVERLAY_MAX) {
		return;  // Invalid overlay type
	}

	if (g_overlayLayouts[overlayType] == NULL) {
		return;  // Already closed
	}

	// Run shutdown callbacks
	g_overlayLayouts[overlayType]->runShutdown();
	// Destroy all windows in layout
	g_overlayLayouts[overlayType]->destroyWindows();
	// Delete the layout instance
	deleteInstance(g_overlayLayouts[overlayType]);
	g_overlayLayouts[overlayType] = NULL;
}

/**
 * GameSpyCloseAllOverlays
 * Close all open GameSpy overlay windows
 *
 * Implementation:
 * - Iterates through all overlay types
 * - Calls GameSpyCloseOverlay() for each open overlay
 * - Ensures clean state for screen transitions
 * ✅ COMPLETE: Real implementation
 */
void GameSpyCloseAllOverlays(void)
{
	for (int i = 0; i < GSOVERLAY_MAX; ++i) {
		GameSpyCloseOverlay((GSOverlayType)i);
	}
}

/**
 * GameSpyIsOverlayOpen
 * Check if a specific overlay is currently displayed
 *
 * Parameters:
 * - overlayType: Overlay to check
 *
 * Returns: TRUE if overlay layout is loaded, FALSE otherwise
 *
 * Implementation:
 * - Returns TRUE if WindowLayout pointer is non-NULL
 * - WindowLayout pointer is NULL when overlay is closed
 * ✅ COMPLETE: Real implementation
 */
Bool GameSpyIsOverlayOpen(GSOverlayType overlayType)
{
	if (overlayType < 0 || overlayType >= GSOVERLAY_MAX) {
		return FALSE;  // Invalid overlay type
	}

	return (g_overlayLayouts[overlayType] != NULL);
}

/**
 * GameSpyToggleOverlay
 * Toggle a GameSpy overlay window (close if open, open if closed)
 *
 * Parameters:
 * - overlayType: Overlay to toggle
 *
 * Implementation:
 * - Checks current state via GameSpyIsOverlayOpen()
 * - Calls GameSpyCloseOverlay() if open
 * - Calls GameSpyOpenOverlay() if closed
 * ✅ COMPLETE: Real implementation
 */
void GameSpyToggleOverlay(GSOverlayType overlayType)
{
	if (GameSpyIsOverlayOpen(overlayType)) {
		GameSpyCloseOverlay(overlayType);
	}
	else {
		GameSpyOpenOverlay(overlayType);
	}
}

/**
 * GameSpyUpdateOverlays
 * Update all open GameSpy overlay windows with current state
 *
 * Implementation:
 * - Called once per game frame
 * - Iterates all overlay types
 * - Calls runUpdate() on each active overlay layout
 * - Updates UI elements with latest player/game data
 * ✅ COMPLETE: Real implementation
 */
void GameSpyUpdateOverlays(void)
{
	for (int i = 0; i < GSOVERLAY_MAX; ++i) {
		if (g_overlayLayouts[i]) {
			g_overlayLayouts[i]->runUpdate();
		}
	}
}

/**
 * ReOpenPlayerInfo
 * Reopen the player information overlay if it was previously open
 *
 * Implementation:
 * - Sets internal flag for next CheckReOpenPlayerInfo() call
 * - Used to restore overlay visibility after screen transitions
 * ✅ COMPLETE: Real implementation
 */
static Bool g_reOpenPlayerInfoFlag = FALSE;

void ReOpenPlayerInfo(void)
{
	g_reOpenPlayerInfoFlag = TRUE;
}

/**
 * CheckReOpenPlayerInfo
 * Check if player info overlay should be reopened after screen change
 *
 * Implementation:
 * - Checks if reopening is requested via flag
 * - Calls GameSpyOpenOverlay() if flag is set
 * - Clears flag after reopening
 * - Called during screen transitions to restore UI state
 * ✅ COMPLETE: Real implementation
 */
void CheckReOpenPlayerInfo(void)
{
	if (!g_reOpenPlayerInfoFlag) {
		return;
	}

	GameSpyOpenOverlay(GSOVERLAY_PLAYERINFO);
	g_reOpenPlayerInfoFlag = FALSE;
}
