#include "GameClient/ReplayIntegration.h"
#include "GameClient/ReplayRecorder.h"
#include "GameClient/ReplayPlayer.h"
#include <cstring>

/**
 * Global state for replay integration
 */
static Bool g_replayIntegration_initialized = FALSE;

/**
 * ReplayIntegration_Initialize
 */
Bool ReplayIntegration_Initialize(void)
{
	if (g_replayIntegration_initialized)
	{
		DEBUG_LOG(("ReplayIntegration already initialized\n"));
		return TRUE;
	}

	// Initialize recorder subsystem
	if (!ReplayRecorder_Initialize())
	{
		DEBUG_LOG(("ERROR: ReplayIntegration_Initialize - ReplayRecorder_Initialize failed\n"));
		return FALSE;
	}

	// Initialize player subsystem
	if (!ReplayPlayer_Initialize())
	{
		DEBUG_LOG(("ERROR: ReplayIntegration_Initialize - ReplayPlayer_Initialize failed\n"));
		ReplayRecorder_Shutdown();
		return FALSE;
	}

	g_replayIntegration_initialized = TRUE;

	DEBUG_LOG(("ReplayIntegration::Initialize - Replay system initialized (Generals)\n"));
	return TRUE;
}

/**
 * ReplayIntegration_Shutdown
 */
Bool ReplayIntegration_Shutdown(void)
{
	if (!g_replayIntegration_initialized)
	{
		return TRUE;
	}

	// Stop any active recording
	if (ReplayIntegration_IsRecording())
	{
		ReplayIntegration_StopRecording();
	}

	// Stop any active playback
	if (ReplayIntegration_IsPlayingBack())
	{
		ReplayIntegration_StopPlayback();
	}

	// Shutdown subsystems in reverse order
	if (!ReplayPlayer_Shutdown())
	{
		DEBUG_LOG(("WARNING: ReplayIntegration_Shutdown - ReplayPlayer_Shutdown failed\n"));
	}

	if (!ReplayRecorder_Shutdown())
	{
		DEBUG_LOG(("WARNING: ReplayIntegration_Shutdown - ReplayRecorder_Shutdown failed\n"));
	}

	g_replayIntegration_initialized = FALSE;

	DEBUG_LOG(("ReplayIntegration::Shutdown - Replay system shutdown\n"));
	return TRUE;
}

/**
 * ReplayIntegration_Update
 */
Bool ReplayIntegration_Update(float deltaTime)
{
	if (!g_replayIntegration_initialized)
	{
		DEBUG_LOG(("ERROR: ReplayIntegration_Update - Replay system not initialized\n"));
		return FALSE;
	}

	// Update playback state (frame advancement, etc)
	// Recording updates are handled automatically by TheRecorder in game loop
	// This is where playback timeline would be updated per frame

	return TRUE;
}

/**
 * ReplayIntegration_StartRecording
 */
Bool ReplayIntegration_StartRecording(Int difficulty, Int gameMode, Int rankPoints)
{
	if (!g_replayIntegration_initialized)
	{
		DEBUG_LOG(("ERROR: ReplayIntegration_StartRecording - Replay system not initialized\n"));
		return FALSE;
	}

	// Validate parameters
	if (difficulty < 0 || difficulty > 3)
	{
		DEBUG_LOG(("WARNING: ReplayIntegration_StartRecording - Invalid difficulty %d\n", difficulty));
		return FALSE;
	}

	// Cannot record while playing back
	if (ReplayIntegration_IsPlayingBack())
	{
		DEBUG_LOG(("ERROR: ReplayIntegration_StartRecording - Cannot record while playing back\n"));
		return FALSE;
	}

	// Stop any existing recording
	if (ReplayIntegration_IsRecording())
	{
		ReplayIntegration_StopRecording();
	}

	// maxFPS = 0 means no FPS limit for replay
	if (!ReplayRecorder_StartRecording(difficulty, gameMode, rankPoints, 0))
	{
		DEBUG_LOG(("ERROR: ReplayIntegration_StartRecording - Failed to start recording\n"));
		return FALSE;
	}

	DEBUG_LOG(("ReplayIntegration::StartRecording - Recording started: Difficulty=%d GameMode=%d\n",
		difficulty, gameMode));

	return TRUE;
}

/**
 * ReplayIntegration_StopRecording
 */
