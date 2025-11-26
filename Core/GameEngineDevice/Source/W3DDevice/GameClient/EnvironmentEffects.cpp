/*
**  Command & Conquer Generals Zero Hour(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  (c) 2025 Electronic Arts Inc.                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// EnvironmentEffects.cpp /////////////////////////////////////////////////////
// Phase 47 Week 3: Advanced Graphics - Environment Effects Implementation
//
#include "PreRTS.h"

#include "W3DDevice/GameClient/EnvironmentEffects.h"
#include "Common/GameMemory.h"
#include <string.h>

// ============================================================================
// Constants
// ============================================================================

#define MAX_DYNAMIC_LIGHTS 32

typedef struct
{
	Coord3D position;
	float radius;
	float red;
	float green;
	float blue;
	float intensity;
	bool active;
	
} DynamicLight;

// ============================================================================
// Global State
// ============================================================================

static EnvironmentEffectParameters g_environmentParams = {
	// Fog
	0.001f,  // fogDensity
	100.0f,  // fogStart
	1000.0f, // fogEnd
	0.8f,    // fogRed
	0.8f,    // fogGreen
	0.8f,    // fogBlue
	TRUE,    // fogEnabled
	
	// Dynamic lighting
	0,       // numActiveLights
	1.0f,    // ambientIntensity
	TRUE,    // dynamicLightingEnabled
	
	// Weather
	FALSE,   // weatherEnabled
	0.0f,    // weatherIntensity
};

static DynamicLight g_dynamicLights[MAX_DYNAMIC_LIGHTS];
static Bool g_initialized = FALSE;

// ============================================================================
// Implementation
// ============================================================================

Bool EnvironmentEffects_Initialize(void)
{
	Int i;
	
	if (g_initialized)
	{
		DEBUG_LOG(("EnvironmentEffects: Already initialized"));
		return TRUE;
	}
	
	// Initialize fog parameters
	g_environmentParams.fogDensity = 0.001f;
	g_environmentParams.fogStart = 100.0f;
	g_environmentParams.fogEnd = 1000.0f;
	g_environmentParams.fogRed = 0.8f;
	g_environmentParams.fogGreen = 0.8f;
	g_environmentParams.fogBlue = 0.8f;
	g_environmentParams.fogEnabled = TRUE;
	
	// Initialize lighting parameters
	g_environmentParams.numActiveLights = 0;
	g_environmentParams.ambientIntensity = 1.0f;
	g_environmentParams.dynamicLightingEnabled = TRUE;
	
	// Initialize weather parameters
	g_environmentParams.weatherEnabled = FALSE;
	g_environmentParams.weatherIntensity = 0.0f;
	
	// Clear dynamic lights array
	for (i = 0; i < MAX_DYNAMIC_LIGHTS; ++i)
	{
		g_dynamicLights[i].position.x = 0.0f;
		g_dynamicLights[i].position.y = 0.0f;
		g_dynamicLights[i].position.z = 0.0f;
		g_dynamicLights[i].radius = 0.0f;
		g_dynamicLights[i].red = 1.0f;
		g_dynamicLights[i].green = 1.0f;
		g_dynamicLights[i].blue = 1.0f;
		g_dynamicLights[i].intensity = 0.0f;
		g_dynamicLights[i].active = FALSE;
	}
	
	g_initialized = TRUE;
	DEBUG_LOG(("EnvironmentEffects: Initialized"));
	
	return TRUE;
}

Bool EnvironmentEffects_Shutdown(void)
{
	if (!g_initialized)
	{
		DEBUG_LOG(("EnvironmentEffects: Already shutdown"));
		return TRUE;
	}
	
	g_initialized = FALSE;
	DEBUG_LOG(("EnvironmentEffects: Shutdown"));
	
	return TRUE;
}

Bool EnvironmentEffects_Update(float deltaTime)
{
	Int activeLightCount = 0;
	Int i;
	
	if (!g_initialized)
	{
		return FALSE;
	}
	
	// Count active lights
	for (i = 0; i < MAX_DYNAMIC_LIGHTS; ++i)
	{
		if (g_dynamicLights[i].active)
		{
			++activeLightCount;
		}
	}
	
	g_environmentParams.numActiveLights = activeLightCount;
	
	return TRUE;
}

Bool EnvironmentEffects_Apply(void)
{
	if (!g_initialized)
	{
		return FALSE;
	}
	
	// Apply fog
	if (g_environmentParams.fogEnabled)
	{
		DEBUG_LOG(("EnvironmentEffects: Applying Fog (density=%.4f, start=%.1f, end=%.1f, color=(%.2f,%.2f,%.2f))",
			g_environmentParams.fogDensity,
			g_environmentParams.fogStart,
			g_environmentParams.fogEnd,
			g_environmentParams.fogRed,
			g_environmentParams.fogGreen,
			g_environmentParams.fogBlue));
	}
	
	// Apply dynamic lighting
	if (g_environmentParams.dynamicLightingEnabled)
	{
		DEBUG_LOG(("EnvironmentEffects: Applying Dynamic Lighting (%d lights, ambient=%.2f)",
			g_environmentParams.numActiveLights,
			g_environmentParams.ambientIntensity));
	}
	
	// Apply weather effects
	if (g_environmentParams.weatherEnabled && g_environmentParams.weatherIntensity > 0.0f)
	{
		DEBUG_LOG(("EnvironmentEffects: Applying Weather (intensity=%.2f)",
			g_environmentParams.weatherIntensity));
	}
	
	return TRUE;
}

EnvironmentEffectParameters* EnvironmentEffects_GetParameters(void)
{
	if (!g_initialized)
	{
		return NULL;
	}
	
	return &g_environmentParams;
}

Bool EnvironmentEffects_SetParameters(const EnvironmentEffectParameters* params)
{
	if (!g_initialized || !params)
	{
		return FALSE;
	}
	
	g_environmentParams = *params;
	DEBUG_LOG(("EnvironmentEffects: Parameters updated"));
	
	return TRUE;
}

void EnvironmentEffects_SetFogEnabled(Bool enabled)
{
	if (g_initialized)
	{
		g_environmentParams.fogEnabled = enabled;
		DEBUG_LOG(("EnvironmentEffects: Fog %s", enabled ? "enabled" : "disabled"));
	}
}

void EnvironmentEffects_SetFogDensity(float density)
{
	if (g_initialized && density >= 0.0f && density <= 0.01f)
	{
		g_environmentParams.fogDensity = density;
	}
}

void EnvironmentEffects_SetFogColor(float red, float green, float blue)
{
	if (g_initialized)
	{
		g_environmentParams.fogRed = red;
		g_environmentParams.fogGreen = green;
		g_environmentParams.fogBlue = blue;
		DEBUG_LOG(("EnvironmentEffects: Fog color set to (%.2f, %.2f, %.2f)",
			red, green, blue));
	}
}

void EnvironmentEffects_SetFogDistance(float startDist, float endDist)
{
	if (g_initialized && startDist >= 0.0f && endDist > startDist)
	{
		g_environmentParams.fogStart = startDist;
		g_environmentParams.fogEnd = endDist;
	}
}

void EnvironmentEffects_SetDynamicLightingEnabled(Bool enabled)
{
	if (g_initialized)
	{
		g_environmentParams.dynamicLightingEnabled = enabled;
		DEBUG_LOG(("EnvironmentEffects: Dynamic Lighting %s", enabled ? "enabled" : "disabled"));
	}
}

void EnvironmentEffects_SetAmbientIntensity(float intensity)
{
	if (g_initialized && intensity >= 0.0f && intensity <= 2.0f)
	{
		g_environmentParams.ambientIntensity = intensity;
	}
}

void EnvironmentEffects_AddDynamicLight(const Coord3D* position, float radius,
										  float red, float green, float blue, float intensity)
{
	Int i;
	
	if (!g_initialized || !position)
	{
		return;
	}
	
	// Find first available light slot
	for (i = 0; i < MAX_DYNAMIC_LIGHTS; ++i)
	{
		if (!g_dynamicLights[i].active)
		{
			g_dynamicLights[i].position = *position;
			g_dynamicLights[i].radius = radius;
			g_dynamicLights[i].red = red;
			g_dynamicLights[i].green = green;
			g_dynamicLights[i].blue = blue;
			g_dynamicLights[i].intensity = intensity;
			g_dynamicLights[i].active = TRUE;
			
			DEBUG_LOG(("EnvironmentEffects: Added dynamic light %d at (%.1f, %.1f, %.1f)",
				i, position->x, position->y, position->z));
			
			return;
		}
	}
	
	DEBUG_LOG(("EnvironmentEffects: Dynamic light pool full, cannot add new light"));
}

void EnvironmentEffects_RemoveDynamicLight(Int lightIndex)
{
	if (!g_initialized || lightIndex < 0 || lightIndex >= MAX_DYNAMIC_LIGHTS)
	{
		return;
	}
	
	if (g_dynamicLights[lightIndex].active)
	{
		g_dynamicLights[lightIndex].active = FALSE;
		DEBUG_LOG(("EnvironmentEffects: Removed dynamic light %d", lightIndex));
	}
}

void EnvironmentEffects_ClearDynamicLights(void)
{
	Int i;
	
	if (!g_initialized)
	{
		return;
	}
	
	for (i = 0; i < MAX_DYNAMIC_LIGHTS; ++i)
	{
		g_dynamicLights[i].active = FALSE;
	}
	
	g_environmentParams.numActiveLights = 0;
	DEBUG_LOG(("EnvironmentEffects: Cleared all dynamic lights"));
}

void EnvironmentEffects_SetWeatherEnabled(Bool enabled)
{
	if (g_initialized)
	{
		g_environmentParams.weatherEnabled = enabled;
		DEBUG_LOG(("EnvironmentEffects: Weather %s", enabled ? "enabled" : "disabled"));
	}
}

void EnvironmentEffects_SetWeatherIntensity(float intensity)
{
	if (g_initialized && intensity >= 0.0f && intensity <= 1.0f)
	{
		g_environmentParams.weatherIntensity = intensity;
	}
}

Bool EnvironmentEffects_IsFogEnabled(void)
{
	return g_initialized && g_environmentParams.fogEnabled;
}

Bool EnvironmentEffects_IsDynamicLightingEnabled(void)
{
	return g_initialized && g_environmentParams.dynamicLightingEnabled;
}

Bool EnvironmentEffects_IsWeatherEnabled(void)
{
	return g_initialized && g_environmentParams.weatherEnabled;
}

Int EnvironmentEffects_GetActiveLightCount(void)
{
	if (!g_initialized)
	{
		return 0;
	}
	
	return g_environmentParams.numActiveLights;
}

const char* EnvironmentEffects_GetStatusString(void)
{
	static char statusStr[512];
	
	if (!g_initialized)
	{
		return "EnvironmentEffects: NOT INITIALIZED";
	}
	
	snprintf(statusStr, sizeof(statusStr),
		"EnvironmentEffects: FOG(%s) DYNLIGHT(%s) WEATHER(%s) LIGHTS=%d",
		g_environmentParams.fogEnabled ? "ON" : "OFF",
		g_environmentParams.dynamicLightingEnabled ? "ON" : "OFF",
		g_environmentParams.weatherEnabled ? "ON" : "OFF",
		g_environmentParams.numActiveLights);
	
	return statusStr;
}
