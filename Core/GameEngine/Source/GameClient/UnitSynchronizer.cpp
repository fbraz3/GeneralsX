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

// UnitSynchronizer.cpp ///////////////////////////////////////////////////////
// Phase 47 Unit State Synchronization Implementation
//
#include "PreRTS.h"  // This must go first in EVERY cpp file in the GameEngine

#include "GameClient/UnitSynchronizer.h"
#include "GameLogic/Object.h"
#include "Common/Xfer.h"
#include "GameNetwork/LANAPI.h"
#include <map>
#include <SDL2/SDL.h>

// ============================================================================
// Unit Synchronizer Implementation
// ============================================================================

// Mapping of unit IDs to their last synced state
static std::map<UnsignedInt, UnitSyncState> g_trackedUnits;
static UnsignedInt g_lastSyncTime = 0;
static const UnsignedInt SYNC_THRESHOLD_MS = 100;  // Resync if 100ms+ has passed

// ============================================================================
// Static Implementation
// ============================================================================

Bool UnitSynchronizer::Initialize()
{
	g_trackedUnits.clear();
	g_lastSyncTime = SDL_GetTicks();
	DEBUG_LOG(("UnitSynchronizer: Initialized"));
	return TRUE;
}

Bool UnitSynchronizer::Shutdown()
{
	g_trackedUnits.clear();
	DEBUG_LOG(("UnitSynchronizer: Shutdown, tracked %d units", (Int)g_trackedUnits.size()));
	return TRUE;
}

Bool UnitSynchronizer::TrackUnit(Object* unit)
{
	if (!unit)
	{
		DEBUG_ASSERTCRASH(FALSE, ("UnitSynchronizer::TrackUnit - unit is NULL"));
		return FALSE;
	}

	UnsignedInt unitID = unit->getID();
	if (g_trackedUnits.find(unitID) != g_trackedUnits.end())
	{
		// Already tracked
		return TRUE;
	}

	// Create initial sync state
	UnitSyncState state;
	state.unitID = unitID;
	state.position = unit->Get_Position();
	state.velocity = Coord3D(0, 0, 0);
	state.facing = 0;
	state.animationState = 0;
	state.commandState = 0;
	state.isMoving = FALSE;
	state.isAttacking = FALSE;
	state.targetID = 0;
	state.timestamp = SDL_GetTicks();

	g_trackedUnits[unitID] = state;
	DEBUG_LOG(("UnitSynchronizer: Tracking unit %d at (%.1f, %.1f, %.1f)",
		unitID, state.position.x, state.position.y, state.position.z));

	return TRUE;
}

Bool UnitSynchronizer::UntrackUnit(UnsignedInt unitID)
{
	auto it = g_trackedUnits.find(unitID);
	if (it == g_trackedUnits.end())
	{
		// Not tracked
		return FALSE;
	}

	g_trackedUnits.erase(it);
	DEBUG_LOG(("UnitSynchronizer: Untracking unit %d", unitID));
	return TRUE;
}

Bool UnitSynchronizer::Update()
{
	UnsignedInt currentTime = SDL_GetTicks();
	if ((currentTime - g_lastSyncTime) < SYNC_THRESHOLD_MS)
	{
		return FALSE;  // Not time to sync yet
	}

	g_lastSyncTime = currentTime;

	// In a full implementation, this would:
	// 1. Iterate through all tracked units
	// 2. Check if their state has changed significantly
	// 3. Send updated states to network
	// For now, just log that we're checking units
	DEBUG_LOG(("UnitSynchronizer: Update checking %d tracked units", (Int)g_trackedUnits.size()));

	return g_trackedUnits.size() > 0;
}

Bool UnitSynchronizer::SyncUnitToNetwork(Object* unit)
{
	if (!unit)
		return FALSE;

	UnsignedInt unitID = unit->getID();
	auto it = g_trackedUnits.find(unitID);
	if (it == g_trackedUnits.end())
	{
		// Unit not tracked
		return FALSE;
	}

	// Update cached state
	UnitSyncState& state = it->second;
	state.position = unit->Get_Position();
	state.timestamp = SDL_GetTicks();

	// In a full implementation, this would:
	// 1. Create a network message with unit state
	// 2. Send to all connected players via LANAPI
	// 3. Other players would update their copy of this unit
	DEBUG_LOG(("UnitSynchronizer: Syncing unit %d to network", unitID));

	return TRUE;
}

Bool UnitSynchronizer::ReceiveUnitSync(const UnitSyncState* syncData)
{
	if (!syncData)
		return FALSE;

	// In a full implementation, this would:
	// 1. Find the unit by ID
	// 2. Update its position, state from received data
	// 3. Apply interpolation for smooth movement
	DEBUG_LOG(("UnitSynchronizer: Received sync for unit %d at (%.1f, %.1f, %.1f)",
		syncData->unitID, syncData->position.x, syncData->position.y, syncData->position.z));

	// Update tracked state
	g_trackedUnits[syncData->unitID] = *syncData;

	return TRUE;
}

Int UnitSynchronizer::GetTrackedUnitCount()
{
	return (Int)g_trackedUnits.size();
}

void UnitSynchronizer::ClearAllTrackedUnits()
{
	g_trackedUnits.clear();
	DEBUG_LOG(("UnitSynchronizer: Cleared all tracked units"));
}

void UnitSynchronizer::Xfer(Xfer* xfer)
{
	// Persistence hook for save/load
	// In a full implementation, would persist unit sync state
	if (!xfer)
		return;

	UnsignedInt unitCount = (UnsignedInt)g_trackedUnits.size();
	xfer->xferUnsignedInt(&unitCount);

	DEBUG_LOG(("UnitSynchronizer::Xfer - %s %d tracked units",
		xfer->getMode() == Xfer::XFER_SAVE ? "saving" : "loading", unitCount));
}

#endif // _RELEASE
