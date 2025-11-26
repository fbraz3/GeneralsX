/*
 * Phase 47: Audio Manager Implementation
 * 
 * High-level audio management for game engine
 * Manages music, sound effects, voice, and ambient audio
 */

#include "Audio/AudioManager.h"
#include "../../../Core/GameEngineDevice/Include/Audio/OpenALAudioDevice.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    AudioHandle handle;
    char filename[256];
    AudioSourceType type;
    OpenAL_ChannelType channel;
    int looping;
    float volume;
    AudioPlaybackState state;
    uint32_t buffer_id;
    int in_use;
} AudioSource;

struct OpenALAudioManager {
    OpenALAudioDevice* device;
    
    /* Current music track */
    char current_music[256];
    AudioHandle current_music_handle;
    
    /* Configuration */
    MusicConfig music_config;
    
    /* Active sources */
    AudioSource* sources;
    uint32_t max_sources;
    uint32_t num_active_sources;
    
    /* Volume levels */
    float master_volume;
    float music_volume;
    float sfx_volume;
    float voice_volume;
    float ambient_volume;
    
    /* Error tracking */
    char last_error[256];
    int error_count;
};

/* Helper to convert AudioSourceType to OpenAL_ChannelType */
static OpenAL_ChannelType audio_source_to_channel(AudioSourceType type) {
    switch (type) {
        case AUDIO_SOURCE_MUSIC:
            return OPENAL_CHANNEL_MUSIC;
        case AUDIO_SOURCE_SFX:
            return OPENAL_CHANNEL_SFX;
        case AUDIO_SOURCE_VOICE:
            return OPENAL_CHANNEL_VOICE;
        case AUDIO_SOURCE_AMBIENT:
            return OPENAL_CHANNEL_AMBIENT;
        default:
            return OPENAL_CHANNEL_SFX;
    }
}

/* Helper to allocate a new handle */
static AudioHandle allocate_audio_handle(void) {
    static AudioHandle next_handle = 1;
    return next_handle++;
}

OpenALAudioManager* AudioManager_Create(void) {
    OpenALAudioManager* manager = (OpenALAudioManager*)malloc(sizeof(AudioManager));
    if (!manager) return NULL;
    
    memset(manager, 0, sizeof(AudioManager));
    
    manager->device = OpenALAudioDevice_Create();
    if (!manager->device) {
        free(manager);
        return NULL;
    }
    
    manager->max_sources = 128;
    manager->sources = (AudioSource*)calloc(manager->max_sources, sizeof(AudioSource));
    if (!manager->sources) {
        OpenALAudioDevice_Destroy(manager->device);
        free(manager);
        return NULL;
    }
    
    /* Default volumes */
    manager->master_volume = 1.0f;
    manager->music_volume = 0.8f;
    manager->sfx_volume = 1.0f;
    manager->voice_volume = 1.0f;
    manager->ambient_volume = 0.5f;
    
    /* Default music config */
    manager->music_config.fade_in_ms = 0;
    manager->music_config.fade_out_ms = 0;
    manager->music_config.cross_fade_ms = 0;
    manager->music_config.master_volume = manager->music_volume;
    
    fprintf(stdout, "Phase 47: AudioManager created\n");
    
    return manager;
}

void AudioManager_Destroy(OpenALAudioManager* manager) {
    if (!manager) return;
    
    AudioManager_StopAll(manager);
    
    if (manager->device) {
        OpenALAudioDevice_Destroy(manager->device);
    }
    
    free(manager->sources);
    free(manager);
    
    fprintf(stdout, "Phase 47: AudioManager destroyed\n");
}

void AudioManager_Initialize(OpenALAudioManager* manager) {
    if (!manager || !manager->device) return;
    
    OpenALAudioDevice_Initialize(manager->device);
    
    /* Set initial listener position at origin */
    OpenAL_Vector3 pos = {0.0f, 0.0f, 0.0f};
    OpenALAudioDevice_SetListenerPosition(manager->device, pos);
    
    OpenAL_Vector3 fwd = {0.0f, 0.0f, -1.0f};
    OpenAL_Vector3 up = {0.0f, 1.0f, 0.0f};
    OpenALAudioDevice_SetListenerOrientation(manager->device, fwd, up);
    
    fprintf(stdout, "Phase 47: AudioManager initialized\n");
}

