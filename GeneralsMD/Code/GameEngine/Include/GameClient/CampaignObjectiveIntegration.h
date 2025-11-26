/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

////////////////////////////////////////////////////////////////////////////////
//
//  Campaign Objective Integration
//
//  Provides functions for integrating ObjectiveTracker with CampaignManager
//  for mission objective tracking during campaign gameplay.
//
//  Phase 47 Implementation - Campaign Progression System
//
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class Mission;
class ObjectiveTracker;

//-----------------------------------------------------------------------------
// FUNCTION DECLARATIONS //////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

/**
 * @brief Initialize mission objectives from mission data
 *
 * Populate the ObjectiveTracker with objectives from the current Mission.
 * Called when a new mission starts in the campaign.
 *
 * @param mission Pointer to Mission containing objective labels
 * @param tracker Pointer to ObjectiveTracker singleton
 */
void initializeMissionObjectives(const Mission *mission, ObjectiveTracker *tracker);

/**
 * @brief Check if mission objectives are satisfied for victory
 *
 * Determine if all mission objectives have been completed.
 * Used to check victory conditions during mission gameplay.
 *
 * @param tracker Pointer to ObjectiveTracker
 * @return TRUE if all objectives completed, FALSE otherwise
 */
Bool areMissionObjectivesComplete(ObjectiveTracker *tracker);

/**
 * @brief Check if mission has failed due to failed objectives
 *
 * Determine if mission is now unwinnable due to failed objectives.
 * Used to check failure conditions during mission gameplay.
 *
 * @param tracker Pointer to ObjectiveTracker
 * @return TRUE if any objectives failed, FALSE otherwise
 */
Bool hasMissionFailed(ObjectiveTracker *tracker);

/**
 * @brief Get mission completion percentage
 *
 * Calculate progress as percentage of objectives completed.
 * Useful for HUD progress bars and mission reports.
 *
 * @param tracker Pointer to ObjectiveTracker
 * @return Percentage (0-100) of completed objectives
 */
int getMissionCompletionPercentage(ObjectiveTracker *tracker);

/**
 * @brief Print all mission objectives for debug display
 *
 * Output all mission objectives and their status to debug logs.
 * Used for testing and development.
 *
 * @param mission Pointer to Mission (for reference, can be NULL)
 * @param tracker Pointer to ObjectiveTracker
 */
void debugPrintMissionObjectives(const Mission *mission, ObjectiveTracker *tracker);

//-----------------------------------------------------------------------------
// END OF FILE ////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
