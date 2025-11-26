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

// AdvancedGraphicsIntegration.cpp ////////////////////////////////////////////
// Phase 47 Week 3: Advanced Graphics - Integration Implementation (Generals)
//
#include "PreRTS.h"

#include "GameClient/AdvancedGraphicsIntegration.h"
#include "W3DDevice/GameClient/PostProcessingEffects.h"
#include "W3DDevice/GameClient/EnvironmentEffects.h"

// ============================================================================
// Global State
// ============================================================================

typedef enum
{
	GRAPHICS_QUALITY_LOW = 0,
	GRAPHICS_QUALITY_MEDIUM = 1,
	GRAPHICS_QUALITY_HIGH = 2,
	GRAPHICS_QUALITY_MAXIMUM = 3,
} GraphicsQualityLevel;

static GraphicsQualityLevel g_qualityLevel = GRAPHICS_QUALITY_HIGH;
static Bool g_initialized = FALSE;

// ============================================================================
// Implementation
// ============================================================================

Bool AdvancedGraphicsIntegration_Initialize(void)
{
	if (g_initialized)
	{
		DEBUG_LOG(("AdvancedGraphicsIntegration: Already initialized"));
		return TRUE;
	}
	
	// Initialize post-processing subsystem
	if (!PostProcessingEffects_Initialize())
	{
		DEBUG_ASSERTCRASH(FALSE, ("Failed to initialize PostProcessingEffects"));
		return FALSE;
	}
	
	// Initialize environment effects subsystem
	if (!EnvironmentEffects_Initialize())
	{
		DEBUG_ASSERTCRASH(FALSE, ("Failed to initialize EnvironmentEffects"));
		PostProcessingEffects_Shutdown();
		return FALSE;
	}
	
	// Apply quality level settings
	AdvancedGraphicsIntegration_SetQualityLevel(g_qualityLevel);
	
	g_initialized = TRUE;
	DEBUG_LOG(("AdvancedGraphicsIntegration: Initialized at quality level %d", g_qualityLevel));
	
	return TRUE;
}

Bool AdvancedGraphicsIntegration_Shutdown(void)
{
	if (!g_initialized)
	{
		DEBUG_LOG(("AdvancedGraphicsIntegration: Already shutdown"));
		return TRUE;
	}
	
	// Shutdown in reverse order
	EnvironmentEffects_Shutdown();
	PostProcessingEffects_Shutdown();
	
	g_initialized = FALSE;
	DEBUG_LOG(("AdvancedGraphicsIntegration: Shutdown"));
	
	return TRUE;
}

Bool AdvancedGraphicsIntegration_Update(float deltaTime)
{
	if (!g_initialized)
	{
		return FALSE;
	}
	
	// Update environment effects (lighting, fog calculations)
	EnvironmentEffects_Update(deltaTime);
	
	return TRUE;
}

Bool AdvancedGraphicsIntegration_ApplyEffects(void)
{
	if (!g_initialized)
	{
		return FALSE;
	}
	
	// Apply environment effects first (fog, lighting)
	EnvironmentEffects_Apply();
	
	// Apply post-processing effects (bloom, color grading, etc.)
	PostProcessingEffects_Apply();
	
	return TRUE;
}

Int AdvancedGraphicsIntegration_GetQualityLevel(void)
{
	return (Int)g_qualityLevel;
}

