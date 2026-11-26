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


// GeneralsX: WHICH Direct3D 8 THIS START USES. DXVK (d3d8.dll + d3d9.dll beside the exe) needs Vulkan 1.3; a PC without it
// gets compatibility graphics instead: d3d8to9, compiled into the x64 exe (Dependencies/d3d8to9), onto Windows' own d3d9.dll.
// Both kinds of player join the same lobbies: nothing here reaches the simulation. Pure, no Windows headers:
// scripts/test/d3d-backend-test.sh. WinMain.cpp gxPickD3D feeds it and acts on the answer.

#pragma once

#include <cstddef>
#include <cstdio>
#include <cstring>

enum GxD3DBackend
{
	GXD3D_DXVK = 0,
	GXD3D_COMPAT = 1,
	GXD3D_SYSTEM_D3D8 = 2,	///< no DXVK pair and no compat built in: Windows' own d3d8.dll, as before
	GXD3D_REFUSED = 3		///< DXVK pair, no Vulkan, no compat built in: the launch-diagnostics box, then exit
};
enum GxD3DReason { GXD3D_R_VULKAN_OK = 0, GXD3D_R_NO_VULKAN, GXD3D_R_NO_DXVK_PAIR, GXD3D_R_FORCED_ENV, GXD3D_R_FORCED_KEY, GXD3D_R_SYSTEM_D3D8,
	GXD3D_R_DXVK_FAILED };	///< the last DXVK start never got a device (gxD3DFallback)
struct GxD3DDecision { int backend; int reason; bool badKey; };

/// The [GXGFX] line and gxgraphics.txt write these words; the launcher reads them.
inline const char *gxD3DBackendWord(int b)
{
	switch (b) { case GXD3D_COMPAT: return "compat"; case GXD3D_SYSTEM_D3D8: return "system-d3d8"; case GXD3D_REFUSED: return "refused"; default: return "dxvk"; }
}
inline const char *gxD3DReasonWord(int r)
{
	switch (r)
	{
	case GXD3D_R_NO_VULKAN: return "no-vulkan";
	case GXD3D_R_NO_DXVK_PAIR: return "no-dxvk-pair";
	case GXD3D_R_FORCED_ENV: return "forced-env";
	case GXD3D_R_FORCED_KEY: return "forced-key";
	case GXD3D_R_SYSTEM_D3D8: return "system-d3d8";
	case GXD3D_R_DXVK_FAILED: return "dxvk-failed";
	default: return "vulkan-ok";
	}
}

/// ASCII case-insensitive whole-word compare (the renderer key's rule, GXRendererKey.h).
inline bool gxD3DWordIs(const char *v, const char *w)
{
	for (; *w; ++v, ++w)
	{
		char c = *v;
		if (c >= 'A' && c <= 'Z')
			c = char(c - 'A' + 'a');
		if (c != *w)
			return false;
	}
	return *v == '\0';
}

/// env = GX_D3D_BACKEND (test-only: shipping builds scrub it, GXEnvAllowList.h), key = Options.ini GXGraphicsBackend.
inline bool gxD3DForced(const char *env, const char *key, int *reason)
{
	if (env != nullptr && gxD3DWordIs(env, "compat")) { *reason = GXD3D_R_FORCED_ENV; return true; }
	if (key != nullptr && gxD3DWordIs(key, "compat")) { *reason = GXD3D_R_FORCED_KEY; return true; }
	return false;
}

/// Run the Vulkan probe? Only when DXVK would render and compat is not forced: a forced compat start never touches a
/// Vulkan driver, so one that crashes inside vkCreateInstance cannot stop it. compatBuilt = GX_COMPAT_D3D9 (MSVC x64).
inline bool gxD3DNeedsProbe(const char *env, const char *key, bool compatBuilt, bool dxvkPair)
{
	int r;
	return dxvkPair && !(compatBuilt && gxD3DForced(env, key, &r));
}

/// vulkanVerdict: 0 = gxvk::VULKAN_OK, > 0 = a refusal (GXVulkanCheck.h), -1 = not probed.
inline GxD3DDecision gxD3DDecide(const char *env, const char *key, bool compatBuilt, bool dxvkPair, int vulkanVerdict)
{
	GxD3DDecision d = { GXD3D_DXVK, GXD3D_R_VULKAN_OK,
		key != nullptr && key[0] != '\0' && !gxD3DWordIs(key, "auto") && !gxD3DWordIs(key, "compat") && !gxD3DWordIs(key, "dxvk") };
	int r;
	if (compatBuilt && gxD3DForced(env, key, &r))
	{
		d.backend = GXD3D_COMPAT;
		d.reason = r;
		return d;
	}
	if (!dxvkPair)
	{
		d.backend = compatBuilt ? GXD3D_COMPAT : GXD3D_SYSTEM_D3D8;
		d.reason = compatBuilt ? GXD3D_R_NO_DXVK_PAIR : GXD3D_R_SYSTEM_D3D8;
		return d;
	}
	if (vulkanVerdict != 0)
	{
		d.backend = compatBuilt ? GXD3D_COMPAT : GXD3D_REFUSED;
		d.reason = GXD3D_R_NO_VULKAN;
	}
	return d;
}

