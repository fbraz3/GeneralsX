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

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GameProfile.h"
#include "GameLogic/Damage.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#if !defined(_WIN32)
#include <unistd.h>
#endif

namespace {

Bool equalsIgnoreCase(const Char *lhs, const Char *rhs)
{
	if (lhs == nullptr || rhs == nullptr) {
		return FALSE;
	}
#ifdef _WIN32
	return _stricmp(lhs, rhs) == 0;
#else
	return strcasecmp(lhs, rhs) == 0;
#endif
}

} // namespace

GameProfile GameProfile::s_instance;
Bool GameProfile::s_selected = FALSE;

GameProfileKind GameProfile::compiledDefault(void)
{
#if RTS_ZEROHOUR
	return GAME_PROFILE_ZERO_HOUR;
#elif RTS_GENERALS
	return GAME_PROFILE_GENERALS;
#else
#error "GameProfile requires RTS_GENERALS or RTS_ZEROHOUR"
#endif
}

Bool GameProfile::parseKindToken(const Char *token, GameProfileKind *outKind)
{
	if (token == nullptr || outKind == nullptr || token[0] == '\0') {
		return FALSE;
	}

	if (equalsIgnoreCase(token, "generals") || equalsIgnoreCase(token, "gen") || equalsIgnoreCase(token, "g")) {
		*outKind = GAME_PROFILE_GENERALS;
		return TRUE;
	}
	if (equalsIgnoreCase(token, "zh") || equalsIgnoreCase(token, "zerohour") ||
		equalsIgnoreCase(token, "zero-hour") || equalsIgnoreCase(token, "md")) {
		*outKind = GAME_PROFILE_ZERO_HOUR;
		return TRUE;
	}
	return FALSE;
}

void GameProfile::select(GameProfileKind kind)
{
	s_instance.m_kind = kind;
	s_selected = TRUE;
	// GeneralsX @feature cheviiot 23/10/2026 DamageType INI names follow profile (FLESHY_SNIPER vs SUBDUAL_MISSILE).
	ApplyDamageTypeNamesForProfile();
	fprintf(stderr, "[GAME_PROFILE] selected '%s' (%s)\n", s_instance.getShortName(), s_instance.getDisplayName());
	fflush(stderr);
}

void GameProfile::selectKind(GameProfileKind kind)
{
	select(kind);
}

Bool GameProfile::isAutomationLaunch(Int argc, Char **argv)
{
	// GeneralsX @feature cheviiot 24/10/2026 Skip GameSelect UI for headless / scripted launches.
	static const Char *kAutomationFlags[] = {
		"-headless",
		"-replay",
		"-loadreplay",
		"-map",
		"-skirmish",
		"-lanProbe",
		"-saveAtFrame",
		"-quitAfterFrames",
		"-file", // some tool paths
	};
	for (Int i = 1; i < argc; ++i) {
		const Char *arg = argv[i];
		if (arg == nullptr) {
			continue;
		}
		for (UnsignedInt f = 0; f < sizeof(kAutomationFlags) / sizeof(kAutomationFlags[0]); ++f) {
			if (equalsIgnoreCase(arg, kAutomationFlags[f])) {
				return TRUE;
			}
		}
	}
	const Char *skip = getenv("ECHELON_SKIP_GAME_SELECT");
	if (skip != nullptr && skip[0] == '1') {
		return TRUE;
	}
	return FALSE;
}

Bool GameProfile::peekPreferredKind(Int argc, Char **argv, GameProfileKind *outKind)
{
	if (outKind == nullptr) {
		return FALSE;
	}
	GameProfileKind fromArg = compiledDefault();
	Bool saw = FALSE;
	for (Int i = 1; i < argc; ++i) {
		const Char *arg = argv[i];
		if (arg == nullptr || !equalsIgnoreCase(arg, "-game")) {
			continue;
		}
		if (i + 1 >= argc || argv[i + 1] == nullptr) {
			return FALSE;
		}
		GameProfileKind parsed;
		if (!parseKindToken(argv[i + 1], &parsed)) {
			return FALSE;
		}
		fromArg = parsed;
		saw = TRUE;
		++i;
	}
	if (!saw) {
		const Char *envGame = getenv("ECHELON_GAME");
		GameProfileKind fromEnv;
		if (envGame != nullptr && envGame[0] != '\0' && parseKindToken(envGame, &fromEnv)) {
			fromArg = fromEnv;
			saw = TRUE;
		}
	}
	*outKind = fromArg;
	return saw;
}

