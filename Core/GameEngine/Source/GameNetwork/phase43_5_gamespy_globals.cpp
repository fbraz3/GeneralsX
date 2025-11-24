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
 * Phase 43.5: GameSpy Integration - Global Objects Implementation
 * 
 * Core GameSpy infrastructure objects and message queues.
 * Cross-platform: Win32 (GameSpy SDK), POSIX (SDL2 + OpenSpy backend)
 * 
 * Architecture:
 * - Win32: Direct GameSpy SDK integration via gamespy/peer/peer.h
 * - POSIX: OpenSpy API compatibility layer (future Phase for open-source backend)
 * - Vulkan: Not directly used (networking layer graphics-independent)
 * - SDL2: Not directly used (pure networking infrastructure)
 */

#include "PreRTS.h"

#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/GSConfig.h"
#include "GameNetwork/GameSpy/BuddyThread.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"

// ============================================================================
// Global GameSpy Objects - Color Palette
// ============================================================================

/**
 * GameSpyColor Array
 * Color definitions for GameSpy UI overlays and menus
 * 
 * Architecture:
 * - Platform-independent: Color values work on all platforms
 * - Used by overlay system for chat, player info, map selection UI
 * - Configurable via future GameSpy configuration system
 */
Color GameSpyColor[GSCOLOR_MAX] = {
	GameMakeColor(255, 0, 0, 255),        // GSCOLOR_PLAYER_NAME - Red
	GameMakeColor(0, 255, 0, 255),        // GSCOLOR_PLAYER_SELECTED - Green
	GameMakeColor(0, 0, 255, 255),        // GSCOLOR_PLAYER_UNSELECTED - Blue
	GameMakeColor(255, 255, 0, 255),      // GSCOLOR_MAP_SELECTED - Yellow
	GameMakeColor(128, 128, 128, 255),    // GSCOLOR_MAP_UNSELECTED - Gray
	GameMakeColor(255, 200, 0, 255),      // GSCOLOR_MOTD - Orange
	GameMakeColor(200, 200, 200, 255),    // GSCOLOR_MOTD_HEADING - Light Gray
};

// ============================================================================
// Global GameSpy Infrastructure Objects
// ============================================================================

/**
 * TheGameSpyConfig
 * Global configuration object containing GameSpy server settings
 * 
 * Initialized via SetUpGameSpy() with config buffer from MOTD/backend
 * Contains: mangler hosts, ping servers, QM settings, rank points, VIP list
 * 
 * Server Configuration:
 * - Win32 backend: EA GameSpy servers (hardcoded in GameSpy SDK)
 * - Future OpenSpy: Configurable via INI file (OpenSpy server endpoint)
 */
GameSpyConfigInterface *TheGameSpyConfig = NULL;

/**
 * TheGameSpyGame
 * Global object containing current game session info
 * 
 * Initialized via SetUpGameSpy()
 * Tracks: local player slot, remote players, staging room state
 */
GameSpyStagingRoom *TheGameSpyGame = NULL;

/**
 * TheGameSpyInfo
 * Global object containing player profile and lobby state
 * 
 * Initialized via GameSpyInfoInterface::createNewGameSpyInfoInterface()
 * Tracks: player profile ID, buddy list, message queues, staging room list
 */
GameSpyInfoInterface *TheGameSpyInfo = NULL;

// ============================================================================
// GameSpy Message Queues
// ============================================================================

/**
 * TheGameSpyBuddyMessageQueue
 * Queue for buddy messages (friend list notifications)
 * 
 * Architecture:
 * - Receives buddy login/logout notifications
 * - Cross-platform: Win32 (GameSpy SDK callbacks) -> POSIX (SDL2 event queue)
 * - Future: OpenSpy buddy system integration
 */
extern GameSpyBuddyMessageQueueInterface *TheGameSpyBuddyMessageQueue;

/**
 * TheGameSpyPeerMessageQueue
 * Queue for peer-to-peer messages during gameplay
 * 
 * Architecture:
 * - Receives P2P messages from connected peers
 * - Cross-platform: Win32 (Winsock2 callbacks) -> POSIX (BSD sockets)
 * - Future: Protocol upgrade from UDP to reliable transport
 */
extern GameSpyPeerMessageQueueInterface *TheGameSpyPeerMessageQueue;

/**
 * TheGameSpyPSMessageQueue
 * Queue for persistent storage messages (ladder stats, profile updates)
 * 
 * Architecture:
 * - Receives responses from persistent storage server
 * - Cross-platform: Win32 (HTTP via Winsock) -> POSIX (SDL2 networking)
 * - Future: Local SQLite cache for offline play
 */
extern GameSpyPSMessageQueueInterface *TheGameSpyPSMessageQueue;

// ============================================================================
// Configuration Server References
// ============================================================================

/**
 * GameSpy Server Configuration (Future OpenSpy Backend)
 * 
 * Currently: Hard-coded to EA GameSpy servers (legacy)
 * Future Phase: Move to configurable INI settings
 * 
 * Environment Variable: GAMESPY_CONFIG_SERVER
 * INI Setting: [GameSpyNetwork]/ConfigServer
 * Default: master.gamespy.com:29900 (legacy EA server)
 * 
 * Cross-Platform Equivalent (Future):
 * - Win32 backend: HTTP request via Winsock2 (WinHTTP API)
 * - POSIX backend: HTTP request via SDL2/curl (libcurl integration)
 * - Certificate validation: Cross-platform OpenSSL support
 * 
 * Note: This global configuration server reference will be replaced
 * in a future phase with configurable backend support for OpenSpy.
 */
// TODO Phase 43.6: Make GameSpy server configurable via INI file
// Planned: AsciiString g_GameSpyConfigServer = "master.gamespy.com";
// Planned: UnsignedShort g_GameSpyConfigPort = 29900;

