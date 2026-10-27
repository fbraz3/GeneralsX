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
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: MemoryInit.cpp
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: MemoryInit.cpp
//
// Created:   Steven Johnson, August 2001
//
// Desc:      Memory manager
//
// ----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#ifndef DISABLE_GAMEMEMORY

// SYSTEM INCLUDES

// USER INCLUDES
#include "Common/GameMemory.h"
#include "Common/GameProfile.h"

struct PoolSizeRec
{
	const char* name;
	Int initial;
	Int overflow;
};

// GeneralsX @refactor cheviiot 23/10/2026 Both pool tables in one binary; GameProfile picks at init.
#define DefaultDMA DefaultDMA_Generals
#define PoolSizes PoolSizes_Generals
#include "GameMemoryInitDMA_Generals.inl"
#include "GameMemoryInitPools_Generals.inl"
#undef DefaultDMA
#undef PoolSizes

#define DefaultDMA DefaultDMA_GeneralsMD
#define PoolSizes PoolSizes_GeneralsMD
#include "GameMemoryInitDMA_GeneralsMD.inl"
#include "GameMemoryInitPools_GeneralsMD.inl"
#undef DefaultDMA
#undef PoolSizes

static Bool useGeneralsMemoryPools(void)
{
	// GeneralsX @bugfix cheviiot 23/10/2026 libhwy (and other .so ctors) may call operator new
	// before main/selectForProcess via preMainInitMemoryManager. Do not call GameProfile::get()
	// here — that would lock the compiled default. Unselected → ZH tables (safe superset).
	return GameProfile::isSelected() && GameProfile::get().isGenerals();
}

static PoolSizeRec *activePoolSizes(void)
{
	return useGeneralsMemoryPools() ? PoolSizes_Generals : PoolSizes_GeneralsMD;
}

//-----------------------------------------------------------------------------
void userMemoryManagerGetDmaParms(Int *numSubPools, const PoolInitRec **pParms)
{
	if (useGeneralsMemoryPools())
	{
		*numSubPools = ARRAY_SIZE(DefaultDMA_Generals);
		*pParms = DefaultDMA_Generals;
	}
	else
	{
		*numSubPools = ARRAY_SIZE(DefaultDMA_GeneralsMD);
		*pParms = DefaultDMA_GeneralsMD;
	}
}

//-----------------------------------------------------------------------------
void userMemoryAdjustPoolSize(const char *poolName, Int& initialAllocationCount, Int& overflowAllocationCount)
{
	if (initialAllocationCount > 0)
		return;

	for (const PoolSizeRec* p = activePoolSizes(); p->name != nullptr; ++p)
	{
		if (strcmp(p->name, poolName) == 0)
		{
			initialAllocationCount = p->initial;
			overflowAllocationCount = p->overflow;
			return;
		}
	}

	DEBUG_CRASH(("Initial size for pool %s not found -- you should add it to MemoryInit.cpp",poolName));
}

//-----------------------------------------------------------------------------
static Int roundUpMemBound(Int i)
{
	const int MEM_BOUND_ALIGNMENT = 4;

	if (i < MEM_BOUND_ALIGNMENT)
		return MEM_BOUND_ALIGNMENT;
	else
		return (i + (MEM_BOUND_ALIGNMENT-1)) & ~(MEM_BOUND_ALIGNMENT-1);
}

//-----------------------------------------------------------------------------
void userMemoryManagerInitPools()
{
	// note that we MUST use stdio stuff here, and not the normal game file system
	// (with bigfile support, etc), because that relies on memory pools, which
	// aren't yet initialized properly! so rely ONLY on straight stdio stuff here.
	// (not even AsciiString. thanks.)

	// since we're called prior to main, the cur dir might not be what
	// we expect. so do it the hard way.
	char buf[_MAX_PATH];
	::GetModuleFileName(nullptr, buf, sizeof(buf));
	if (char* pEnd = strrchr(buf, '\\'))
	{
		*pEnd = 0;
	}
	strlcat(buf, "\\Data\\INI\\MemoryPools.ini", ARRAY_SIZE(buf));

	FILE* fp = fopen(buf, "r");
	if (fp)
	{
		char poolName[256];
		int initial, overflow;
		while (fgets(buf, _MAX_PATH, fp))
		{
			if (buf[0] == ';')
				continue;
			if (sscanf(buf, "%s %d %d", poolName, &initial, &overflow ) == 3)
			{
				for (PoolSizeRec* p = activePoolSizes(); p->name != nullptr; ++p)
				{
					if (stricmp(p->name, poolName) == 0)
					{
						// currently, these must be multiples of 4. so round up.
						p->initial = roundUpMemBound(initial);
						p->overflow = roundUpMemBound(overflow);
						break;	// from for-p
					}
				}
			}
		}
		fclose(fp);
	}
}

#endif // DISABLE_GAMEMEMORY
