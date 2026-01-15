/*
 * Phase 07: OpenALDevice Implementation - VC6 Compatible Version
 * 
 * Complete OpenAL audio system for cross-platform audio support
 * Replaces legacy Miles Sound System with modern OpenAL-soft library
 */

#include "../Include/AudioDevice/OpenALDevice.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

// OpenAL headers
#include <AL/al.h>
#include <AL/alc.h>

/* Force linker to include OpenAL32.lib and OpenAL32.dll */
#ifdef _WIN32
#pragma comment(lib, "OpenAL32.lib")
#pragma comment(linker, "/include:_alGenSources")
#endif

namespace GeneralsX {
namespace Audio {

// ============================================================
// OpenALDevice Constructor/Destructor
// ============================================================

OpenALDevice::OpenALDevice()
    : m_initialized(false)
    , m_alDevice(NULL)
    , m_alContext(NULL)
    , m_nextHandle(1)
    , m_currentMusicId(-1)
    , m_currentVoiceId(-1)
    , m_masterVolume(1.0f)
    , m_allPaused(false)
    , m_listenerX(0.0f), m_listenerY(0.0f), m_listenerZ(0.0f)
    , m_listenerVelX(0.0f), m_listenerVelY(0.0f), m_listenerVelZ(0.0f)
    , m_listenerForwardX(0.0f), m_listenerForwardY(0.0f), m_listenerForwardZ(-1.0f)
    , m_listenerUpX(0.0f), m_listenerUpY(1.0f), m_listenerUpZ(0.0f)
{
}

OpenALDevice::~OpenALDevice()
{
    shutdown();
}

// ============================================================
// Lifecycle
// ============================================================

bool OpenALDevice::init()
{
    if (m_initialized) return true;

    // Open default audio device
    m_alDevice = alcOpenDevice(NULL);
    if (!m_alDevice) {
        m_lastError = "Failed to open OpenAL device";
        return false;
    }

    // Create audio context
    m_alContext = alcCreateContext(m_alDevice, NULL);
    if (!m_alContext) {
        alcCloseDevice(m_alDevice);
        m_alDevice = NULL;
        m_lastError = "Failed to create OpenAL context";
        return false;
    }

    // Make context current
    alcMakeContextCurrent(m_alContext);

    // Set default distance model for 3D audio
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

    m_initialized = true;
    m_lastError = "";
    return true;
}

void OpenALDevice::shutdown()
{
    if (!m_initialized) return;

    // Stop all sources
    stopAll();

    // Clean up sources and buffers
    m_soundSources.clear();
    m_audioCache.clear();

    // Destroy OpenAL context
    if (m_alContext) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(m_alContext);
        m_alContext = NULL;
    }

    // Close device
    if (m_alDevice) {
        alcCloseDevice(m_alDevice);
        m_alDevice = NULL;
    }

