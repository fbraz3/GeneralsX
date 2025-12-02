#include "Audio/OpenALAudioDevice.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

typedef struct {
  uint32_t buffer_id;
  OpenAL_AudioInfo info;
} OpenAL_Buffer;

typedef struct {
  uint32_t source_id;
  AudioSourceHandle handle;
  OpenAL_ChannelType channel;
  uint32_t current_buffer;
  float volume;
  float pitch;
  int looping;
  OpenAL_AudioState state;
  int in_use;
} OpenAL_Source;

struct OpenALAudioDevice {
  ALCdevice* device;
  ALCcontext* context;

  /* Buffers */
  OpenAL_Buffer* buffers;
  uint32_t max_buffers;
  uint32_t num_buffers;

  /* Sources */
  OpenAL_Source* sources;
  uint32_t max_sources;
  uint32_t num_active_sources;

  /* Channel volumes */
  float channel_volumes[4];
  float master_volume;

  /* Listener state */
  OpenAL_Vector3 listener_position;
  OpenAL_Vector3 listener_velocity;

  /* Error handling */
  char last_error[256];
  int error_count;
};

static void openal_check_error(const char* function_name) {
  ALenum error = alGetError();
  if (error != AL_NO_ERROR) {
    fprintf(stderr, "[OpenAL Error in %s] %d\n", function_name, error);
  }
}

static const char* openal_error_string(ALenum error) {
  switch (error) {
  case AL_NO_ERROR: return "AL_NO_ERROR";
  case AL_INVALID_NAME: return "AL_INVALID_NAME";
  case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
  case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
  case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
  case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";
  default: return "UNKNOWN_ERROR";
  }
}

static AudioSourceHandle openal_allocate_handle(OpenAL_ChannelType channel) {
  switch (channel) {
  case OPENAL_CHANNEL_MUSIC:
    return OPENAL_HANDLE_MUSIC_MIN + (rand() % (OPENAL_HANDLE_MUSIC_MAX - OPENAL_HANDLE_MUSIC_MIN));
  case OPENAL_CHANNEL_SFX:
    return OPENAL_HANDLE_SFX_MIN + (rand() % (OPENAL_HANDLE_SFX_MAX - OPENAL_HANDLE_SFX_MIN));
  case OPENAL_CHANNEL_VOICE:
    return OPENAL_HANDLE_VOICE_MIN + (rand() % (OPENAL_HANDLE_VOICE_MAX - OPENAL_HANDLE_VOICE_MIN));
  case OPENAL_CHANNEL_AMBIENT:
  default:
    return OPENAL_HANDLE_AMBIENT_MIN + (rand() % (OPENAL_HANDLE_AMBIENT_MAX - OPENAL_HANDLE_AMBIENT_MIN));
  }
}

OpenALAudioDevice* OpenALAudioDevice_Create(void) {
  OpenALAudioDevice* device = (OpenALAudioDevice*)malloc(sizeof(OpenALAudioDevice));
  if (!device) return NULL;

  memset(device, 0, sizeof(OpenALAudioDevice));

  device->max_buffers = 512;
  device->max_sources = OPENAL_MAX_SOURCES;
  device->master_volume = 1.0f;

  device->buffers = (OpenAL_Buffer*)calloc(device->max_buffers, sizeof(OpenAL_Buffer));
  device->sources = (OpenAL_Source*)calloc(device->max_sources, sizeof(OpenAL_Source));

  if (!device->buffers || !device->sources) {
    free(device->buffers);
    free(device->sources);
    free(device);
    return NULL;
  }

  /* Initialize channel volumes */
  device->channel_volumes[OPENAL_CHANNEL_MUSIC] = 0.8f;
  device->channel_volumes[OPENAL_CHANNEL_SFX] = 1.0f;
  device->channel_volumes[OPENAL_CHANNEL_VOICE] = 1.0f;
  device->channel_volumes[OPENAL_CHANNEL_AMBIENT] = 0.5f;

  return device;
}

void OpenALAudioDevice_Destroy(OpenALAudioDevice* device) {
  if (!device) return;

  if (device->context) {
    alcMakeContextCurrent(NULL);
    alcDestroyContext(device->context);
  }

  if (device->device) {
    alcCloseDevice(device->device);
  }

  free(device->buffers);
  free(device->sources);
  free(device);
}

