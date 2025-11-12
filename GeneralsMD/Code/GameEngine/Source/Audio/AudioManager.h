/**
 * @file AudioManager.h
 * @brief Phase 26: Audio Integration (Phase 1) - Audio Playback Manager
 *
 * Handles audio playback during menu and game, including music, SFX, and volume control.
 */

#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
#define AUDIO_OK                     0
#define AUDIO_ERROR_NOT_INITIALIZED -1
#define AUDIO_ERROR_INVALID_HANDLE  -2
#define AUDIO_ERROR_FILE_NOT_FOUND  -3
#define AUDIO_ERROR_DEVICE_FAILED   -4
#define AUDIO_ERROR_FORMAT_INVALID  -5

// Audio source types
typedef enum {
    AUDIO_SOURCE_MUSIC,
    AUDIO_SOURCE_SFX,
    AUDIO_SOURCE_AMBIENT,
    AUDIO_SOURCE_DIALOG,
    AUDIO_SOURCE_UI
} AudioSourceType;

// Audio playback states
typedef enum {
    AUDIO_STATE_STOPPED = 0,
    AUDIO_STATE_PLAYING = 1,
    AUDIO_STATE_PAUSED = 2,
    AUDIO_STATE_LOADING = 3,
    AUDIO_STATE_ERROR = 4
} AudioPlayState;

// Audio format types
typedef enum {
    AUDIO_FORMAT_PCM_16,
    AUDIO_FORMAT_PCM_24,
    AUDIO_FORMAT_PCM_32,
    AUDIO_FORMAT_FLOAT_32,
    AUDIO_FORMAT_COMPRESSED_OGG,
    AUDIO_FORMAT_COMPRESSED_MP3
} AudioFormat;

// Audio device capabilities
typedef struct {
    uint32_t device_id;
    char device_name[128];
    uint32_t max_channels;
    uint32_t sample_rate;
    AudioFormat supported_formats[8];
    uint32_t format_count;
    int is_default;
} AudioDeviceInfo;

// Audio source structure
typedef struct {
    uint32_t handle;
    AudioSourceType type;
    AudioPlayState state;
    char file_path[256];
    float volume;
    int looping;
    uint32_t current_position;
    uint32_t total_duration;
    uint32_t sample_rate;
    uint32_t channel_count;
} AudioSource;

// Audio mixer settings
typedef struct {
    float master_volume;
    float music_volume;
    float sfx_volume;
    float ambient_volume;
    float dialog_volume;
    float ui_volume;
} AudioMixerSettings;

// Audio device callback
typedef void (*AudioDeviceCallback)(uint32_t device_id, int connected, void* user_data);

// Audio playback callback
typedef void (*AudioPlaybackCallback)(uint32_t source_handle, AudioPlayState new_state, void* user_data);

/**
 * Initialize audio system
 * @return AUDIO_OK on success
 */
int AudioManager_Initialize(void);

/**
 * Shutdown audio system
 * @return AUDIO_OK on success
 */
int AudioManager_Shutdown(void);

/**
 * Enumerate audio devices
 * @param devices Array to receive device info
 * @param max_devices Maximum number of devices to return
 * @return Number of devices found
 */
uint32_t AudioManager_EnumerateDevices(AudioDeviceInfo* devices, uint32_t max_devices);

/**
 * Set active audio device
 * @param device_id Device ID to activate
 * @return AUDIO_OK on success
 */
int AudioManager_SetActiveDevice(uint32_t device_id);

/**
 * Get active audio device
 * @return Device ID of active device
 */
uint32_t AudioManager_GetActiveDevice(void);

/**
 * Register device connection callback
 * @param callback Callback function for device changes
 * @param user_data User data to pass to callback
 * @return AUDIO_OK on success
 */
int AudioManager_RegisterDeviceCallback(AudioDeviceCallback callback, void* user_data);

/**
 * Load audio from file
 * @param file_path Path to audio file
 * @param source_type Type of audio source
 * @return Handle to audio source, or 0 on failure
 */
uint32_t AudioManager_LoadAudio(const char* file_path, AudioSourceType source_type);

/**
 * Load audio from VFS (.big archive)
 * @param archive_path Path within archive
 * @param source_type Type of audio source
 * @return Handle to audio source, or 0 on failure
 */