    m_initialized = false;
}

void OpenALDevice::update(float deltaTime)
{
    if (!m_initialized) return;

    // Remove stopped sources
    std::vector<int> stoppedSounds;
    for (std::map<int, SoundSource>::iterator it = m_soundSources.begin(); 
         it != m_soundSources.end(); ++it) {
        ALint state;
        alGetSourcei(it->second.alSource, AL_SOURCE_STATE, &state);
        if (state == AL_STOPPED) {
            stoppedSounds.push_back(it->first);
        }
    }

    // Remove stopped sounds
    for (size_t i = 0; i < stoppedSounds.size(); ++i) {
        int handle = stoppedSounds[i];
        std::map<int, SoundSource>::iterator it = m_soundSources.find(handle);
        if (it != m_soundSources.end()) {
            alDeleteSources(1, &it->second.alSource);
            m_soundSources.erase(it);
        }
    }
}

bool OpenALDevice::isInitialized() const
{
    return m_initialized;
}

const char* OpenALDevice::getLastError() const
{
    return m_lastError.c_str();
}

// ============================================================
// Music Playback (AudioDevice interface)
// ============================================================

int OpenALDevice::playMusic(const char* filename, bool loop)
{
    if (!m_initialized) return -1;

    // Stop current music if any
    if (m_currentMusicId >= 0) {
        stopMusic(m_currentMusicId);
    }

    // Load audio buffer
    ALuint alBuffer = loadAudioBuffer(filename);
    if (alBuffer == 0) {
        return -1;
    }

    // Create OpenAL source
    ALuint alSource;
    alGenSources(1, &alSource);
    if (alSource == 0) {
        return -1;
    }

    // Set source properties (music is usually 2D)
    alSourcei(alSource, AL_BUFFER, alBuffer);
    alSourcef(alSource, AL_GAIN, m_masterVolume);
    alSourcei(alSource, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSourcei(alSource, AL_SOURCE_RELATIVE, AL_FALSE);

    // Play music
    alSourcePlay(alSource);

    // Create sound source entry
    int handle = allocateSoundHandle();
    SoundSource& source = m_soundSources[handle];
    source.alSource = alSource;
    source.alBuffer = alBuffer;
    source.volume = 1.0f;
    source.pitch = 1.0f;
    source.is3D = false;
    source.isPaused = false;

    m_currentMusicId = handle;
    return handle;
}

void OpenALDevice::stopMusic(int musicId)
{
    SoundSource* source = getSoundSource(musicId);
    if (source) {
        alSourceStop(source->alSource);
        alDeleteSources(1, &source->alSource);
        m_soundSources.erase(musicId);
        if (m_currentMusicId == musicId) {
            m_currentMusicId = -1;
        }
    }
}

bool OpenALDevice::isMusicPlaying(int musicId) const
{
    const SoundSource* source = getSoundSource(musicId);
    if (!source) return false;

    ALint state;
    alGetSourcei(source->alSource, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

void OpenALDevice::setMusicVolume(int musicId, float volume)
{
    SoundSource* source = getSoundSource(musicId);
    if (source) {
        source->volume = volume;
        alSourcef(source->alSource, AL_GAIN, volume * m_masterVolume);
    }
}

float OpenALDevice::getMusicVolume(int musicId) const
{
    const SoundSource* source = getSoundSource(musicId);
    if (source) {
        return source->volume;
    }
    return m_masterVolume;
}

void OpenALDevice::pauseMusic(int musicId)
{
    SoundSource* source = getSoundSource(musicId);
    if (source && !source->isPaused) {
        alSourcePause(source->alSource);
        source->isPaused = true;
    }
}

void OpenALDevice::resumeMusic(int musicId)
{
    SoundSource* source = getSoundSource(musicId);
    if (source && source->isPaused) {
        alSourcePlay(source->alSource);
        source->isPaused = false;
    }
}

// ============================================================
// Sound Effects (Stub Implementation)
// ============================================================

int OpenALDevice::playSound(const char* filename, float volume)
{
    if (!m_initialized) return -1;
    
    // Load audio buffer (or get from cache)
    ALuint alBuffer = loadAudioBuffer(filename);
    if (alBuffer == 0) {
        return -1;
    }

    // Create OpenAL source
    ALuint alSource;
    alGenSources(1, &alSource);
    if (alSource == 0) {
        return -1;
    }

    // Set source properties
    alSourcei(alSource, AL_BUFFER, alBuffer);
    alSourcef(alSource, AL_GAIN, volume * m_masterVolume);
    alSourcei(alSource, AL_SOURCE_RELATIVE, AL_FALSE);

    // Play sound
    alSourcePlay(alSource);

    // Create sound source entry
    int handle = allocateSoundHandle();
    SoundSource& source = m_soundSources[handle];
    source.alSource = alSource;
    source.alBuffer = alBuffer;
    source.volume = volume;
    source.pitch = 1.0f;
    source.is3D = false;
    source.isPaused = false;

    return handle;
}

void OpenALDevice::stopSound(int soundId)
{
    SoundSource* source = getSoundSource(soundId);
    if (source) {
        alSourceStop(source->alSource);
        alDeleteSources(1, &source->alSource);
        m_soundSources.erase(soundId);
    }
}

bool OpenALDevice::isSoundPlaying(int soundId) const
{
    const SoundSource* source = getSoundSource(soundId);
    if (!source) return false;

    ALint state;
    alGetSourcei(source->alSource, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

void OpenALDevice::setSoundVolume(int soundId, float volume)
{
    SoundSource* source = getSoundSource(soundId);
    if (source) {
        source->volume = volume;
        alSourcef(source->alSource, AL_GAIN, volume * m_masterVolume);
    }
}

void OpenALDevice::setSoundPitch(int soundId, float pitch)
{
    SoundSource* source = getSoundSource(soundId);
    if (source) {
        source->pitch = pitch;
        alSourcef(source->alSource, AL_PITCH, pitch);
    }
}

// ============================================================
// 3D Positional Audio (Stub Implementation)
// ============================================================

int OpenALDevice::play3DSound(const char* filename, float worldX, float worldY, float worldZ, float volume, float pitch)
{
    if (!m_initialized) return -1;
    
    ALuint alBuffer = loadAudioBuffer(filename);
    if (alBuffer == 0) {
        m_lastError = "Failed to load audio file for 3D sound: ";
        m_lastError += filename;
        return -1;
    }
    
    ALuint alSource;
    alGenSources(1, &alSource);
    alSourcei(alSource, AL_BUFFER, alBuffer);
    alSourcef(alSource, AL_GAIN, volume * m_masterVolume);
    alSourcef(alSource, AL_PITCH, pitch);
    
    /* Set 3D position and velocity */
    alSource3f(alSource, AL_POSITION, worldX, worldY, worldZ);
    alSource3f(alSource, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    
    /* Enable 3D processing with distance attenuation */
    alSourcei(alSource, AL_SOURCE_RELATIVE, AL_FALSE);
    alSourcef(alSource, AL_REFERENCE_DISTANCE, 1.0f);
    alSourcef(alSource, AL_MAX_DISTANCE, 200.0f);
    alSourcef(alSource, AL_ROLLOFF_FACTOR, 1.0f);
    
    alSourcePlay(alSource);
    
    int handle = allocateSoundHandle();
    SoundSource src;
    src.alSource = alSource;
    src.alBuffer = alBuffer;
    src.filename = filename;
    src.posX = worldX;
    src.posY = worldY;
    src.posZ = worldZ;
    src.velX = 0.0f;
    src.velY = 0.0f;
    src.velZ = 0.0f;
    src.volume = volume;
    src.pitch = pitch;
    src.is3D = true;
    src.isPaused = false;
    m_soundSources[handle] = src;
    
    return handle;
}

void OpenALDevice::set3DSoundPosition(int soundId, float worldX, float worldY, float worldZ)
{
    if (!m_initialized || soundId < 0) return;
    
    std::map<int, SoundSource>::iterator it = m_soundSources.find(soundId);
    if (it == m_soundSources.end()) return;
    
    it->second.posX = worldX;
    it->second.posY = worldY;
    it->second.posZ = worldZ;
    
    alSource3f(it->second.alSource, AL_POSITION, worldX, worldY, worldZ);
}

void OpenALDevice::set3DSoundVelocity(int soundId, float velX, float velY, float velZ)
{
    if (!m_initialized || soundId < 0) return;
    
    std::map<int, SoundSource>::iterator it = m_soundSources.find(soundId);
    if (it == m_soundSources.end()) return;
    
    it->second.velX = velX;
    it->second.velY = velY;
    it->second.velZ = velZ;
    
    alSource3f(it->second.alSource, AL_VELOCITY, velX, velY, velZ);
}

void OpenALDevice::stop3DSound(int soundId)
{
    stopSound(soundId);
}

void OpenALDevice::get3DSoundPosition(int soundId, float& outX, float& outY, float& outZ) const
{
    outX = outY = outZ = 0.0f;
    
    if (soundId < 0) return;
    
    std::map<int, SoundSource>::const_iterator it = m_soundSources.find(soundId);
    if (it != m_soundSources.end()) {
        outX = it->second.posX;
        outY = it->second.posY;
        outZ = it->second.posZ;
    }
}

float OpenALDevice::get3DSoundDistance(int soundId) const
{
    if (soundId < 0) return 0.0f;
    
    std::map<int, SoundSource>::const_iterator it = m_soundSources.find(soundId);
    if (it == m_soundSources.end()) return 0.0f;
    
    /* Calculate distance from listener to sound */
    float dx = it->second.posX - m_listenerX;
    float dy = it->second.posY - m_listenerY;
    float dz = it->second.posZ - m_listenerZ;
    
    float distance = sqrt(dx * dx + dy * dy + dz * dz);
    return distance;
}

// ============================================================
// Listener Positioning
// ============================================================

void OpenALDevice::setListenerPosition(float worldX, float worldY, float worldZ)
{
    m_listenerX = worldX;
    m_listenerY = worldY;
    m_listenerZ = worldZ;
    
    if (m_initialized) {
        alListener3f(AL_POSITION, worldX, worldY, worldZ);
    }
}

void OpenALDevice::getListenerPosition(float& outX, float& outY, float& outZ) const
{
    outX = m_listenerX;
    outY = m_listenerY;
    outZ = m_listenerZ;
}

void OpenALDevice::setListenerOrientation(float forwardX, float forwardY, float forwardZ, float upX, float upY, float upZ)
{
    m_listenerForwardX = forwardX;
    m_listenerForwardY = forwardY;
    m_listenerForwardZ = forwardZ;
    m_listenerUpX = upX;
    m_listenerUpY = upY;
    m_listenerUpZ = upZ;

    if (m_initialized) {
        ALfloat orientation[] = { forwardX, forwardY, forwardZ, upX, upY, upZ };
        alListenerfv(AL_ORIENTATION, orientation);
    }
}

void OpenALDevice::setListenerVelocity(float velX, float velY, float velZ)
{
    m_listenerVelX = velX;
    m_listenerVelY = velY;
    m_listenerVelZ = velZ;

    if (m_initialized) {
        alListener3f(AL_VELOCITY, velX, velY, velZ);
    }
}

void OpenALDevice::getListenerOrientation(float& outForwardX, float& outForwardY, float& outForwardZ, float& outUpX, float& outUpY, float& outUpZ) const
{
    outForwardX = m_listenerForwardX;
    outForwardY = m_listenerForwardY;
    outForwardZ = m_listenerForwardZ;
    outUpX = m_listenerUpX;
    outUpY = m_listenerUpY;
    outUpZ = m_listenerUpZ;
}

// ============================================================
// Voice/Speech (Stub Implementation)
// ============================================================

int OpenALDevice::playVoice(const char* filename, float volume)
{
    if (!m_initialized) return -1;
    
    /* Stop current voice if playing */
    if (m_currentVoiceId >= 0) {
        stopVoice(m_currentVoiceId);
    }
    
    ALuint alBuffer = loadAudioBuffer(filename);
    if (alBuffer == 0) {
        m_lastError = "Failed to load voice file: ";
        m_lastError += filename;
        return -1;
    }
    
    ALuint alSource;
    alGenSources(1, &alSource);
    alSourcei(alSource, AL_BUFFER, alBuffer);
    alSourcef(alSource, AL_GAIN, volume * m_masterVolume);
    alSourcef(alSource, AL_PITCH, 1.0f);
    alSourcei(alSource, AL_SOURCE_RELATIVE, AL_FALSE);
    alSourcePlay(alSource);
    
    int handle = allocateSoundHandle();
    SoundSource src;
    src.alSource = alSource;
    src.alBuffer = alBuffer;
    src.filename = filename;
    src.posX = 0.0f;
    src.posY = 0.0f;
    src.posZ = 0.0f;
    src.velX = 0.0f;
    src.velY = 0.0f;
    src.velZ = 0.0f;
    src.volume = volume;
    src.pitch = 1.0f;
    src.is3D = false;
    src.isPaused = false;
    m_soundSources[handle] = src;
    m_currentVoiceId = handle;
    
    return handle;
}

void OpenALDevice::stopVoice(int voiceId)
{
    if (!m_initialized || voiceId < 0) return;
    
    std::map<int, SoundSource>::iterator it = m_soundSources.find(voiceId);
    if (it != m_soundSources.end()) {
        alSourceStop(it->second.alSource);
        alDeleteSources(1, &it->second.alSource);
        m_soundSources.erase(it);
        
        if (m_currentVoiceId == voiceId) {
            m_currentVoiceId = -1;
        }
    }
}

bool OpenALDevice::isVoicePlaying(int voiceId) const
{
    if (!m_initialized || voiceId < 0) return false;
    
    std::map<int, SoundSource>::const_iterator it = m_soundSources.find(voiceId);
    if (it == m_soundSources.end()) return false;
    
    ALint state;
    alGetSourcei(it->second.alSource, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

void OpenALDevice::setVoiceVolume(int voiceId, float volume)
{
    if (!m_initialized || voiceId < 0) return;
    
    std::map<int, SoundSource>::iterator it = m_soundSources.find(voiceId);
    if (it != m_soundSources.end()) {
        it->second.volume = volume;
        alSourcef(it->second.alSource, AL_GAIN, volume * m_masterVolume);
    }
}

void OpenALDevice::pauseVoice(int voiceId)
{
    if (!m_initialized || voiceId < 0) return;
    
    std::map<int, SoundSource>::iterator it = m_soundSources.find(voiceId);
    if (it != m_soundSources.end()) {
        alSourcePause(it->second.alSource);
        it->second.isPaused = true;
    }
}

void OpenALDevice::resumeVoice(int voiceId)
{
    if (!m_initialized || voiceId < 0) return;
    
    std::map<int, SoundSource>::iterator it = m_soundSources.find(voiceId);
    if (it != m_soundSources.end()) {
        if (it->second.isPaused) {
            alSourcePlay(it->second.alSource);
            it->second.isPaused = false;
        }
    }
}

// ============================================================
// Global Audio Control
// ============================================================

void OpenALDevice::setMasterVolume(float volume)
{
    m_masterVolume = volume;
    if (m_initialized) {
        alListenerf(AL_GAIN, volume);
    }
}

float OpenALDevice::getMasterVolume() const
{
    return m_masterVolume;
}

void OpenALDevice::pauseAll()
{
    if (!m_initialized) return;
    
    m_allPaused = true;
    
    for (std::map<int, SoundSource>::iterator it = m_soundSources.begin();
         it != m_soundSources.end(); ++it) {
        alSourcePause(it->second.alSource);
    }
}

void OpenALDevice::resumeAll()
{
    if (!m_initialized) return;
    
    m_allPaused = false;
    
    for (std::map<int, SoundSource>::iterator it = m_soundSources.begin();
         it != m_soundSources.end(); ++it) {
        alSourcePlay(it->second.alSource);
    }
}

void OpenALDevice::stopAll()
{
    if (!m_initialized) return;
    
    for (std::map<int, SoundSource>::iterator it = m_soundSources.begin();
         it != m_soundSources.end(); ++it) {
        alSourceStop(it->second.alSource);
        alDeleteSources(1, &it->second.alSource);
    }
    m_soundSources.clear();
    m_currentMusicId = -1;
    m_currentVoiceId = -1;
}

// ============================================================
// Audio File Management (Stub Implementation)
// ============================================================

bool OpenALDevice::preloadAudio(const char* filename)
{
    // TODO: Implement audio preloading
    return false;
}

void OpenALDevice::unloadAudio(const char* filename)
{
    // TODO: Unload audio
}

bool OpenALDevice::isAudioPreloaded(const char* filename) const
{
    return false;
}

void OpenALDevice::clearAudioCache()
{
    m_audioCache.clear();
}

size_t OpenALDevice::getAudioCacheSize() const
{
    return 0;
}

// ============================================================
// Internal Helper Methods
// ============================================================

ALuint OpenALDevice::loadAudioBuffer(const char* filename)
{
    if (!filename) return 0;

    // Check if already cached
    std::map<std::string, AudioBuffer>::iterator it = m_audioCache.find(filename);
    if (it != m_audioCache.end()) {
        it->second.refCount++;
        return it->second.alBuffer;
    }

    // Load WAV file
    ALuint alBuffer;
    int frequency, channels;
    if (!loadWAVFile(filename, alBuffer, frequency, channels)) {
        return 0;
    }

    // Add to cache
    AudioBuffer buffer;
    buffer.alBuffer = alBuffer;
    buffer.dataSize = 0;  // TODO: Track actual size
    buffer.refCount = 1;
    m_audioCache[filename] = buffer;

    return alBuffer;
}

int OpenALDevice::createSoundSource(const char* filename, float volume, float pitch, bool is3D)
{
    // TODO: Implement sound source creation
    return -1;
}

OpenALDevice::SoundSource* OpenALDevice::getSoundSource(int handle)
{
    std::map<int, SoundSource>::iterator it = m_soundSources.find(handle);
    if (it != m_soundSources.end()) {
        return &it->second;
    }
    return NULL;
}

const OpenALDevice::SoundSource* OpenALDevice::getSoundSource(int handle) const
{
    std::map<int, SoundSource>::const_iterator it = m_soundSources.find(handle);
    if (it != m_soundSources.end()) {
        return &it->second;
    }
    return NULL;
}

int OpenALDevice::allocateSoundHandle()
{
    return m_nextHandle++;
}

bool OpenALDevice::loadWAVFile(const char* filename, ALuint& outBuffer, int& outFrequency, int& outChannels)
{
    // Open WAV file in binary mode
    FILE* file = fopen(filename, "rb");
    if (!file) {
        m_lastError = "Failed to open WAV file";
        return false;
    }

    // Read RIFF header (12 bytes)
    char riffID[4];
    unsigned int riffSize;
    char waveID[4];
    
    if (fread(riffID, 1, 4, file) != 4 || 
        fread(&riffSize, 1, 4, file) != 4 ||
        fread(waveID, 1, 4, file) != 4) {
        m_lastError = "Failed to read RIFF header";
        fclose(file);
        return false;
    }

    // Validate RIFF header
    if (strncmp(riffID, "RIFF", 4) != 0 || strncmp(waveID, "WAVE", 4) != 0) {
        m_lastError = "Invalid RIFF/WAVE header";
        fclose(file);
        return false;
    }

    // Find fmt subchunk
    unsigned int audioFormat = 0, numChannels = 0, sampleRate = 0;
    unsigned short bitsPerSample = 0;
    unsigned int dataSize = 0;
    char* audioData = NULL;

    while (!feof(file)) {
        char subchunkID[4];
        unsigned int subchunkSize;

        if (fread(subchunkID, 1, 4, file) != 4) {
            break;
        }

        if (fread(&subchunkSize, 1, 4, file) != 4) {
            break;
        }

        if (strncmp(subchunkID, "fmt ", 4) == 0) {
            // Read format subchunk
            unsigned short blockAlign;
            unsigned int byteRate;
            
            fread(&audioFormat, 1, 2, file);
            fread(&numChannels, 1, 2, file);
            fread(&sampleRate, 1, 4, file);
            fread(&byteRate, 1, 4, file);
            fread(&blockAlign, 1, 2, file);
            fread(&bitsPerSample, 1, 2, file);

            // Skip any extra fmt data
            if (subchunkSize > 16) {
                fseek(file, subchunkSize - 16, SEEK_CUR);
            }
        } 
        else if (strncmp(subchunkID, "data", 4) == 0) {
            // Read audio data
            dataSize = subchunkSize;
            audioData = (char*)malloc(dataSize);
            if (!audioData) {
                m_lastError = "Failed to allocate memory for audio data";
                fclose(file);
                return false;
            }

            if (fread(audioData, 1, dataSize, file) != dataSize) {
                m_lastError = "Failed to read audio data";
                free(audioData);
                fclose(file);
                return false;
            }
            break;
        }
        else {
            // Skip unknown subchunk
            fseek(file, subchunkSize, SEEK_CUR);
        }
    }

    fclose(file);

    // Validate audio format
    if (audioFormat != 1) {  // 1 = PCM
        m_lastError = "Unsupported audio format (only PCM supported)";
        if (audioData) free(audioData);
        return false;
    }

    if (numChannels != 1 && numChannels != 2) {
        m_lastError = "Unsupported channel count (only mono/stereo supported)";
        if (audioData) free(audioData);
        return false;
    }

    if (bitsPerSample != 8 && bitsPerSample != 16) {
        m_lastError = "Unsupported bits per sample (only 8/16 bit supported)";
        if (audioData) free(audioData);
        return false;
    }

    if (!audioData || dataSize == 0) {
        m_lastError = "No audio data found in WAV file";
        if (audioData) free(audioData);
        return false;
    }

    // Determine OpenAL format
    ALenum alFormat;
    if (numChannels == 1) {
        alFormat = (bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
    } else {
        alFormat = (bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
    }

    // Create OpenAL buffer
    alGenBuffers(1, &outBuffer);
    if (outBuffer == 0) {
        m_lastError = "Failed to create OpenAL buffer";
        free(audioData);
        return false;
    }

    // Load data into buffer
    alBufferData(outBuffer, alFormat, audioData, dataSize, sampleRate);

    // Check for OpenAL errors
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        m_lastError = "Failed to upload audio data to OpenAL buffer";
        alDeleteBuffers(1, &outBuffer);
        outBuffer = 0;
        free(audioData);
        return false;
    }

    // Set output parameters
    outFrequency = (int)sampleRate;
    outChannels = (int)numChannels;

    // Clean up
    free(audioData);

    return true;
}

void OpenALDevice::logALError(const char* context, int alError)
{
    // TODO: Log OpenAL errors
}

} // namespace Audio
} // namespace GeneralsX
