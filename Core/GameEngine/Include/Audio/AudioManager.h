/*
 * Phase 47: Audio Manager
 * 
 * High-level audio management API for game engine
 * Handles music playback, sound effects, voice, and ambient audio
 * Built on top of OpenALAudioDevice for cross-platform support
 */

#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <stdint.h>
#include <stddef.h>

typedef struct OpenALAudioManager OpenALAudioManager;
typedef uint32_t AudioHandle;

/* Audio priority levels (manager-specific, distinct from AudioEventRTS) */
typedef enum {
    AUDIO_PRIORITY_MUSIC = 100,
    AUDIO_PRIORITY_VOICE = 50,
    AUDIO_PRIORITY_SFX = 10,
    AUDIO_PRIORITY_AMBIENT = 1,
} AudioManagerPriority;

/* Audio source types */
typedef enum {
    AUDIO_SOURCE_MUSIC = 0,
    AUDIO_SOURCE_SFX = 1,
    AUDIO_SOURCE_VOICE = 2,
    AUDIO_SOURCE_AMBIENT = 3,
} AudioSourceType;

/* Audio file format */
typedef enum {
    AUDIO_FORMAT_WAV = 0,
    AUDIO_FORMAT_MP3 = 1,
    AUDIO_FORMAT_OGG = 2,
} AudioFormat;

/* Audio playback state */
typedef enum {
    AUDIO_STATE_STOPPED = 0,
    AUDIO_STATE_PLAYING = 1,
    AUDIO_STATE_PAUSED = 2,
} AudioPlaybackState;

/* Audio file information */
typedef struct {
    const char* filename;      /* Audio file path */
    AudioFormat format;        /* File format (WAV, MP3, etc) */
    uint32_t duration_ms;      /* Duration in milliseconds */
    uint32_t sample_rate;      /* Sample rate (Hz) */
    uint16_t channels;         /* Number of channels (1=mono, 2=stereo) */
    uint16_t bits_per_sample;  /* Bits per sample (8, 16, 24, 32) */
    int looping;               /* Whether audio loops */
    float volume;              /* Playback volume (0.0 - 1.0) */
} AudioFileInfo;

/* Music configuration */
typedef struct {
    int fade_in_ms;            /* Fade in duration in milliseconds */
    int fade_out_ms;           /* Fade out duration in milliseconds */
    int cross_fade_ms;         /* Cross-fade duration between tracks */
    float master_volume;       /* Master music volume */
} MusicConfig;

/* Audio manager creation and destruction */
OpenALAudioManager* AudioManager_Create(void);
void AudioManager_Destroy(OpenALAudioManager* manager);

/* Initialization and shutdown */
void AudioManager_Initialize(OpenALAudioManager* manager);
void AudioManager_Shutdown(OpenALAudioManager* manager);

/* Music playback */
AudioHandle AudioManager_PlayMusic(OpenALAudioManager* manager, const char* filename);
void AudioManager_StopMusic(OpenALAudioManager* manager);
void AudioManager_PauseMusic(OpenALAudioManager* manager);
void AudioManager_ResumeMusic(OpenALAudioManager* manager);
void AudioManager_FadeOutMusic(OpenALAudioManager* manager, int duration_ms);
void AudioManager_CrossFadeToMusic(OpenALAudioManager* manager, const char* filename, int duration_ms);

/* Music queries */
AudioPlaybackState AudioManager_GetMusicState(OpenALAudioManager* manager);
const char* AudioManager_GetCurrentMusicTrack(OpenALAudioManager* manager);
uint32_t AudioManager_GetMusicPlaybackPosition(OpenALAudioManager* manager);

/* Sound effects */
AudioHandle AudioManager_PlaySFX(OpenALAudioManager* manager, const char* filename);
void AudioManager_StopSFX(OpenALAudioManager* manager, AudioHandle handle);
void AudioManager_PauseSFX(OpenALAudioManager* manager, AudioHandle handle);
void AudioManager_ResumeSFX(OpenALAudioManager* manager, AudioHandle handle);

/* Voice/dialogue */
AudioHandle AudioManager_PlayVoice(OpenALAudioManager* manager, const char* filename);
void AudioManager_StopVoice(OpenALAudioManager* manager, AudioHandle handle);
void AudioManager_PauseVoice(OpenALAudioManager* manager, AudioHandle handle);
void AudioManager_ResumeVoice(OpenALAudioManager* manager, AudioHandle handle);

/* Ambient audio */
AudioHandle AudioManager_PlayAmbient(OpenALAudioManager* manager, const char* filename);
void AudioManager_StopAmbient(OpenALAudioManager* manager, AudioHandle handle);
void AudioManager_PauseAmbient(OpenALAudioManager* manager, AudioHandle handle);
void AudioManager_ResumeAmbient(OpenALAudioManager* manager, AudioHandle handle);

/* Volume control */
void AudioManager_SetMasterVolume(OpenALAudioManager* manager, float volume);
float AudioManager_GetMasterVolume(OpenALAudioManager* manager);

void AudioManager_SetMusicVolume(OpenALAudioManager* manager, float volume);
float AudioManager_GetMusicVolume(OpenALAudioManager* manager);

void AudioManager_SetSFXVolume(OpenALAudioManager* manager, float volume);
float AudioManager_GetSFXVolume(OpenALAudioManager* manager);

void AudioManager_SetVoiceVolume(OpenALAudioManager* manager, float volume);
float AudioManager_GetVoiceVolume(OpenALAudioManager* manager);

void AudioManager_SetAmbientVolume(OpenALAudioManager* manager, float volume);
float AudioManager_GetAmbientVolume(OpenALAudioManager* manager);

/* 3D audio positioning */
typedef struct {
    float x, y, z;
} AudioVector3;

void AudioManager_SetListenerPosition(OpenALAudioManager* manager, AudioVector3 position);
void AudioManager_SetListenerVelocity(OpenALAudioManager* manager, AudioVector3 velocity);
void AudioManager_SetListenerOrientation(OpenALAudioManager* manager, AudioVector3 forward, AudioVector3 up);

void AudioManager_SetSourcePosition(OpenALAudioManager* manager, AudioHandle handle, AudioVector3 position);
void AudioManager_SetSourceVelocity(OpenALAudioManager* manager, AudioHandle handle, AudioVector3 velocity);

/* Batch operations */
void AudioManager_PauseAll(OpenALAudioManager* manager);
void AudioManager_ResumeAll(OpenALAudioManager* manager);
void AudioManager_StopAll(OpenALAudioManager* manager);

/* Configuration */
void AudioManager_SetMusicConfig(OpenALAudioManager* manager, MusicConfig* config);
MusicConfig* AudioManager_GetMusicConfig(OpenALAudioManager* manager);

/* Update (call regularly, e.g., once per frame) */
void AudioManager_Update(OpenALAudioManager* manager, float delta_time);

/* Audio file information */
int AudioManager_GetAudioFileInfo(OpenALAudioManager* manager, const char* filename, AudioFileInfo* info);

/* Error handling */
const char* AudioManager_GetLastError(OpenALAudioManager* manager);
int AudioManager_ClearErrors(OpenALAudioManager* manager);

/* Debug information */
void AudioManager_PrintStats(OpenALAudioManager* manager);
void AudioManager_PrintSourceInfo(OpenALAudioManager* manager, AudioHandle handle);

#endif /* AUDIOMANAGER_H */
