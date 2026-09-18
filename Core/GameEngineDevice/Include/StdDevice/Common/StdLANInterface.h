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

// StdLANInterface.h ///////////////////////////////////////////////////////////
// Standard POSIX implementation of local network interface operations
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameNetwork/LANInterfaceDevice.h"

// GeneralsX @feature Mr. Meesseeks 17/09/2026 POSIX implementation of LAN interface device operations.
class StdLANInterface
{
public:
	static Int getLocalHostAddresses(UnsignedInt *outAddrs, Int maxAddrs);
	static Int getSubnetBroadcastAddresses(UnsignedInt localIP, UnsignedInt *outAddrs, Int maxAddrs);
};
