/**
 * @file MacOSDisplay.cpp
 * @brief MacOS-specific display implementations (stubs for cross-platform compilation)
 */

#include "Lib/BaseType.h"
#include "Common/OSDisplay.h"
#include "GameClient/GameText.h"
#include "Common/AsciiString.h"
#include "Common/UnicodeString.h"

#include <iostream>

/**
 * Display a warning message box (macOS stub implementation)
 * 
 * CRITICAL FIX (Phase 34.1): Changed return value from OSDBT_OK to OSDBT_CANCEL
 * to prevent infinite loop in GameAudio CD loading fallback.
 * 
 * Root cause: GameAudio.cpp lines 232-253 has while(TRUE) loop that only breaks if:
 * 1. Music files are found (isMusicAlreadyLoaded() returns TRUE), OR
 * 2. User cancels via dialog box (OSDisplayWarningBox returns OSDBT_CANCEL)
 * 
 * On macOS without physical CD drive, music files are in .big archives and should
 * be loaded via VFS, not CD. This stub returning OSDBT_CANCEL allows graceful exit
 * from legacy CD loading mechanism.
 * 
 * @return OSDBT_CANCEL to allow loop exit (acts as user cancellation)
 */
OSDisplayButtonType OSDisplayWarningBox(AsciiString p, AsciiString m, UnsignedInt buttonFlags, UnsignedInt otherFlags)
{
    if (!TheGameText)
    {
        std::cerr << "Warning Box (No Text System): " << p.str() << " - " << m.str() << std::endl;
        std::cerr << "Returning OSDBT_CANCEL to prevent infinite loop in CD loading fallback" << std::endl;
        return OSDBT_CANCEL;  // CRITICAL: Return CANCEL to break CD loading loop
    }

    UnicodeString promptStr = TheGameText->fetch(p);
    UnicodeString mesgStr = TheGameText->fetch(m);

    AsciiString promptA, mesgA;
    promptA.translate(promptStr);
    mesgA.translate(mesgStr);
    
    // Print to console for now (TODO: implement native macOS dialog)
    std::cerr << "Warning Box: " << promptA.str() << " - " << mesgA.str() << std::endl;
    std::cerr << "Returning OSDBT_CANCEL to allow graceful CD loading fallback exit" << std::endl;
    
    return OSDBT_CANCEL;  // CRITICAL: Return CANCEL instead of OK to break infinite loop
}

/**
 * Set busy state for the display (macOS stub implementation)
 * @param busy If true, set to busy state
 * @param force Force the state change
 */
void OSDisplaySetBusyState(bool busy, bool force)
{
    // Stub implementation - do nothing for now
    // TODO: Implement macOS cursor/busy indicator
    (void)busy;
    (void)force;
}
