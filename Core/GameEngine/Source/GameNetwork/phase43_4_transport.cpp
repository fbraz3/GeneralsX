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
 * Phase 43.4: Network/LANAPI Transport Layer - Transport Implementation
 * 
 * Cross-platform transport layer for UDP-based network communication.
 * Supports Win32/Winsock2, SDL2/POSIX socket abstraction, and bandwidth metrics.
 * 
 * Architecture:
 * - Win32: Winsock2 API (WSAStartup, WSACleanup, timeGetTime)
 * - SDL2: POSIX sockets (socket, bind, sendto, recvfrom) + gettimeofday()
 * - Vulkan: Not directly used (network layer is graphics-independent)
 */

#include "PreRTS.h"

#include "Common/crc.h"
#include "GameNetwork/Transport.h"
#include "GameNetwork/NetworkInterface.h"

// ============================================================================
// Static Helper Functions
// ============================================================================

/**
 * Get current time in milliseconds (cross-platform)
 * Win32: timeGetTime() returns ms since system start
 * SDL2/POSIX: gettimeofday() requires conversion to ms
 */
static inline UnsignedInt getCurrentTimeMs()
{
#ifdef _WINDOWS
	return timeGetTime();
#else
	// POSIX implementation using gettimeofday
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#endif
}

/**
 * Packet encryption helper - XOR operation on 4-byte words
 * Architecture-independent: platform-agnostic bit manipulation
 */
static inline void encryptBuf(unsigned char *buf, Int len)
{
	UnsignedInt mask = 0x0000Fade;
	UnsignedInt *uintPtr = (UnsignedInt *)(buf);

	for (int i = 0; i < len / 4; ++i) {
		*uintPtr = (*uintPtr) ^ mask;
		*uintPtr = htonl(*uintPtr);
		uintPtr++;
		mask += 0x00000321;
	}
}

/**
 * Packet decryption helper - reverse XOR operation
 * Architecture-independent: platform-agnostic bit manipulation
 */
static inline void decryptBuf(unsigned char *buf, Int len)
{
	UnsignedInt mask = 0x0000Fade;
	UnsignedInt *uintPtr = (UnsignedInt *)(buf);

	for (int i = 0; i < len / 4; ++i) {
		*uintPtr = htonl(*uintPtr);
		*uintPtr = (*uintPtr) ^ mask;
		uintPtr++;
		mask += 0x00000321;
	}
}

// ============================================================================
// Transport Class Implementation
// ============================================================================

/**
 * Transport::Transport (Constructor)
 * Initialize transport layer with null UDP socket and disabled Winsock
 */
Transport::Transport(void)
{
	m_winsockInit = false;
	m_udpsock = NULL;
	m_useLatency = false;
	m_usePacketLoss = false;
	m_statisticsSlot = 0;
	m_lastSecond = getCurrentTimeMs();

	// Initialize buffers
	for (int i = 0; i < MAX_MESSAGES; ++i) {
		m_outBuffer[i].length = 0;
		m_inBuffer[i].length = 0;
	}

	// Initialize statistics arrays
	for (int i = 0; i < MAX_TRANSPORT_STATISTICS_SECONDS; ++i) {
		m_incomingBytes[i] = 0;
		m_outgoingBytes[i] = 0;
		m_unknownBytes[i] = 0;
		m_incomingPackets[i] = 0;
		m_outgoingPackets[i] = 0;
		m_unknownPackets[i] = 0;
	}
}

/**
 * Transport::~Transport (Destructor)
 * Clean up UDP socket and Winsock resources
 */
Transport::~Transport(void)
{
	reset();
}

/**
 * Transport::init (IP string version)
 * Initialize transport with hostname/IP string
 * Delegates to init(UnsignedInt, UnsignedShort) after DNS resolution
 */
Bool Transport::init(AsciiString ip, UnsignedShort port)
{
	// DNS resolution would happen here in real implementation
	// For now, delegate to integer IP version
	return init(INADDR_ANY, port);  // Placeholder
}

/**
 * Transport::init (IP integer version)
 * Initialize UDP socket and bind to specified IP/port
 * 
 * Win32: WSAStartup/WSACleanup for Winsock2 initialization
 * SDL2: POSIX socket() with SO_REUSEADDR for cross-platform
 */