void OpenALAudioDevice_Initialize(OpenALAudioDevice* device) {
  if (!device || !(device->device)) {
    return;
  }
  /* Open default audio device */
  device->device = alcOpenDevice(NULL);
  if (!device->device) {
    snprintf(device->last_error, sizeof(device->last_error), "Failed to open OpenAL device");
    device->error_count++;
    return;
  }

  /* Create audio context */
  device->context = alcCreateContext(device->device, NULL);
  if (!device->context) {
    snprintf(device->last_error, sizeof(device->last_error), "Failed to create OpenAL context");
    device->error_count++;
    alcCloseDevice(device->device);
    device->device = NULL;
    return;
  }

  alcMakeContextCurrent(device->context);

  /* Set listener properties */
  alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
  alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);

  ALfloat listener_orientation[] = {
      0.0f, 0.0f, -1.0f,  /* forward */
      0.0f, 1.0f, 0.0f    /* up */
  };
  alListenerfv(AL_ORIENTATION, listener_orientation);

  alListenerf(AL_GAIN, device->master_volume);

  openal_check_error("OpenALAudioDevice_Initialize");

  fprintf(stdout, "Phase 32: OpenAL device initialized\n");
  fprintf(stdout, "Device: %s\n", alcGetString(device->device, ALC_DEVICE_SPECIFIER));
}

void OpenALAudioDevice_Shutdown(OpenALAudioDevice* device) {
  if (!device) return;

  /* Stop all sources */
  OpenALAudioDevice_StopAllSources(device);

  /* Delete sources */
  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].in_use && device->sources[i].source_id) {
      alDeleteSources(1, &device->sources[i].source_id);
    }
  }

  /* Delete buffers */
  for (uint32_t i = 0; i < device->num_buffers; i++) {
    if (device->buffers[i].buffer_id) {
      alDeleteBuffers(1, &device->buffers[i].buffer_id);
    }
  }

  fprintf(stdout, "Phase 47: OpenAL device shutdown\n");
}

void OpenALAudioDevice_SetListenerPosition(OpenALAudioDevice* device, OpenAL_Vector3 position) {
  if (!device) return;

  device->listener_position = position;
  alListener3f(AL_POSITION, position.x, position.y, position.z);
}

