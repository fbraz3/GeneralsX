/**
 * @file AudioManager.cpp
 * @brief Phase 26: Audio Integration (Phase 1) - Implementation
 *
 * Audio playback, device management, and volume control.
 */

#include "AudioManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_AUDIO_SOURCES 256
#define MAX_AUDIO_DEVICES 16
#define MAX_CALLBACKS 32

// Error handling
static char g_error_message[256] = {0};

// Audio source callback
typedef struct {
    uint32_t source_handle;
    AudioPlaybackCallback callback;
    void* user_data;
} PlaybackCallbackEntry;

// Device callback entry
typedef struct {
    AudioDeviceCallback callback;
    void* user_data;
} DeviceCallbackEntry;

// Audio system state
typedef struct {
    AudioSource sources[MAX_AUDIO_SOURCES];
    uint32_t source_count;
    
    AudioDeviceInfo devices[MAX_AUDIO_DEVICES];
    uint32_t device_count;
    uint32_t active_device;
    
    PlaybackCallbackEntry playback_callbacks[MAX_CALLBACKS];
    uint32_t playback_callback_count;
    
    DeviceCallbackEntry device_callbacks[MAX_CALLBACKS];
    uint32_t device_callback_count;
    
    AudioMixerSettings mixer_settings;
    int is_muted;
    
    int is_initialized;
} AudioSystemState;

static AudioSystemState g_audio_system = {0};

/**
 * Set error message
 */
static void SetError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(g_error_message, sizeof(g_error_message), format, args);
    va_end(args);
}

/**
 * Initialize audio system
 */
int AudioManager_Initialize(void) {
    if (g_audio_system.is_initialized) {
        return AUDIO_OK;
    }
    
    memset(&g_audio_system, 0, sizeof(AudioSystemState));
    
    // Initialize mixer settings to default
    g_audio_system.mixer_settings.master_volume = 1.0f;
    g_audio_system.mixer_settings.music_volume = 0.8f;
    g_audio_system.mixer_settings.sfx_volume = 1.0f;
    g_audio_system.mixer_settings.ambient_volume = 0.6f;
    g_audio_system.mixer_settings.dialog_volume = 1.0f;
    g_audio_system.mixer_settings.ui_volume = 0.7f;
    
    g_audio_system.is_muted = 0;
    g_audio_system.active_device = 0;
    
    // Add default audio device
    AudioDeviceInfo* device = &g_audio_system.devices[0];
    device->device_id = 0;
    strcpy(device->device_name, "Default Audio Device");
    device->max_channels = 2;
    device->sample_rate = 44100;
    device->supported_formats[0] = AUDIO_FORMAT_PCM_16;
    device->supported_formats[1] = AUDIO_FORMAT_COMPRESSED_OGG;
    device->format_count = 2;
    device->is_default = 1;
    
    g_audio_system.device_count = 1;
    g_audio_system.is_initialized = 1;
    
    return AUDIO_OK;
}

/**
 * Shutdown audio system
 */
int AudioManager_Shutdown(void) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    // Stop all audio sources
    for (uint32_t i = 0; i < g_audio_system.source_count; i++) {
        g_audio_system.sources[i].state = AUDIO_STATE_STOPPED;
    }
    
    g_audio_system.source_count = 0;
    g_audio_system.device_count = 0;
    g_audio_system.playback_callback_count = 0;
    g_audio_system.device_callback_count = 0;
    g_audio_system.is_initialized = 0;
    
    return AUDIO_OK;
}

/**
 * Enumerate audio devices
 */
uint32_t AudioManager_EnumerateDevices(AudioDeviceInfo* devices, uint32_t max_devices) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return 0;
    }
    
    if (!devices || max_devices == 0) {
        return 0;
    }
    
    uint32_t count = (g_audio_system.device_count < max_devices) ? g_audio_system.device_count : max_devices;
    
    for (uint32_t i = 0; i < count; i++) {
        devices[i] = g_audio_system.devices[i];
    }
    
    return count;
}

/**
 * Set active audio device
 */
int AudioManager_SetActiveDevice(uint32_t device_id) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    if (device_id >= g_audio_system.device_count) {
        SetError("Invalid device ID");
        return AUDIO_ERROR_DEVICE_FAILED;
    }
    
    g_audio_system.active_device = device_id;
    return AUDIO_OK;
}

/**
 * Get active audio device
 */
uint32_t AudioManager_GetActiveDevice(void) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return 0;
    }
    
    return g_audio_system.active_device;
}

/**
 * Register device callback
 */