void AudioManager_Shutdown(OpenALAudioManager* manager) {
    if (!manager || !manager->device) return;
    
    AudioManager_StopAll(manager);
    OpenALAudioDevice_Shutdown(manager->device);
    
    fprintf(stdout, "Phase 47: AudioManager shutdown complete\n");
}

AudioHandle AudioManager_PlayMusic(OpenALAudioManager* manager, const char* filename) {
    if (!manager || !manager->device || !filename) return 0;
    
    /* Stop current music */
    AudioManager_StopMusic(manager);
    
    /* Load the music file */
    uint32_t buffer_id = OpenALAudioDevice_LoadWAV(manager->device, filename);
    if (!buffer_id) {
        snprintf(manager->last_error, sizeof(manager->last_error),
                "Failed to load music file: %s", filename);
        manager->error_count++;
        return 0;
    }
    
    /* Create a music source */
    AudioSourceHandle handle = OpenALAudioDevice_CreateSource(manager->device, OPENAL_CHANNEL_MUSIC);
    if (!handle) {
        snprintf(manager->last_error, sizeof(manager->last_error),
                "Failed to create music source");
        manager->error_count++;
        return 0;
    }
    
    /* Bind buffer to source */
    OpenALAudioDevice_BindBufferToSource(manager->device, handle, buffer_id);
    
    /* Set volume */
    float effective_volume = manager->music_volume * manager->master_volume;
    OpenALAudioDevice_SetSourceVolume(manager->device, handle, effective_volume);
    
    /* Enable looping for background music */
    OpenALAudioDevice_SetSourceLooping(manager->device, handle, 1);
    
    /* Start playback */
    OpenALAudioDevice_Play(manager->device, handle);
    
    /* Track current music */
    manager->current_music_handle = handle;
    strncpy(manager->current_music, filename, sizeof(manager->current_music) - 1);
    
    fprintf(stdout, "Phase 47: Playing music: %s (handle=%u)\n", filename, handle);
    
    return handle;
}

void AudioManager_StopMusic(OpenALAudioManager* manager) {
    if (!manager || !manager->device) return;
    
    if (manager->current_music_handle) {
        OpenALAudioDevice_Stop(manager->device, manager->current_music_handle);
        OpenALAudioDevice_DestroySource(manager->device, manager->current_music_handle);
        manager->current_music_handle = 0;
        manager->current_music[0] = '\0';
        
        fprintf(stdout, "Phase 47: Music stopped\n");
    }
}

void AudioManager_PauseMusic(OpenALAudioManager* manager) {
    if (!manager || !manager->device) return;
    
    if (manager->current_music_handle) {
        OpenALAudioDevice_Pause(manager->device, manager->current_music_handle);
        fprintf(stdout, "Phase 47: Music paused\n");
    }
}

void AudioManager_ResumeMusic(OpenALAudioManager* manager) {
    if (!manager || !manager->device) return;
    
    if (manager->current_music_handle) {
        OpenALAudioDevice_Play(manager->device, manager->current_music_handle);
        fprintf(stdout, "Phase 47: Music resumed\n");
    }
}

void AudioManager_FadeOutMusic(OpenALAudioManager* manager, int duration_ms) {
    if (!manager || !manager->device) return;
    
    /* Phase 47: Fade out not yet implemented
     * For now, just stop immediately
     * TODO: Implement gradual volume reduction over duration_ms
     */
    AudioManager_StopMusic(manager);
}

