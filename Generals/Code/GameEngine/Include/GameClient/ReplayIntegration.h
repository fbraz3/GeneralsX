#ifndef REPLAY_INTEGRATION_H
#define REPLAY_INTEGRATION_H

#include "Common/Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Replay System Integration (Generals - Base Game)
 * 
 * High-level API combining replay recording and playback subsystems.
 * Provides unified lifecycle management and status reporting.
 */

/**
 * Initialize replay system (both recording and playback)
 * 
 * @return TRUE if initialization successful, FALSE otherwise
 */
Bool ReplayIntegration_Initialize(void);

/**
 * Shutdown replay system
 * 
 * Stops any active recording/playback and cleans up resources
 * 
 * @return TRUE if shutdown successful, FALSE otherwise
 */
Bool ReplayIntegration_Shutdown(void);

/**
 * Update replay system (call once per frame)
 * 
 * Handles playback frame advancement and state management
 * 
 * @param deltaTime Time since last update in seconds
 * @return TRUE if update successful, FALSE otherwise
 */
Bool ReplayIntegration_Update(float deltaTime);

/**
 * Start recording a new replay
 * 
 * @param difficulty Game difficulty (0=easy, 1=normal, 2=hard, 3=brutal)
 * @param gameMode Game mode (skirmish, campaign, multiplayer, etc.)
 * @param rankPoints Player rank points at game start
 * @return TRUE if recording started, FALSE otherwise
 */
Bool ReplayIntegration_StartRecording(Int difficulty, Int gameMode, Int rankPoints);

/**
 * Stop recording (auto-saves replay)
 * 
 * @return TRUE if recording stopped, FALSE otherwise
 */
Bool ReplayIntegration_StopRecording(void);

/**
 * Start playback of a replay file
 * 
 * @param replayFilename Path to .rep file
 * @return TRUE if playback started, FALSE if file invalid
 */
Bool ReplayIntegration_StartPlayback(const char* replayFilename);

/**
 * Stop playback
 * 
 * @return TRUE if playback stopped, FALSE if no playback active
 */
Bool ReplayIntegration_StopPlayback(void);

/**
 * Check if currently recording
 * 
 * @return TRUE if recording, FALSE otherwise
 */
Bool ReplayIntegration_IsRecording(void);

/**
 * Check if currently playing back
 * 
 * @return TRUE if playing back, FALSE otherwise
 */
Bool ReplayIntegration_IsPlayingBack(void);

/**
 * Get comprehensive system status
 * 
 * Format: "ReplayIntegration: Recording=YES|NO Playback=YES|NO"
 * Combined with recorder/player status strings
 * 
 * @return Status string
 */
const char* ReplayIntegration_GetDetailedStatus(void);

#ifdef __cplusplus
}
#endif

#endif // REPLAY_INTEGRATION_H
