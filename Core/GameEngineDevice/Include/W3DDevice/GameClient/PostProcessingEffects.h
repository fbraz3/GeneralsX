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

#ifndef POSTPROCESSINGEFFECTS_H
#define POSTPROCESSINGEFFECTS_H

#pragma once

// ============================================================================
// PostProcessingEffects
// ============================================================================
// Phase 47 Week 3: Advanced Graphics - Post-Processing Effects
// Implements bloom, color grading, film grain, FXAA for enhanced visuals
//
// Architecture:
// - Each effect is independent and can be toggled on/off
// - Effects are applied as render passes in sequence
// - Integrates with Vulkan graphics backend for efficient GPU rendering
// - Supports parameter adjustment for runtime customization
//

typedef struct
{
	// Bloom parameters
	float bloomThreshold;
	float bloomIntensity;
	float bloomBlurRadius;
	bool bloomEnabled;
	
	// Color grading parameters
	float colorSaturation;
	float colorBrightness;
	float colorContrast;
	bool colorGradingEnabled;
	
	// Film grain parameters
	float filmGrainIntensity;
	bool filmGrainEnabled;
	
	// Motion blur parameters
	float motionBlurAmount;
	bool motionBlurEnabled;
	
	// FXAA parameters
	float fxaaSpanMax;
	bool fxaaEnabled;
	
} PostProcessingParameters;

// ============================================================================
// PostProcessingEffects API
// ============================================================================

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize post-processing system
 * Must be called during graphics initialization
 */
Bool PostProcessingEffects_Initialize(void);

/**
 * Shutdown post-processing system
 * Must be called during graphics shutdown
 */
Bool PostProcessingEffects_Shutdown(void);

/**
 * Apply all enabled post-processing effects to rendered frame
 * Called after game rendering, before present
 */
Bool PostProcessingEffects_Apply(void);

/**
 * Get current post-processing parameters
 */
PostProcessingParameters* PostProcessingEffects_GetParameters(void);

/**
 * Set post-processing parameters
 */
Bool PostProcessingEffects_SetParameters(const PostProcessingParameters* params);

/**
 * Enable/disable individual effects
 */
void PostProcessingEffects_SetBloomEnabled(Bool enabled);
void PostProcessingEffects_SetColorGradingEnabled(Bool enabled);
void PostProcessingEffects_SetFilmGrainEnabled(Bool enabled);
void PostProcessingEffects_SetMotionBlurEnabled(Bool enabled);
void PostProcessingEffects_SetFXAAEnabled(Bool enabled);

/**
 * Get effect enabled status
 */
Bool PostProcessingEffects_IsBloomEnabled(void);
Bool PostProcessingEffects_IsColorGradingEnabled(void);
Bool PostProcessingEffects_IsFilmGrainEnabled(void);
Bool PostProcessingEffects_IsMotionBlurEnabled(void);
Bool PostProcessingEffects_IsFXAAEnabled(void);

/**
 * Adjust individual effect parameters
 */
void PostProcessingEffects_SetBloomThreshold(float threshold);
void PostProcessingEffects_SetBloomIntensity(float intensity);
void PostProcessingEffects_SetColorSaturation(float saturation);
void PostProcessingEffects_SetColorBrightness(float brightness);
void PostProcessingEffects_SetColorContrast(float contrast);
void PostProcessingEffects_SetFilmGrainIntensity(float intensity);
void PostProcessingEffects_SetMotionBlurAmount(float amount);
void PostProcessingEffects_SetFXAASpanMax(float spanMax);

/**
 * Query effect status
 */
const char* PostProcessingEffects_GetStatusString(void);

#ifdef __cplusplus
}
#endif

#endif  // POSTPROCESSINGEFFECTS_H