void AudioManager_CrossFadeToMusic(OpenALAudioManager* manager, const char* filename, int duration_ms) {
    if (!manager || !manager->device || !filename) return;
    
    /* Phase 47: Cross-fade not yet implemented
     * For now, just fade out current and play new
     * TODO: Implement simultaneous fade out/in over duration_ms
     */
    AudioManager_StopMusic(manager);
    AudioManager_PlayMusic(manager, filename);
}

AudioPlaybackState AudioManager_GetMusicState(OpenALAudioManager* manager) {
    if (!manager || !manager->device) return AUDIO_STATE_STOPPED;
    
    if (!manager->current_music_handle) {
        return AUDIO_STATE_STOPPED;
    }
    
    OpenAL_AudioState state = OpenALAudioDevice_GetSourceState(manager->device, 
                                                               manager->current_music_handle);
    
    switch (state) {
        case OPENAL_AUDIO_STATE_PLAYING:
            return AUDIO_STATE_PLAYING;
        case OPENAL_AUDIO_STATE_PAUSED:
            return AUDIO_STATE_PAUSED;
        default:
            return AUDIO_STATE_STOPPED;
    }
}

const char* AudioManager_GetCurrentMusicTrack(OpenALAudioManager* manager) {
    if (!manager) return NULL;
    
    if (manager->current_music[0] == '\0') {
        return NULL;
    }
    
    return manager->current_music;
}

uint32_t AudioManager_GetMusicPlaybackPosition(OpenALAudioManager* manager) {
    if (!manager) return 0;
    
    /* Phase 47: Playback position not yet implemented
     * Would require tracking elapsed time per source
     * TODO: Implement position tracking
     */
    return 0;
}

AudioHandle AudioManager_PlaySFX(OpenALAudioManager* manager, const char* filename) {
    if (!manager || !manager->device || !filename) return 0;
    
    /* Load the SFX file */
    uint32_t buffer_id = OpenALAudioDevice_LoadWAV(manager->device, filename);
    if (!buffer_id) {
        snprintf(manager->last_error, sizeof(manager->last_error),
                "Failed to load SFX file: %s", filename);
        manager->error_count++;
        return 0;
    }
    
    /* Create SFX source */
    AudioSourceHandle handle = OpenALAudioDevice_CreateSource(manager->device, OPENAL_CHANNEL_SFX);
    if (!handle) {
        snprintf(manager->last_error, sizeof(manager->last_error),
                "Failed to create SFX source");
        manager->error_count++;
        return 0;
    }
    
    /* Bind buffer to source */
    OpenALAudioDevice_BindBufferToSource(manager->device, handle, buffer_id);
    
    /* Set volume */
    float effective_volume = manager->sfx_volume * manager->master_volume;
    OpenALAudioDevice_SetSourceVolume(manager->device, handle, effective_volume);
    
    /* Don't loop SFX by default */
    OpenALAudioDevice_SetSourceLooping(manager->device, handle, 0);
    
    /* Start playback */
    OpenALAudioDevice_Play(manager->device, handle);
    
    fprintf(stdout, "Phase 47: Playing SFX: %s (handle=%u)\n", filename, handle);
    
    return handle;
}

void AudioManager_StopSFX(OpenALAudioManager* manager, AudioHandle handle) {
    if (!manager || !manager->device || !handle) return;
    
    OpenALAudioDevice_Stop(manager->device, handle);
    OpenALAudioDevice_DestroySource(manager->device, handle);
}

void AudioManager_PauseSFX(OpenALAudioManager* manager, AudioHandle handle) {
    if (!manager || !manager->device || !handle) return;
    
    OpenALAudioDevice_Pause(manager->device, handle);
}

void AudioManager_ResumeSFX(OpenALAudioManager* manager, AudioHandle handle) {
    if (!manager || !manager->device || !handle) return;
    
    OpenALAudioDevice_Play(manager->device, handle);
}

