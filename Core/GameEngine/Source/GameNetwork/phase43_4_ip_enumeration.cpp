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
 * Phase 43.4: Network/LANAPI - IP Enumeration Implementation
 * 
 * Network interface discovery and enumeration.
 * Cross-platform: Win32 (Winsock2), macOS (getifaddrs), Linux (socket enumeration)
 * 
 * Architecture:
 * - Win32: Winsock2 socket enumeration via GetAdaptersInfo()
 * - macOS/Linux: getifaddrs() POSIX API for interface enumeration
 * - Vulkan: Not directly used (network layer graphics-independent)
 * - SDL2: gethostname() for machine name query
 */

#include "PreRTS.h"
#include "GameNetwork/IPEnumeration.h"

#ifdef _WINDOWS
	#include <winsock2.h>
	#include <iphlpapi.h>
	#include <stdio.h>
	#pragma comment(lib, "ws2_32.lib")
	#pragma comment(lib, "iphlpapi.lib")
#else
	#include <sys/socket.h>
	#include <net/if.h>
	#include <ifaddrs.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <string.h>
	#include <unistd.h>
#endif

/**
 * IPEnumeration::IPEnumeration (Constructor)
 * Initialize IP enumeration object
 * 
 * Architecture:
 * - Performs minimal initialization
 * - Network setup handled by Transport class
 * - Enumeration performed on-demand via getAddresses()
 */
IPEnumeration::IPEnumeration()
{
	// Initialize with no addresses enumerated yet
	// Address enumeration happens on-demand via getAddresses()
}

/**
 * IPEnumeration::~IPEnumeration (Destructor)
 * Clean up IP enumeration resources
 * 
 * Architecture:
 * - Cleanup any enumerated address list if cached
 * - Network cleanup handled by Transport class
 */
IPEnumeration::~IPEnumeration()
{
	// Cleanup handled by owning Transport class
	// or LANAPI class if IPEnumeration is used independently
}

/**
 * IPEnumeration::getAddresses
 * Enumerate all local network interfaces and their addresses
 * 
 * Returns: Pointer to first EnumeratedIP node in linked list (or NULL if none)
 * 
 * Implementation Notes:
 * - WIN32: GetAdaptersInfo() API for Winsock2 interface enumeration
 * - POSIX: getifaddrs() API for cross-platform interface discovery
 * - Future: Phase 43.5 will implement actual interface enumeration
 * 
 * Current Status (Phase 43.4):
 * - Stub implementation returning NULL (satisfies symbol requirement)
 * - Full implementation deferred to Phase 43.5
 * - Headers and placeholder logic prepared for interface enumeration
 * - No blocking on network initialization
 */
EnumeratedIP *IPEnumeration::getAddresses(void)
{
	// Phase 43.5 implementation will add actual interface enumeration
	// For now, return NULL to satisfy linker symbol requirement
	// and prevent blocking on network discovery during game startup.
	//
	// Win32 approach: GetAdaptersInfo() from iphlpapi.h
	// POSIX approach: getifaddrs() with IFF_UP and IFF_LOOPBACK filtering
	//
	// See Transport class for active network operations.
	
	m_IPlist = NULL;  // Placeholder: no addresses returned yet
	return m_IPlist;
}

/**
 * IPEnumeration::getMachineName
 * Retrieve local machine hostname
 * 
 * Returns: Pointer to machine name string (or NULL if unavailable)
 * 
 * Architecture:
 * - Win32: GetComputerName() or gethostname()
 * - POSIX: gethostname() standard API
 */
AsciiString IPEnumeration::getMachineName(void)
{
	static char hostname[256];
	hostname[0] = '\0';

#ifdef _WINDOWS
	// Try GetComputerName first (native Windows API)
	DWORD size = sizeof(hostname);
	if (GetComputerName(hostname, &size)) {
		return hostname;
	}

	// Fallback to gethostname
	if (gethostname(hostname, sizeof(hostname)) == 0) {
		return hostname;
	}
#else
	// POSIX: Use gethostname
	if (gethostname(hostname, sizeof(hostname)) == 0) {
		return hostname;
	}
#endif

	// Failed to retrieve hostname
	return NULL;
}
