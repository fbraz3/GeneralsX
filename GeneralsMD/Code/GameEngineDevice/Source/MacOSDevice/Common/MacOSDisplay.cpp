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
 * @return OSDBT_OK always (no actual dialog shown)
 */
OSDisplayButtonType OSDisplayWarningBox(AsciiString p, AsciiString m, UnsignedInt buttonFlags, UnsignedInt otherFlags)
{
    if (!TheGameText)
    {
        std::cerr << "Warning Box (No Text System): " << p.str() << " - " << m.str() << std::endl;
        return OSDBT_ERROR;
    }

    UnicodeString promptStr = TheGameText->fetch(p);
    UnicodeString mesgStr = TheGameText->fetch(m);

    AsciiString promptA, mesgA;
    promptA.translate(promptStr);
    mesgA.translate(mesgStr);
    
    // Print to console for now (TODO: implement native macOS dialog)
    std::cerr << "Warning Box: " << promptA.str() << " - " << mesgA.str() << std::endl;
    
    return OSDBT_OK;
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