AudioHandle AudioManager_PlayVoice(OpenALAudioManager* manager, const char* filename) {
    if (!manager || !manager->device || !filename) return 0;
    
    /* Load voice file */
    uint32_t buffer_id = OpenALAudioDevice_LoadWAV(manager->device, filename);
    if (!buffer_id) {
        snprintf(manager->last_error, sizeof(manager->last_error),
                "Failed to load voice file: %s", filename);
        manager->error_count++;
        return 0;
    }
    
    /* Create voice source */
    AudioSourceHandle handle = OpenALAudioDevice_CreateSource(manager->device, OPENAL_CHANNEL_VOICE);
    if (!handle) {
        snprintf(manager->last_error, sizeof(manager->last_error),
                "Failed to create voice source");
        manager->error_count++;
        return 0;
    }
    
    /* Bind buffer to source */
    OpenALAudioDevice_BindBufferToSource(manager->device, handle, buffer_id);
    
    /* Set volume */
    float effective_volume = manager->voice_volume * manager->master_volume;
    OpenALAudioDevice_SetSourceVolume(manager->device, handle, effective_volume);
    
    /* Voice typically doesn't loop */
    OpenALAudioDevice_SetSourceLooping(manager->device, handle, 0);
    
    /* Start playback */
    OpenALAudioDevice_Play(manager->device, handle);
    
    fprintf(stdout, "Phase 47: Playing voice: %s (handle=%u)\n", filename, handle);
    
    return handle;
}

void AudioManager_StopVoice(OpenALAudioManager* manager, AudioHandle handle) {
    if (!manager || !manager->device || !handle) return;
    
    OpenALAudioDevice_Stop(manager->device, handle);
    OpenALAudioDevice_DestroySource(manager->device, handle);
}

void AudioManager_PauseVoice(OpenALAudioManager* manager, AudioHandle handle) {
    if (!manager || !manager->device || !handle) return;
    
    OpenALAudioDevice_Pause(manager->device, handle);
}

void AudioManager_ResumeVoice(OpenALAudioManager* manager, AudioHandle handle) {
    if (!manager || !manager->device || !handle) return;
    
    OpenALAudioDevice_Play(manager->device, handle);
}

AudioHandle AudioManager_PlayAmbient(OpenALAudioManager* manager, const char* filename) {
    if (!manager || !manager->device || !filename) return 0;
    
    /* Load ambient file */
    uint32_t buffer_id = OpenALAudioDevice_LoadWAV(manager->device, filename);
    if (!buffer_id) {
        snprintf(manager->last_error, sizeof(manager->last_error),
                "Failed to load ambient file: %s", filename);
        manager->error_count++;
        return 0;
    }
    
    /* Create ambient source */
    AudioSourceHandle handle = OpenALAudioDevice_CreateSource(manager->device, OPENAL_CHANNEL_AMBIENT);
    if (!handle) {
        snprintf(manager->last_error, sizeof(manager->last_error),
                "Failed to create ambient source");
        manager->error_count++;
        return 0;
    }
    
    /* Bind buffer to source */
    OpenALAudioDevice_BindBufferToSource(manager->device, handle, buffer_id);
    
    /* Set volume */
    float effective_volume = manager->ambient_volume * manager->master_volume;
    OpenALAudioDevice_SetSourceVolume(manager->device, handle, effective_volume);
    
    /* Ambient typically loops */
    OpenALAudioDevice_SetSourceLooping(manager->device, handle, 1);
    
    /* Start playback */
    OpenALAudioDevice_Play(manager->device, handle);
    
    fprintf(stdout, "Phase 47: Playing ambient: %s (handle=%u)\n", filename, handle);
    
    return handle;
}

void AudioManager_StopAmbient(OpenALAudioManager* manager, AudioHandle handle) {
    if (!manager || !manager->device || !handle) return;
    
    OpenALAudioDevice_Stop(manager->device, handle);
    OpenALAudioDevice_DestroySource(manager->device, handle);
}

void AudioManager_PauseAmbient(OpenALAudioManager* manager, AudioHandle handle) {
    if (!manager || !manager->device || !handle) return;
    
    OpenALAudioDevice_Pause(manager->device, handle);
}