void OpenALAudioDevice_SetListenerVelocity(OpenALAudioDevice* device, OpenAL_Vector3 velocity) {
  if (!device) return;

  device->listener_velocity = velocity;
  alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void OpenALAudioDevice_SetListenerOrientation(OpenALAudioDevice* device,
  OpenAL_Vector3 forward,
  OpenAL_Vector3 up) {
  if (!device) return;

  ALfloat orientation[] = {
      forward.x, forward.y, forward.z,
      up.x, up.y, up.z
  };
  alListenerfv(AL_ORIENTATION, orientation);
}

uint32_t OpenALAudioDevice_CreateBuffer(OpenALAudioDevice* device,
  const void* data,
  size_t data_size,
  OpenAL_AudioInfo info) {
  if (!device || !data || device->num_buffers >= device->max_buffers) return 0;

  ALuint buffer_id;
  alGenBuffers(1, &buffer_id);

  ALenum format;
  switch (info.format) {
  case OPENAL_AUDIO_FORMAT_MONO8: format = AL_FORMAT_MONO8; break;
  case OPENAL_AUDIO_FORMAT_MONO16: format = AL_FORMAT_MONO16; break;
  case OPENAL_AUDIO_FORMAT_STEREO8: format = AL_FORMAT_STEREO8; break;
  case OPENAL_AUDIO_FORMAT_STEREO16: format = AL_FORMAT_STEREO16; break;
  default: format = AL_FORMAT_MONO16;
  }

  alBufferData(buffer_id, format, data, (ALsizei)data_size, info.sample_rate);

  device->buffers[device->num_buffers].buffer_id = buffer_id;
  device->buffers[device->num_buffers].info = info;
  device->num_buffers++;

  return buffer_id;
}

void OpenALAudioDevice_DestroyBuffer(OpenALAudioDevice* device, uint32_t buffer_id) {
  if (!device || !buffer_id) return;

  for (uint32_t i = 0; i < device->num_buffers; i++) {
    if (device->buffers[i].buffer_id == buffer_id) {
      alDeleteBuffers(1, &buffer_id);

      /* Shift remaining buffers */
      if (i < device->num_buffers - 1) {
        memmove(&device->buffers[i], &device->buffers[i + 1],
          (device->num_buffers - i - 1) * sizeof(OpenAL_Buffer));
      }
      device->num_buffers--;
      break;
    }
  }
}

uint32_t OpenALAudioDevice_LoadWAV(OpenALAudioDevice* device, const char* filepath) {
  if (!device || !filepath) return 0;

  FILE* file = fopen(filepath, "rb");
  if (!file) {
    snprintf(device->last_error, sizeof(device->last_error), "Failed to open WAV file: %s", filepath);
    device->error_count++;
    return 0;
  }

  /* Read WAV header */
  uint32_t chunk_id, chunk_size, format, subchunk1_id, subchunk1_size;
  uint16_t audio_format, channels, block_align, bits_per_sample;
  uint32_t sample_rate, byte_rate, subchunk2_id, subchunk2_size;

  fread(&chunk_id, 4, 1, file);
  fread(&chunk_size, 4, 1, file);
  fread(&format, 4, 1, file);

  fread(&subchunk1_id, 4, 1, file);
  fread(&subchunk1_size, 4, 1, file);
  fread(&audio_format, 2, 1, file);
  fread(&channels, 2, 1, file);
  fread(&sample_rate, 4, 1, file);
  fread(&byte_rate, 4, 1, file);
  fread(&block_align, 2, 1, file);
  fread(&bits_per_sample, 2, 1, file);

  fread(&subchunk2_id, 4, 1, file);
  fread(&subchunk2_size, 4, 1, file);

  /* Read audio data */
  void* audio_data = malloc(subchunk2_size);
  if (!audio_data) {
    fclose(file);
    return 0;
  }

  fread(audio_data, 1, subchunk2_size, file);
  fclose(file);

  /* Determine format */
  OpenAL_AudioFormat format_enum;
  if (channels == 1) {
    format_enum = (bits_per_sample == 8) ? OPENAL_AUDIO_FORMAT_MONO8 : OPENAL_AUDIO_FORMAT_MONO16;
  }
  else {
    format_enum = (bits_per_sample == 8) ? OPENAL_AUDIO_FORMAT_STEREO8 : OPENAL_AUDIO_FORMAT_STEREO16;
  }

  OpenAL_AudioInfo info = {
      .sample_rate = sample_rate,
      .num_samples = subchunk2_size / (bits_per_sample / 8),
      .channels = channels,
      .bits_per_sample = bits_per_sample,
      .format = format_enum
  };

  uint32_t buffer_id = OpenALAudioDevice_CreateBuffer(device, audio_data, subchunk2_size, info);

  free(audio_data);
  return buffer_id;
}

uint32_t OpenALAudioDevice_LoadWAVFromMemory(OpenALAudioDevice* device,
  const void* data,
  size_t data_size) {
  if (!device || !data || data_size < 44) return 0;

  const uint8_t* buffer = (const uint8_t*)data;

  /* Parse WAV header */
  uint16_t channels = *(uint16_t*)(buffer + 8);
  uint32_t sample_rate = *(uint32_t*)(buffer + 12);
  uint16_t bits_per_sample = *(uint16_t*)(buffer + 22);
  uint32_t audio_data_size = *(uint32_t*)(buffer + 40);

  const void* audio_data = buffer + 44;

  OpenAL_AudioFormat format_enum;
  if (channels == 1) {
    format_enum = (bits_per_sample == 8) ? OPENAL_AUDIO_FORMAT_MONO8 : OPENAL_AUDIO_FORMAT_MONO16;
  }
  else {
    format_enum = (bits_per_sample == 8) ? OPENAL_AUDIO_FORMAT_STEREO8 : OPENAL_AUDIO_FORMAT_STEREO16;
  }

  OpenAL_AudioInfo info = {
      .sample_rate = sample_rate,
      .num_samples = audio_data_size / (bits_per_sample / 8),
      .channels = channels,
      .bits_per_sample = bits_per_sample,
      .format = format_enum
  };

  return OpenALAudioDevice_CreateBuffer(device, audio_data, audio_data_size, info);
}

AudioSourceHandle OpenALAudioDevice_CreateSource(OpenALAudioDevice* device,
  OpenAL_ChannelType channel) {
  if (!device || device->num_active_sources >= device->max_sources) return 0;

  /* Find available source slot */
  OpenAL_Source* source = NULL;
  uint32_t source_index = device->num_active_sources;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (!device->sources[i].in_use) {
      source = &device->sources[i];
      source_index = i;
      break;
    }
  }

  if (!source) return 0;

  /* Create OpenAL source */
  ALuint al_source;
  alGenSources(1, &al_source);

  /* Initialize source */
  AudioSourceHandle handle = openal_allocate_handle(channel);

  source->source_id = al_source;
  source->handle = handle;
  source->channel = channel;
  source->volume = 1.0f;
  source->pitch = 1.0f;
  source->looping = 0;
  source->state = OPENAL_AUDIO_STATE_INITIAL;
  source->in_use = 1;
  source->current_buffer = 0;

  device->num_active_sources++;

  /* Set initial properties */
  alSourcef(al_source, AL_GAIN, source->volume * device->channel_volumes[channel] * device->master_volume);
  alSourcef(al_source, AL_PITCH, source->pitch);
  alSourcei(al_source, AL_LOOPING, source->looping);

  return handle;
}