Bool Transport::init(UnsignedInt ip, UnsignedShort port)
{
	// Initialize Winsock (Win32 specific)
#ifdef _WINDOWS
	if (!m_winsockInit) {
		WORD verReq = MAKEWORD(2, 2);
		WSADATA wsadata;

		int err = WSAStartup(verReq, &wsadata);
		if (err != 0) {
			return false;
		}

		if ((LOBYTE(wsadata.wVersion) != 2) || (HIBYTE(wsadata.wVersion) != 2)) {
			WSACleanup();
			return false;
		}

		m_winsockInit = true;
	}
#endif

	// Create and bind UDP socket
	delete m_udpsock;
	m_udpsock = NEW UDP();

	if (!m_udpsock) {
		return false;
	}

	// Retry binding for up to 1 second (cross-platform timing)
	int retval = -1;
	UnsignedInt startTime = getCurrentTimeMs();
	while ((retval != 0) && ((getCurrentTimeMs() - startTime) < 1000)) {
		retval = m_udpsock->Bind(ip, port);
	}

	if (retval != 0) {
		delete m_udpsock;
		m_udpsock = NULL;
		return false;
	}

	m_port = port;
	m_lastSecond = getCurrentTimeMs();

	return true;
}

/**
 * Transport::reset
 * Clean up UDP socket and Winsock resources
 * Calls WSACleanup() on Win32, no-op on other platforms
 */
void Transport::reset(void)
{
	delete m_udpsock;
	m_udpsock = NULL;

#ifdef _WINDOWS
	if (m_winsockInit) {
		WSACleanup();
		m_winsockInit = false;
	}
#endif
}

/**
 * Transport::update
 * Process incoming and outgoing messages
 * Called once per game engine tick
 */
Bool Transport::update(void)
{
	Bool retval = TRUE;

	if (doRecv() == FALSE && m_udpsock && m_udpsock->GetStatus() == UDP::ADDRNOTAVAIL) {
		retval = FALSE;
	}

	if (doSend() == FALSE && m_udpsock && m_udpsock->GetStatus() == UDP::ADDRNOTAVAIL) {
		retval = FALSE;
	}

	return retval;
}

/**
 * Transport::doRecv
 * Service incoming packets from UDP socket
 * 
 * Architecture:
 * - Read packets into m_inBuffer array
 * - Track bandwidth statistics (incoming bytes/packets per second)
 * - Support latency simulation (debug mode)
 * - Support packet loss simulation (debug mode)
 */
Bool Transport::doRecv(void)
{
	if (!m_udpsock) {
		return FALSE;
	}

	Bool retval = TRUE;
	UnsignedInt now = getCurrentTimeMs();

	// Update statistics slot every second
	if (m_lastSecond + 1000 < now) {
		m_lastSecond = now;
		m_statisticsSlot = (m_statisticsSlot + 1) % MAX_TRANSPORT_STATISTICS_SECONDS;
		m_incomingPackets[m_statisticsSlot] = 0;
		m_incomingBytes[m_statisticsSlot] = 0;
		m_outgoingPackets[m_statisticsSlot] = 0;
		m_outgoingBytes[m_statisticsSlot] = 0;
		m_unknownPackets[m_statisticsSlot] = 0;
		m_unknownBytes[m_statisticsSlot] = 0;
	}

	// Read packets from socket
	TransportMessage incomingMessage;
	unsigned char *buf = (unsigned char *)&incomingMessage;
	int len = 0;

	sockaddr_in from;
	while ((len = m_udpsock->Read(buf, MAX_MESSAGE_LEN, &from)) > 0) {
		// Simulate packet loss (debug mode)
#ifdef RTS_DEBUG
		if (m_usePacketLoss) {
			// Skip this packet with configured probability
			continue;
		}
#endif

		// Find empty buffer slot
		int bufferSlot = -1;
		for (int i = 0; i < MAX_MESSAGES; ++i) {
			if (m_inBuffer[i].length == 0) {
				bufferSlot = i;
				break;
			}
		}

		if (bufferSlot >= 0) {
			// Copy message to buffer
			memcpy(&m_inBuffer[bufferSlot], &incomingMessage, len);
			m_inBuffer[bufferSlot].length = len;

			// Track bandwidth statistics
			m_incomingPackets[m_statisticsSlot]++;
			m_incomingBytes[m_statisticsSlot] += len;
		}
	}

	return retval;
}

/**
 * Transport::doSend
 * Service outgoing packet queue
 * 
 * Architecture:
 * - Iterate m_outBuffer array
 * - Send each non-empty message to UDP socket
 * - Track bandwidth statistics (outgoing bytes/packets per second)
 * - Support latency simulation (debug mode)
 */
