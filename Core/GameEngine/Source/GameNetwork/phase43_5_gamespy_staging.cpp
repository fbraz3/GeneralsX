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
 * Phase 43.5: GameSpy Integration - Staging Room Helper Implementation
 * 
 * Staging room game session management functions for GameSpyStagingRoom.
 * Wrapper functions around existing GameSpyStagingRoom class for Phase 43.5.
 * 
 * Cross-platform: Win32 (GameSpy SDK), POSIX (SDL2 + OpenSpy backend)
 * 
 * Architecture:
 * - Win32: Native GameSpy staging room via peer API
 * - POSIX: OpenSpy API compatibility layer (future Phase)
 * - Vulkan: Not directly used (networking layer graphics-independent)
 * - SDL2: Not directly used (pure networking infrastructure)
 */

#include "PreRTS.h"

#include "GameNetwork/GameSpy/StagingRoomGameInfo.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/Transport.h"

// ============================================================================
// GameSpyStagingRoom Helper Functions - Phase 43.5 Wrappers
// ============================================================================

/**
 * GetGameSpySlotInfo
 * Retrieve player information from a staging room slot
 * 
 * Parameters:
 * - stagingRoom: Pointer to GameSpyStagingRoom
 * - slotIndex: Slot number (0 = host, 1-7 = players/observers)
 * 
 * Returns: GameSpyGameSlot pointer or NULL if invalid
 * 
 * Architecture:
 * - Wrapper around GameSpyStagingRoom::getGameSpySlot(int)
 * - Cross-platform: Slot data platform-independent
 * - Used to query player names, IDs, statistics
 */
GameSpyGameSlot* GetGameSpySlotInfo(GameSpyStagingRoom *stagingRoom, Int slotIndex)
{
	if (!stagingRoom || slotIndex < 0 || slotIndex >= MAX_SLOTS) {
		return NULL;
	}

	return stagingRoom->getGameSpySlot(slotIndex);
}

/**
 * PostGameResultsToGameSpy
 * Format and post game results packet for ladder ranking
 * 
 * Parameters:
 * - stagingRoom: Pointer to GameSpyStagingRoom with completed game
 * 
 * Returns: Formatted results packet for posting to GameSpy ladder
 * 
 * Architecture:
 * - Wrapper around GameSpyStagingRoom::generateGameSpyGameResultsPacket()
 * - Creates packet with: player names, faction, win/loss stats, rank points
 * - Sent to GameSpyPeerMessageQueue for transmission to ranking server
 * - Win32: GameSpy SDK handles formatting
 * - POSIX: OpenSpy API compatibility
 */
AsciiString PostGameResultsToGameSpy(GameSpyStagingRoom *stagingRoom)
{
	if (!stagingRoom) {
		return AsciiString("");
	}

	return stagingRoom->generateGameSpyGameResultsPacket();
}

/**
 * PostLadderResultsToGameSpy
 * Format ladder ranking delta packet with player ranking changes
 * 
 * Parameters:
 * - stagingRoom: Pointer to GameSpyStagingRoom with completed game
 * 
 * Returns: Formatted ladder ranking packet
 * 
 * Architecture:
 * - Wrapper around GameSpyStagingRoom::generateLadderGameResultsPacket()
 * - Creates packet with: rank point deltas, win/loss changes
 * - Sent to persistent storage server for ladder updates
 * - Win32: GameSpy persistent storage API
 * - POSIX: SQLite cache + OpenSpy backend
 */
AsciiString PostLadderResultsToGameSpy(GameSpyStagingRoom *stagingRoom)
{
	if (!stagingRoom) {
		return AsciiString("");
	}

	return stagingRoom->generateLadderGameResultsPacket();
}

/**
 * InitStagingRoom
 * Initialize staging room with defaults and slots
 * 
 * Parameters:
 * - stagingRoom: Pointer to GameSpyStagingRoom to initialize
 * 
 * Architecture:
 * - Wrapper around GameSpyStagingRoom::init()
 * - Sets up: game name, hosting flags, slot list, player slots
 * - Called after joining/creating a staging room
 */
void InitStagingRoom(GameSpyStagingRoom *stagingRoom)
{
	if (!stagingRoom) {
		return;
	}

	stagingRoom->init();
}

/**
 * ResetStagingRoomAccepted
 * Reset "player ready" flags for all slots (for countdown abort)
 * 
 * Parameters:
 * - stagingRoom: Pointer to GameSpyStagingRoom
 * 
 * Architecture:
 * - Wrapper around GameSpyStagingRoom::resetAccepted()
 * - Called when host cancels countdown or player abandons slot
 * - Resets all accepted flags so countdown restarts
 */
void ResetStagingRoomAccepted(GameSpyStagingRoom *stagingRoom)
{
	if (!stagingRoom) {
		return;
	}

	stagingRoom->resetAccepted();
}

/**
 * GetLocalPlayerSlot
 * Find local player's slot index in the staging room
 * 
 * Parameters:
 * - stagingRoom: Pointer to GameSpyStagingRoom
 * 
 * Returns: Slot index (0-7) or -1 if not in room
 * 
 * Architecture:
 * - Wrapper around GameSpyStagingRoom::getLocalSlotNum()
 * - Returns index for current player in joined/hosted room
 */
Int GetLocalPlayerSlot(GameSpyStagingRoom *stagingRoom)
{
	if (!stagingRoom) {
		return -1;
	}

	return stagingRoom->getLocalSlotNum();
}

/**
 * MarkGameStarted
 * Record that game has started with given game ID
 * 
 * Parameters:
 * - stagingRoom: Pointer to GameSpyStagingRoom
 * - gameID: Unique game session ID from game launch
 * 
 * Architecture:
 * - Wrapper around GameSpyStagingRoom::startGame(int)
 * - Records game ID for ladder results posting
 * - Sent to persistent storage server
 */
void MarkGameStarted(GameSpyStagingRoom *stagingRoom, Int gameID)
{
	if (!stagingRoom) {
		return;
	}

	stagingRoom->startGame(gameID);
}

/**
 * LaunchGameFromStagingRoom
 * Start NAT negotiation and launch game from staging room
 * 
 * Parameters:
 * - stagingRoom: Pointer to GameSpyStagingRoom
 * 
 * Architecture:
 * - Wrapper around GameSpyStagingRoom::launchGame()
 * - Called after all players accepted and countdown completed
 * - Establishes P2P connections between players via Transport
 * - Win32: Winsock2 P2P negotiation
 * - POSIX: BSD sockets P2P negotiation
 */
void LaunchGameFromStagingRoom(GameSpyStagingRoom *stagingRoom)
{
	if (!stagingRoom) {
		return;
	}

	stagingRoom->launchGame();
}

/**
 * GetStagingRoomTransport
 * Get the Transport object for P2P connection negotiation
 * 
 * Returns: Transport pointer for active staging room or NULL
 * 
 * Architecture:
 * - Retrieve P2P transport from TheGameSpyGame
 * - Used for NAT traversal, ping measurement, P2P handshake
 * - Win32: Winsock2 transport
 * - POSIX: BSD sockets transport (Phase 43.4 implementation)
 */
Transport* GetStagingRoomTransport(void)
{
	if (!TheGameSpyGame) {
		return NULL;
	}

	// Note: Transport pointer stored internally in GameSpyStagingRoom
	// This is a placeholder - actual access requires StagingRoomGameInfo.h member
	// Access pattern: stagingRoom->m_transport or getter method if provided
	return NULL;  // TODO: Implement transport getter if needed
}
