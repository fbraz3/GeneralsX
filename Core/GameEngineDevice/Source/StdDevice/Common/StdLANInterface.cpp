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
//                                                                            //
//  (c) 2001-2003 Electronic Arts Inc.                                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// StdLANInterface.cpp /////////////////////////////////////////////////////////
// Standard POSIX implementation of local network interface operations
////////////////////////////////////////////////////////////////////////////////

#include "StdDevice/Common/StdLANInterface.h"
#include "Common/Debug.h"

#ifndef _WIN32
#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string.h>

// GeneralsX @bugfix BenderAI 16/09/2026 Detect Docker user-defined bridges, which are named
// "br-" followed by a 12 character hexadecimal network id (e.g. br-ffd9ee17a8f4). These carry
// private RFC1918 addresses that must never be offered as the local LAN/online address.
static bool IsDockerUserBridgeName(const char *name)
{
	if (name == nullptr || strncmp(name, "br-", 3) != 0)
	{
		return false;
	}

	const char *suffix = name + 3;
	int digits = 0;
	for (; suffix[digits] != '\0'; ++digits)
	{
		const char c = suffix[digits];
		const bool isHex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
		if (!isHex)
		{
			return false;
		}
	}

	return digits == 12;
}
#endif

// GeneralsX @feature Mr. Meesseeks 17/09/2026 Enumerate local active IPv4 interface addresses on POSIX.
Int StdLANInterface::getLocalHostAddresses(UnsignedInt *outAddrs, Int maxAddrs)
{
	if (outAddrs == nullptr || maxAddrs <= 0)
	{
		return 0;
	}

#ifndef _WIN32
	struct ifaddrs *ifaddr = nullptr;
	if (getifaddrs(&ifaddr) != 0)
	{
		DEBUG_LOG(("Failed call to getifaddrs; errno returned %d", errno));
		return 0;
	}

	Int count = 0;
	for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == nullptr)
		{
			continue;
		}

		if (ifa->ifa_addr->sa_family != AF_INET)
		{
			continue;
		}

		if ((ifa->ifa_flags & IFF_UP) == 0 || (ifa->ifa_flags & IFF_LOOPBACK) != 0)
		{
			continue;
		}

		// GeneralsX @bugfix BenderAI 16/09/2026 Require an active carrier (IFF_RUNNING). Idle Docker/VM
		// bridges stay administratively UP without a link, and were being offered as the local address,
		// which broke LAN discovery and online hosting behind an unreachable 172.x.y.z bridge IP.
		if ((ifa->ifa_flags & IFF_RUNNING) == 0)
		{
			continue;
		}

		// GeneralsX @feature Mr. Meesseeks 11/07/2026 Ignore point-to-point and non-broadcast interfaces.
		if ((ifa->ifa_flags & IFF_BROADCAST) == 0 || (ifa->ifa_flags & IFF_POINTOPOINT) != 0)
		{
			continue;
		}

		// GeneralsX @feature Mr. Meesseeks 11/07/2026 Ignore known virtual interfaces (Docker, VPN, Apple AWDL)
		if (ifa->ifa_name != nullptr)
		{
			const char *name = ifa->ifa_name;
			if (strncmp(name, "docker", 6) == 0 ||
				strncmp(name, "veth", 4) == 0 ||
				strncmp(name, "virbr", 5) == 0 ||
				strncmp(name, "awdl", 4) == 0 ||
				strncmp(name, "llw", 3) == 0 ||
				strncmp(name, "utun", 4) == 0 ||
				IsDockerUserBridgeName(name))
			{
				continue;
			}
		}

		const sockaddr_in *addr = reinterpret_cast<const sockaddr_in *>(ifa->ifa_addr);
		// GeneralsX @bugfix BenderAI 31/03/2026 Use ntohl to convert from network byte order before extracting octets;
		// reading s_addr byte-by-byte on little-endian platforms reverses the IPv4 octets.
		const UnsignedInt hostAddr = ntohl(addr->sin_addr.s_addr);

		Bool duplicate = FALSE;
		for (Int i = 0; i < count; ++i)
		{
			if (outAddrs[i] == hostAddr)
			{
				duplicate = TRUE;
				break;
			}
		}

		if (!duplicate && count < maxAddrs)
		{
			outAddrs[count++] = hostAddr;
		}
	}

	freeifaddrs(ifaddr);
	return count;
#else
	return 0;
#endif
}

// GeneralsX @feature Mr. Meesseeks 17/09/2026 Discover per-interface IPv4 subnet broadcast addresses for LAN discovery on POSIX.
Int StdLANInterface::getSubnetBroadcastAddresses(UnsignedInt localIP, UnsignedInt *outAddrs, Int maxAddrs)
{
	if (outAddrs == nullptr || maxAddrs <= 0)
	{
		return 0;
	}

#ifndef _WIN32
	Int count = 0;
	struct ifaddrs *ifaddr = nullptr;
	if (getifaddrs(&ifaddr) != 0)
	{
		return 0;
	}

	for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == nullptr || ifa->ifa_addr->sa_family != AF_INET)
		{
			continue;
		}
		if ((ifa->ifa_flags & IFF_UP) == 0 || (ifa->ifa_flags & IFF_LOOPBACK) != 0)
		{
			continue;
		}

		// GeneralsX @bugfix BenderAI 16/09/2026 Skip interfaces without an active carrier so LAN discovery
		// broadcasts are not sent to idle Docker/VM bridges.
		if ((ifa->ifa_flags & IFF_RUNNING) == 0)
		{
			continue;
		}

		const sockaddr_in *addr = reinterpret_cast<const sockaddr_in *>(ifa->ifa_addr);
		const UnsignedInt hostAddr = ntohl(addr->sin_addr.s_addr);
		if (localIP != 0 && hostAddr != localIP)
		{
			continue;
		}

		UnsignedInt bcast = 0;
		if (ifa->ifa_broadaddr != nullptr && ifa->ifa_broadaddr->sa_family == AF_INET)
		{
			const sockaddr_in *baddr = reinterpret_cast<const sockaddr_in *>(ifa->ifa_broadaddr);
			bcast = ntohl(baddr->sin_addr.s_addr);
		}
		else if (ifa->ifa_netmask != nullptr && ifa->ifa_netmask->sa_family == AF_INET)
		{
			const sockaddr_in *nmask = reinterpret_cast<const sockaddr_in *>(ifa->ifa_netmask);
			const UnsignedInt mask = ntohl(nmask->sin_addr.s_addr);
			bcast = (hostAddr & mask) | (~mask);
		}
		else
		{
			continue;
		}

		Bool duplicate = FALSE;
		for (Int i = 0; i < count; ++i)
		{
			if (outAddrs[i] == bcast)
			{
				duplicate = TRUE;
				break;
			}
		}

		if (!duplicate && count < maxAddrs)
		{
			outAddrs[count++] = bcast;
		}
	}

	freeifaddrs(ifaddr);
	return count;
#else
	return 0;
#endif
}

// GeneralsX @feature Mr. Meesseeks 17/09/2026 Bridge LANInterfaceDevice platform abstraction to StdLANInterface on POSIX.
Int LANInterfaceDevice::getLocalHostAddresses(UnsignedInt *outAddrs, Int maxAddrs)
{
	return StdLANInterface::getLocalHostAddresses(outAddrs, maxAddrs);
}

Int LANInterfaceDevice::getSubnetBroadcastAddresses(UnsignedInt localIP, UnsignedInt *outAddrs, Int maxAddrs)
{
	return StdLANInterface::getSubnetBroadcastAddresses(localIP, outAddrs, maxAddrs);
}
