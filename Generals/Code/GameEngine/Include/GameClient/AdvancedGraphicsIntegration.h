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
//                                                                            //
//  (c) 2025 Electronic Arts Inc.                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef ADVANCEDGRAPHICSINTEGRATION_H
#define ADVANCEDGRAPHICSINTEGRATION_H

#pragma once

// ============================================================================
// AdvancedGraphicsIntegration
// ============================================================================
// Phase 47 Week 3: Advanced Graphics - Integration Module
// Provides unified API for all graphics effects (post-processing + environment)
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize all advanced graphics subsystems
 * Called during game initialization
 */
Bool AdvancedGraphicsIntegration_Initialize(void);

/**
 * Shutdown all advanced graphics subsystems
 * Called during game shutdown
 */
Bool AdvancedGraphicsIntegration_Shutdown(void);

/**
 * Update graphics effects each frame
 * Called once per frame in render phase
 */
Bool AdvancedGraphicsIntegration_Update(float deltaTime);

/**
 * Apply all graphics effects for rendering
 * Called after scene rendering, before present
 */
Bool AdvancedGraphicsIntegration_ApplyEffects(void);

/**
 * Get overall graphics quality level
 * Returns quality setting (0-3): LOW, MEDIUM, HIGH, MAXIMUM
 */
Int AdvancedGraphicsIntegration_GetQualityLevel(void);

/**
 * Set overall graphics quality level
 * Quality: 0=LOW, 1=MEDIUM, 2=HIGH, 3=MAXIMUM
 * Automatically adjusts all effect parameters for best performance/quality balance
 */
void AdvancedGraphicsIntegration_SetQualityLevel(Int quality);

/**
 * Get complete status report for all graphics systems
 */
const char* AdvancedGraphicsIntegration_GetDetailedStatus(void);

#ifdef __cplusplus
}
#endif

#endif  // ADVANCEDGRAPHICSINTEGRATION_H