void OpenALAudioDevice_DestroySource(OpenALAudioDevice* device, AudioSourceHandle handle) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      alDeleteSources(1, &device->sources[i].source_id);
      memset(&device->sources[i], 0, sizeof(OpenAL_Source));
      device->num_active_sources--;
      break;
    }
  }
}

void OpenALAudioDevice_BindBufferToSource(OpenALAudioDevice* device,
  AudioSourceHandle handle,
  uint32_t buffer_id) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      alSourcei(device->sources[i].source_id, AL_BUFFER, buffer_id);
      device->sources[i].current_buffer = buffer_id;
      break;
    }
  }
}

void OpenALAudioDevice_Play(OpenALAudioDevice* device, AudioSourceHandle handle) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      alSourcePlay(device->sources[i].source_id);
      device->sources[i].state = OPENAL_AUDIO_STATE_PLAYING;
      break;
    }
  }
}

void OpenALAudioDevice_Pause(OpenALAudioDevice* device, AudioSourceHandle handle) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      alSourcePause(device->sources[i].source_id);
      device->sources[i].state = OPENAL_AUDIO_STATE_PAUSED;
      break;
    }
  }
}

void OpenALAudioDevice_Stop(OpenALAudioDevice* device, AudioSourceHandle handle) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      alSourceStop(device->sources[i].source_id);
      device->sources[i].state = OPENAL_AUDIO_STATE_STOPPED;
      break;
    }
  }
}

void OpenALAudioDevice_Rewind(OpenALAudioDevice* device, AudioSourceHandle handle) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      alSourceRewind(device->sources[i].source_id);
      break;
    }
  }
}

void OpenALAudioDevice_SetSourcePosition(OpenALAudioDevice* device,
  AudioSourceHandle handle,
  OpenAL_Vector3 position) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      alSource3f(device->sources[i].source_id, AL_POSITION, position.x, position.y, position.z);
      break;
    }
  }
}

void OpenALAudioDevice_SetSourceVelocity(OpenALAudioDevice* device,
  AudioSourceHandle handle,
  OpenAL_Vector3 velocity) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      alSource3f(device->sources[i].source_id, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
      break;
    }
  }
}

void OpenALAudioDevice_SetSourceVolume(OpenALAudioDevice* device,
  AudioSourceHandle handle,
  float volume) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      device->sources[i].volume = volume;
      float effective_volume = volume * device->channel_volumes[device->sources[i].channel] * device->master_volume;
      alSourcef(device->sources[i].source_id, AL_GAIN, effective_volume);
      break;
    }
  }
}

void OpenALAudioDevice_SetSourcePitch(OpenALAudioDevice* device,
  AudioSourceHandle handle,
  float pitch) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      device->sources[i].pitch = pitch;
      alSourcef(device->sources[i].source_id, AL_PITCH, pitch);
      break;
    }
  }
}

void OpenALAudioDevice_SetSourceLooping(OpenALAudioDevice* device,
  AudioSourceHandle handle,
  int looping) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      device->sources[i].looping = looping;
      alSourcei(device->sources[i].source_id, AL_LOOPING, looping);
      break;
    }
  }
}

void OpenALAudioDevice_SetSourceReferenceDistance(OpenALAudioDevice* device,
  AudioSourceHandle handle,
  float distance) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      alSourcef(device->sources[i].source_id, AL_REFERENCE_DISTANCE, distance);
      break;
    }
  }
}

void OpenALAudioDevice_SetSourceMaxDistance(OpenALAudioDevice* device,
  AudioSourceHandle handle,
  float distance) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      alSourcef(device->sources[i].source_id, AL_MAX_DISTANCE, distance);
      break;
    }
  }
}

