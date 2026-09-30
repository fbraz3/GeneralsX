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

// Win32LANInterface.cpp ///////////////////////////////////////////////////////
// Windows implementation of local network interface operations
////////////////////////////////////////////////////////////////////////////////

#include "Win32Device/Common/Win32LANInterface.h"

// GeneralsX @feature Mr. Meesseeks 17/09/2026 Windows fallback implementation of LAN interface device operations.
// Preserves existing Windows behavior (IPEnumeration relies on gethostname/gethostbyname, LANAPI falls back to m_broadcastAddr).
Int Win32LANInterface::getLocalHostAddresses(UnsignedInt *outAddrs, Int maxAddrs)
{
	(void)outAddrs;
	(void)maxAddrs;
	return 0;
}

Int Win32LANInterface::getSubnetBroadcastAddresses(UnsignedInt localIP, UnsignedInt *outAddrs, Int maxAddrs)
{
	(void)localIP;
	(void)outAddrs;
	(void)maxAddrs;
	return 0;
}

Int LANInterfaceDevice::getLocalHostAddresses(UnsignedInt *outAddrs, Int maxAddrs)
{
	return Win32LANInterface::getLocalHostAddresses(outAddrs, maxAddrs);
}

Int LANInterfaceDevice::getSubnetBroadcastAddresses(UnsignedInt localIP, UnsignedInt *outAddrs, Int maxAddrs)
{
	return Win32LANInterface::getSubnetBroadcastAddresses(localIP, outAddrs, maxAddrs);
}
