#ifndef _WIN32

#include "dsound_compat.h"
#include <cstring>
#include <algorithm>
#include <iostream>

//=============================================================================
// OpenAL Context Management
//=============================================================================

OpenALContext& OpenALContext::getInstance()
{
    static OpenALContext instance;
    return instance;
}

bool OpenALContext::initialize()
{
    if (m_initialized) {
        return true;
    }
    
    // Open OpenAL device
    m_device = alcOpenDevice(nullptr); // Use default device
    if (!m_device) {
        std::cerr << "OpenAL: Failed to open audio device" << std::endl;
        return false;
    }
    
    // Create OpenAL context
    m_context = alcCreateContext(m_device, nullptr);
    if (!m_context) {
        std::cerr << "OpenAL: Failed to create audio context" << std::endl;
        alcCloseDevice(m_device);
        m_device = nullptr;
        return false;
    }
    
    // Make context current
    if (!alcMakeContextCurrent(m_context)) {
        std::cerr << "OpenAL: Failed to make context current" << std::endl;
        alcDestroyContext(m_context);
        alcCloseDevice(m_device);
        m_context = nullptr;
        m_device = nullptr;
        return false;
    }
    
    // Check for errors
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "OpenAL: Initialization error: " << error << std::endl;
        shutdown();
        return false;
    }
    
    // Set up 3D audio environment
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    alDopplerFactor(1.0f);
    alSpeedOfSound(343.3f); // Speed of sound in m/s
    
    m_initialized = true;
    std::cout << "OpenAL: Successfully initialized audio system" << std::endl;
    
    return true;
}

void OpenALContext::shutdown()
{
    if (m_context) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(m_context);
        m_context = nullptr;
    }
    
    if (m_device) {
        alcCloseDevice(m_device);
        m_device = nullptr;
    }
    
    m_initialized = false;
}

OpenALContext::~OpenALContext()
{
    shutdown();
}

//=============================================================================
// DirectSoundBuffer Implementation
//=============================================================================

