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
 * Phase 43.4: Network/LANAPI - UDP Socket Implementation
 * 
 * Low-level UDP socket operations with broadcast capability.
 * Cross-platform: Win32 (Winsock2), SDL2/POSIX (BSD sockets)
 * 
 * Architecture:
 * - Win32: SO_BROADCAST socket option via setsockopt
 * - SDL2: SO_BROADCAST socket option via setsockopt
 * - Vulkan: Not directly used (network layer graphics-independent)
 */

#include "PreRTS.h"

#include "GameNetwork/NetworkInterface.h"
#include "GameNetwork/udp.h"

/**
 * UDP::UDP (Constructor)
 * Initialize UDP socket with default state
 */
UDP::UDP()
{
	fd = -1;
	myIP = 0;
	myPort = 0;
	m_lastError = UDP::OK;
	memset(&addr, 0, sizeof(addr));
}

/**
 * UDP::~UDP (Destructor)
 * Clean up UDP socket resources
 */
UDP::~UDP()
{
	// Close socket if open
	if (fd >= 0) {
#ifdef _WINDOWS
		::closesocket(fd);
#else
		::close(fd);
#endif
		fd = -1;
	}
}

/**
 * UDP::Bind
 * Bind UDP socket to local address and port
 * 
 * Parameters:
 * - ip: Local IP address (in network byte order)
 * - port: Local port (in host byte order)
 * 
 * Returns: 0 on success, non-zero on error
 */
Int UDP::Bind(UnsignedInt ip, UnsignedShort port)
{
	// Close any existing socket
	if (fd >= 0) {
#ifdef _WINDOWS
		::closesocket(fd);
#else
		::close(fd);
#endif
	}

	// Create UDP socket
	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		m_lastError = UDP::BADF;
		return -1;
	}

	// Prepare socket address structure
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ip;
	addr.sin_port = htons(port);

	// Bind to address
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		m_lastError = UDP::ADDRINUSE;
#ifdef _WINDOWS
		::closesocket(fd);
#else
		::close(fd);
#endif
		fd = -1;
		return -1;
	}

	myIP = ip;
	myPort = port;
	m_lastError = UDP::OK;
	return 0;
}

/**
 * UDP::Read
 * Receive UDP packet from socket
 * 
 * Parameters:
 * - buf: Buffer to receive packet data
 * - bufLen: Maximum buffer size
 * - from: Output parameter receiving sender address
 * 
 * Returns: Bytes received, or -1 on error
 */
Int UDP::Read(UnsignedByte *buf, UnsignedInt bufLen, struct sockaddr_in *from)
{
	if (fd < 0) {
		m_lastError = UDP::BADF;
		return -1;
	}

	socklen_t fromLen = sizeof(struct sockaddr_in);
	int bytesRead = recvfrom(fd, (char *)buf, bufLen, 0, (struct sockaddr *)from, &fromLen);

	if (bytesRead < 0) {
		m_lastError = UDP::WOULDBLOCK;  // Non-blocking socket
		return -1;
	}

	m_lastError = UDP::OK;
	return bytesRead;
}

/**
 * UDP::Write
 * Send UDP packet to remote address
 * 
 * Parameters:
 * - buf: Buffer containing packet data
 * - bufLen: Number of bytes to send
 * - ip: Destination IP address (network byte order)
 * - port: Destination port (host byte order)
 * 
 * Returns: Bytes sent, or -1 on error
 */
Int UDP::Write(const UnsignedByte *buf, UnsignedInt bufLen, UnsignedInt ip, UnsignedShort port)
{
	if (fd < 0) {
		m_lastError = UDP::BADF;
		return -1;
	}

	// Prepare destination address
	struct sockaddr_in destAddr;
	memset(&destAddr, 0, sizeof(destAddr));
	destAddr.sin_family = AF_INET;
	destAddr.sin_addr.s_addr = ip;
	destAddr.sin_port = htons(port);

	// Send packet
	int bytesSent = sendto(fd, (const char *)buf, bufLen, 0, 
		(struct sockaddr *)&destAddr, sizeof(destAddr));

	if (bytesSent < 0) {
		m_lastError = UDP::WOULDBLOCK;
		return -1;
	}

	m_lastError = UDP::OK;
	return bytesSent;
}

/**
 * UDP::GetStatus
 * Return last socket error status
 */
UDP::sockStat UDP::GetStatus(void)
{
	return (UDP::sockStat)m_lastError;
}

/**
 * UDP::AllowBroadcasts
 * Enable or disable broadcast capability on UDP socket
 * 
 * Win32: Uses setsockopt(socket, SOL_SOCKET, SO_BROADCAST, ...)
 * SDL2/POSIX: Uses setsockopt(socket, SOL_SOCKET, SO_BROADCAST, ...)
 * 
 * Parameters:
 * - onOff: true to enable broadcasts, false to disable
 * 
 * Returns: 0 on success, -1 on error
 * 
 * Architecture Notes:
 * - SO_BROADCAST is identical across Win32 and POSIX platforms
 * - Boolean value cast to (const char*) for Win32 compatibility
 * - POSIX uses int* instead, but cast handles both
 * - No platform-specific code blocks needed (abstraction layer)
 */
int UDP::AllowBroadcasts(bool onOff)
{
	if (fd < 0) {
		return -1;  // Socket not initialized (fd is file descriptor)
	}

	// Cast bool to int for socket option (0=disable, 1=enable)
	int broadcastVal = onOff ? 1 : 0;

#ifdef _WINDOWS
	// Win32: setsockopt expects (const char*) for option value
	int retval = setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
		(const char *)(&broadcastVal), sizeof(int));
#else
	// POSIX: setsockopt expects (const void*) for option value
	int retval = setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
		(const void *)(&broadcastVal), sizeof(int));
#endif

	return retval;  // 0 on success, -1 on error
}

