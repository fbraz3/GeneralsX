#include "GameClient/ReplayRecorder.h"
#include "Common/Recorder.h"
#include "Common/GlobalData.h"
#include "Common/Version.h"
#include <time.h>
#include <cstring>

/**
 * Global state for replay recording
 */
static Bool g_replayRecorder_initialized = FALSE;
static time_t g_recording_start_time = 0;

/**
 * ReplayRecorder_Initialize
 */
Bool ReplayRecorder_Initialize(void)
{
	if (g_replayRecorder_initialized)
	{
		DEBUG_LOG(("ReplayRecorder already initialized\n"));
		return TRUE;
	}

	if (TheRecorder == NULL)
	{
		DEBUG_LOG(("ERROR: ReplayRecorder_Initialize - TheRecorder is NULL\n"));
		return FALSE;
	}

	g_replayRecorder_initialized = TRUE;
	g_recording_start_time = 0;

	DEBUG_LOG(("ReplayRecorder::Initialize - Recording system initialized\n"));
	return TRUE;
}

/**
 * ReplayRecorder_Shutdown
 */
Bool ReplayRecorder_Shutdown(void)
{
	if (!g_replayRecorder_initialized)
	{
		return TRUE;
	}

	// Stop any active recording
	if (ReplayRecorder_IsRecording())
	{
		ReplayRecorder_StopRecording();
	}

	g_replayRecorder_initialized = FALSE;
	g_recording_start_time = 0;

	DEBUG_LOG(("ReplayRecorder::Shutdown - Recording system shutdown\n"));
	return TRUE;
}

/**
 * ReplayRecorder_StartRecording
 */
Bool ReplayRecorder_StartRecording(Int difficulty, Int originalGameMode, Int rankPoints, Int maxFPS)
{
	if (!g_replayRecorder_initialized)
	{
		DEBUG_LOG(("ERROR: ReplayRecorder_StartRecording - Recording system not initialized\n"));
		return FALSE;
	}

	if (TheRecorder == NULL)
	{
		DEBUG_LOG(("ERROR: ReplayRecorder_StartRecording - TheRecorder is NULL\n"));
		return FALSE;
	}

	// Validate difficulty range (0-3 typical)
	if (difficulty < 0 || difficulty > 3)
	{
		DEBUG_LOG(("WARNING: ReplayRecorder_StartRecording - Invalid difficulty %d, clamping to 0-3\n", difficulty));
		difficulty = MAX(0, MIN(3, difficulty));
	}

	// Record start time for duration calculation
	g_recording_start_time = time(NULL);

	// Call existing RecorderClass::startRecording
	TheRecorder->startRecording((GameDifficulty)difficulty, originalGameMode, rankPoints, maxFPS);

	DEBUG_LOG(("ReplayRecorder::StartRecording - Difficulty=%d Mode=%d RankPts=%d MaxFPS=%d\n", 
		difficulty, originalGameMode, rankPoints, maxFPS));

	return TRUE;
}

/**
 * ReplayRecorder_StopRecording
 */
Bool ReplayRecorder_StopRecording(void)
{
	if (!g_replayRecorder_initialized)
	{
		DEBUG_LOG(("WARNING: ReplayRecorder_StopRecording - Recording system not initialized\n"));
		return FALSE;
	}

	if (TheRecorder == NULL)
	{
		DEBUG_LOG(("ERROR: ReplayRecorder_StopRecording - TheRecorder is NULL\n"));
		return FALSE;
	}

	if (!ReplayRecorder_IsRecording())
	{
		DEBUG_LOG(("WARNING: ReplayRecorder_StopRecording - Not currently recording\n"));
		return FALSE;
	}

	// Call existing RecorderClass::stopRecording
	TheRecorder->stopRecording();

	g_recording_start_time = 0;

	DEBUG_LOG(("ReplayRecorder::StopRecording - Recording stopped\n"));
	return TRUE;
}

/**
 * ReplayRecorder_IsRecording
 */
Bool ReplayRecorder_IsRecording(void)
{
	if (TheRecorder == NULL)
	{
		return FALSE;
	}

	// Check if recorder is in RECORD mode
	RecorderModeType mode = TheRecorder->getMode();
	return (mode == RECORDERMODETYPE_RECORD);
}

/**
 * ReplayRecorder_GetCurrentReplayFilename
 */
AsciiString ReplayRecorder_GetCurrentReplayFilename(void)
{
	if (TheRecorder == NULL)
	{
		return AsciiString::TheEmptyString;
	}

	if (!ReplayRecorder_IsRecording())
	{
		return AsciiString::TheEmptyString;
	}

	return TheRecorder->getLastReplayFileName();
}

/**
 * ReplayRecorder_ArchiveReplay
 */
Bool ReplayRecorder_ArchiveReplay(const char* filename)
{
	if (!g_replayRecorder_initialized)
	{
		DEBUG_LOG(("ERROR: ReplayRecorder_ArchiveReplay - Recording system not initialized\n"));
		return FALSE;
	}

	if (TheRecorder == NULL)
	{
		DEBUG_LOG(("ERROR: ReplayRecorder_ArchiveReplay - TheRecorder is NULL\n"));
		return FALSE;
	}

	if (filename == NULL)
	{
		DEBUG_LOG(("ERROR: ReplayRecorder_ArchiveReplay - filename is NULL\n"));
		return FALSE;
	}

	// Call existing RecorderClass::archiveReplay (Phase 47 feature)
	TheRecorder->archiveReplay(AsciiString(filename));

	DEBUG_LOG(("ReplayRecorder::ArchiveReplay - Archived: %s\n", filename));
	return TRUE;
}

/**
 * ReplayRecorder_GetReplayDirectory
 */
AsciiString ReplayRecorder_GetReplayDirectory(void)
{
	if (TheRecorder == NULL)
	{
		return AsciiString::TheEmptyString;
	}

	return TheRecorder->getReplayDir();
}

/**
 * ReplayRecorder_GetReplayExtension
 */
AsciiString ReplayRecorder_GetReplayExtension(void)
{
	if (TheRecorder == NULL)
	{
		return AsciiString(".rep");
	}

	return TheRecorder->getReplayExtention();
}

/**
 * ReplayRecorder_GetStatusString
 */
const char* ReplayRecorder_GetStatusString(void)
{
	static char status_buffer[256];

	if (!g_replayRecorder_initialized)
	{
		snprintf(status_buffer, sizeof(status_buffer), "ReplayRecorder: NOT INITIALIZED");
		return status_buffer;
	}

	Bool is_recording = ReplayRecorder_IsRecording();
	AsciiString filename = ReplayRecorder_GetCurrentReplayFilename();

	if (is_recording && g_recording_start_time > 0)
	{
		time_t current_time = time(NULL);
		time_t duration = current_time - g_recording_start_time;
		Int minutes = duration / 60;
		Int seconds = duration % 60;

		snprintf(status_buffer, sizeof(status_buffer),
			"ReplayRecorder: Recording(YES) File=%s Duration=%02d:%02d",
			filename.str(), minutes, seconds);
	}
	else
	{
		snprintf(status_buffer, sizeof(status_buffer),
			"ReplayRecorder: Recording(NO) File=<none>");
	}

	return status_buffer;
}