DirectSoundBuffer::DirectSoundBuffer(const DSBUFFERDESC* desc)
    : m_source(0)
    , m_buffer(0)
    , m_bufferSize(desc ? desc->dwBufferBytes : 0)
    , m_frequency(22050) // Default frequency
    , m_flags(desc ? desc->dwFlags : 0)
    , m_is3D((desc && (desc->dwFlags & DSBCAPS_CTRL3D)) != 0)
    , m_isPlaying(false)
    , m_isLooping(false)
    , m_volume(0)        // 0 dB (full volume in DirectSound scale)
    , m_pan(0)           // Center pan
{
    if (!OpenALContext::getInstance().isInitialized()) {
        OpenALContext::getInstance().initialize();
    }
    
    // Generate OpenAL source
    alGenSources(1, &m_source);
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "OpenAL: Failed to generate source: " << error << std::endl;
        return;
    }
    
    // Generate OpenAL buffer
    alGenBuffers(1, &m_buffer);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "OpenAL: Failed to generate buffer: " << error << std::endl;
        alDeleteSources(1, &m_source);
        m_source = 0;
        return;
    }
    
    // Configure source for 3D if needed
    if (m_is3D) {
        alSourcef(m_source, AL_REFERENCE_DISTANCE, 1.0f);
        alSourcef(m_source, AL_MAX_DISTANCE, 1000.0f);
        alSourcef(m_source, AL_ROLLOFF_FACTOR, 1.0f);
        alSource3f(m_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
        alSource3f(m_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    } else {
        // For 2D audio, disable spatialization
        alSourcei(m_source, AL_SOURCE_RELATIVE, AL_TRUE);
        alSource3f(m_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    }
    
    // Initialize audio data buffer
    if (m_bufferSize > 0) {
        m_audioData.resize(m_bufferSize, 0);
    }
    
    // Set initial volume and pan
    updateVolume();
    updatePan();
}

DirectSoundBuffer::~DirectSoundBuffer()
{
    if (m_source) {
        alSourceStop(m_source);
        alDeleteSources(1, &m_source);
    }
    
    if (m_buffer) {
        alDeleteBuffers(1, &m_buffer);
    }
}

HRESULT DirectSoundBuffer::Lock(DWORD offset, DWORD bytes, void** audioPtr1, DWORD* audioBytes1,
                                void** audioPtr2, DWORD* audioBytes2, DWORD flags)
{
    if (!audioPtr1 || !audioBytes1) {
        return DSERR_INVALIDPARAM;
    }
    
    // Ensure buffer is large enough
    if (offset + bytes > m_audioData.size()) {
        if (bytes > m_audioData.size()) {
            m_audioData.resize(bytes);
            m_bufferSize = bytes;
        }
    }
    
    // Simple lock implementation - just return pointer to buffer
    *audioPtr1 = &m_audioData[offset];
    *audioBytes1 = std::min(bytes, static_cast<DWORD>(m_audioData.size() - offset));
    
    // Second buffer (for circular buffers) - not implemented for simplicity
    if (audioPtr2) *audioPtr2 = nullptr;
    if (audioBytes2) *audioBytes2 = 0;
    
    return DS_OK;
}

HRESULT DirectSoundBuffer::Unlock(void* audioPtr1, DWORD audioBytes1, void* audioPtr2, DWORD audioBytes2)
{
    // Upload data to OpenAL buffer
    if (!m_audioData.empty()) {
        // Assume 16-bit stereo for now (should be determined from WAVEFORMATEX)
        ALenum format = AL_FORMAT_STEREO16;
        alBufferData(m_buffer, format, m_audioData.data(), static_cast<ALsizei>(m_audioData.size()), m_frequency);
        
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            std::cerr << "OpenAL: Failed to upload buffer data: " << error << std::endl;
            return DSERR_INVALIDCALL;
        }
        
        // Attach buffer to source
        alSourcei(m_source, AL_BUFFER, m_buffer);
    }
    
    return DS_OK;
}

HRESULT DirectSoundBuffer::Play(DWORD reserved1, DWORD priority, DWORD flags)
{
    // Set looping if requested
    m_isLooping = (flags & DSBSTATUS_LOOPING) != 0;
    alSourcei(m_source, AL_LOOPING, m_isLooping ? AL_TRUE : AL_FALSE);
    
    // Play the source
    alSourcePlay(m_source);
    
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "OpenAL: Failed to play source: " << error << std::endl;
        return DSERR_INVALIDCALL;
    }
    
    m_isPlaying = true;
    return DS_OK;
}

HRESULT DirectSoundBuffer::Stop()
{
    alSourceStop(m_source);
    m_isPlaying = false;
    return DS_OK;
}

HRESULT DirectSoundBuffer::SetCurrentPosition(DWORD position)
{
    // Convert byte position to sample position (assuming 16-bit stereo)
    ALfloat samplePos = static_cast<ALfloat>(position) / 4.0f / m_frequency;
    alSourcef(m_source, AL_SEC_OFFSET, samplePos);
    return DS_OK;
}

HRESULT DirectSoundBuffer::GetCurrentPosition(DWORD* playPos, DWORD* writePos)
{
    if (playPos) {
        ALfloat offset;
        alGetSourcef(m_source, AL_SEC_OFFSET, &offset);
        *playPos = static_cast<DWORD>(offset * m_frequency * 4); // Assuming 16-bit stereo
    }
    
    if (writePos) {
        *writePos = playPos ? *playPos : 0;
    }
    
    return DS_OK;
}

HRESULT DirectSoundBuffer::SetVolume(LONG volume)
{
    m_volume = std::max(-10000L, std::min(0L, volume)); // Clamp to DirectSound range
    updateVolume();
    return DS_OK;
}

HRESULT DirectSoundBuffer::GetVolume(LONG* volume)
{
    if (volume) {
        *volume = m_volume;
    }
    return DS_OK;
}