void AudioManager_ResumeAmbient(OpenALAudioManager* manager, AudioHandle handle) {
    if (!manager || !manager->device || !handle) return;
    
    OpenALAudioDevice_Play(manager->device, handle);
}

void AudioManager_SetMasterVolume(OpenALAudioManager* manager, float volume) {
    if (!manager || !manager->device) return;
    
    manager->master_volume = volume;
    OpenALAudioDevice_SetMasterVolume(manager->device, volume);
    
    fprintf(stdout, "Phase 47: Master volume set to %.2f\n", volume);
}

float AudioManager_GetMasterVolume(OpenALAudioManager* manager) {
    if (!manager) return 0.0f;
    return manager->master_volume;
}

void AudioManager_SetMusicVolume(OpenALAudioManager* manager, float volume) {
    if (!manager || !manager->device) return;
    
    manager->music_volume = volume;
    OpenALAudioDevice_SetChannelVolume(manager->device, OPENAL_CHANNEL_MUSIC, 
                                       volume * manager->master_volume);
}

float AudioManager_GetMusicVolume(OpenALAudioManager* manager) {
    if (!manager) return 0.0f;
    return manager->music_volume;
}

void AudioManager_SetSFXVolume(OpenALAudioManager* manager, float volume) {
    if (!manager || !manager->device) return;
    
    manager->sfx_volume = volume;
    OpenALAudioDevice_SetChannelVolume(manager->device, OPENAL_CHANNEL_SFX, 
                                       volume * manager->master_volume);
}

float AudioManager_GetSFXVolume(OpenALAudioManager* manager) {
    if (!manager) return 0.0f;
    return manager->sfx_volume;
}

void AudioManager_SetVoiceVolume(OpenALAudioManager* manager, float volume) {
    if (!manager || !manager->device) return;
    
    manager->voice_volume = volume;
    OpenALAudioDevice_SetChannelVolume(manager->device, OPENAL_CHANNEL_VOICE, 
                                       volume * manager->master_volume);
}

float AudioManager_GetVoiceVolume(OpenALAudioManager* manager) {
    if (!manager) return 0.0f;
    return manager->voice_volume;
}

void AudioManager_SetAmbientVolume(OpenALAudioManager* manager, float volume) {
    if (!manager || !manager->device) return;
    
    manager->ambient_volume = volume;
    OpenALAudioDevice_SetChannelVolume(manager->device, OPENAL_CHANNEL_AMBIENT, 
                                       volume * manager->master_volume);
}

float AudioManager_GetAmbientVolume(OpenALAudioManager* manager) {
    if (!manager) return 0.0f;
    return manager->ambient_volume;
}

void AudioManager_SetListenerPosition(OpenALAudioManager* manager, AudioVector3 position) {
    if (!manager || !manager->device) return;
    
    OpenAL_Vector3 pos = {position.x, position.y, position.z};
    OpenALAudioDevice_SetListenerPosition(manager->device, pos);
}

void AudioManager_SetListenerVelocity(OpenALAudioManager* manager, AudioVector3 velocity) {
    if (!manager || !manager->device) return;
    
    OpenAL_Vector3 vel = {velocity.x, velocity.y, velocity.z};
    OpenALAudioDevice_SetListenerVelocity(manager->device, vel);
}

void AudioManager_SetListenerOrientation(OpenALAudioManager* manager, AudioVector3 forward, AudioVector3 up) {
    if (!manager || !manager->device) return;
    
    OpenAL_Vector3 fwd = {forward.x, forward.y, forward.z};
    OpenAL_Vector3 up_vec = {up.x, up.y, up.z};
    OpenALAudioDevice_SetListenerOrientation(manager->device, fwd, up_vec);
}

void AudioManager_SetSourcePosition(OpenALAudioManager* manager, AudioHandle handle, AudioVector3 position) {
    if (!manager || !manager->device || !handle) return;
    
    OpenAL_Vector3 pos = {position.x, position.y, position.z};
    OpenALAudioDevice_SetSourcePosition(manager->device, handle, pos);
}

