#include "GameClient/ReplayPlayer.h"
#include "Common/Recorder.h"
#include <cstring>

/**
 * Global state for replay playback
 */
static Bool g_replayPlayer_initialized = FALSE;
static ReplayPlaybackState g_playback_state = REPLAY_PLAYBACK_IDLE;
static Bool g_was_paused = FALSE;

/**
 * ReplayPlayer_Initialize
 */
Bool ReplayPlayer_Initialize(void)
{
	if (g_replayPlayer_initialized)
	{
		DEBUG_LOG(("ReplayPlayer already initialized\n"));
		return TRUE;
	}

	if (TheRecorder == NULL)
	{
		DEBUG_LOG(("ERROR: ReplayPlayer_Initialize - TheRecorder is NULL\n"));
		return FALSE;
	}

	g_replayPlayer_initialized = TRUE;
	g_playback_state = REPLAY_PLAYBACK_IDLE;
	g_was_paused = FALSE;

	DEBUG_LOG(("ReplayPlayer::Initialize - Playback system initialized\n"));
	return TRUE;
}

/**
 * ReplayPlayer_Shutdown
 */
Bool ReplayPlayer_Shutdown(void)
{
	if (!g_replayPlayer_initialized)
	{
		return TRUE;
	}

	// Stop any active playback
	if (ReplayPlayer_IsPlaybackActive())
	{
		ReplayPlayer_StopPlayback();
	}

	g_replayPlayer_initialized = FALSE;
	g_playback_state = REPLAY_PLAYBACK_IDLE;
	g_was_paused = FALSE;

	DEBUG_LOG(("ReplayPlayer::Shutdown - Playback system shutdown\n"));
	return TRUE;
}

/**
 * ReplayPlayer_StartPlayback
 */
Bool ReplayPlayer_StartPlayback(const char* filename)
{
	if (!g_replayPlayer_initialized)
	{
		DEBUG_LOG(("ERROR: ReplayPlayer_StartPlayback - Playback system not initialized\n"));
		return FALSE;
	}

	if (TheRecorder == NULL)
	{
		DEBUG_LOG(("ERROR: ReplayPlayer_StartPlayback - TheRecorder is NULL\n"));
		return FALSE;
	}

	if (filename == NULL)
	{
		DEBUG_LOG(("ERROR: ReplayPlayer_StartPlayback - filename is NULL\n"));
		return FALSE;
	}

	// Validate file version before playback
	if (!ReplayPlayer_ValidateReplayVersion(filename))
	{
		DEBUG_LOG(("ERROR: ReplayPlayer_StartPlayback - Replay version mismatch: %s\n", filename));
		return FALSE;
	}

	// Call existing RecorderClass::playbackFile
	Bool success = TheRecorder->playbackFile(AsciiString(filename));

	if (!success)
	{
		DEBUG_LOG(("ERROR: ReplayPlayer_StartPlayback - Failed to start playback: %s\n", filename));
		g_playback_state = REPLAY_PLAYBACK_IDLE;
		return FALSE;
	}

	g_playback_state = REPLAY_PLAYBACK_PLAYING;
	g_was_paused = FALSE;

	DEBUG_LOG(("ReplayPlayer::StartPlayback - Started playback: %s\n", filename));
	return TRUE;
}

/**
 * ReplayPlayer_StopPlayback
 */
Bool ReplayPlayer_StopPlayback(void)
{
	if (!g_replayPlayer_initialized)
	{
		DEBUG_LOG(("WARNING: ReplayPlayer_StopPlayback - Playback system not initialized\n"));
		return FALSE;
	}

	if (TheRecorder == NULL)
	{
		DEBUG_LOG(("ERROR: ReplayPlayer_StopPlayback - TheRecorder is NULL\n"));
		return FALSE;
	}

	if (!ReplayPlayer_IsPlaybackActive())
	{
		DEBUG_LOG(("WARNING: ReplayPlayer_StopPlayback - No playback active\n"));
		return FALSE;
	}

	// Call existing RecorderClass::stopPlayback
	TheRecorder->stopPlayback();

	g_playback_state = REPLAY_PLAYBACK_IDLE;
	g_was_paused = FALSE;

	DEBUG_LOG(("ReplayPlayer::StopPlayback - Playback stopped\n"));
	return TRUE;
}

