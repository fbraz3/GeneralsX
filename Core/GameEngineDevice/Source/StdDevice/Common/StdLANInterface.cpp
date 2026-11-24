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
#if defined(__EMSCRIPTEN__)
#include "GameNetwork/udp.h"
#endif

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
// GeneralsX @bugfix Maxim Sintsov 17/11/2026 Extract the AF_INET netmask in host byte order.
static bool HostNetmaskOf(const struct ifaddrs *ifa, UnsignedInt *outMask)
{
	if (ifa->ifa_netmask == nullptr || ifa->ifa_netmask->sa_family != AF_INET)
	{
		return false;
	}

	*outMask = ntohl(reinterpret_cast<const sockaddr_in *>(ifa->ifa_netmask)->sin_addr.s_addr);
	return true;
}
// GeneralsX @bugfix Maxim Sintsov 17/11/2026 True if the netmask is a real subnet (not /0, not /32);
// WireGuard-style meshes carry one, PPP dial-up and host-route VPN adapters do not.
static bool HasSubnetNetmask(const struct ifaddrs *ifa)
{
	UnsignedInt mask = 0;
	return HostNetmaskOf(ifa, &mask) && mask != 0 && mask != 0xFFFFFFFFu;
}
// GeneralsX @refactor Mr. Meesseeks 17/09/2026 Common interface suitability predicate for address enumeration and broadcast discovery.
static bool IsUsableInterface(const struct ifaddrs *ifa)
{
	if (ifa == nullptr || ifa->ifa_addr == nullptr)
	{
		return false;
	}

	if (ifa->ifa_addr->sa_family != AF_INET)
	{
		return false;
	}

	if ((ifa->ifa_flags & IFF_UP) == 0 || (ifa->ifa_flags & IFF_LOOPBACK) != 0)
	{
		return false;
	}

	// GeneralsX @bugfix BenderAI 16/09/2026 Require an active carrier (IFF_RUNNING). Idle Docker/VM
	// bridges stay administratively UP without a link, and were being offered as the local address,
	// which broke LAN discovery and online hosting behind an unreachable 172.x.y.z bridge IP.
	if ((ifa->ifa_flags & IFF_RUNNING) == 0)
	{
		return false;
	}

	// GeneralsX @bugfix Maxim Sintsov 17/11/2026 Accept broadcast links and subnetted point-to-point
	// tunnels (WireGuard-style VPN meshes) so their addresses can be used for LAN games. P2P links
	// with /0 or /32 netmasks stay excluded, as under the blanket rule replaced here
	// (GeneralsX @feature Mr. Meesseeks 11/07/2026).
	const bool isBroadcastLink = (ifa->ifa_flags & IFF_BROADCAST) != 0;
	const bool isSubnetP2PLink = (ifa->ifa_flags & (IFF_POINTOPOINT | IFF_BROADCAST)) == IFF_POINTOPOINT
		&& HasSubnetNetmask(ifa);
	if (!isBroadcastLink && !isSubnetP2PLink)
	{
		return false;
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
			return false;
		}
	}

	return true;
}
// GeneralsX @bugfix Mr. Meesseeks 29/09/2026 Identify wireless interface name prefixes (e.g. wlan*, wlp*, wls*, wlo*, ra*)
// to allow prioritizing wired physical Ethernet adapters before Wi-Fi adapters in enumeration.
static bool IsWirelessInterfaceName(const char *name)
{
	if (name == nullptr)
	{
		return false;
	}

	if (strncmp(name, "wl", 2) == 0 || strncmp(name, "ra", 2) == 0)
	{
		return true;
	}

	return false;
}
#endif

// GeneralsX @feature Mr. Meesseeks 17/09/2026 Enumerate local active IPv4 interface addresses on POSIX.
// GeneralsX @bugfix Mr. Meesseeks 29/09/2026 Prioritize wired physical Ethernet interfaces before wireless adapters.
Int StdLANInterface::getLocalHostAddresses(UnsignedInt *outAddrs, Int maxAddrs)
{
	if (outAddrs == nullptr || maxAddrs <= 0)
	{
		return 0;
	}

#if defined(__EMSCRIPTEN__)
	const UnsignedInt webIP = GetWebLanVirtualIP();
	if (webIP != 0)
	{
		outAddrs[0] = webIP;
		return 1;
	}
	return 0;
#elif !defined(_WIN32)
	struct ifaddrs *ifaddr = nullptr;
	if (getifaddrs(&ifaddr) != 0)
	{
		DEBUG_LOG(("Failed call to getifaddrs; errno returned %d", errno));
		return 0;
	}

	Int count = 0;
	auto appendIfa = [&](const struct ifaddrs *ifa)
	{
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
	};

	// Pass 1: Active wired physical interfaces (eth*, en*, etc.)
	for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
	{
		if (IsUsableInterface(ifa) && !IsWirelessInterfaceName(ifa->ifa_name))
		{
			appendIfa(ifa);
		}
	}

	// Pass 2: Active wireless interfaces (wlan*, wlp*, etc.)
	for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
	{
		if (IsUsableInterface(ifa) && IsWirelessInterfaceName(ifa->ifa_name))
		{
			appendIfa(ifa);
		}
	}

	freeifaddrs(ifaddr);
	return count;
#else
	return 0;
#endif
}