Bool ReplayIntegration_StopRecording(void)
{
	if (!g_replayIntegration_initialized)
	{
		DEBUG_LOG(("ERROR: ReplayIntegration_StopRecording - Replay system not initialized\n"));
		return FALSE;
	}

	if (!ReplayIntegration_IsRecording())
	{
		DEBUG_LOG(("WARNING: ReplayIntegration_StopRecording - Not currently recording\n"));
		return FALSE;
	}

	if (!ReplayRecorder_StopRecording())
	{
		DEBUG_LOG(("ERROR: ReplayIntegration_StopRecording - Failed to stop recording\n"));
		return FALSE;
	}

	DEBUG_LOG(("ReplayIntegration::StopRecording - Recording stopped\n"));
	return TRUE;
}

/**
 * ReplayIntegration_StartPlayback
 */
Bool ReplayIntegration_StartPlayback(const char* replayFilename)
{
	if (!g_replayIntegration_initialized)
	{
		DEBUG_LOG(("ERROR: ReplayIntegration_StartPlayback - Replay system not initialized\n"));
		return FALSE;
	}

	if (replayFilename == NULL)
	{
		DEBUG_LOG(("ERROR: ReplayIntegration_StartPlayback - replayFilename is NULL\n"));
		return FALSE;
	}

	// Cannot playback while recording
	if (ReplayIntegration_IsRecording())
	{
		DEBUG_LOG(("ERROR: ReplayIntegration_StartPlayback - Cannot playback while recording\n"));
		return FALSE;
	}

	// Stop any existing playback
	if (ReplayIntegration_IsPlayingBack())
	{
		ReplayIntegration_StopPlayback();
	}

	if (!ReplayPlayer_StartPlayback(replayFilename))
	{
		DEBUG_LOG(("ERROR: ReplayIntegration_StartPlayback - Failed to start playback: %s\n",
			replayFilename));
		return FALSE;
	}

	DEBUG_LOG(("ReplayIntegration::StartPlayback - Playback started: %s\n", replayFilename));
	return TRUE;
}

/**
 * ReplayIntegration_StopPlayback
 */
Bool ReplayIntegration_StopPlayback(void)
{
	if (!g_replayIntegration_initialized)
	{
		DEBUG_LOG(("ERROR: ReplayIntegration_StopPlayback - Replay system not initialized\n"));
		return FALSE;
	}

	if (!ReplayIntegration_IsPlayingBack())
	{
		DEBUG_LOG(("WARNING: ReplayIntegration_StopPlayback - Not currently playing back\n"));
		return FALSE;
	}

	if (!ReplayPlayer_StopPlayback())
	{
		DEBUG_LOG(("ERROR: ReplayIntegration_StopPlayback - Failed to stop playback\n"));
		return FALSE;
	}

	DEBUG_LOG(("ReplayIntegration::StopPlayback - Playback stopped\n"));
	return TRUE;
}

/**
 * ReplayIntegration_IsRecording
 */
Bool ReplayIntegration_IsRecording(void)
{
	if (!g_replayIntegration_initialized)
	{
		return FALSE;
	}

	return ReplayRecorder_IsRecording();
}

/**
 * ReplayIntegration_IsPlayingBack
 */
Bool ReplayIntegration_IsPlayingBack(void)
{
	if (!g_replayIntegration_initialized)
	{
		return FALSE;
	}

	return ReplayPlayer_IsPlaybackActive();
}

/**
 * ReplayIntegration_GetDetailedStatus
 */
const char* ReplayIntegration_GetDetailedStatus(void)
{
	static char status_buffer[512];

	if (!g_replayIntegration_initialized)
	{
		snprintf(status_buffer, sizeof(status_buffer),
			"ReplayIntegration: NOT INITIALIZED");
		return status_buffer;
	}

	Bool is_recording = ReplayIntegration_IsRecording();
	Bool is_playback = ReplayIntegration_IsPlayingBack();

	// Build comprehensive status
	snprintf(status_buffer, sizeof(status_buffer),
		"ReplayIntegration: Recording=%s Playback=%s\n"
		"  Recorder: %s\n"
		"  Player: %s",
		is_recording ? "YES" : "NO",
		is_playback ? "YES" : "NO",
		ReplayRecorder_GetStatusString(),
		ReplayPlayer_GetStatusString());

	return status_buffer;
}