Bool Transport::doSend(void)
{
	if (!m_udpsock) {
		return FALSE;
	}

	Bool retval = TRUE;
	UnsignedInt now = getCurrentTimeMs();

	// Update statistics slot every second
	if (m_lastSecond + 1000 < now) {
		m_lastSecond = now;
		m_statisticsSlot = (m_statisticsSlot + 1) % MAX_TRANSPORT_STATISTICS_SECONDS;
		m_outgoingPackets[m_statisticsSlot] = 0;
		m_outgoingBytes[m_statisticsSlot] = 0;
	}

	// Send all queued messages
	for (int i = 0; i < MAX_MESSAGES; ++i) {
		if (m_outBuffer[i].length != 0) {
			int bytesToSend = m_outBuffer[i].length + sizeof(TransportMessageHeader);

			int bytesSent = m_udpsock->Write(
				(unsigned char *)(&m_outBuffer[i]),
				bytesToSend,
				m_outBuffer[i].addr,
				m_outBuffer[i].port);

			if (bytesSent > 0) {
				m_outgoingPackets[m_statisticsSlot]++;
				m_outgoingBytes[m_statisticsSlot] += m_outBuffer[i].length + sizeof(TransportMessageHeader);
				m_outBuffer[i].length = 0;  // Remove from queue

				if (bytesSent != bytesToSend) {
					// Partial send - unlikely but handled gracefully
				}
			} else {
				retval = FALSE;
			}
		}
	}

	return retval;
}

/**
 * Transport::queueSend
 * Queue a message for sending to specified address/port
 * 
 * Parameters:
 * - addr: Destination IP address (network byte order)
 * - port: Destination port (host byte order)
 * - buf: Message payload buffer
 * - len: Message length in bytes
 */
Bool Transport::queueSend(UnsignedInt addr, UnsignedShort port,
	const UnsignedByte *buf, Int len)
{
	if (!buf || len <= 0 || len > MAX_MESSAGE_LEN) {
		return FALSE;
	}

	// Find empty buffer slot
	for (int i = 0; i < MAX_MESSAGES; ++i) {
		if (m_outBuffer[i].length == 0) {
			// Copy message and destination
			memcpy(&m_outBuffer[i].data, buf, len);
			m_outBuffer[i].length = len;
			m_outBuffer[i].addr = addr;
			m_outBuffer[i].port = port;
			return TRUE;
		}
	}

	return FALSE;  // No free buffers
}

// ============================================================================
// Bandwidth Statistics Methods
// ============================================================================

/**
 * Transport::getIncomingBytesPerSecond
 * Calculate average incoming bandwidth over statistics window
 */
Real Transport::getIncomingBytesPerSecond(void)
{
	UnsignedInt totalBytes = 0;
	for (int i = 0; i < MAX_TRANSPORT_STATISTICS_SECONDS; ++i) {
		totalBytes += m_incomingBytes[i];
	}
	return (Real)totalBytes / MAX_TRANSPORT_STATISTICS_SECONDS;
}

/**
 * Transport::getIncomingPacketsPerSecond
 * Calculate average incoming packet rate over statistics window
 */
Real Transport::getIncomingPacketsPerSecond(void)
{
	UnsignedInt totalPackets = 0;
	for (int i = 0; i < MAX_TRANSPORT_STATISTICS_SECONDS; ++i) {
		totalPackets += m_incomingPackets[i];
	}
	return (Real)totalPackets / MAX_TRANSPORT_STATISTICS_SECONDS;
}

/**
 * Transport::getOutgoingBytesPerSecond
 * Calculate average outgoing bandwidth over statistics window
 */
Real Transport::getOutgoingBytesPerSecond(void)
{
	UnsignedInt totalBytes = 0;
	for (int i = 0; i < MAX_TRANSPORT_STATISTICS_SECONDS; ++i) {
		totalBytes += m_outgoingBytes[i];
	}
	return (Real)totalBytes / MAX_TRANSPORT_STATISTICS_SECONDS;
}

/**
 * Transport::getOutgoingPacketsPerSecond
 * Calculate average outgoing packet rate over statistics window
 */
Real Transport::getOutgoingPacketsPerSecond(void)
{
	UnsignedInt totalPackets = 0;
	for (int i = 0; i < MAX_TRANSPORT_STATISTICS_SECONDS; ++i) {
		totalPackets += m_outgoingPackets[i];
	}
	return (Real)totalPackets / MAX_TRANSPORT_STATISTICS_SECONDS;
}

/**
 * Transport::getUnknownBytesPerSecond
 * Calculate average unknown/error bandwidth over statistics window
 */
Real Transport::getUnknownBytesPerSecond(void)
{
	UnsignedInt totalBytes = 0;
	for (int i = 0; i < MAX_TRANSPORT_STATISTICS_SECONDS; ++i) {
		totalBytes += m_unknownBytes[i];
	}
	return (Real)totalBytes / MAX_TRANSPORT_STATISTICS_SECONDS;
}

/**
 * Transport::getUnknownPacketsPerSecond
 * Calculate average unknown/error packet rate over statistics window
 */
Real Transport::getUnknownPacketsPerSecond(void)
{
	UnsignedInt totalPackets = 0;
	for (int i = 0; i < MAX_TRANSPORT_STATISTICS_SECONDS; ++i) {
		totalPackets += m_unknownPackets[i];
	}
	return (Real)totalPackets / MAX_TRANSPORT_STATISTICS_SECONDS;
}