Bool GameProfile::selectForProcess(Int argc, Char **argv, Bool allowAnyProfile)
{
	const GameProfileKind compiled = compiledDefault();
	GameProfileKind requested = compiled;
	Bool sawExplicit = FALSE;

	for (Int i = 1; i < argc; ++i) {
		const Char *arg = argv[i];
		if (arg == nullptr) {
			continue;
		}
		if (!equalsIgnoreCase(arg, "-game")) {
			continue;
		}
		if (i + 1 >= argc || argv[i + 1] == nullptr) {
			fprintf(stderr, "[GAME_PROFILE] FATAL: -game requires generals|zh\n");
			fflush(stderr);
			return FALSE;
		}
		GameProfileKind parsed;
		if (!parseKindToken(argv[i + 1], &parsed)) {
			fprintf(stderr, "[GAME_PROFILE] FATAL: unknown -game '%s' (use generals|zh)\n", argv[i + 1]);
			fflush(stderr);
			return FALSE;
		}
		if (sawExplicit && parsed != requested) {
			fprintf(stderr, "[GAME_PROFILE] FATAL: conflicting -game values\n");
			fflush(stderr);
			return FALSE;
		}
		requested = parsed;
		sawExplicit = TRUE;
		++i;
	}

	// GeneralsX @feature cheviiot 23/10/2026 Migration binaries stay single-profile; echelon allows both.
	if (!allowAnyProfile && requested != compiled) {
		GameProfile tmp;
		tmp.m_kind = compiled;
		fprintf(stderr,
			"[GAME_PROFILE] FATAL: this executable is built for '%s'; requested '%s'. "
			"Use echelon -game <profile> or the matching GeneralsX / GeneralsXZH binary.\n",
			tmp.getShortName(),
			(requested == GAME_PROFILE_ZERO_HOUR) ? "zh" : "generals");
		fflush(stderr);
		return FALSE;
	}

	// GeneralsX @feature cheviiot 23/10/2026 Startup chooser path for unified echelon (PLAN-023 Gate 6).
	// Precedence: -game > ECHELON_GAME > optional console prompt (ECHELON_ASK_PROFILE=1) > compiled default.
	if (allowAnyProfile && !sawExplicit) {
		const Char *envGame = getenv("ECHELON_GAME");
		GameProfileKind fromEnv;
		if (envGame != nullptr && envGame[0] != '\0' && parseKindToken(envGame, &fromEnv)) {
			requested = fromEnv;
			sawExplicit = TRUE;
			fprintf(stderr, "[GAME_PROFILE] ECHELON_GAME='%s'\n", envGame);
			fflush(stderr);
		}
	}
	if (allowAnyProfile && !sawExplicit) {
		const Char *ask = getenv("ECHELON_ASK_PROFILE");
#if !defined(_WIN32)
		const Bool canAsk = ask != nullptr && ask[0] == '1' && isatty(STDIN_FILENO) && isatty(STDERR_FILENO);
#else
		const Bool canAsk = ask != nullptr && ask[0] == '1';
#endif
		if (canAsk) {
			fprintf(stderr,
				"[GAME_PROFILE] Select game profile:\n"
				"  1) generals\n"
				"  2) zh (Zero Hour) [default]\n"
				"> ");
			fflush(stderr);
			Char line[64];
			if (fgets(line, (Int)sizeof(line), stdin) != nullptr) {
				if (line[0] == '1') {
					requested = GAME_PROFILE_GENERALS;
				} else if (line[0] == '2' || line[0] == '\n' || line[0] == '\0') {
					requested = GAME_PROFILE_ZERO_HOUR;
				} else {
					GameProfileKind parsed;
					if (parseKindToken(line, &parsed)) {
						requested = parsed;
					}
				}
			}
		}
	}

	select(requested);
	return TRUE;
}

void GameProfile::ensureSelected(void)
{
	if (!s_selected) {
		// GeneralsX @bugfix cheviiot 23/10/2026 Avoid locking ZH pools during static ctors before selectForProcess.
		fprintf(stderr, "[GAME_PROFILE] WARNING: no early selection; using compiled default\n");
		fflush(stderr);
		select(compiledDefault());
	}
}

Bool GameProfile::isSelected(void)
{
	return s_selected;
}

const GameProfile &GameProfile::get(void)
{
	ensureSelected();
	return s_instance;
}

const Char *GameProfile::getShortName(void) const
{
	return isZeroHour() ? "zh" : "generals";
}

const Char *GameProfile::getDisplayName(void) const
{
	return isZeroHour() ? "Command & Conquer Generals: Zero Hour" : "Command & Conquer Generals";
}

const Char *GameProfile::getPrimaryAssetEnvName(void) const
{
	return isZeroHour() ? "CNC_GENERALS_ZH_PATH" : "CNC_GENERALS_PATH";
}

const Char *GameProfile::getLegacyInstallEnvName(void) const
{
	return isZeroHour() ? "CNC_ZH_INSTALLPATH" : "CNC_GENERALS_INSTALLPATH";
}

const Char *GameProfile::getRegistryGamePath(void) const
{
	return isZeroHour()
		? "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour"
		: "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
}

const Char *GameProfile::getUserDataLeafName(void) const
{
	// GeneralsX @feature cheviiot 23/10/2026 Keep Generals and ZH user data trees separate under one binary.
	return isZeroHour() ? "GeneralsZH" : "Generals";
}

UnsignedInt GameProfile::getNameKeySocketCount(void) const
{
	// Prime socket counts from retail Generals (6473) and Zero Hour (45007).
	return isZeroHour() ? 45007u : 6473u;
}

UnsignedInt GameProfile::getExpectedNameKeyIdAfterScienceIni(void) const
{
	// Both profiles expect the first post-Science key to be 1 when the priming CRC matches.
	return 1u;
}

UnsignedInt GameProfile::getExpectedNameKeyIdAfterUpgradeIni(void) const
{
	// Retail verifyNameKeyID checkpoints differ: Generals 1586, Zero Hour 2265.
	return isZeroHour() ? 2265u : 1586u;
}