void OpenALAudioDevice_SetSourceRolloffFactor(OpenALAudioDevice* device,
  AudioSourceHandle handle,
  float factor) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      alSourcef(device->sources[i].source_id, AL_ROLLOFF_FACTOR, factor);
      break;
    }
  }
}

void OpenALAudioDevice_SetChannelVolume(OpenALAudioDevice* device,
  OpenAL_ChannelType channel,
  float volume) {
  if (!device || channel >= 4) return;

  device->channel_volumes[channel] = volume;

  /* Update all sources in this channel */
  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].in_use && device->sources[i].channel == channel) {
      float effective_volume = device->sources[i].volume * volume * device->master_volume;
      alSourcef(device->sources[i].source_id, AL_GAIN, effective_volume);
    }
  }
}

float OpenALAudioDevice_GetChannelVolume(OpenALAudioDevice* device,
  OpenAL_ChannelType channel) {
  if (!device || channel >= 4) return 0.0f;
  return device->channel_volumes[channel];
}

OpenAL_AudioState OpenALAudioDevice_GetSourceState(OpenALAudioDevice* device,
  AudioSourceHandle handle) {
  if (!device) return OPENAL_AUDIO_STATE_STOPPED;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      ALint state;
      alGetSourcei(device->sources[i].source_id, AL_SOURCE_STATE, &state);

      switch (state) {
      case AL_PLAYING: return OPENAL_AUDIO_STATE_PLAYING;
      case AL_PAUSED: return OPENAL_AUDIO_STATE_PAUSED;
      case AL_STOPPED: return OPENAL_AUDIO_STATE_STOPPED;
      default: return OPENAL_AUDIO_STATE_INITIAL;
      }
    }
  }

  return OPENAL_AUDIO_STATE_STOPPED;
}

float OpenALAudioDevice_GetSourceVolume(OpenALAudioDevice* device,
  AudioSourceHandle handle) {
  if (!device) return 0.0f;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      return device->sources[i].volume;
    }
  }

  return 0.0f;
}

float OpenALAudioDevice_GetSourcePitch(OpenALAudioDevice* device,
  AudioSourceHandle handle) {
  if (!device) return 0.0f;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      return device->sources[i].pitch;
    }
  }

  return 0.0f;
}

uint32_t OpenALAudioDevice_GetSourceBufferId(OpenALAudioDevice* device,
  AudioSourceHandle handle) {
  if (!device) return 0;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      return device->sources[i].current_buffer;
    }
  }

  return 0;
}

uint32_t OpenALAudioDevice_CreateEffect(OpenALAudioDevice* device, int effect_type) {
  if (!device) return 0;

  /* Phase 47: EFX extension not supported on macOS
   * Return a placeholder ID for API compatibility
   * Effect parameters will be ignored
   */
#ifdef __APPLE__
  if (!alIsExtensionPresent("AL_EXT_EFX")) {
    fprintf(stderr, "Phase 47: AL_EXT_EFX not available on macOS, effects disabled\n");
    return 1;  /* Return non-zero placeholder */
  }
#endif

  return 0;
}

void OpenALAudioDevice_DestroyEffect(OpenALAudioDevice* device, uint32_t effect_id) {
  if (!device || !effect_id) return;

  /* Phase 47: EFX not available on macOS, no-op */
}

void OpenALAudioDevice_AttachEffectToSource(OpenALAudioDevice* device,
  AudioSourceHandle handle,
  uint32_t effect_id) {
  if (!device || !effect_id) return;

  /* Phase 47: EFX not available on macOS, no-op */
}

void OpenALAudioDevice_SetEffectParameter(OpenALAudioDevice* device,
  uint32_t effect_id,
  int param_id,
  float value) {
  if (!device || !effect_id) return;

  /* Phase 47: EFX not available on macOS, no-op */
}

int OpenALAudioDevice_HasEFXSupport(OpenALAudioDevice* device) {
  if (!device) return 0;
  /* Phase 47: Disable EFX on macOS - framework doesn't expose EFX */
#ifdef __APPLE__
  return 0;
#else
  return alIsExtensionPresent("AL_EXT_EFX") ? 1 : 0;
#endif
}