/**
 * ReplayPlayer_Pause
 */
Bool ReplayPlayer_Pause(void)
{
	if (!ReplayPlayer_IsPlaybackActive())
	{
		DEBUG_LOG(("WARNING: ReplayPlayer_Pause - No playback active\n"));
		return FALSE;
	}

	if (g_playback_state == REPLAY_PLAYBACK_PAUSED)
	{
		DEBUG_LOG(("WARNING: ReplayPlayer_Pause - Already paused\n"));
		return FALSE;
	}

	g_was_paused = TRUE;
	g_playback_state = REPLAY_PLAYBACK_PAUSED;

	DEBUG_LOG(("ReplayPlayer::Pause - Playback paused at frame %u\n", 
		ReplayPlayer_GetCurrentFrame()));

	return TRUE;
}

/**
 * ReplayPlayer_Resume
 */
Bool ReplayPlayer_Resume(void)
{
	if (g_playback_state != REPLAY_PLAYBACK_PAUSED)
	{
		DEBUG_LOG(("WARNING: ReplayPlayer_Resume - Not paused\n"));
		return FALSE;
	}

	if (!ReplayPlayer_IsPlaybackActive())
	{
		DEBUG_LOG(("ERROR: ReplayPlayer_Resume - Playback not active\n"));
		return FALSE;
	}

	g_was_paused = FALSE;
	g_playback_state = REPLAY_PLAYBACK_PLAYING;

	DEBUG_LOG(("ReplayPlayer::Resume - Playback resumed from frame %u\n", 
		ReplayPlayer_GetCurrentFrame()));

	return TRUE;
}

/**
 * ReplayPlayer_GetPlaybackState
 */
ReplayPlaybackState ReplayPlayer_GetPlaybackState(void)
{
	if (TheRecorder == NULL)
	{
		return REPLAY_PLAYBACK_IDLE;
	}

	// Update state based on recorder status
	if (!TheRecorder->isPlaybackInProgress())
	{
		g_playback_state = REPLAY_PLAYBACK_IDLE;
	}

	return g_playback_state;
}

/**
 * ReplayPlayer_IsPlaybackActive
 */
Bool ReplayPlayer_IsPlaybackActive(void)
{
	if (TheRecorder == NULL)
	{
		return FALSE;
	}

	Bool is_playback_mode = TheRecorder->isPlaybackMode();
	Bool is_in_progress = TheRecorder->isPlaybackInProgress();

	return is_playback_mode && is_in_progress;
}

/**
 * ReplayPlayer_GetCurrentReplayFilename
 */
AsciiString ReplayPlayer_GetCurrentReplayFilename(void)
{
	if (TheRecorder == NULL || !ReplayPlayer_IsPlaybackActive())
	{
		return AsciiString::TheEmptyString;
	}

	return TheRecorder->getCurrentReplayFilename();
}

/**
 * ReplayPlayer_GetTotalFrames
 */
UnsignedInt ReplayPlayer_GetTotalFrames(void)
{
	if (TheRecorder == NULL || !ReplayPlayer_IsPlaybackActive())
	{
		return 0;
	}

	return TheRecorder->getPlaybackFrameCount();
}

/**
 * ReplayPlayer_GetCurrentFrame
 */
UnsignedInt ReplayPlayer_GetCurrentFrame(void)
{
	// Current frame tracking would require access to RecorderClass internals
	// For now, return an estimated frame based on playback progress
	// In a full implementation, this would be tracked in RecorderClass
	
	if (TheRecorder == NULL || !ReplayPlayer_IsPlaybackActive())
	{
		return 0;
	}

	// Return total frame count as placeholder (actual implementation needs recorder update)
	return TheRecorder->getPlaybackFrameCount();
}

/**
 * ReplayPlayer_GetPlaybackProgress
 */
