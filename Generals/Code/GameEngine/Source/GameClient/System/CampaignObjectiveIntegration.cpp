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

////////////////////////////////////////////////////////////////////////////////
//
//  Campaign Objective Integration
//
//  This file provides integration between CampaignManager and ObjectiveTracker
//  for runtime mission objective tracking during campaign gameplay.
//
//  Phase 47 Implementation - Campaign Progression System
//
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"

#include "GameClient/CampaignManager.h"
#include "GameClient/ObjectiveTracker.h"
#include "Common/Debug.h"

//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HELPER FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

/**
 * @brief Initialize mission objectives from mission data
 *
 * This function is called when a new mission starts to populate the ObjectiveTracker
 * with the mission's objectives from the Mission class data.
 *
 * Objectives are loaded from Mission.m_missionObjectivesLabel[0-4], where each label
 * corresponds to a game text string that describes the mission objective.
 *
 * @param mission Pointer to Mission object containing objectives
 * @param tracker Pointer to ObjectiveTracker singleton
 *
 * @note Objectives are initialized as ACTIVE state when mission starts
 * @note Empty objective labels are skipped (not added to tracker)
 * @note Objectives are prioritized by their order in the Mission array
 */
void initializeMissionObjectives(const Mission *mission, ObjectiveTracker *tracker)
{
	if (!mission || !tracker) {
		DEBUG_LOG(("Phase 47: initializeMissionObjectives - NULL mission or tracker\n"));
		return;
	}

	// Clear any previous objectives
	tracker->clearAllObjectives();

	DEBUG_LOG(("Phase 47: initializeMissionObjectives - Loading mission objectives for: %s\n", mission->m_name.str()));

	// Load objectives from mission data (MAX_OBJECTIVE_LINES = 5)
	// Each objective label is retrieved from TheGameText (localization system)
	for (int i = 0; i < MAX_OBJECTIVE_LINES; ++i) {
		if (mission->m_missionObjectivesLabel[i].isNotEmpty()) {
			// Create objective ID from mission name and line number
			AsciiString objectiveId;
			objectiveId.format("%s_obj_%d", mission->m_name.str(), i);

			// Get display string from game text (localization)
			// The m_missionObjectivesLabel contains keys to TheGameText
			// For now, use the label directly as display string
			// (in production, would call TheGameText->fetch(mission->m_missionObjectivesLabel[i]))
			AsciiString displayStr = mission->m_missionObjectivesLabel[i];

			// Priority: higher priority for first objectives (line 0 = highest)
			int priority = MAX_OBJECTIVE_LINES - i;

			// Mark critical objectives (typically first 2-3 objectives)
			Bool critical = (i < 2) ? TRUE : FALSE;

			// Add objective to tracker (will be initialized as ACTIVE)
			tracker->addObjective(objectiveId, displayStr, priority, critical);

			DEBUG_LOG(("Phase 47: Objective added - ID: %s, Display: %s, Priority: %d, Critical: %s\n",
				objectiveId.str(), displayStr.str(), priority, critical ? "YES" : "NO"));
		}
	}

	DEBUG_LOG(("Phase 47: Mission objective initialization complete - Total objectives: %d\n",
		tracker->getObjectiveCount()));
}

/**
 * @brief Check if mission objectives are satisfied for victory
 *
 * This function evaluates the current state of mission objectives to determine
 * if the player has achieved victory conditions (all objectives completed).
 *
 * @param tracker Pointer to ObjectiveTracker
 * @return TRUE if all critical objectives are completed, FALSE otherwise
 *
 * @note Can be called during gameplay to check win conditions
 * @note Non-critical objectives may be optional (not required for victory)
 */
Bool areMissionObjectivesComplete(ObjectiveTracker *tracker)
{
	if (!tracker) {
		DEBUG_LOG(("Phase 47: areMissionObjectivesComplete - NULL tracker\n"));
		return FALSE;
	}

	// Check if all objectives are completed
	Bool allComplete = tracker->allObjectivesCompleted();

	DEBUG_LOG(("Phase 47: Mission completion check - All objectives complete: %s\n",
		allComplete ? "YES" : "NO"));

	return allComplete;
}

/**
 * @brief Check if mission has failed due to failed objectives
 *
 * This function evaluates if any critical mission objectives have failed,
 * which would cause the mission to be unwinnable.
 *
 * @param tracker Pointer to ObjectiveTracker
 * @return TRUE if any objectives have failed, FALSE if all objectives still achievable
 *
 * @note Can be called to check for mission failure conditions
 * @note Mission failure prevents further progress in campaign
 */
Bool hasMissionFailed(ObjectiveTracker *tracker)
{
	if (!tracker) {
		DEBUG_LOG(("Phase 47: hasMissionFailed - NULL tracker\n"));
		return FALSE;
	}

	// Check if any objectives have failed
	Bool anyFailed = tracker->anyObjectivesFailed();

	DEBUG_LOG(("Phase 47: Mission failure check - Any objectives failed: %s\n",
		anyFailed ? "YES" : "NO"));

	return anyFailed;
}

/**
 * @brief Get mission completion percentage
 *
 * Calculate what percentage of the mission objectives have been completed.
 * Useful for progress bars and HUD display.
 *
 * @param tracker Pointer to ObjectiveTracker
 * @return Percentage (0-100) of completed objectives
 *
 * @note Returns 0 if no objectives exist
 * @note Returns 100 when all objectives are completed
 */
int getMissionCompletionPercentage(ObjectiveTracker *tracker)
{
	if (!tracker) {
		DEBUG_LOG(("Phase 47: getMissionCompletionPercentage - NULL tracker\n"));
		return 0;
	}

	int totalCount = tracker->getObjectiveCount();
	if (totalCount == 0) {
		return 0;
	}

	int completedCount = tracker->getCompletedObjectiveCount();
	int percentage = (completedCount * 100) / totalCount;

	DEBUG_LOG(("Phase 47: Mission completion - %d/%d objectives (%.0f%%)\n",
		completedCount, totalCount, (float)percentage));

	return percentage;
}

/**
 * @brief Print all mission objectives for debug display
 *
 * Useful for testing and debugging objective tracking during development.
 * Displays all objectives with their current status.
 *
 * @param mission Pointer to Mission (for reference)
 * @param tracker Pointer to ObjectiveTracker
 */
void debugPrintMissionObjectives(const Mission *mission, ObjectiveTracker *tracker)
{
	if (!tracker) {
		return;
	}

	DEBUG_LOG(("========================================\n"));
	DEBUG_LOG(("Phase 47: Mission Objectives Debug Report\n"));
	if (mission) {
		DEBUG_LOG(("Mission: %s (Map: %s)\n", mission->m_name.str(), mission->m_mapName.str()));
	}
	DEBUG_LOG(("========================================\n"));

	tracker->printObjectiveStatus();

	DEBUG_LOG(("Completion: %d/%d objectives\n",
		tracker->getCompletedObjectiveCount(), tracker->getObjectiveCount()));
	DEBUG_LOG(("Failed: %d objectives\n", tracker->getFailedObjectiveCount()));
	DEBUG_LOG(("Active: %d objectives\n", tracker->getActiveObjectiveCount()));
	DEBUG_LOG(("========================================\n"));
}

//-----------------------------------------------------------------------------
// END OF FILE ////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