void OpenALAudioDevice_SetMasterVolume(OpenALAudioDevice* device, float volume) {
  if (!device) return;

  device->master_volume = volume;
  alListenerf(AL_GAIN, volume);

  /* Update all sources */
  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].in_use) {
      float effective_volume = device->sources[i].volume *
        device->channel_volumes[device->sources[i].channel] *
        volume;
      alSourcef(device->sources[i].source_id, AL_GAIN, effective_volume);
    }
  }
}

float OpenALAudioDevice_GetMasterVolume(OpenALAudioDevice* device) {
  if (!device) return 0.0f;
  return device->master_volume;
}

void OpenALAudioDevice_StopAllSources(OpenALAudioDevice* device) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].in_use) {
      alSourceStop(device->sources[i].source_id);
      device->sources[i].state = OPENAL_AUDIO_STATE_STOPPED;
    }
  }
}

void OpenALAudioDevice_PauseAllSources(OpenALAudioDevice* device) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].in_use) {
      alSourcePause(device->sources[i].source_id);
      device->sources[i].state = OPENAL_AUDIO_STATE_PAUSED;
    }
  }
}

void OpenALAudioDevice_ResumeAllSources(OpenALAudioDevice* device) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].in_use && device->sources[i].state == OPENAL_AUDIO_STATE_PAUSED) {
      alSourcePlay(device->sources[i].source_id);
      device->sources[i].state = OPENAL_AUDIO_STATE_PLAYING;
    }
  }
}

void OpenALAudioDevice_Update(OpenALAudioDevice* device, float delta_time) {
  if (!device) return;

  /* Update source states */
  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].in_use) {
      ALint state;
      alGetSourcei(device->sources[i].source_id, AL_SOURCE_STATE, &state);

      if (state == AL_STOPPED) {
        device->sources[i].state = OPENAL_AUDIO_STATE_STOPPED;
      }
      else if (state == AL_PAUSED) {
        device->sources[i].state = OPENAL_AUDIO_STATE_PAUSED;
      }
      else if (state == AL_PLAYING) {
        device->sources[i].state = OPENAL_AUDIO_STATE_PLAYING;
      }
    }
  }
}

const char* OpenALAudioDevice_GetLastError(OpenALAudioDevice* device) {
  if (!device) return "Invalid device";
  return device->last_error;
}

int OpenALAudioDevice_ClearErrors(OpenALAudioDevice* device) {
  if (!device) return 0;

  int count = device->error_count;
  device->error_count = 0;
  memset(device->last_error, 0, sizeof(device->last_error));

  return count;
}

void OpenALAudioDevice_PrintDeviceInfo(OpenALAudioDevice* device) {
  if (!device || !device->device) return;

  fprintf(stdout, "OpenAL Device Information\n");
  fprintf(stdout, "Device: %s\n", alcGetString(device->device, ALC_DEVICE_SPECIFIER));
  fprintf(stdout, "Vendor: %s\n", alGetString(AL_VENDOR));
  fprintf(stdout, "Renderer: %s\n", alGetString(AL_RENDERER));
  fprintf(stdout, "Version: %s\n", alGetString(AL_VERSION));
  fprintf(stdout, "Extensions: %s\n", alGetString(AL_EXTENSIONS));
  fprintf(stdout, "Buffers: %u\n", device->num_buffers);
  fprintf(stdout, "Active Sources: %u\n", device->num_active_sources);
  fprintf(stdout, "EFX Support: %s\n", OpenALAudioDevice_HasEFXSupport(device) ? "Yes" : "No");
}

void OpenALAudioDevice_PrintSourceInfo(OpenALAudioDevice* device, AudioSourceHandle handle) {
  if (!device) return;

  for (uint32_t i = 0; i < device->max_sources; i++) {
    if (device->sources[i].handle == handle && device->sources[i].in_use) {
      fprintf(stdout, "Source Handle: %u\n", handle);
      fprintf(stdout, "Channel: %d\n", device->sources[i].channel);
      fprintf(stdout, "State: %d\n", device->sources[i].state);
      fprintf(stdout, "Volume: %.2f\n", device->sources[i].volume);
      fprintf(stdout, "Pitch: %.2f\n", device->sources[i].pitch);
      fprintf(stdout, "Looping: %d\n", device->sources[i].looping);
      fprintf(stdout, "Buffer ID: %u\n", device->sources[i].current_buffer);
      break;
    }
  }
}

uint32_t OpenALAudioDevice_GetActiveSourceCount(OpenALAudioDevice* device) {
  if (!device) return 0;
  return device->num_active_sources;
}