void AdvancedGraphicsIntegration_SetQualityLevel(Int quality)
{
	PostProcessingParameters* postParams;
	EnvironmentEffectParameters* envParams;
	
	if (quality < GRAPHICS_QUALITY_LOW || quality > GRAPHICS_QUALITY_MAXIMUM)
	{
		return;
	}
	
	g_qualityLevel = (GraphicsQualityLevel)quality;
	
	if (!g_initialized)
	{
		return;
	}
	
	postParams = PostProcessingEffects_GetParameters();
	envParams = EnvironmentEffects_GetParameters();
	
	if (!postParams || !envParams)
	{
		return;
	}
	
	// Apply quality presets
	switch (g_qualityLevel)
	{
		case GRAPHICS_QUALITY_LOW:
			// Low quality: minimal effects for performance
			postParams->bloomEnabled = FALSE;
			postParams->colorGradingEnabled = TRUE;
			postParams->filmGrainEnabled = FALSE;
			postParams->motionBlurEnabled = FALSE;
			postParams->fxaaEnabled = FALSE;
			
			envParams->fogEnabled = TRUE;
			envParams->dynamicLightingEnabled = FALSE;
			envParams->weatherEnabled = FALSE;
			break;
		
		case GRAPHICS_QUALITY_MEDIUM:
			// Medium quality: balanced effects
			postParams->bloomEnabled = TRUE;
			postParams->bloomIntensity = 0.6f;
			postParams->colorGradingEnabled = TRUE;
			postParams->filmGrainEnabled = FALSE;
			postParams->motionBlurEnabled = FALSE;
			postParams->fxaaEnabled = TRUE;
			postParams->fxaaSpanMax = 4.0f;
			
			envParams->fogEnabled = TRUE;
			envParams->dynamicLightingEnabled = TRUE;
			envParams->weatherEnabled = FALSE;
			break;
		
		case GRAPHICS_QUALITY_HIGH:
			// High quality: most effects enabled
			postParams->bloomEnabled = TRUE;
			postParams->bloomIntensity = 1.0f;
			postParams->colorGradingEnabled = TRUE;
			postParams->filmGrainEnabled = FALSE;
			postParams->motionBlurEnabled = TRUE;
			postParams->motionBlurAmount = 0.1f;
			postParams->fxaaEnabled = TRUE;
			postParams->fxaaSpanMax = 8.0f;
			
			envParams->fogEnabled = TRUE;
			envParams->dynamicLightingEnabled = TRUE;
			envParams->weatherEnabled = FALSE;
			break;
		
		case GRAPHICS_QUALITY_MAXIMUM:
			// Maximum quality: all effects at full strength
			postParams->bloomEnabled = TRUE;
			postParams->bloomIntensity = 1.2f;
			postParams->bloomBlurRadius = 6.0f;
			postParams->colorGradingEnabled = TRUE;
			postParams->filmGrainEnabled = TRUE;
			postParams->filmGrainIntensity = 0.05f;
			postParams->motionBlurEnabled = TRUE;
			postParams->motionBlurAmount = 0.15f;
			postParams->fxaaEnabled = TRUE;
			postParams->fxaaSpanMax = 12.0f;
			
			envParams->fogEnabled = TRUE;
			envParams->fogDensity = 0.0015f;
			envParams->dynamicLightingEnabled = TRUE;
			envParams->weatherEnabled = TRUE;
			envParams->weatherIntensity = 0.5f;
			break;
	}
	
	// Update parameters
	PostProcessingEffects_SetParameters(postParams);
	EnvironmentEffects_SetParameters(envParams);
	
	DEBUG_LOG(("AdvancedGraphicsIntegration: Quality level set to %d", g_qualityLevel));
}

const char* AdvancedGraphicsIntegration_GetDetailedStatus(void)
{
	static char statusStr[1024];
	const char* qualityNames[] = {"LOW", "MEDIUM", "HIGH", "MAXIMUM"};
	
	if (!g_initialized)
	{
		return "AdvancedGraphicsIntegration: NOT INITIALIZED";
	}
	
	snprintf(statusStr, sizeof(statusStr),
		"AdvancedGraphicsIntegration [Quality=%s]\n  PostProcessing: %s\n  EnvironmentEffects: %s",
		qualityNames[g_qualityLevel],
		PostProcessingEffects_GetStatusString(),
		EnvironmentEffects_GetStatusString());
	
	return statusStr;
}
