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

// LANInterfaceDevice.h ////////////////////////////////////////////////////////
// Platform abstraction for local network interface operations
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Lib/BaseType.h"

// GeneralsX @feature Mr. Meesseeks 17/09/2026 Platform abstraction for LAN interface enumeration and subnet broadcast calculation.
class LANInterfaceDevice
{
public:
	/**
	 * Enumerate local active IPv4 host addresses suitable for LAN play and online hosting.
	 * Populates outAddrs (up to maxAddrs) with IPv4 addresses in host byte order.
	 * Returns the count of addresses populated.
	 */
	static Int getLocalHostAddresses(UnsignedInt *outAddrs, Int maxAddrs);

	/**
	 * Calculate directed subnet broadcast addresses for the given localIP (in host byte order).
	 * If localIP is 0, gathers broadcast addresses for all valid interfaces.
	 * Populates outAddrs (up to maxAddrs) with broadcast addresses in host byte order.
	 * Returns the count of broadcast addresses populated.
	 */
	static Int getSubnetBroadcastAddresses(UnsignedInt localIP, UnsignedInt *outAddrs, Int maxAddrs);
};
