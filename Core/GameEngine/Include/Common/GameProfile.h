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

// GeneralsX @feature cheviiot 23/10/2026 Immutable process game profile for PLAN-023 Stage 03.
// Selected before asset mount and subsystem construction. Spelling of -game is provisional.

#pragma once

#include "Lib/BaseType.h"

enum GameProfileKind CPP_11(: Int)
{
	GAME_PROFILE_GENERALS = 0,
	GAME_PROFILE_ZERO_HOUR = 1
};

class GameProfile
{
public:
	// Scan argv for provisional "-game <name>", defaulting to the compiled game.
	// Returns FALSE on invalid tokens or a profile that this binary cannot run yet.
	// allowAnyProfile: TRUE for the unified echelon binary (both profiles); FALSE for
	// migration binaries GeneralsX / GeneralsXZH (compiled profile only).
	static Bool selectForProcess(Int argc, Char **argv, Bool allowAnyProfile = FALSE);
	// Tools or late paths that skip main selection fall back to the compiled default.
	static void ensureSelected(void);
	static Bool isSelected(void);
	static const GameProfile &get(void);

	GameProfileKind getKind(void) const { return m_kind; }
	Bool isGenerals(void) const { return m_kind == GAME_PROFILE_GENERALS; }
	Bool isZeroHour(void) const { return m_kind == GAME_PROFILE_ZERO_HOUR; }
	const Char *getShortName(void) const;
	const Char *getDisplayName(void) const;
	const Char *getPrimaryAssetEnvName(void) const;
	const Char *getLegacyInstallEnvName(void) const;
	// Windows registry / registry.ini game identity path under EA Games.
	const Char *getRegistryGamePath(void) const;
	// Cross-platform user-data leaf under GeneralsX/ (Generals vs GeneralsZH).
	const Char *getUserDataLeafName(void) const;
	// Retail-compatible NameKeyGenerator hash socket counts (must stay profile-specific).
	UnsignedInt getNameKeySocketCount(void) const;
	// Expected next NameKey IDs after Science/Upgrade INI load (retail CRC checkpoints).
	UnsignedInt getExpectedNameKeyIdAfterScienceIni(void) const;
	UnsignedInt getExpectedNameKeyIdAfterUpgradeIni(void) const;
	Bool loadsBaseGeneralsAssets(void) const { return isZeroHour(); }

private:
	GameProfileKind m_kind;

	static GameProfile s_instance;
	static Bool s_selected;

	static GameProfileKind compiledDefault(void);
	static Bool parseKindToken(const Char *token, GameProfileKind *outKind);
	static void select(GameProfileKind kind);
};
