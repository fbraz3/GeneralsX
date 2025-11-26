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

#ifndef ENVIRONMENTEFFECTS_H
#define ENVIRONMENTEFFECTS_H

#pragma once

#include "Common/GameTypes.h"
#include "Common/Coord.h"

// ============================================================================
// EnvironmentEffects
// ============================================================================
// Phase 47 Week 3: Advanced Graphics - Environment Effects
// Implements fog, dynamic lighting, weather effects for immersive gameplay
//
// Architecture:
// - Fog provides depth cuing and scene depth perception
// - Dynamic lighting affects unit visibility and gameplay atmosphere
// - Weather effects (if applicable) for environmental ambiance
// - All effects integrate with Vulkan shader system for GPU execution
//

typedef struct
{
	// Fog parameters
	float fogDensity;
	float fogStart;
	float fogEnd;
	float fogRed;
	float fogGreen;
	float fogBlue;
	bool fogEnabled;
	
	// Dynamic lighting parameters
	Int numActiveLights;
	float ambientIntensity;
	bool dynamicLightingEnabled;
	
	// Weather parameters
	bool weatherEnabled;
	float weatherIntensity;
	
} EnvironmentEffectParameters;

// ============================================================================
// EnvironmentEffects API
// ============================================================================

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize environment effects system
 * Must be called during graphics initialization
 */
Bool EnvironmentEffects_Initialize(void);

/**
 * Shutdown environment effects system
 * Must be called during graphics shutdown
 */
Bool EnvironmentEffects_Shutdown(void);

/**
 * Update environment effects (fog, lighting calculations)
 * Called once per frame during render phase
 */
Bool EnvironmentEffects_Update(float deltaTime);

/**
 * Apply environment effects to current render pass
 * Called during rendering after scene setup
 */
Bool EnvironmentEffects_Apply(void);

/**
 * Get current environment parameters
 */
EnvironmentEffectParameters* EnvironmentEffects_GetParameters(void);

/**
 * Set environment parameters
 */
Bool EnvironmentEffects_SetParameters(const EnvironmentEffectParameters* params);

/**
 * Fog control
 */
void EnvironmentEffects_SetFogEnabled(Bool enabled);
void EnvironmentEffects_SetFogDensity(float density);
void EnvironmentEffects_SetFogColor(float red, float green, float blue);
void EnvironmentEffects_SetFogDistance(float startDist, float endDist);

/**
 * Dynamic lighting control
 */
void EnvironmentEffects_SetDynamicLightingEnabled(Bool enabled);
void EnvironmentEffects_SetAmbientIntensity(float intensity);
void EnvironmentEffects_AddDynamicLight(const Coord3D* position, float radius, 
										  float red, float green, float blue, float intensity);
void EnvironmentEffects_RemoveDynamicLight(Int lightIndex);
void EnvironmentEffects_ClearDynamicLights(void);

/**
 * Weather control
 */
void EnvironmentEffects_SetWeatherEnabled(Bool enabled);
void EnvironmentEffects_SetWeatherIntensity(float intensity);

/**
 * Query effect status
 */
Bool EnvironmentEffects_IsFogEnabled(void);
Bool EnvironmentEffects_IsDynamicLightingEnabled(void);
Bool EnvironmentEffects_IsWeatherEnabled(void);
Int EnvironmentEffects_GetActiveLightCount(void);
const char* EnvironmentEffects_GetStatusString(void);

#ifdef __cplusplus
}
#endif

#endif  // ENVIRONMENTEFFECTS_H
