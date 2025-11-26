#ifndef REPLAY_RECORDER_H
#define REPLAY_RECORDER_H

#include "Common/Types.h"
#include "Common/AsciiString.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Replay Recording System
 * 
 * Wraps the legacy RecorderClass with a modern C-compatible API for replay recording.
 * Provides higher-level control over replay file management, event recording, and archival.
 */

/**
 * Initialize replay recording system
 * 
 * @return TRUE if initialization successful, FALSE otherwise
 */
Bool ReplayRecorder_Initialize(void);

/**
 * Shutdown replay recording system
 * 
 * @return TRUE if shutdown successful, FALSE otherwise
 */
Bool ReplayRecorder_Shutdown(void);

/**
 * Start a new replay recording session
 * 
 * Uses existing RecorderClass::startRecording() internally
 * 
 * @param difficulty Game difficulty level (0=easy, 1=normal, 2=hard, 3=brutal)
 * @param originalGameMode Original game mode (skirmish, campaign, multiplayer, etc.)
 * @param rankPoints Current player rank points
 * @param maxFPS Maximum FPS cap for replay (0 = no limit)
 * @return TRUE if recording started, FALSE otherwise
 */
Bool ReplayRecorder_StartRecording(Int difficulty, Int originalGameMode, Int rankPoints, Int maxFPS);

/**
 * Stop the current replay recording session
 * 
 * Uses existing RecorderClass::stopRecording() internally
 * 
 * @return TRUE if recording stopped, FALSE otherwise
 */
Bool ReplayRecorder_StopRecording(void);

/**
 * Check if replay is currently recording
 * 
 * @return TRUE if currently recording, FALSE otherwise
 */
Bool ReplayRecorder_IsRecording(void);

/**
 * Get the current replay filename
 * 
 * Returns the file path of the last/current replay being recorded.
 * Returns empty string if no active recording.
 * 
 * @return Replay file path (AsciiString)
 */
AsciiString ReplayRecorder_GetCurrentReplayFilename(void);

/**
 * Archive the current replay to replay archive directory
 * 
 * Copies the replay file to the archive folder with timestamp-based naming
 * for long-term storage and analysis.
 * 
 * @param filename Replay file to archive
 * @return TRUE if archive successful, FALSE otherwise
 */
Bool ReplayRecorder_ArchiveReplay(const char* filename);

/**
 * Get the default replay directory
 * 
 * Returns the directory where replay files are stored.
 * 
 * @return Directory path (AsciiString)
 */
AsciiString ReplayRecorder_GetReplayDirectory(void);

/**
 * Get replay file extension
 * 
 * Returns the standard extension for replay files (.rep)
 * 
 * @return File extension (AsciiString)
 */
AsciiString ReplayRecorder_GetReplayExtension(void);

/**
 * Get detailed status string
 * 
 * Returns comprehensive status of replay recording system
 * Format: "ReplayRecorder: Recording(YES|NO) File=path Duration=MM:SS"
 * 
 * @return Status string
 */
const char* ReplayRecorder_GetStatusString(void);

#ifdef __cplusplus
}
#endif

#endif // REPLAY_RECORDER_H
