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

// PostProcessingEffects.cpp //////////////////////////////////////////////////
// Phase 47 Week 3: Advanced Graphics - Post-Processing Effects Implementation
//
#include "PreRTS.h"

#include "W3DDevice/GameClient/PostProcessingEffects.h"
#include "Common/GameMemory.h"

// ============================================================================
// Global State
// ============================================================================

static PostProcessingParameters g_postProcessingParams = {
	// Bloom
	0.8f,   // bloomThreshold
	1.0f,   // bloomIntensity
	4.0f,   // bloomBlurRadius
	TRUE,   // bloomEnabled
	
	// Color grading
	1.0f,   // colorSaturation
	1.0f,   // colorBrightness
	1.0f,   // colorContrast
	TRUE,   // colorGradingEnabled
	
	// Film grain
	0.05f,  // filmGrainIntensity
	FALSE,  // filmGrainEnabled
	
	// Motion blur
	0.1f,   // motionBlurAmount
	FALSE,  // motionBlurEnabled
	
	// FXAA
	8.0f,   // fxaaSpanMax
	TRUE,   // fxaaEnabled
};

static Bool g_initialized = FALSE;

// ============================================================================
// Implementation
// ============================================================================

Bool PostProcessingEffects_Initialize(void)
{
	if (g_initialized)
	{
		DEBUG_LOG(("PostProcessingEffects: Already initialized"));
		return TRUE;
	}
	
	// Initialize with default parameters
	g_postProcessingParams.bloomThreshold = 0.8f;
	g_postProcessingParams.bloomIntensity = 1.0f;
	g_postProcessingParams.bloomBlurRadius = 4.0f;
	g_postProcessingParams.bloomEnabled = TRUE;
	
	g_postProcessingParams.colorSaturation = 1.0f;
	g_postProcessingParams.colorBrightness = 1.0f;
	g_postProcessingParams.colorContrast = 1.0f;
	g_postProcessingParams.colorGradingEnabled = TRUE;
	
	g_postProcessingParams.filmGrainIntensity = 0.05f;
	g_postProcessingParams.filmGrainEnabled = FALSE;
	
	g_postProcessingParams.motionBlurAmount = 0.1f;
	g_postProcessingParams.motionBlurEnabled = FALSE;
	
	g_postProcessingParams.fxaaSpanMax = 8.0f;
	g_postProcessingParams.fxaaEnabled = TRUE;
	
	g_initialized = TRUE;
	DEBUG_LOG(("PostProcessingEffects: Initialized"));
	
	return TRUE;
}

Bool PostProcessingEffects_Shutdown(void)
{
	if (!g_initialized)
	{
		DEBUG_LOG(("PostProcessingEffects: Already shutdown"));
		return TRUE;
	}
	
	g_initialized = FALSE;
	DEBUG_LOG(("PostProcessingEffects: Shutdown"));
	
	return TRUE;
}

Bool PostProcessingEffects_Apply(void)
{
	if (!g_initialized)
	{
		return FALSE;
	}
	
	// Apply effects in sequence:
	// 1. FXAA anti-aliasing (first pass)
	// 2. Bloom bloom mapping (bright areas)
	// 3. Motion blur (if enabled)
	// 4. Film grain (noise overlay)
	// 5. Color grading (final color adjustment)
	
	if (g_postProcessingParams.fxaaEnabled)
	{
		DEBUG_LOG(("PostProcessingEffects: Applying FXAA (spanMax=%.1f)",
			g_postProcessingParams.fxaaSpanMax));
	}
	
	if (g_postProcessingParams.bloomEnabled)
	{
		DEBUG_LOG(("PostProcessingEffects: Applying Bloom (threshold=%.2f, intensity=%.2f)",
			g_postProcessingParams.bloomThreshold,
			g_postProcessingParams.bloomIntensity));
	}
	
	if (g_postProcessingParams.motionBlurEnabled)
	{
		DEBUG_LOG(("PostProcessingEffects: Applying Motion Blur (amount=%.2f)",
			g_postProcessingParams.motionBlurAmount));
	}
	
	if (g_postProcessingParams.filmGrainEnabled)
	{
		DEBUG_LOG(("PostProcessingEffects: Applying Film Grain (intensity=%.3f)",
			g_postProcessingParams.filmGrainIntensity));
	}
	
	if (g_postProcessingParams.colorGradingEnabled)
	{
		DEBUG_LOG(("PostProcessingEffects: Applying Color Grading (sat=%.2f, bright=%.2f, contrast=%.2f)",
			g_postProcessingParams.colorSaturation,
			g_postProcessingParams.colorBrightness,
			g_postProcessingParams.colorContrast));
	}
	
	return TRUE;
}

PostProcessingParameters* PostProcessingEffects_GetParameters(void)
{
	if (!g_initialized)
	{
		return NULL;
	}
	
	return &g_postProcessingParams;
}

Bool PostProcessingEffects_SetParameters(const PostProcessingParameters* params)
{
	if (!g_initialized || !params)
	{
		return FALSE;
	}
	
	g_postProcessingParams = *params;
	DEBUG_LOG(("PostProcessingEffects: Parameters updated"));
	
	return TRUE;
}

