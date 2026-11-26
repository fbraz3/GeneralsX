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

// GeneralsX @bugfix NO VULKAN 1.3, NO GAME - SAID IN WORDS, NOT A CRASH.
//
// Windows renders through DXVK 2.6 (d3d8.dll/d3d9.dll, scripts/build/windows/dxvk/dxvk-win64.pins), or
// through the native renderer; both need Vulkan 1.3. A player's PC had no Vulkan loader at all. DXVK logged
// "err: Vulkan: vkGetInstanceProcAddr not found" (vulkan_loader.cpp) and then called a null pointer while
// creating the device: 18 crash logs of 0xC0000005 at 0x0 <- d3d9.dll <- D3D8.DLL <- generalszh.exe, and
// two older ones of an uncaught GCC throw (0x20474343) out of D3D8.DLL. No box, no message: a black window
// for ~20 s, then nothing (a player's 0.9.72 install).
//
// So WinMain asks first - when this start goes through the DXVK pair at all (usesDxvkPair) - before the
// game window exists, with exactly DXVK 2.6's own requirements and nothing stricter: a loader (winevulkan.dll
// or vulkan-1.dll, DXVK's order) loads and exports vkGetInstanceProcAddr; a Vulkan 1.3 instance can be
// created (dxvk_instance.cpp asks for apiVersion 1.3); at least one physical device reports 1.3 or newer
// (dxvk_device_filter.cpp skips every adapter below it). A machine this refuses is one DXVK would refuse
// too, so the check never locks out a PC that could run the game - it only turns a crash into a sentence.
//
// No Vulkan SDK header: the handful of types the probe touches are declared here, so the check builds in
// every Windows configuration, including the ones without GX_VULKAN_INCLUDE_DIR. The probe takes its loader
// by injection, so scripts/test/vulkan-check-test.cpp drives every branch on any host.

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace gxvk
{

/// VK_MAKE_API_VERSION(0, 1, 3, 0): what DXVK 2.6 asks of the instance and of every adapter.
constexpr uint32_t kNeedApi = (1u << 22) | (3u << 12);

inline uint32_t apiMajor(uint32_t v) { return (v >> 22) & 0x7Fu; }
inline uint32_t apiMinor(uint32_t v) { return (v >> 12) & 0x3FFu; }
inline uint32_t apiPatch(uint32_t v) { return v & 0xFFFu; }

enum Verdict
{
	VULKAN_OK = 0,
	VULKAN_NO_LOADER,      ///< neither winevulkan.dll nor vulkan-1.dll loaded
	VULKAN_NO_ENTRY,       ///< one loaded, but none exports vkGetInstanceProcAddr (the player's case in DXVK's words)
	VULKAN_NO_INSTANCE,    ///< vkCreateInstance for 1.3 failed (a 1.0 loader answers VK_ERROR_INCOMPATIBLE_DRIVER)
	VULKAN_NO_DEVICE,      ///< an instance, but no physical device at all
	VULKAN_DEVICE_TOO_OLD  ///< devices, but none reports Vulkan 1.3
};

/// What the probe saw. Everything a message or a log line needs, and nothing it had to keep open.
struct Probe
{
	bool loaderLoaded = false;           ///< some candidate in kLoaderNames loaded
	bool hasGetInstanceProcAddr = false; ///< ...and one of them exports vkGetInstanceProcAddr
	const char *loaderName = "";         ///< the one used
	uint32_t instanceVersion = 0;  ///< the loader's own version; 0 when it has no vkEnumerateInstanceVersion (1.0)
	bool instanceAttempted = false;
	int32_t createResult = 0;      ///< vkCreateInstance's VkResult, when attempted
	uint32_t deviceCount = 0;
	uint32_t bestApi = 0;          ///< the highest apiVersion any device reports
	char bestName[256] = {};       ///< that device's name
};

/// Does this start go through DXVK at all? Only then is Vulkan needed. The Windows
/// loader finds an app-local d3d8.dll before System32's, so the DXVK pair the packages ship
/// (package-windows-share.ps1: d3d8.dll + d3d9.dll beside generalszh.exe) is what decides. A classic build on
/// the system Direct3D 8 has neither file beside it, and a lone d3d8.dll (a d3d8-to-d3d9 shim on the
/// system d3d9) has no d3d9.dll: both skip the probe and keep the retained DX8 path working. The native
/// renderer is chosen later, from Options.ini, but every install that can choose it ships this pair, so the
/// probe covers it too.
inline bool usesDxvkPair(bool appLocalD3D8, bool appLocalD3D9)
{
	return appLocalD3D8 && appLocalD3D9;
}

inline Verdict judge(const Probe &p)
{
	if (!p.loaderLoaded)
		return VULKAN_NO_LOADER;
	if (!p.hasGetInstanceProcAddr)
		return VULKAN_NO_ENTRY;
	if (!p.instanceAttempted || p.createResult != 0)
		return VULKAN_NO_INSTANCE;
	if (p.deviceCount == 0)
		return VULKAN_NO_DEVICE;
	if (p.bestApi < kNeedApi)
		return VULKAN_DEVICE_TOO_OLD;
	return VULKAN_OK;
}

/// One stderr line, for the log a player sends us. Says what was found in either case.
inline void logLine(Verdict v, const Probe &p, char *out, size_t n)
{
	switch (v)
	{
	case VULKAN_OK:
		snprintf(out, n, "[GXVULKAN] ok: %s %u.%u.%u, %u device(s), best '%s' Vulkan %u.%u.%u", p.loaderName,
			apiMajor(p.instanceVersion), apiMinor(p.instanceVersion), apiPatch(p.instanceVersion), p.deviceCount,
			p.bestName, apiMajor(p.bestApi), apiMinor(p.bestApi), apiPatch(p.bestApi));
		return;
	case VULKAN_NO_LOADER:
		snprintf(out, n, "[GXVULKAN] REFUSED: no Vulkan loader (winevulkan.dll / vulkan-1.dll) loaded - no Vulkan driver is installed");
		return;
	case VULKAN_NO_ENTRY:
		snprintf(out, n, "[GXVULKAN] REFUSED: a Vulkan loader loaded, but none exports vkGetInstanceProcAddr");
		return;
	case VULKAN_NO_INSTANCE:
		snprintf(out, n, "[GXVULKAN] REFUSED: a Vulkan 1.3 instance could not be created (VkResult %d, loader %u.%u)",
			(int)p.createResult, apiMajor(p.instanceVersion), apiMinor(p.instanceVersion));
		return;
	case VULKAN_NO_DEVICE:
		snprintf(out, n, "[GXVULKAN] REFUSED: Vulkan is installed but reports no graphics device");
		return;
	case VULKAN_DEVICE_TOO_OLD:
		snprintf(out, n, "[GXVULKAN] REFUSED: the best device, '%s', reports Vulkan %u.%u; 1.3 is needed",
			p.bestName, apiMajor(p.bestApi), apiMinor(p.bestApi));
		return;
	}
	snprintf(out, n, "[GXVULKAN] REFUSED: verdict %d", (int)v);
}

/// The sentence the player reads in the box. Plain English, one cause, one thing to do.
inline void playerMessage(Verdict v, const Probe &p, char *out, size_t n)
{
	char detail[400];
	if (v == VULKAN_DEVICE_TOO_OLD)
		snprintf(detail, sizeof(detail), "Your graphics card (%s) offers Vulkan %u.%u; GeneralsX needs Vulkan 1.3.",
			p.bestName, apiMajor(p.bestApi), apiMinor(p.bestApi));
	else if (v == VULKAN_NO_DEVICE)
		snprintf(detail, sizeof(detail), "Vulkan is installed, but no graphics card offers it.");
	else
		snprintf(detail, sizeof(detail), "Your graphics driver does not support Vulkan, which GeneralsX needs on Windows.");
	snprintf(out, n,
		"%s\n\n"
		"Install the latest graphics driver from NVIDIA, AMD or Intel, then start GeneralsX again.\n\n"
		"Graphics cards and processor graphics older than about 2016 may not support Vulkan 1.3 at all; "
		"on those, GeneralsX cannot run.",
		detail);
}

// ---- the probe ---------------------------------------------------------------------------------

#if defined(_WIN32) && !defined(_WIN64)
#define GXVK_CALL __stdcall
#else
#define GXVK_CALL
#endif

typedef int32_t VkResultT;
typedef void *VkHandleT;  ///< VkInstance and VkPhysicalDevice: dispatchable handles are pointers
typedef void (GXVK_CALL *PfnVoid)(void);
typedef PfnVoid (GXVK_CALL *PfnGetInstanceProcAddr)(VkHandleT instance, const char *name);
typedef VkResultT (GXVK_CALL *PfnEnumerateInstanceVersion)(uint32_t *version);

struct ApplicationInfo  ///< VkApplicationInfo, field for field
{
	int32_t sType;  ///< VK_STRUCTURE_TYPE_APPLICATION_INFO = 0
	const void *pNext;
	const char *pApplicationName;
	uint32_t applicationVersion;
	const char *pEngineName;
	uint32_t engineVersion;
	uint32_t apiVersion;
};

struct InstanceCreateInfo  ///< VkInstanceCreateInfo, field for field
{
	int32_t sType;  ///< VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 1
	const void *pNext;
	uint32_t flags;
	const ApplicationInfo *pApplicationInfo;
	uint32_t enabledLayerCount;
	const char *const *ppEnabledLayerNames;
	uint32_t enabledExtensionCount;
	const char *const *ppEnabledExtensionNames;
};

typedef VkResultT (GXVK_CALL *PfnCreateInstance)(const InstanceCreateInfo *info, const void *allocator, VkHandleT *instance);
typedef void (GXVK_CALL *PfnDestroyInstance)(VkHandleT instance, const void *allocator);
typedef VkResultT (GXVK_CALL *PfnEnumeratePhysicalDevices)(VkHandleT instance, uint32_t *count, VkHandleT *devices);
typedef void (GXVK_CALL *PfnGetPhysicalDeviceProperties)(VkHandleT device, void *properties);

/// VkPhysicalDeviceProperties is ~824 bytes on x64; the probe reads only apiVersion (offset 0) and deviceName
/// (offset 20: apiVersion, driverVersion, vendorID, deviceID, deviceType). Written into a buffer far larger.
constexpr size_t kPropertiesBuffer = 4096;
constexpr size_t kDeviceNameOffset = 20;
constexpr size_t kDeviceNameSize = 256;  ///< VK_MAX_PHYSICAL_DEVICE_NAME_SIZE
constexpr uint32_t kMaxDevices = 16;

/// The loaders tried, in order: the renderer's own list (GXRender/Vk/VkCtx.cpp platformLoaderNames,
/// GxNativeHostWin32.cpp) and DXVK's (vulkan_loader.cpp) - winevulkan.dll first, so a Wine prefix that only
/// has winevulkan is accepted exactly as the renderer accepts it. A candidate that
/// loads without vkGetInstanceProcAddr is let go and the next one tried, as DXVK does.
constexpr const char *kLoaderNames[] = { "winevulkan.dll", "vulkan-1.dll" };

/// How the probe reaches a loader. Windows passes LoadLibraryA/GetProcAddress/FreeLibrary; the host test fakes.
struct Loader
{
	void *(*open)(const char *name);
	void *(*symbol)(void *library, const char *name);
	void (*close)(void *library);
};

/// Ask Vulkan what DXVK 2.6 will ask, and let go of everything before returning.
inline Probe probe(const Loader &loader)
{
	Probe p;
	void *library = nullptr;
	PfnGetInstanceProcAddr gipa = nullptr;
	for (const char *name : kLoaderNames)
	{
		void *candidate = loader.open(name);
		if (candidate == nullptr)
			continue;
		p.loaderLoaded = true;
		gipa = reinterpret_cast<PfnGetInstanceProcAddr>(loader.symbol(candidate, "vkGetInstanceProcAddr"));
		if (gipa == nullptr)
		{
			loader.close(candidate);
			continue;
		}
		library = candidate;
		p.loaderName = name;
		break;
	}
	if (library == nullptr)
		return p;
	p.hasGetInstanceProcAddr = true;

	// A 1.0 loader has no vkEnumerateInstanceVersion. Not refused on its own: the instance creation below
	// is the real question, and a 1.0 loader answers it with VK_ERROR_INCOMPATIBLE_DRIVER.
	PfnEnumerateInstanceVersion eiv = reinterpret_cast<PfnEnumerateInstanceVersion>(gipa(nullptr, "vkEnumerateInstanceVersion"));
	if (eiv != nullptr)
	{
		uint32_t version = 0;
		if (eiv(&version) == 0)
			p.instanceVersion = version;
	}

	PfnCreateInstance createInstance = reinterpret_cast<PfnCreateInstance>(gipa(nullptr, "vkCreateInstance"));
	if (createInstance == nullptr)
	{
		loader.close(library);
		return p;
	}
	ApplicationInfo app = { 0, nullptr, "GeneralsX", 1, "GeneralsX", 1, kNeedApi };
	InstanceCreateInfo info = { 1, nullptr, 0, &app, 0, nullptr, 0, nullptr };
	VkHandleT instance = nullptr;
	p.instanceAttempted = true;
	p.createResult = createInstance(&info, nullptr, &instance);
	if (p.createResult != 0 || instance == nullptr)
	{
		if (p.createResult == 0)
			p.createResult = -1;  // VK_SUCCESS with no handle: still no instance
		loader.close(library);
		return p;
	}

	PfnEnumeratePhysicalDevices enumerate = reinterpret_cast<PfnEnumeratePhysicalDevices>(gipa(instance, "vkEnumeratePhysicalDevices"));
	PfnGetPhysicalDeviceProperties properties = reinterpret_cast<PfnGetPhysicalDeviceProperties>(gipa(instance, "vkGetPhysicalDeviceProperties"));
	PfnDestroyInstance destroy = reinterpret_cast<PfnDestroyInstance>(gipa(instance, "vkDestroyInstance"));
	if (enumerate != nullptr && properties != nullptr)
	{
		uint32_t count = kMaxDevices;
		VkHandleT devices[kMaxDevices] = {};
		// VK_SUCCESS (0) or VK_INCOMPLETE (5, more devices than kMaxDevices): both filled `devices`.
		if (enumerate(instance, &count, devices) >= 0)
		{
			if (count > kMaxDevices)
				count = kMaxDevices;
			for (uint32_t i = 0; i < count; ++i)
			{
				if (devices[i] == nullptr)
					continue;
				alignas(16) unsigned char buffer[kPropertiesBuffer] = {};
				properties(devices[i], buffer);
				uint32_t api = 0;
				memcpy(&api, buffer, sizeof(api));
				++p.deviceCount;
				if (p.deviceCount == 1 || api > p.bestApi)
				{
					p.bestApi = api;
					memcpy(p.bestName, buffer + kDeviceNameOffset, kDeviceNameSize - 1);
					p.bestName[kDeviceNameSize - 1] = '\0';
				}
			}
		}
	}
	if (destroy != nullptr)
		destroy(instance, nullptr);
	loader.close(library);
	return p;
}

} // namespace gxvk