HRESULT DirectSoundBuffer::SetPan(LONG pan)
{
    m_pan = std::max(-10000L, std::min(10000L, pan)); // Clamp to DirectSound range
    updatePan();
    return DS_OK;
}

HRESULT DirectSoundBuffer::GetPan(LONG* pan)
{
    if (pan) {
        *pan = m_pan;
    }
    return DS_OK;
}

HRESULT DirectSoundBuffer::SetFrequency(DWORD frequency)
{
    m_frequency = frequency;
    alSourcef(m_source, AL_PITCH, static_cast<ALfloat>(frequency) / 22050.0f); // Relative to base frequency
    return DS_OK;
}

HRESULT DirectSoundBuffer::GetFrequency(DWORD* frequency)
{
    if (frequency) {
        *frequency = m_frequency;
    }
    return DS_OK;
}

HRESULT DirectSoundBuffer::GetStatus(DWORD* status)
{
    if (!status) {
        return DSERR_INVALIDPARAM;
    }
    
    *status = 0;
    
    ALint sourceState;
    alGetSourcei(m_source, AL_SOURCE_STATE, &sourceState);
    
    if (sourceState == AL_PLAYING) {
        *status |= DSBSTATUS_PLAYING;
        if (m_isLooping) {
            *status |= DSBSTATUS_LOOPING;
        }
    }
    
    return DS_OK;
}

HRESULT DirectSoundBuffer::QueryInterface(const void* riid, void** ppvObj)
{
    // For 3D buffer interface, create DirectSound3DBuffer wrapper
    if (ppvObj && m_is3D) {
        *ppvObj = new DirectSound3DBuffer(this);
        return DS_OK;
    }
    
    return DSERR_UNSUPPORTED;
}

void DirectSoundBuffer::updateVolume()
{
    ALfloat gain = directSoundVolumeToOpenAL(m_volume);
    alSourcef(m_source, AL_GAIN, gain);
}

void DirectSoundBuffer::updatePan()
{
    if (!m_is3D) {
        // For 2D audio, simulate pan using source position
        ALfloat panPos = static_cast<ALfloat>(m_pan) / 10000.0f; // Convert to -1.0 to 1.0
        alSource3f(m_source, AL_POSITION, panPos, 0.0f, 0.0f);
    }
}

float DirectSoundBuffer::directSoundVolumeToOpenAL(LONG dsVolume)
{
    // DirectSound volume: -10000 (silence) to 0 (full volume) in centibels
    // OpenAL gain: 0.0 (silence) to 1.0 (full volume) linear
    if (dsVolume <= -10000) {
        return 0.0f;
    }
    
    // Convert centibels to linear gain: gain = 10^(dB/20)
    float dB = static_cast<float>(dsVolume) / 100.0f;
    return powf(10.0f, dB / 20.0f);
}

void DirectSoundBuffer::directSoundPanToOpenAL(LONG dsPan, float& leftGain, float& rightGain)
{
    // DirectSound pan: -10000 (full left) to 10000 (full right)
    float pan = static_cast<float>(dsPan) / 10000.0f; // Convert to -1.0 to 1.0
    
    if (pan <= 0.0f) {
        // Left side
        leftGain = 1.0f;
        rightGain = 1.0f + pan; // pan is negative, so this reduces right
    } else {
        // Right side
        leftGain = 1.0f - pan;
        rightGain = 1.0f;
    }
    
    // Ensure gains are in valid range
    leftGain = std::max(0.0f, std::min(1.0f, leftGain));
    rightGain = std::max(0.0f, std::min(1.0f, rightGain));
}

//=============================================================================
// DirectSound3DBuffer Implementation
//=============================================================================

DirectSound3DBuffer::DirectSound3DBuffer(DirectSoundBuffer* buffer)
    : m_buffer(buffer)
    , m_position{0.0f, 0.0f, 0.0f}
    , m_velocity{0.0f, 0.0f, 0.0f}
    , m_minDistance(1.0f)
    , m_maxDistance(1000.0f)
    , m_mode(DS3DMODE_NORMAL)
{
}

