#ifndef REPLAY_PLAYER_H
#define REPLAY_PLAYER_H

#include "Common/Types.h"
#include "Common/AsciiString.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Replay Playback System
 * 
 * Wraps the legacy RecorderClass with a modern C-compatible API for replay playback.
 * Provides timeline controls and playback state management.
 */

/**
 * Playback state enumeration
 */
typedef enum ReplayPlaybackState {
	REPLAY_PLAYBACK_IDLE,           ///< No playback active
	REPLAY_PLAYBACK_PLAYING,        ///< Playback in progress
	REPLAY_PLAYBACK_PAUSED,         ///< Playback paused (seeking)
	REPLAY_PLAYBACK_FINISHED        ///< Playback completed
} ReplayPlaybackState;

/**
 * Initialize replay playback system
 * 
 * @return TRUE if initialization successful, FALSE otherwise
 */
Bool ReplayPlayer_Initialize(void);

/**
 * Shutdown replay playback system
 * 
 * @return TRUE if shutdown successful, FALSE otherwise
 */
Bool ReplayPlayer_Shutdown(void);

/**
 * Start playback of a replay file
 * 
 * Loads and validates the replay file, then begins playback from frame 0.
 * Uses existing RecorderClass::playbackFile() internally
 * 
 * @param filename Path to replay file (.rep format)
 * @return TRUE if playback started, FALSE if file invalid or not found
 */
Bool ReplayPlayer_StartPlayback(const char* filename);

/**
 * Stop the current playback session
 * 
 * Uses existing RecorderClass::stopPlayback() internally
 * 
 * @return TRUE if playback stopped, FALSE if no playback active
 */
Bool ReplayPlayer_StopPlayback(void);

/**
 * Pause playback (freeze at current frame)
 * 
 * @return TRUE if paused, FALSE if not currently playing
 */
Bool ReplayPlayer_Pause(void);

/**
 * Resume playback from pause
 * 
 * @return TRUE if resumed, FALSE if not paused
 */
Bool ReplayPlayer_Resume(void);

/**
 * Check current playback state
 * 
 * @return Current playback state
 */
ReplayPlaybackState ReplayPlayer_GetPlaybackState(void);

/**
 * Check if playback is currently active (playing or paused)
 * 
 * @return TRUE if playback active, FALSE otherwise
 */
Bool ReplayPlayer_IsPlaybackActive(void);

/**
 * Get the current replay filename being played
 * 
 * @return Replay file path (AsciiString)
 */
AsciiString ReplayPlayer_GetCurrentReplayFilename(void);

/**
 * Get total frame count of current replay
 * 
 * @return Frame count, 0 if no playback active
 */
UnsignedInt ReplayPlayer_GetTotalFrames(void);

/**
 * Get current playback frame
 * 
 * @return Current frame number
 */
UnsignedInt ReplayPlayer_GetCurrentFrame(void);

/**
 * Get playback progress as percentage
 * 
 * @return Progress 0.0-100.0, 0.0 if no playback active
 */
float ReplayPlayer_GetPlaybackProgress(void);

/**
 * Seek to specific frame in replay
 * 
 * @param frame Target frame number (0 to total frames)
 * @return TRUE if seek successful, FALSE otherwise
 */
Bool ReplayPlayer_SeekToFrame(UnsignedInt frame);

/**
 * Seek forward by N frames
 * 
 * @param frameOffset Number of frames to skip forward
 * @return TRUE if seek successful, FALSE otherwise
 */
Bool ReplayPlayer_SkipFrames(Int frameOffset);

/**
 * Validate if replay file matches current game version
 * 
 * Checks version compatibility before starting playback
 * 
 * @param filename Path to replay file to validate
 * @return TRUE if replay matches current version, FALSE otherwise
 */
Bool ReplayPlayer_ValidateReplayVersion(const char* filename);

/**
 * Get detailed status string
 * 
 * Returns comprehensive status of replay playback system
 * Format: "ReplayPlayer: State=PLAYING|PAUSED|IDLE Frame=N/Total Progress=X%"
 * 
 * @return Status string
 */
const char* ReplayPlayer_GetStatusString(void);

#ifdef __cplusplus
}
#endif

#endif // REPLAY_PLAYER_H