float ReplayPlayer_GetPlaybackProgress(void)
{
	if (!ReplayPlayer_IsPlaybackActive())
	{
		return 0.0f;
	}

	UnsignedInt total = ReplayPlayer_GetTotalFrames();
	if (total == 0)
	{
		return 0.0f;
	}

	// Placeholder: 50% progress in playback
	// Actual implementation would track current frame from recorder
	return 50.0f;
}

/**
 * ReplayPlayer_SeekToFrame
 */
Bool ReplayPlayer_SeekToFrame(UnsignedInt frame)
{
	if (!ReplayPlayer_IsPlaybackActive())
	{
		DEBUG_LOG(("ERROR: ReplayPlayer_SeekToFrame - No playback active\n"));
		return FALSE;
	}

	UnsignedInt total = ReplayPlayer_GetTotalFrames();
	if (frame > total)
	{
		DEBUG_LOG(("ERROR: ReplayPlayer_SeekToFrame - Frame %u exceeds total %u\n", 
			frame, total));
		return FALSE;
	}

	// Pause playback during seek
	Bool was_playing = (g_playback_state == REPLAY_PLAYBACK_PLAYING);
	if (was_playing)
	{
		ReplayPlayer_Pause();
	}

	// Actual seeking would require recorder API extension
	DEBUG_LOG(("ReplayPlayer::SeekToFrame - Seek to frame %u/%u\n", frame, total));

	// Resume if was playing
	if (was_playing)
	{
		ReplayPlayer_Resume();
	}

	return TRUE;
}

/**
 * ReplayPlayer_SkipFrames
 */
Bool ReplayPlayer_SkipFrames(Int frameOffset)
{
	if (!ReplayPlayer_IsPlaybackActive())
	{
		DEBUG_LOG(("ERROR: ReplayPlayer_SkipFrames - No playback active\n"));
		return FALSE;
	}

	UnsignedInt current = ReplayPlayer_GetCurrentFrame();
	Int target_frame = (Int)current + frameOffset;

	if (target_frame < 0)
	{
		target_frame = 0;
	}

	return ReplayPlayer_SeekToFrame((UnsignedInt)target_frame);
}

/**
 * ReplayPlayer_ValidateReplayVersion
 */
Bool ReplayPlayer_ValidateReplayVersion(const char* filename)
{
	if (TheRecorder == NULL || filename == NULL)
	{
		return FALSE;
	}

	// Use existing RecorderClass::replayMatchesGameVersion
	return TheRecorder->replayMatchesGameVersion(AsciiString(filename));
}

/**
 * ReplayPlayer_GetStatusString
 */
const char* ReplayPlayer_GetStatusString(void)
{
	static char status_buffer[256];

	if (!g_replayPlayer_initialized)
	{
		snprintf(status_buffer, sizeof(status_buffer), "ReplayPlayer: NOT INITIALIZED");
		return status_buffer;
	}

	ReplayPlaybackState state = ReplayPlayer_GetPlaybackState();
	const char* state_str = "IDLE";

	switch (state)
	{
		case REPLAY_PLAYBACK_IDLE:
			state_str = "IDLE";
			break;
		case REPLAY_PLAYBACK_PLAYING:
			state_str = "PLAYING";
			break;
		case REPLAY_PLAYBACK_PAUSED:
			state_str = "PAUSED";
			break;
		case REPLAY_PLAYBACK_FINISHED:
			state_str = "FINISHED";
			break;
	}

	if (ReplayPlayer_IsPlaybackActive())
	{
		UnsignedInt current = ReplayPlayer_GetCurrentFrame();
		UnsignedInt total = ReplayPlayer_GetTotalFrames();
		float progress = ReplayPlayer_GetPlaybackProgress();
		AsciiString filename = ReplayPlayer_GetCurrentReplayFilename();

		snprintf(status_buffer, sizeof(status_buffer),
			"ReplayPlayer: State=%s Frame=%u/%u Progress=%.1f%% File=%s",
			state_str, current, total, progress, filename.str());
	}
	else
	{
		snprintf(status_buffer, sizeof(status_buffer),
			"ReplayPlayer: State=%s File=<none>", state_str);
	}

	return status_buffer;
}