HRESULT DirectSound3DBuffer::SetPosition(float x, float y, float z, DWORD applyMode)
{
    m_position.x = x;
    m_position.y = y;
    m_position.z = z;
    
    if (m_buffer) {
        alSource3f(m_buffer->getSource(), AL_POSITION, x, y, z);
    }
    
    return DS_OK;
}

HRESULT DirectSound3DBuffer::GetPosition(D3DVECTOR* position)
{
    if (position) {
        *position = m_position;
    }
    return DS_OK;
}

HRESULT DirectSound3DBuffer::SetVelocity(float x, float y, float z, DWORD applyMode)
{
    m_velocity.x = x;
    m_velocity.y = y;
    m_velocity.z = z;
    
    if (m_buffer) {
        alSource3f(m_buffer->getSource(), AL_VELOCITY, x, y, z);
    }
    
    return DS_OK;
}

HRESULT DirectSound3DBuffer::GetVelocity(D3DVECTOR* velocity)
{
    if (velocity) {
        *velocity = m_velocity;
    }
    return DS_OK;
}

HRESULT DirectSound3DBuffer::SetMinDistance(float minDistance, DWORD applyMode)
{
    m_minDistance = minDistance;
    
    if (m_buffer) {
        alSourcef(m_buffer->getSource(), AL_REFERENCE_DISTANCE, minDistance);
    }
    
    return DS_OK;
}

HRESULT DirectSound3DBuffer::GetMinDistance(float* minDistance)
{
    if (minDistance) {
        *minDistance = m_minDistance;
    }
    return DS_OK;
}

HRESULT DirectSound3DBuffer::SetMaxDistance(float maxDistance, DWORD applyMode)
{
    m_maxDistance = maxDistance;
    
    if (m_buffer) {
        alSourcef(m_buffer->getSource(), AL_MAX_DISTANCE, maxDistance);
    }
    
    return DS_OK;
}

HRESULT DirectSound3DBuffer::GetMaxDistance(float* maxDistance)
{
    if (maxDistance) {
        *maxDistance = m_maxDistance;
    }
    return DS_OK;
}

HRESULT DirectSound3DBuffer::SetMode(DWORD mode, DWORD applyMode)
{
    m_mode = mode;
    
    if (m_buffer) {
        ALuint source = m_buffer->getSource();
        
        switch (mode) {
            case DS3DMODE_HEADRELATIVE:
                alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
                break;
            case DS3DMODE_DISABLE:
                // Disable 3D processing - treat as 2D
                alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
                alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
                break;
            case DS3DMODE_NORMAL:
            default:
                alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);
                break;
        }
    }
    
    return DS_OK;
}

HRESULT DirectSound3DBuffer::GetMode(DWORD* mode)
{
    if (mode) {
        *mode = m_mode;
    }
    return DS_OK;
}

//=============================================================================
// DirectSound3DListener Implementation
//=============================================================================

DirectSound3DListener::DirectSound3DListener()
    : m_position{0.0f, 0.0f, 0.0f}
    , m_velocity{0.0f, 0.0f, 0.0f}
    , m_orientFront{0.0f, 0.0f, -1.0f}  // Looking down negative Z
    , m_orientTop{0.0f, 1.0f, 0.0f}     // Up is positive Y
    , m_distanceFactor(1.0f)
    , m_dopplerFactor(1.0f)
    , m_rolloffFactor(1.0f)
{
    applyChanges();
}

HRESULT DirectSound3DListener::SetPosition(float x, float y, float z, DWORD applyMode)
{
    m_position.x = x;
    m_position.y = y;
    m_position.z = z;
    applyChanges();
    return DS_OK;
}

HRESULT DirectSound3DListener::GetPosition(D3DVECTOR* position)
{
    if (position) {
        *position = m_position;
    }
    return DS_OK;
}

