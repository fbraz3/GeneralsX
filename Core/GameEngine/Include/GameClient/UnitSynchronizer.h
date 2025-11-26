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

// UnitSynchronizer.h //////////////////////////////////////////////////////
// Phase 47 Unit State Synchronization for Multiplayer
// Manages synchronization of unit positions, states, and actions across network
//
#pragma once

#include "Common/GameType.h"
#include "GameLogic/Object.h"

class Xfer;

// ============================================================================
// Unit State Synchronization Cache
// ============================================================================

/**
 * Unit state snapshot for network synchronization
 * Contains all relevant unit information for multiplayer sync
 */
struct UnitSyncState
{
	UnsignedInt unitID;              ///< Unique unit identifier
	Coord3D position;                ///< Current world position
	Coord3D velocity;                ///< Current movement velocity
	Real facing;                     ///< Unit facing angle
	UnsignedInt animationState;      ///< Current animation state
	UnsignedInt commandState;        ///< Command execution state
	Bool isMoving;                   ///< Currently moving?
	Bool isAttacking;                ///< Currently attacking?
	UnsignedInt targetID;            ///< Target unit ID (0 if none)
	UnsignedInt timestamp;           ///< Sync timestamp (server time)
};

// ============================================================================
// Unit Synchronizer Class
// ============================================================================

class UnitSynchronizer
{
public:

	/**
	 * Initialize unit synchronization system
	 *
	 * @return true if successful
	 */
	static Bool Initialize();

	/**
	 * Shutdown unit synchronization system
	 *
	 * @return true if successful
	 */
	static Bool Shutdown();

	/**
	 * Add unit to synchronization tracking
	 * Called when a new unit is created in multiplayer game
	 *
	 * @param unit The Object to track
	 * @return true if added successfully
	 */
	static Bool TrackUnit(Object* unit);

	/**
	 * Remove unit from synchronization tracking
	 * Called when a unit is destroyed
	 *
	 * @param unitID The unit identifier to stop tracking
	 * @return true if removed successfully
	 */
	static Bool UntrackUnit(UnsignedInt unitID);

	/**
	 * Update unit synchronization state
	 * Called once per frame to check if units need sync
	 *
	 * @return true if any units were synced
	 */
	static Bool Update();

	/**
	 * Send unit state to network
	 * Broadcasts this unit's current state to other players
	 *
	 * @param unit The unit to synchronize
	 * @return true if sync was sent
	 */
	static Bool SyncUnitToNetwork(Object* unit);

	/**
	 * Receive unit state from network
	 * Updates local unit based on network data
	 *
	 * @param syncData The received unit state
	 * @return true if sync was applied successfully
	 */
	static Bool ReceiveUnitSync(const UnitSyncState* syncData);

	/**
	 * Get number of tracked units
	 *
	 * @return Number of units being synchronized
	 */
	static Int GetTrackedUnitCount();

	/**
	 * Clear all tracked units
	 * Called when leaving a game
	 */
	static void ClearAllTrackedUnits();

	/**
	 * Persistence interface for unit synchronizer state
	 *
	 * @param xfer The transfer object for serialization
	 */
	static void Xfer(Xfer* xfer);

private:

	// Prevent instantiation - all methods are static
	UnitSynchronizer();
	~UnitSynchronizer();
};

#endif // UNITSYNCHRONIZER_H