// THE CRASH MARKER. <userdata>gxgraphics.txt is "<backend> <reason>" (what the launcher shows) and then state lines:
//   pending=probe        the Vulkan probe is running            (gxPickD3D, around the probe)
//   pending=dxvk-device  DXVK is bringing its device up         (DX8Wrapper's device-stage hook, cleared once a device exists)
//   compat-left=N        compatibility starts still owed after a failed DXVK start
//   failures=N           DXVK starts that died since the last DXVK device (the backoff)
// A start that finds "pending" died there - a Vulkan driver that crashes in the probe, or a DXVK that cannot make its device - so
// it and the next ones run compatibility graphics without probing; then DXVK is tried again, so a crash from some other cause never
// pins a working GPU to compat. The run doubles with each failure in a row - 3, 6, 12 starts, then 12 every time - so a driver
// that always crashes costs one crash in 13 starts, not one in 4. A DXVK device, Options.ini GXGraphicsBackend=dxvk, or deleting
// the file clears it all.
enum { GXD3D_PENDING_NONE = 0, GXD3D_PENDING_PROBE, GXD3D_PENDING_DEVICE };
enum { GXD3D_COMPAT_STARTS_AFTER_FAILURE = 3, GXD3D_COMPAT_STARTS_MAX = 12 };
struct GxD3DState { int pending; int compatLeft; int failures; };

/// How many compatibility starts follow the failures-th DXVK failure in a row (1 -> 3, 2 -> 6, 3 and on -> 12).
inline int gxD3DCompatStarts(int failures)
{
	int n = GXD3D_COMPAT_STARTS_AFTER_FAILURE;
	for (int i = 1; i < failures && n < GXD3D_COMPAT_STARTS_MAX; ++i)
		n *= 2;
	return n < GXD3D_COMPAT_STARTS_MAX ? n : int(GXD3D_COMPAT_STARTS_MAX);
}

/// The state lines of gxgraphics.txt (null or unreadable = none). The first line is the decision, never state.
inline GxD3DState gxD3DParseState(const char *text)
{
	GxD3DState st = { GXD3D_PENDING_NONE, 0, 0 };
	if (text == nullptr)
		return st;
	const char *line = text;
	bool first = true;
	while (*line != '\0')
	{
		const char *end = line;
		while (*end != '\0' && *end != '\n' && *end != '\r')
			++end;
		const int len = int(end - line);
		if (!first)
		{
			if (len == 13 && std::strncmp(line, "pending=probe", 13) == 0)
				st.pending = GXD3D_PENDING_PROBE;
			else if (len == 19 && std::strncmp(line, "pending=dxvk-device", 19) == 0)
				st.pending = GXD3D_PENDING_DEVICE;
			else if (len > 12 && std::strncmp(line, "compat-left=", 12) == 0 && line[12] >= '0' && line[12] <= '9')
			{
				int n = 0;
				for (const char *c = line + 12; c < end && *c >= '0' && *c <= '9' && n < 1000; ++c)
					n = n * 10 + (*c - '0');
				st.compatLeft = n < GXD3D_COMPAT_STARTS_MAX ? n : int(GXD3D_COMPAT_STARTS_MAX);
			}
			else if (len > 9 && std::strncmp(line, "failures=", 9) == 0 && line[9] >= '0' && line[9] <= '9')
			{
				int n = 0;
				for (const char *c = line + 9; c < end && *c >= '0' && *c <= '9' && n < 1000; ++c)
					n = n * 10 + (*c - '0');
				st.failures = n < 99 ? n : 99;
			}
		}
		first = false;
		line = end;
		while (*line == '\n' || *line == '\r')
			++line;
	}
	return st;
}

/// The whole file. Returns snprintf's count (<= 0 or >= n: the caller writes nothing).
inline int gxD3DFormatFile(char *out, std::size_t n, int backend, int reason, GxD3DState st)
{
	char left[32] = "", failed[32] = "";
	if (st.compatLeft > 0)
		std::snprintf(left, sizeof(left), "compat-left=%d\n", st.compatLeft);
	if (st.failures > 0)
		std::snprintf(failed, sizeof(failed), "failures=%d\n", st.failures);
	return std::snprintf(out, n, "%s %s\n%s%s%s", gxD3DBackendWord(backend), gxD3DReasonWord(reason),
		st.pending == GXD3D_PENDING_PROBE ? "pending=probe\n" : st.pending == GXD3D_PENDING_DEVICE ? "pending=dxvk-device\n" : "", left,
		failed);
}

/// Does THIS start run compatibility graphics because a DXVK start failed? Only where DXVK would otherwise run (the pair is
/// there, compat is built, compat is not forced). *next = the state to write for this start. GXGraphicsBackend=dxvk is the
/// player's "try Vulkan again": no fallback, state cleared.
inline bool gxD3DFallback(const char *env, const char *key, bool compatBuilt, bool dxvkPair, const GxD3DState &st, GxD3DState *next)
{
	*next = st;
	int r;
	if (!compatBuilt || !dxvkPair || gxD3DForced(env, key, &r))
		return false;
	if (key != nullptr && gxD3DWordIs(key, "dxvk"))
	{
		next->pending = GXD3D_PENDING_NONE;
		next->compatLeft = 0;
		next->failures = 0;
		return false;
	}
	if (st.pending != GXD3D_PENDING_NONE)
	{
		next->pending = GXD3D_PENDING_NONE;
		next->failures = st.failures < 99 ? st.failures + 1 : 99;
		next->compatLeft = gxD3DCompatStarts(next->failures) - 1;	// this start is the first of them
		return true;
	}
	if (st.compatLeft > 0)
	{
		next->compatLeft = st.compatLeft - 1;
		return true;
	}
	return false;
}

/// gxgraphics.txt for the first instance - the player's copy, the file the launcher reads - and gxgraphics_InstanceNN.txt for
/// a copy started beside it (test harnesses, replays), as Options.ini / Options_InstanceNN.ini: two copies never share a marker.
inline void gxD3DStateFileName(char *out, std::size_t n, unsigned int instanceId)
{
	if (instanceId > 1u)
		std::snprintf(out, n, "gxgraphics_Instance%.2u.txt", instanceId);
	else
		std::snprintf(out, n, "gxgraphics.txt");
}