int AudioManager_RegisterDeviceCallback(AudioDeviceCallback callback, void* user_data) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    if (g_audio_system.device_callback_count >= MAX_CALLBACKS) {
        SetError("Device callback limit exceeded");
        return AUDIO_ERROR_DEVICE_FAILED;
    }
    
    DeviceCallbackEntry* entry = &g_audio_system.device_callbacks[g_audio_system.device_callback_count++];
    entry->callback = callback;
    entry->user_data = user_data;
    
    return AUDIO_OK;
}

/**
 * Load audio from file
 */
uint32_t AudioManager_LoadAudio(const char* file_path, AudioSourceType source_type) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return 0;
    }
    
    if (!file_path) {
        SetError("File path is NULL");
        return 0;
    }
    
    if (g_audio_system.source_count >= MAX_AUDIO_SOURCES) {
        SetError("Audio source limit exceeded");
        return 0;
    }
    
    uint32_t source_idx = g_audio_system.source_count;
    AudioSource* source = &g_audio_system.sources[source_idx];
    
    memset(source, 0, sizeof(AudioSource));
    source->handle = 26000 + source_idx;
    source->type = source_type;
    source->state = AUDIO_STATE_LOADING;
    source->volume = 1.0f;
    source->looping = 0;
    source->sample_rate = 44100;
    source->channel_count = 2;
    
    strncpy(source->file_path, file_path, sizeof(source->file_path) - 1);
    
    // Simulate loading
    source->total_duration = 180000;  // 3 minutes default
    source->state = AUDIO_STATE_STOPPED;
    
    g_audio_system.source_count++;
    
    return source->handle;
}

/**
 * Load audio from VFS
 */
uint32_t AudioManager_LoadAudioFromVFS(const char* archive_path, AudioSourceType source_type) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return 0;
    }
    
    if (!archive_path) {
        SetError("Archive path is NULL");
        return 0;
    }
    
    // Use same loading mechanism as file load
    return AudioManager_LoadAudio(archive_path, source_type);
}

/**
 * Unload audio source
 */
int AudioManager_UnloadAudio(uint32_t handle) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    uint32_t source_idx = handle - 26000;
    if (source_idx >= g_audio_system.source_count) {
        SetError("Invalid audio handle");
        return AUDIO_ERROR_INVALID_HANDLE;
    }
    
    // Stop playback
    g_audio_system.sources[source_idx].state = AUDIO_STATE_STOPPED;
    
    // Swap with last source
    if (source_idx < g_audio_system.source_count - 1) {
        g_audio_system.sources[source_idx] = g_audio_system.sources[g_audio_system.source_count - 1];
        g_audio_system.sources[source_idx].handle = 26000 + source_idx;
    }
    
    g_audio_system.source_count--;
    
    return AUDIO_OK;
}

/**
 * Play audio source
 */
int AudioManager_Play(uint32_t handle) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    uint32_t source_idx = handle - 26000;
    if (source_idx >= g_audio_system.source_count) {
        SetError("Invalid audio handle");
        return AUDIO_ERROR_INVALID_HANDLE;
    }
    
    AudioSource* source = &g_audio_system.sources[source_idx];
    AudioPlayState old_state = source->state;
    source->state = AUDIO_STATE_PLAYING;
    
    // Trigger callback
    for (uint32_t i = 0; i < g_audio_system.playback_callback_count; i++) {
        PlaybackCallbackEntry* entry = &g_audio_system.playback_callbacks[i];
        if (entry->source_handle == handle && entry->callback) {
            entry->callback(handle, AUDIO_STATE_PLAYING, entry->user_data);
        }
    }
    
    return AUDIO_OK;
}

/**
 * Stop audio source
 */
int AudioManager_Stop(uint32_t handle) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    uint32_t source_idx = handle - 26000;
    if (source_idx >= g_audio_system.source_count) {
        SetError("Invalid audio handle");
        return AUDIO_ERROR_INVALID_HANDLE;
    }
    
    AudioSource* source = &g_audio_system.sources[source_idx];
    source->state = AUDIO_STATE_STOPPED;
    source->current_position = 0;
    
    // Trigger callback
    for (uint32_t i = 0; i < g_audio_system.playback_callback_count; i++) {
        PlaybackCallbackEntry* entry = &g_audio_system.playback_callbacks[i];
        if (entry->source_handle == handle && entry->callback) {
            entry->callback(handle, AUDIO_STATE_STOPPED, entry->user_data);
        }
    }
    
    return AUDIO_OK;
}

/**
 * Pause audio source
 */