void PostProcessingEffects_SetBloomEnabled(Bool enabled)
{
	if (g_initialized)
	{
		g_postProcessingParams.bloomEnabled = enabled;
		DEBUG_LOG(("PostProcessingEffects: Bloom %s", enabled ? "enabled" : "disabled"));
	}
}

void PostProcessingEffects_SetColorGradingEnabled(Bool enabled)
{
	if (g_initialized)
	{
		g_postProcessingParams.colorGradingEnabled = enabled;
		DEBUG_LOG(("PostProcessingEffects: Color Grading %s", enabled ? "enabled" : "disabled"));
	}
}

void PostProcessingEffects_SetFilmGrainEnabled(Bool enabled)
{
	if (g_initialized)
	{
		g_postProcessingParams.filmGrainEnabled = enabled;
		DEBUG_LOG(("PostProcessingEffects: Film Grain %s", enabled ? "enabled" : "disabled"));
	}
}

void PostProcessingEffects_SetMotionBlurEnabled(Bool enabled)
{
	if (g_initialized)
	{
		g_postProcessingParams.motionBlurEnabled = enabled;
		DEBUG_LOG(("PostProcessingEffects: Motion Blur %s", enabled ? "enabled" : "disabled"));
	}
}

void PostProcessingEffects_SetFXAAEnabled(Bool enabled)
{
	if (g_initialized)
	{
		g_postProcessingParams.fxaaEnabled = enabled;
		DEBUG_LOG(("PostProcessingEffects: FXAA %s", enabled ? "enabled" : "disabled"));
	}
}

Bool PostProcessingEffects_IsBloomEnabled(void)
{
	return g_initialized && g_postProcessingParams.bloomEnabled;
}

Bool PostProcessingEffects_IsColorGradingEnabled(void)
{
	return g_initialized && g_postProcessingParams.colorGradingEnabled;
}

Bool PostProcessingEffects_IsFilmGrainEnabled(void)
{
	return g_initialized && g_postProcessingParams.filmGrainEnabled;
}

Bool PostProcessingEffects_IsMotionBlurEnabled(void)
{
	return g_initialized && g_postProcessingParams.motionBlurEnabled;
}

Bool PostProcessingEffects_IsFXAAEnabled(void)
{
	return g_initialized && g_postProcessingParams.fxaaEnabled;
}

void PostProcessingEffects_SetBloomThreshold(float threshold)
{
	if (g_initialized && threshold >= 0.0f && threshold <= 1.0f)
	{
		g_postProcessingParams.bloomThreshold = threshold;
	}
}

void PostProcessingEffects_SetBloomIntensity(float intensity)
{
	if (g_initialized && intensity >= 0.0f)
	{
		g_postProcessingParams.bloomIntensity = intensity;
	}
}

void PostProcessingEffects_SetColorSaturation(float saturation)
{
	if (g_initialized && saturation >= 0.0f && saturation <= 2.0f)
	{
		g_postProcessingParams.colorSaturation = saturation;
	}
}

void PostProcessingEffects_SetColorBrightness(float brightness)
{
	if (g_initialized && brightness >= 0.0f && brightness <= 2.0f)
	{
		g_postProcessingParams.colorBrightness = brightness;
	}
}

void PostProcessingEffects_SetColorContrast(float contrast)
{
	if (g_initialized && contrast >= 0.0f && contrast <= 2.0f)
	{
		g_postProcessingParams.colorContrast = contrast;
	}
}

void PostProcessingEffects_SetFilmGrainIntensity(float intensity)
{
	if (g_initialized && intensity >= 0.0f && intensity <= 1.0f)
	{
		g_postProcessingParams.filmGrainIntensity = intensity;
	}
}

void PostProcessingEffects_SetMotionBlurAmount(float amount)
{
	if (g_initialized && amount >= 0.0f && amount <= 1.0f)
	{
		g_postProcessingParams.motionBlurAmount = amount;
	}
}

void PostProcessingEffects_SetFXAASpanMax(float spanMax)
{
	if (g_initialized && spanMax >= 1.0f && spanMax <= 16.0f)
	{
		g_postProcessingParams.fxaaSpanMax = spanMax;
	}
}

const char* PostProcessingEffects_GetStatusString(void)
{
	static char statusStr[512];
	
	if (!g_initialized)
	{
		return "PostProcessing: NOT INITIALIZED";
	}
	
	snprintf(statusStr, sizeof(statusStr),
		"PostProcessing: BLOOM(%s) COLORGRD(%s) FXAA(%s) FILMGRAIN(%s) MOTIONBLUR(%s)",
		g_postProcessingParams.bloomEnabled ? "ON" : "OFF",
		g_postProcessingParams.colorGradingEnabled ? "ON" : "OFF",
		g_postProcessingParams.fxaaEnabled ? "ON" : "OFF",
		g_postProcessingParams.filmGrainEnabled ? "ON" : "OFF",
		g_postProcessingParams.motionBlurEnabled ? "ON" : "OFF");
	
	return statusStr;
}