void AudioManager_SetSourceVelocity(OpenALAudioManager* manager, AudioHandle handle, AudioVector3 velocity) {
    if (!manager || !manager->device || !handle) return;
    
    OpenAL_Vector3 vel = {velocity.x, velocity.y, velocity.z};
    OpenALAudioDevice_SetSourceVelocity(manager->device, handle, vel);
}

void AudioManager_PauseAll(OpenALAudioManager* manager) {
    if (!manager || !manager->device) return;
    
    OpenALAudioDevice_PauseAllSources(manager->device);
}

void AudioManager_ResumeAll(OpenALAudioManager* manager) {
    if (!manager || !manager->device) return;
    
    OpenALAudioDevice_ResumeAllSources(manager->device);
}

void AudioManager_StopAll(OpenALAudioManager* manager) {
    if (!manager || !manager->device) return;
    
    OpenALAudioDevice_StopAllSources(manager->device);
    manager->current_music_handle = 0;
    manager->current_music[0] = '\0';
}

void AudioManager_SetMusicConfig(OpenALAudioManager* manager, MusicConfig* config) {
    if (!manager || !config) return;
    
    memcpy(&manager->music_config, config, sizeof(MusicConfig));
}

MusicConfig* AudioManager_GetMusicConfig(OpenALAudioManager* manager) {
    if (!manager) return NULL;
    
    return &manager->music_config;
}

void AudioManager_Update(OpenALAudioManager* manager, float delta_time) {
    if (!manager || !manager->device) return;
    
    /* Phase 47: Update audio system
     * Currently just calls the device update for any pending operations
     * TODO: Implement fade effects, position interpolation, etc.
     */
    OpenALAudioDevice_Update(manager->device, delta_time);
}

int AudioManager_GetAudioFileInfo(OpenALAudioManager* manager, const char* filename, AudioFileInfo* info) {
    if (!manager || !filename || !info) return 0;
    
    /* Phase 47: Audio file info not yet implemented
     * TODO: Parse WAV headers to get duration, sample rate, etc.
     */
    return 0;
}

const char* AudioManager_GetLastError(OpenALAudioManager* manager) {
    if (!manager) return NULL;
    
    if (manager->last_error[0] == '\0') {
        return NULL;
    }
    
    return manager->last_error;
}

int AudioManager_ClearErrors(OpenALAudioManager* manager) {
    if (!manager) return 0;
    
    int count = manager->error_count;
    manager->last_error[0] = '\0';
    manager->error_count = 0;
    
    return count;
}

void AudioManager_PrintStats(OpenALAudioManager* manager) {
    if (!manager || !manager->device) return;
    
    fprintf(stdout, "\n=== Audio Manager Statistics ===\n");
    fprintf(stdout, "Master Volume: %.2f\n", manager->master_volume);
    fprintf(stdout, "Music Volume: %.2f\n", manager->music_volume);
    fprintf(stdout, "SFX Volume: %.2f\n", manager->sfx_volume);
    fprintf(stdout, "Voice Volume: %.2f\n", manager->voice_volume);
    fprintf(stdout, "Ambient Volume: %.2f\n", manager->ambient_volume);
    fprintf(stdout, "Current Music: %s\n", 
            manager->current_music[0] ? manager->current_music : "None");
    fprintf(stdout, "Active Sources: %u / %u\n", 
            manager->num_active_sources, manager->max_sources);
    fprintf(stdout, "Error Count: %d\n", manager->error_count);
    fprintf(stdout, "================================\n\n");
    
    OpenALAudioDevice_PrintDeviceInfo(manager->device);
}

void AudioManager_PrintSourceInfo(OpenALAudioManager* manager, AudioHandle handle) {
    if (!manager || !manager->device || !handle) return;
    
    fprintf(stdout, "Source %u info:\n", handle);
    OpenALAudioDevice_PrintSourceInfo(manager->device, handle);
}