uint32_t AudioManager_LoadAudioFromVFS(const char* archive_path, AudioSourceType source_type);

/**
 * Unload audio source
 * @param handle Audio source handle
 * @return AUDIO_OK on success
 */
int AudioManager_UnloadAudio(uint32_t handle);

/**
 * Play audio source
 * @param handle Audio source handle
 * @return AUDIO_OK on success
 */
int AudioManager_Play(uint32_t handle);

/**
 * Stop audio source
 * @param handle Audio source handle
 * @return AUDIO_OK on success
 */
int AudioManager_Stop(uint32_t handle);

/**
 * Pause audio source
 * @param handle Audio source handle
 * @return AUDIO_OK on success
 */
int AudioManager_Pause(uint32_t handle);

/**
 * Resume audio source
 * @param handle Audio source handle
 * @return AUDIO_OK on success
 */
int AudioManager_Resume(uint32_t handle);

/**
 * Set volume for audio source
 * @param handle Audio source handle
 * @param volume Volume (0.0 - 1.0)
 * @return AUDIO_OK on success
 */
int AudioManager_SetVolume(uint32_t handle, float volume);

/**
 * Get volume for audio source
 * @param handle Audio source handle
 * @return Current volume (0.0 - 1.0)
 */
float AudioManager_GetVolume(uint32_t handle);

/**
 * Set looping for audio source
 * @param handle Audio source handle
 * @param looping 1 to enable looping, 0 to disable
 * @return AUDIO_OK on success
 */
int AudioManager_SetLooping(uint32_t handle, int looping);

/**
 * Get playback state
 * @param handle Audio source handle
 * @return Current playback state
 */
AudioPlayState AudioManager_GetPlayState(uint32_t handle);

/**
 * Get audio source info
 * @param handle Audio source handle
 * @return Pointer to audio source info, or NULL if not found
 */
const AudioSource* AudioManager_GetSourceInfo(uint32_t handle);

/**
 * Set current playback position
 * @param handle Audio source handle
 * @param position Position in milliseconds
 * @return AUDIO_OK on success
 */
int AudioManager_SetPosition(uint32_t handle, uint32_t position);

/**
 * Get current playback position
 * @param handle Audio source handle
 * @return Current position in milliseconds
 */
uint32_t AudioManager_GetPosition(uint32_t handle);

/**
 * Get total duration
 * @param handle Audio source handle
 * @return Duration in milliseconds
 */
uint32_t AudioManager_GetDuration(uint32_t handle);

/**
 * Register playback state callback
 * @param handle Audio source handle
 * @param callback Callback function for state changes
 * @param user_data User data to pass to callback
 * @return AUDIO_OK on success
 */
int AudioManager_RegisterPlaybackCallback(uint32_t handle,
                                         AudioPlaybackCallback callback,
                                         void* user_data);

/**
 * Set mixer settings
 * @param settings Mixer settings
 * @return AUDIO_OK on success
 */
int AudioManager_SetMixerSettings(const AudioMixerSettings* settings);

/**
 * Get mixer settings
 * @return Current mixer settings
 */
AudioMixerSettings AudioManager_GetMixerSettings(void);

/**
 * Set category volume
 * @param category Audio source type category
 * @param volume Volume (0.0 - 1.0)
 * @return AUDIO_OK on success
 */
int AudioManager_SetCategoryVolume(AudioSourceType category, float volume);

/**
 * Get category volume
 * @param category Audio source type category
 * @return Current volume (0.0 - 1.0)
 */
float AudioManager_GetCategoryVolume(AudioSourceType category);

/**
 * Mute all audio
 * @return AUDIO_OK on success
 */
int AudioManager_MuteAll(void);

/**
 * Unmute all audio
 * @return AUDIO_OK on success
 */
int AudioManager_UnmuteAll(void);

/**
 * Check if audio is muted
 * @return 1 if muted, 0 if not muted
 */
int AudioManager_IsMuted(void);

/**
 * Update audio system (call regularly)
 * @param delta_time_ms Time delta in milliseconds
 * @return AUDIO_OK on success
 */
int AudioManager_Update(float delta_time_ms);

/**
 * Get error message from last operation
 * @return Error message string
 */
const char* AudioManager_GetError(void);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_MANAGER_H
