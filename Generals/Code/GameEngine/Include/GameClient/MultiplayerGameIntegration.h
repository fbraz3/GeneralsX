/*
**  Command & Conquer Generals(tm)
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

// MultiplayerGameIntegration.h ///////////////////////////////////////////////
// Phase 47 Multiplayer Integration System
// Provides helper functions for LAN multiplayer game management
//
#pragma once

#include "GameNetwork/LANAPI.h"
#include "Common/GameState.h"
#include "GameLogic/GameLogic.h"

/**
 * Initialize multiplayer subsystem for game session
 * Sets up LANAPI singleton and connects to server/client network
 *
 * @return true if initialization successful, false on error
 */
Bool MultiplayerGameIntegration_Initialize();

/**
 * Check if currently in multiplayer game
 *
 * @return true if multiplayer game is active
 */
Bool MultiplayerGameIntegration_IsMultiplayer();

/**
 * Get current multiplayer session information
 *
 * @return LANGameInfo pointer or NULL if not in multiplayer
 */
LANGameInfo* MultiplayerGameIntegration_GetGameInfo();

/**
 * Synchronize unit state across network to all players
 * Called during game update loop to broadcast unit movements/actions
 *
 * @param unitID The unit to synchronize
 * @param position Current position
 * @param state Unit animation state (IDLE, MOVING, ATTACKING, etc)
 * @return true if sync successful
 */
Bool MultiplayerGameIntegration_SyncUnitState(UnsignedInt unitID, const Coord3D* position, UnsignedInt state);

/**
 * Send command message to other players
 * Routes player commands (move, attack, build) across network
 *
 * @param commandType The command type (BUILD, ATTACK_MOVE, STOP, etc)
 * @param commandData Opaque command data (interpreted based on type)
 * @return true if message sent successfully
 */
Bool MultiplayerGameIntegration_SendCommand(UnsignedInt commandType, const void* commandData);

/**
 * Handle player disconnection from game
 * Cleans up disconnected player resources and notifies remaining players
 *
 * @param playerIP The IP address of disconnected player
 * @return true if handled successfully
 */
Bool MultiplayerGameIntegration_HandlePlayerDisconnect(UnsignedInt playerIP);

/**
 * Record match result to persistent storage
 * Saves winner, final stats, and game duration for replay/leaderboard
 *
 * @param winnerId ID of winning player (0-7)
 * @param gameDuration Duration of game in seconds
 * @param playerStats Array of player statistics
 * @return true if recorded successfully
 */
Bool MultiplayerGameIntegration_RecordMatchResult(UnsignedInt winnerId, UnsignedInt gameDuration, void* playerStats);

/**
 * Shutdown multiplayer subsystem
 * Disconnects from network and cleans up LANAPI resources
 *
 * @return true if shutdown successful
 */
Bool MultiplayerGameIntegration_Shutdown();

#endif // MULTIPLAYERGAMEINTEGRATION_H