int AudioManager_Pause(uint32_t handle) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    uint32_t source_idx = handle - 26000;
    if (source_idx >= g_audio_system.source_count) {
        SetError("Invalid audio handle");
        return AUDIO_ERROR_INVALID_HANDLE;
    }
    
    AudioSource* source = &g_audio_system.sources[source_idx];
    source->state = AUDIO_STATE_PAUSED;
    
    // Trigger callback
    for (uint32_t i = 0; i < g_audio_system.playback_callback_count; i++) {
        PlaybackCallbackEntry* entry = &g_audio_system.playback_callbacks[i];
        if (entry->source_handle == handle && entry->callback) {
            entry->callback(handle, AUDIO_STATE_PAUSED, entry->user_data);
        }
    }
    
    return AUDIO_OK;
}

/**
 * Resume audio source
 */
int AudioManager_Resume(uint32_t handle) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    uint32_t source_idx = handle - 26000;
    if (source_idx >= g_audio_system.source_count) {
        SetError("Invalid audio handle");
        return AUDIO_ERROR_INVALID_HANDLE;
    }
    
    AudioSource* source = &g_audio_system.sources[source_idx];
    if (source->state == AUDIO_STATE_PAUSED) {
        source->state = AUDIO_STATE_PLAYING;
    }
    
    // Trigger callback
    for (uint32_t i = 0; i < g_audio_system.playback_callback_count; i++) {
        PlaybackCallbackEntry* entry = &g_audio_system.playback_callbacks[i];
        if (entry->source_handle == handle && entry->callback) {
            entry->callback(handle, AUDIO_STATE_PLAYING, entry->user_data);
        }
    }
    
    return AUDIO_OK;
}

/**
 * Set volume
 */
int AudioManager_SetVolume(uint32_t handle, float volume) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    if (volume < 0.0f || volume > 1.0f) {
        SetError("Volume out of range");
        return AUDIO_ERROR_INVALID_HANDLE;
    }
    
    uint32_t source_idx = handle - 26000;
    if (source_idx >= g_audio_system.source_count) {
        SetError("Invalid audio handle");
        return AUDIO_ERROR_INVALID_HANDLE;
    }
    
    g_audio_system.sources[source_idx].volume = volume;
    return AUDIO_OK;
}

/**
 * Get volume
 */
float AudioManager_GetVolume(uint32_t handle) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return 0.0f;
    }
    
    uint32_t source_idx = handle - 26000;
    if (source_idx >= g_audio_system.source_count) {
        SetError("Invalid audio handle");
        return 0.0f;
    }
    
    return g_audio_system.sources[source_idx].volume;
}

/**
 * Set looping
 */
int AudioManager_SetLooping(uint32_t handle, int looping) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    uint32_t source_idx = handle - 26000;
    if (source_idx >= g_audio_system.source_count) {
        SetError("Invalid audio handle");
        return AUDIO_ERROR_INVALID_HANDLE;
    }
    
    g_audio_system.sources[source_idx].looping = looping;
    return AUDIO_OK;
}

/**
 * Get playback state
 */
AudioPlayState AudioManager_GetPlayState(uint32_t handle) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_STATE_STOPPED;
    }
    
    uint32_t source_idx = handle - 26000;
    if (source_idx >= g_audio_system.source_count) {
        SetError("Invalid audio handle");
        return AUDIO_STATE_ERROR;
    }
    
    return g_audio_system.sources[source_idx].state;
}

/**
 * Get source info
 */
const AudioSource* AudioManager_GetSourceInfo(uint32_t handle) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return NULL;
    }
    
    uint32_t source_idx = handle - 26000;
    if (source_idx >= g_audio_system.source_count) {
        SetError("Invalid audio handle");
        return NULL;
    }
    
    return &g_audio_system.sources[source_idx];
}

/**
 * Set position
 */
int AudioManager_SetPosition(uint32_t handle, uint32_t position) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    uint32_t source_idx = handle - 26000;
    if (source_idx >= g_audio_system.source_count) {
        SetError("Invalid audio handle");
        return AUDIO_ERROR_INVALID_HANDLE;
    }
    
    AudioSource* source = &g_audio_system.sources[source_idx];
    if (position > source->total_duration) {
        position = source->total_duration;
    }
    
    source->current_position = position;
    return AUDIO_OK;
}

/**
 * Get position
 */
uint32_t AudioManager_GetPosition(uint32_t handle) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return 0;
    }
    
    uint32_t source_idx = handle - 26000;
    if (source_idx >= g_audio_system.source_count) {
        SetError("Invalid audio handle");
        return 0;
    }
    
    return g_audio_system.sources[source_idx].current_position;
}

/**
 * Get duration
 */