HRESULT DirectSound3DListener::SetVelocity(float x, float y, float z, DWORD applyMode)
{
    m_velocity.x = x;
    m_velocity.y = y;
    m_velocity.z = z;
    applyChanges();
    return DS_OK;
}

HRESULT DirectSound3DListener::GetVelocity(D3DVECTOR* velocity)
{
    if (velocity) {
        *velocity = m_velocity;
    }
    return DS_OK;
}

HRESULT DirectSound3DListener::SetOrientation(float xFront, float yFront, float zFront,
                                             float xTop, float yTop, float zTop, DWORD applyMode)
{
    m_orientFront.x = xFront;
    m_orientFront.y = yFront;
    m_orientFront.z = zFront;
    m_orientTop.x = xTop;
    m_orientTop.y = yTop;
    m_orientTop.z = zTop;
    applyChanges();
    return DS_OK;
}

HRESULT DirectSound3DListener::GetOrientation(D3DVECTOR* orientFront, D3DVECTOR* orientTop)
{
    if (orientFront) {
        *orientFront = m_orientFront;
    }
    if (orientTop) {
        *orientTop = m_orientTop;
    }
    return DS_OK;
}

HRESULT DirectSound3DListener::SetDistanceFactor(float factor, DWORD applyMode)
{
    m_distanceFactor = factor;
    applyChanges();
    return DS_OK;
}

HRESULT DirectSound3DListener::GetDistanceFactor(float* factor)
{
    if (factor) {
        *factor = m_distanceFactor;
    }
    return DS_OK;
}

HRESULT DirectSound3DListener::SetDopplerFactor(float factor, DWORD applyMode)
{
    m_dopplerFactor = factor;
    alDopplerFactor(factor);
    return DS_OK;
}

HRESULT DirectSound3DListener::GetDopplerFactor(float* factor)
{
    if (factor) {
        *factor = m_dopplerFactor;
    }
    return DS_OK;
}

HRESULT DirectSound3DListener::SetRolloffFactor(float factor, DWORD applyMode)
{
    m_rolloffFactor = factor;
    applyChanges();
    return DS_OK;
}

HRESULT DirectSound3DListener::GetRolloffFactor(float* factor)
{
    if (factor) {
        *factor = m_rolloffFactor;
    }
    return DS_OK;
}

void DirectSound3DListener::applyChanges()
{
    // Set listener position
    alListener3f(AL_POSITION, m_position.x, m_position.y, m_position.z);
    
    // Set listener velocity
    alListener3f(AL_VELOCITY, m_velocity.x, m_velocity.y, m_velocity.z);
    
    // Set listener orientation (at and up vectors)
    ALfloat orientation[6] = {
        m_orientFront.x, m_orientFront.y, m_orientFront.z,  // At vector
        m_orientTop.x, m_orientTop.y, m_orientTop.z         // Up vector
    };
    alListenerfv(AL_ORIENTATION, orientation);
    
    // Apply distance and rolloff factors (these affect all sources globally in OpenAL)
    // Note: In a real implementation, you might want to apply these per-source
}

//=============================================================================
// DirectSound API Implementation
//=============================================================================

// Implementation classes that inherit from the interfaces
class DirectSoundImpl : public IDirectSound8
{
private:
    ULONG m_refCount;
    DWORD m_speakerConfig;
    
public:
    DirectSoundImpl() : m_refCount(1), m_speakerConfig(DSSPEAKER_STEREO) {
        OpenALContext::getInstance().initialize();
    }
    
    virtual ~DirectSoundImpl() = default;
    
    // IUnknown methods
    virtual HRESULT QueryInterface(const void* riid, void** ppvObj) override {
        if (ppvObj) {
            *ppvObj = this;
            AddRef();
            return DS_OK;
        }
        return DSERR_INVALIDPARAM;
    }
    
    virtual ULONG AddRef() override {
        return ++m_refCount;
    }
    
