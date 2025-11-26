#pragma once

#include <cstdint>
#include <cstddef>

/* Phase 32: OpenAL Audio Device
 * 
 * Provides cross-platform audio backend using OpenAL for:
 * - WAV/MP3 file loading from .big archives
 * - 3D positional audio
 * - Music/SFX/voice channel mixing
 * - Audio effect support
 * - Device/context management
 */

typedef struct OpenALAudioDevice OpenALAudioDevice;
typedef uint32_t AudioSourceHandle;

#define OPENAL_MAX_SOURCES 256
#define OPENAL_MUSIC_CHANNEL 0
#define OPENAL_SFX_CHANNEL_START 1
#define OPENAL_VOICE_CHANNEL_START 64
#define OPENAL_MUSIC_PRIORITY 100
#define OPENAL_VOICE_PRIORITY 50
#define OPENAL_SFX_PRIORITY 10

/* Audio source handle ranges (Phase 32: 32000-32999) */
#define OPENAL_HANDLE_MUSIC_MIN 32000
#define OPENAL_HANDLE_MUSIC_MAX 32099
#define OPENAL_HANDLE_SFX_MIN 32100
#define OPENAL_HANDLE_SFX_MAX 32199
#define OPENAL_HANDLE_VOICE_MIN 32200
#define OPENAL_HANDLE_VOICE_MAX 32255
#define OPENAL_HANDLE_AMBIENT_MIN 32256
#define OPENAL_HANDLE_AMBIENT_MAX 32999

typedef enum {
    OPENAL_AUDIO_FORMAT_MONO8 = 0,
    OPENAL_AUDIO_FORMAT_MONO16 = 1,
    OPENAL_AUDIO_FORMAT_STEREO8 = 2,
    OPENAL_AUDIO_FORMAT_STEREO16 = 3,
} OpenAL_AudioFormat;

typedef enum {
    OPENAL_AUDIO_STATE_INITIAL = 0,
    OPENAL_AUDIO_STATE_PLAYING = 1,
    OPENAL_AUDIO_STATE_PAUSED = 2,
    OPENAL_AUDIO_STATE_STOPPED = 3,
} OpenAL_AudioState;

typedef enum {
    OPENAL_CHANNEL_MUSIC = 0,
    OPENAL_CHANNEL_SFX = 1,
    OPENAL_CHANNEL_VOICE = 2,
    OPENAL_CHANNEL_AMBIENT = 3,
} OpenAL_ChannelType;

typedef struct {
    uint32_t sample_rate;
    uint32_t num_samples;
    uint16_t channels;
    uint16_t bits_per_sample;
    OpenAL_AudioFormat format;
} OpenAL_AudioInfo;

typedef struct {
    float x, y, z;
} OpenAL_Vector3;

typedef struct {
    float x, y, z;
    float vx, vy, vz;
} OpenAL_Vector3WithVelocity;

/* Device Management */
OpenALAudioDevice* OpenALAudioDevice_Create(void);
void OpenALAudioDevice_Destroy(OpenALAudioDevice* device);
void OpenALAudioDevice_Initialize(OpenALAudioDevice* device);
void OpenALAudioDevice_Shutdown(OpenALAudioDevice* device);

/* Listener/3D Audio Setup */
void OpenALAudioDevice_SetListenerPosition(OpenALAudioDevice* device, OpenAL_Vector3 position);
void OpenALAudioDevice_SetListenerVelocity(OpenALAudioDevice* device, OpenAL_Vector3 velocity);
void OpenALAudioDevice_SetListenerOrientation(OpenALAudioDevice* device, 
                                              OpenAL_Vector3 forward, 
                                              OpenAL_Vector3 up);

/* Audio Buffer Management */
uint32_t OpenALAudioDevice_CreateBuffer(OpenALAudioDevice* device, 
                                        const void* data, 
                                        size_t data_size, 
                                        OpenAL_AudioInfo info);
void OpenALAudioDevice_DestroyBuffer(OpenALAudioDevice* device, uint32_t buffer_id);
uint32_t OpenALAudioDevice_LoadWAV(OpenALAudioDevice* device, const char* filepath);
uint32_t OpenALAudioDevice_LoadWAVFromMemory(OpenALAudioDevice* device, 
                                             const void* data, 
                                             size_t data_size);

/* Source/Playback Management */
AudioSourceHandle OpenALAudioDevice_CreateSource(OpenALAudioDevice* device, 
                                                 OpenAL_ChannelType channel);
void OpenALAudioDevice_DestroySource(OpenALAudioDevice* device, AudioSourceHandle handle);
void OpenALAudioDevice_BindBufferToSource(OpenALAudioDevice* device, 
                                          AudioSourceHandle handle, 
                                          uint32_t buffer_id);