uint32_t AudioManager_GetDuration(uint32_t handle) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return 0;
    }
    
    uint32_t source_idx = handle - 26000;
    if (source_idx >= g_audio_system.source_count) {
        SetError("Invalid audio handle");
        return 0;
    }
    
    return g_audio_system.sources[source_idx].total_duration;
}

/**
 * Register playback callback
 */
int AudioManager_RegisterPlaybackCallback(uint32_t handle,
                                         AudioPlaybackCallback callback,
                                         void* user_data) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    if (g_audio_system.playback_callback_count >= MAX_CALLBACKS) {
        SetError("Playback callback limit exceeded");
        return AUDIO_ERROR_DEVICE_FAILED;
    }
    
    PlaybackCallbackEntry* entry = &g_audio_system.playback_callbacks[g_audio_system.playback_callback_count++];
    entry->source_handle = handle;
    entry->callback = callback;
    entry->user_data = user_data;
    
    return AUDIO_OK;
}

/**
 * Set mixer settings
 */
int AudioManager_SetMixerSettings(const AudioMixerSettings* settings) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    if (!settings) {
        SetError("Settings pointer is NULL");
        return AUDIO_ERROR_INVALID_HANDLE;
    }
    
    g_audio_system.mixer_settings = *settings;
    return AUDIO_OK;
}

/**
 * Get mixer settings
 */
AudioMixerSettings AudioManager_GetMixerSettings(void) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
    }
    
    return g_audio_system.mixer_settings;
}

/**
 * Set category volume
 */
int AudioManager_SetCategoryVolume(AudioSourceType category, float volume) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    if (volume < 0.0f || volume > 1.0f) {
        SetError("Volume out of range");
        return AUDIO_ERROR_INVALID_HANDLE;
    }
    
    switch (category) {
        case AUDIO_SOURCE_MUSIC:
            g_audio_system.mixer_settings.music_volume = volume;
            break;
        case AUDIO_SOURCE_SFX:
            g_audio_system.mixer_settings.sfx_volume = volume;
            break;
        case AUDIO_SOURCE_AMBIENT:
            g_audio_system.mixer_settings.ambient_volume = volume;
            break;
        case AUDIO_SOURCE_DIALOG:
            g_audio_system.mixer_settings.dialog_volume = volume;
            break;
        case AUDIO_SOURCE_UI:
            g_audio_system.mixer_settings.ui_volume = volume;
            break;
    }
    
    return AUDIO_OK;
}

/**
 * Get category volume
 */
float AudioManager_GetCategoryVolume(AudioSourceType category) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return 0.0f;
    }
    
    switch (category) {
        case AUDIO_SOURCE_MUSIC:
            return g_audio_system.mixer_settings.music_volume;
        case AUDIO_SOURCE_SFX:
            return g_audio_system.mixer_settings.sfx_volume;
        case AUDIO_SOURCE_AMBIENT:
            return g_audio_system.mixer_settings.ambient_volume;
        case AUDIO_SOURCE_DIALOG:
            return g_audio_system.mixer_settings.dialog_volume;
        case AUDIO_SOURCE_UI:
            return g_audio_system.mixer_settings.ui_volume;
        default:
            return 0.0f;
    }
}

/**
 * Mute all
 */
int AudioManager_MuteAll(void) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    g_audio_system.is_muted = 1;
    return AUDIO_OK;
}

/**
 * Unmute all
 */
int AudioManager_UnmuteAll(void) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    g_audio_system.is_muted = 0;
    return AUDIO_OK;
}

/**
 * Is muted
 */
int AudioManager_IsMuted(void) {
    if (!g_audio_system.is_initialized) {
        return 0;
    }
    
    return g_audio_system.is_muted;
}

/**
 * Update audio system
 */
int AudioManager_Update(float delta_time_ms) {
    if (!g_audio_system.is_initialized) {
        SetError("Audio system not initialized");
        return AUDIO_ERROR_NOT_INITIALIZED;
    }
    
    // Update playback positions
    for (uint32_t i = 0; i < g_audio_system.source_count; i++) {
        AudioSource* source = &g_audio_system.sources[i];
        
        if (source->state == AUDIO_STATE_PLAYING) {
            source->current_position += (uint32_t)delta_time_ms;
            
            if (source->current_position >= source->total_duration) {
                if (source->looping) {
                    source->current_position = 0;
                } else {
                    source->state = AUDIO_STATE_STOPPED;
                }
            }
        }
    }
    
    return AUDIO_OK;
}

/**
 * Get error message
 */
const char* AudioManager_GetError(void) {
    if (g_error_message[0] == '\0') {
        return "No error";
    }
    return g_error_message;
}