    virtual ULONG Release() override {
        ULONG count = --m_refCount;
        if (count == 0) {
            delete this;
        }
        return count;
    }
    
    // IDirectSound methods
    virtual HRESULT CreateSoundBuffer(const DSBUFFERDESC* desc, IDirectSoundBuffer8** buffer, void* outer) override;
    virtual HRESULT GetCaps(DSCAPS* caps) override;
    virtual HRESULT DuplicateSoundBuffer(IDirectSoundBuffer8* original, IDirectSoundBuffer8** duplicate) override;
    virtual HRESULT SetCooperativeLevel(void* hwnd, DWORD level) override;
    virtual HRESULT Compact() override;
    virtual HRESULT GetSpeakerConfig(DWORD* speakerConfig) override;
    virtual HRESULT SetSpeakerConfig(DWORD speakerConfig) override;
    virtual HRESULT Initialize(const void* guid) override;
    virtual HRESULT VerifyCertification(DWORD* certified) override;
};

class DirectSoundBufferImpl : public IDirectSoundBuffer8
{
private:
    ULONG m_refCount;
    std::unique_ptr<DirectSoundBuffer> m_buffer;
    
public:
    DirectSoundBufferImpl(const DSBUFFERDESC* desc) 
        : m_refCount(1), m_buffer(std::make_unique<DirectSoundBuffer>(desc)) {}
    
    virtual ~DirectSoundBufferImpl() = default;
    
    // IUnknown methods
    virtual HRESULT QueryInterface(const void* riid, void** ppvObj) override {
        return m_buffer->QueryInterface(riid, ppvObj);
    }
    
    virtual ULONG AddRef() override {
        return ++m_refCount;
    }
    
    virtual ULONG Release() override {
        ULONG count = --m_refCount;
        if (count == 0) {
            delete this;
        }
        return count;
    }
    
    // IDirectSoundBuffer methods
    virtual HRESULT GetCaps(void* caps) override { return DSERR_UNSUPPORTED; }
    virtual HRESULT GetCurrentPosition(DWORD* playPos, DWORD* writePos) override {
        return m_buffer->GetCurrentPosition(playPos, writePos);
    }
    virtual HRESULT GetFormat(void* wfxFormat, DWORD sizeAllocated, DWORD* sizeWritten) override { return DSERR_UNSUPPORTED; }
    virtual HRESULT GetVolume(LONG* volume) override {
        return m_buffer->GetVolume(volume);
    }
    virtual HRESULT GetPan(LONG* pan) override {
        return m_buffer->GetPan(pan);
    }
    virtual HRESULT GetFrequency(DWORD* frequency) override {
        return m_buffer->GetFrequency(frequency);
    }
    virtual HRESULT GetStatus(DWORD* status) override {
        return m_buffer->GetStatus(status);
    }
    virtual HRESULT Initialize(IDirectSound8* directSound, const DSBUFFERDESC* desc) override { return DSERR_UNSUPPORTED; }
    virtual HRESULT Lock(DWORD offset, DWORD bytes, void** audioPtr1, DWORD* audioBytes1,
                        void** audioPtr2, DWORD* audioBytes2, DWORD flags) override {
        return m_buffer->Lock(offset, bytes, audioPtr1, audioBytes1, audioPtr2, audioBytes2, flags);
    }
    virtual HRESULT Play(DWORD reserved1, DWORD priority, DWORD flags) override {
        return m_buffer->Play(reserved1, priority, flags);
    }
    virtual HRESULT SetCurrentPosition(DWORD position) override {
        return m_buffer->SetCurrentPosition(position);
    }
    virtual HRESULT SetFormat(const void* wfxFormat) override { return DSERR_UNSUPPORTED; }
    virtual HRESULT SetVolume(LONG volume) override {
        return m_buffer->SetVolume(volume);
    }
    virtual HRESULT SetPan(LONG pan) override {
        return m_buffer->SetPan(pan);
    }
    virtual HRESULT SetFrequency(DWORD frequency) override {
        return m_buffer->SetFrequency(frequency);
    }
    virtual HRESULT Stop() override {
        return m_buffer->Stop();
    }
    virtual HRESULT Unlock(void* audioPtr1, DWORD audioBytes1, void* audioPtr2, DWORD audioBytes2) override {
        return m_buffer->Unlock(audioPtr1, audioBytes1, audioPtr2, audioBytes2);
    }
    virtual HRESULT Restore() override { return DS_OK; }
    virtual HRESULT SetFX(DWORD effectsCount, void* fxDesc, DWORD* resultCodes) override { return DSERR_UNSUPPORTED; }
    virtual HRESULT AcquireResources(DWORD flags, DWORD effectsCount, DWORD* resultCodes) override { return DSERR_UNSUPPORTED; }
    virtual HRESULT GetObjectInPath(const void* guidObject, DWORD index, const void* guidInterface, void** ppvObj) override { return DSERR_UNSUPPORTED; }
};