// GeneralsX @bugfix Mr. Meesseeks 29/09/2026 Test if an IPv4 address belongs to a real active local network adapter (excluding loopback).
Bool StdLANInterface::isRealLocalInterfaceAddress(UnsignedInt ip)
{
	if (ip == 0 || (ip >> 24) == 127)
	{
		return FALSE;
	}

#if defined(__EMSCRIPTEN__)
	return (ip == GetWebLanVirtualIP()) ? TRUE : FALSE;
#elif !defined(_WIN32)
	UnsignedInt addrs[16];
	Int count = getLocalHostAddresses(addrs, ARRAY_SIZE(addrs));
	for (Int i = 0; i < count; ++i)
	{
		if (addrs[i] == ip)
		{
			return TRUE;
		}
	}
#endif

	return FALSE;
}

// GeneralsX @bugfix Mr. Meesseeks 29/09/2026 Test if an IPv4 address belongs to any active local interface on this machine.
Bool StdLANInterface::isLocalHostAddress(UnsignedInt ip)
{
	if (ip == 0)
	{
		return FALSE;
	}

	// 127.0.0.0/8 loopback range
	if ((ip >> 24) == 127)
	{
		return TRUE;
	}

	return isRealLocalInterfaceAddress(ip);
}

// GeneralsX @feature Mr. Meesseeks 17/09/2026 Discover per-interface IPv4 subnet broadcast addresses for LAN discovery on POSIX.
Int StdLANInterface::getSubnetBroadcastAddresses(UnsignedInt localIP, UnsignedInt *outAddrs, Int maxAddrs)
{
	if (outAddrs == nullptr || maxAddrs <= 0)
	{
		return 0;
	}

#if defined(__EMSCRIPTEN__)
	return 0;
#elif !defined(_WIN32)
	Int count = 0;
	struct ifaddrs *ifaddr = nullptr;
	if (getifaddrs(&ifaddr) != 0)
	{
		return 0;
	}

	for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
	{
		if (!IsUsableInterface(ifa))
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
		UnsignedInt mask = 0;
		if ((ifa->ifa_flags & (IFF_POINTOPOINT | IFF_BROADCAST)) == IFF_POINTOPOINT)
		{
			// GeneralsX @bugfix Maxim Sintsov 17/11/2026 On point-to-point links the ifa_ifu union is
			// the peer address, not a broadcast; derive it from the netmask instead.
			if (!HostNetmaskOf(ifa, &mask))
			{
				continue;
			}
			if (mask == 0xFFFFFFFEu)
			{
				// GeneralsX @bugfix Maxim Sintsov 22/11/2026 A /31 (RFC 3021) carries no
				// subnet-directed broadcast: the all-ones derivation would return the local
				// address on the odd host of the pair. Send to the reported peer instead; when
				// no peer is known (WireGuard tunnels report none) emit nothing and let LANAPI
				// fall back to the global broadcast.
				if (ifa->ifa_dstaddr == nullptr || ifa->ifa_dstaddr->sa_family != AF_INET)
				{
					continue;
				}
				bcast = ntohl(reinterpret_cast<const sockaddr_in *>(ifa->ifa_dstaddr)->sin_addr.s_addr);
				if (bcast == 0 || bcast == hostAddr)
				{
					continue;
				}
			}
			else
			{
				bcast = (hostAddr & mask) | (~mask);
			}
		}
		else if (ifa->ifa_broadaddr != nullptr && ifa->ifa_broadaddr->sa_family == AF_INET)
		{
			const sockaddr_in *baddr = reinterpret_cast<const sockaddr_in *>(ifa->ifa_broadaddr);
			bcast = ntohl(baddr->sin_addr.s_addr);
		}
		else if (HostNetmaskOf(ifa, &mask))
		{
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

// GeneralsX @bugfix Mr. Meesseeks 29/09/2026 Bridge isLocalHostAddress to StdLANInterface on POSIX.
Bool LANInterfaceDevice::isLocalHostAddress(UnsignedInt ip)
{
	return StdLANInterface::isLocalHostAddress(ip);
}

// GeneralsX @bugfix Mr. Meesseeks 29/09/2026 Bridge isRealLocalInterfaceAddress to StdLANInterface on POSIX.
Bool LANInterfaceDevice::isRealLocalInterfaceAddress(UnsignedInt ip)
{
	return StdLANInterface::isRealLocalInterfaceAddress(ip);
}