/* Playback Control */
void OpenALAudioDevice_Play(OpenALAudioDevice* device, AudioSourceHandle handle);
void OpenALAudioDevice_Pause(OpenALAudioDevice* device, AudioSourceHandle handle);
void OpenALAudioDevice_Stop(OpenALAudioDevice* device, AudioSourceHandle handle);
void OpenALAudioDevice_Rewind(OpenALAudioDevice* device, AudioSourceHandle handle);

/* Source Properties */
void OpenALAudioDevice_SetSourcePosition(OpenALAudioDevice* device, 
                                         AudioSourceHandle handle, 
                                         OpenAL_Vector3 position);
void OpenALAudioDevice_SetSourceVelocity(OpenALAudioDevice* device, 
                                         AudioSourceHandle handle, 
                                         OpenAL_Vector3 velocity);
void OpenALAudioDevice_SetSourceVolume(OpenALAudioDevice* device, 
                                       AudioSourceHandle handle, 
                                       float volume);
void OpenALAudioDevice_SetSourcePitch(OpenALAudioDevice* device, 
                                      AudioSourceHandle handle, 
                                      float pitch);
void OpenALAudioDevice_SetSourceLooping(OpenALAudioDevice* device, 
                                        AudioSourceHandle handle, 
                                        int looping);
void OpenALAudioDevice_SetSourceReferenceDistance(OpenALAudioDevice* device, 
                                                  AudioSourceHandle handle, 
                                                  float distance);
void OpenALAudioDevice_SetSourceMaxDistance(OpenALAudioDevice* device, 
                                            AudioSourceHandle handle, 
                                            float distance);
void OpenALAudioDevice_SetSourceRolloffFactor(OpenALAudioDevice* device, 
                                              AudioSourceHandle handle, 
                                              float factor);

/* Channel Volume Control */
void OpenALAudioDevice_SetChannelVolume(OpenALAudioDevice* device, 
                                        OpenAL_ChannelType channel, 
                                        float volume);
float OpenALAudioDevice_GetChannelVolume(OpenALAudioDevice* device, 
                                         OpenAL_ChannelType channel);

/* Source Query */
OpenAL_AudioState OpenALAudioDevice_GetSourceState(OpenALAudioDevice* device, 
                                                    AudioSourceHandle handle);
float OpenALAudioDevice_GetSourceVolume(OpenALAudioDevice* device, 
                                        AudioSourceHandle handle);
float OpenALAudioDevice_GetSourcePitch(OpenALAudioDevice* device, 
                                       AudioSourceHandle handle);
uint32_t OpenALAudioDevice_GetSourceBufferId(OpenALAudioDevice* device, 
                                             AudioSourceHandle handle);

/* Audio Effects (EFX Extension) */
int OpenALAudioDevice_HasEFXSupport(OpenALAudioDevice* device);
uint32_t OpenALAudioDevice_CreateEffect(OpenALAudioDevice* device, int effect_type);
void OpenALAudioDevice_DestroyEffect(OpenALAudioDevice* device, uint32_t effect_id);
void OpenALAudioDevice_AttachEffectToSource(OpenALAudioDevice* device, 
                                            AudioSourceHandle handle, 
                                            uint32_t effect_id);
void OpenALAudioDevice_SetEffectParameter(OpenALAudioDevice* device, 
                                          uint32_t effect_id, 
                                          int param_id, 
                                          float value);

/* Master Volume */
void OpenALAudioDevice_SetMasterVolume(OpenALAudioDevice* device, float volume);
float OpenALAudioDevice_GetMasterVolume(OpenALAudioDevice* device);

/* Batch Operations */
void OpenALAudioDevice_StopAllSources(OpenALAudioDevice* device);
void OpenALAudioDevice_PauseAllSources(OpenALAudioDevice* device);
void OpenALAudioDevice_ResumeAllSources(OpenALAudioDevice* device);
void OpenALAudioDevice_Update(OpenALAudioDevice* device, float delta_time);

/* Error Handling */
const char* OpenALAudioDevice_GetLastError(OpenALAudioDevice* device);
int OpenALAudioDevice_ClearErrors(OpenALAudioDevice* device);

/* Debug Info */
void OpenALAudioDevice_PrintDeviceInfo(OpenALAudioDevice* device);
void OpenALAudioDevice_PrintSourceInfo(OpenALAudioDevice* device, AudioSourceHandle handle);
uint32_t OpenALAudioDevice_GetActiveSourceCount(OpenALAudioDevice* device);