HRESULT DirectSoundImpl::CreateSoundBuffer(const DSBUFFERDESC* desc, IDirectSoundBuffer8** buffer, void* outer)
{
    if (!desc || !buffer) {
        return DSERR_INVALIDPARAM;
    }
    
    *buffer = new DirectSoundBufferImpl(desc);
    return DS_OK;
}

HRESULT DirectSoundImpl::GetCaps(DSCAPS* caps)
{
    if (!caps) {
        return DSERR_INVALIDPARAM;
    }
    
    // Fill in some reasonable default capabilities
    memset(caps, 0, sizeof(DSCAPS));
    caps->dwSize = sizeof(DSCAPS);
    caps->dwMinSecondarySampleRate = 4000;
    caps->dwMaxSecondarySampleRate = 48000;
    caps->dwPrimaryBuffers = 1;
    caps->dwMaxHwMixingAllBuffers = 32;
    caps->dwMaxHwMixingStaticBuffers = 32;
    caps->dwMaxHwMixingStreamingBuffers = 32;
    caps->dwMaxHw3DAllBuffers = 16;
    caps->dwMaxHw3DStaticBuffers = 16;
    caps->dwMaxHw3DStreamingBuffers = 16;
    
    return DS_OK;
}

HRESULT DirectSoundImpl::DuplicateSoundBuffer(IDirectSoundBuffer8* original, IDirectSoundBuffer8** duplicate)
{
    return DSERR_UNSUPPORTED; // Not implemented for simplicity
}

HRESULT DirectSoundImpl::SetCooperativeLevel(void* hwnd, DWORD level)
{
    return DS_OK; // Always succeed
}

HRESULT DirectSoundImpl::Compact()
{
    return DS_OK; // No-op
}

HRESULT DirectSoundImpl::GetSpeakerConfig(DWORD* speakerConfig)
{
    if (speakerConfig) {
        *speakerConfig = m_speakerConfig;
    }
    return DS_OK;
}

HRESULT DirectSoundImpl::SetSpeakerConfig(DWORD speakerConfig)
{
    m_speakerConfig = speakerConfig;
    return DS_OK;
}

HRESULT DirectSoundImpl::Initialize(const void* guid)
{
    return DS_OK; // Already initialized in constructor
}

HRESULT DirectSoundImpl::VerifyCertification(DWORD* certified)
{
    if (certified) {
        *certified = 1; // Always certified
    }
    return DS_OK;
}

//=============================================================================
// Public API Functions
//=============================================================================

HRESULT DirectSoundCreate8(const void* deviceGuid, IDirectSound8** directSound, void* outer)
{
    if (!directSound) {
        return DSERR_INVALIDPARAM;
    }
    
    *directSound = new DirectSoundImpl();
    return DS_OK;
}

BOOL DirectSoundEnumerate(void* enumCallback, void* context)
{
    // Stub implementation - just return true to indicate success
    return TRUE;
}

#endif // !_WIN32