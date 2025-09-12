#pragma once

#ifndef DSOUND_COMPAT_H_INCLUDED
#define DSOUND_COMPAT_H_INCLUDED

#pragma once

// Include Windows compatibility layer first
#include "win32_compat.h"

#ifndef _WIN32

// OpenAL includes
#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <vector>
#include <memory>
#include <map>

// DirectSound compatibility layer using OpenAL backend
// This provides DirectSound API compatibility for cross-platform audio

//=============================================================================
// DirectSound Constants and Types
//=============================================================================

// DirectSound constants
#define DS_OK                           0
#define DSERR_ALLOCATED                 0x8878000A
#define DSERR_CONTROLUNAVAIL            0x8878001E
#define DSERR_INVALIDPARAM              0x80070057
#define DSERR_INVALIDCALL               0x88780032
#define DSERR_NOAGGREGATION             0x80040110
#define DSERR_OUTOFMEMORY               0x8007000E
#define DSERR_UNINITIALIZED             0x88780030
#define DSERR_UNSUPPORTED               0x80004001

// Speaker configuration constants
#define DSSPEAKER_DIRECTOUT             0x00000000
#define DSSPEAKER_HEADPHONE             0x00000001
#define DSSPEAKER_MONO                  0x00000002
#define DSSPEAKER_QUAD                  0x00000003
#define DSSPEAKER_STEREO                0x00000004
#define DSSPEAKER_SURROUND              0x00000005
#define DSSPEAKER_5POINT1               0x00000006
#define DSSPEAKER_7POINT1               0x00000007

#define DSSPEAKER_CONFIG(a)             ((a) & 0x000000FF)

// DirectSound buffer flags
#define DSBCAPS_PRIMARYBUFFER           0x00000001
#define DSBCAPS_STATIC                  0x00000002
#define DSBCAPS_LOCHARDWARE             0x00000004
#define DSBCAPS_LOCSOFTWARE             0x00000008
#define DSBCAPS_CTRL3D                  0x00000010
#define DSBCAPS_CTRLFREQUENCY           0x00000020
#define DSBCAPS_CTRLPAN                 0x00000040
#define DSBCAPS_CTRLVOLUME              0x00000080
#define DSBCAPS_CTRLPOSITIONNOTIFY      0x00000100
#define DSBCAPS_CTRLFX                  0x00000200
#define DSBCAPS_STICKYFOCUS             0x00004000
#define DSBCAPS_GLOBALFOCUS             0x00008000

// DirectSound buffer states
#define DSBSTATUS_PLAYING               0x00000001
#define DSBSTATUS_BUFFERLOST            0x00000002
#define DSBSTATUS_LOOPING               0x00000004

// DirectSound 3D buffer modes
#define DS3DMODE_NORMAL                 0x00000000
#define DS3DMODE_HEADRELATIVE           0x00000001
#define DS3DMODE_DISABLE                0x00000002

// Forward declarations
struct IDirectSound8;
struct IDirectSoundBuffer8;
struct IDirectSound3DListener8;
struct IDirectSound3DBuffer8;

typedef IDirectSound8* LPDIRECTSOUND8;
typedef IDirectSound8* LPDIRECTSOUND;
typedef IDirectSoundBuffer8* LPDIRECTSOUNDBUFFER8;
typedef IDirectSoundBuffer8* LPDIRECTSOUNDBUFFER;
typedef IDirectSound3DListener8* LPDIRECTSOUND3DLISTENER8;
typedef IDirectSound3DBuffer8* LPDIRECTSOUND3DBUFFER8;

// DirectSound structures
typedef struct _DSBUFFERDESC
{
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwBufferBytes;
    DWORD           dwReserved;
    void*           lpwfxFormat;    // WAVEFORMATEX*
    void*           guid3DAlgorithm; // GUID* (not used in our implementation)
} DSBUFFERDESC, *LPDSBUFFERDESC;

typedef struct _DSCAPS
{
    DWORD   dwSize;
    DWORD   dwFlags;
    DWORD   dwMinSecondarySampleRate;
    DWORD   dwMaxSecondarySampleRate;
    DWORD   dwPrimaryBuffers;
    DWORD   dwMaxHwMixingAllBuffers;
    DWORD   dwMaxHwMixingStaticBuffers;
    DWORD   dwMaxHwMixingStreamingBuffers;
    DWORD   dwFreeHwMixingAllBuffers;
    DWORD   dwFreeHwMixingStaticBuffers;
    DWORD   dwFreeHwMixingStreamingBuffers;
    DWORD   dwMaxHw3DAllBuffers;
    DWORD   dwMaxHw3DStaticBuffers;
    DWORD   dwMaxHw3DStreamingBuffers;
    DWORD   dwFreeHw3DAllBuffers;
    DWORD   dwFreeHw3DStaticBuffers;
    DWORD   dwFreeHw3DStreamingBuffers;
    DWORD   dwTotalHwMemBytes;
    DWORD   dwFreeHwMemBytes;
    DWORD   dwMaxContigFreeHwMemBytes;
    DWORD   dwUnlockTransferRateHwBuffers;
    DWORD   dwPlayCpuOverheadSwBuffers;
    DWORD   dwReserved1;
    DWORD   dwReserved2;
} DSCAPS, *LPDSCAPS;

// 3D vector structure
// D3DVECTOR structure (only define if not already defined)
#ifndef D3DVECTOR_DEFINED
#define D3DVECTOR_DEFINED
typedef struct _D3DVECTOR {
    float x;
    float y;
    float z;
} D3DVECTOR;
#endif

//=============================================================================
// OpenAL Backend Classes
//=============================================================================

class OpenALContext
{
public:
    static OpenALContext& getInstance();
    
    bool initialize();
    void shutdown();
    
    ALCdevice* getDevice() const { return m_device; }
    ALCcontext* getContext() const { return m_context; }
    
    bool isInitialized() const { return m_initialized; }

private:
    OpenALContext() = default;
    ~OpenALContext();
    
    ALCdevice* m_device = nullptr;
    ALCcontext* m_context = nullptr;
    bool m_initialized = false;
};

class DirectSoundBuffer
{
public:
    DirectSoundBuffer(const DSBUFFERDESC* desc);
    ~DirectSoundBuffer();
    
    // Buffer management
    HRESULT Lock(DWORD offset, DWORD bytes, void** audioPtr1, DWORD* audioBytes1,
                 void** audioPtr2, DWORD* audioBytes2, DWORD flags);
    HRESULT Unlock(void* audioPtr1, DWORD audioBytes1, void* audioPtr2, DWORD audioBytes2);
    
    // Playback control
    HRESULT Play(DWORD reserved1, DWORD priority, DWORD flags);
    HRESULT Stop();
    HRESULT SetCurrentPosition(DWORD position);
    HRESULT GetCurrentPosition(DWORD* playPos, DWORD* writePos);
    
    // Audio properties
    HRESULT SetVolume(LONG volume);
    HRESULT GetVolume(LONG* volume);
    HRESULT SetPan(LONG pan);
    HRESULT GetPan(LONG* pan);
    HRESULT SetFrequency(DWORD frequency);
    HRESULT GetFrequency(DWORD* frequency);
    
    // Status
    HRESULT GetStatus(DWORD* status);
    
    // 3D interface
    HRESULT QueryInterface(const void* riid, void** ppvObj);
    
    ALuint getSource() const { return m_source; }
    ALuint getBuffer() const { return m_buffer; }

private:
    ALuint m_source;
    ALuint m_buffer;
    std::vector<unsigned char> m_audioData;
    DWORD m_bufferSize;
    DWORD m_frequency;
    DWORD m_flags;
    bool m_is3D;
    bool m_isPlaying;
    bool m_isLooping;
    LONG m_volume;      // DirectSound volume (-10000 to 0)
    LONG m_pan;         // DirectSound pan (-10000 to 10000)
    
    void updateVolume();
    void updatePan();
    float directSoundVolumeToOpenAL(LONG dsVolume);
    void directSoundPanToOpenAL(LONG dsPan, float& leftGain, float& rightGain);
};

class DirectSound3DBuffer
{
public:
    DirectSound3DBuffer(DirectSoundBuffer* buffer);
    ~DirectSound3DBuffer() = default;
    
    // 3D positioning
    HRESULT SetPosition(float x, float y, float z, DWORD applyMode);
    HRESULT GetPosition(D3DVECTOR* position);
    HRESULT SetVelocity(float x, float y, float z, DWORD applyMode);
    HRESULT GetVelocity(D3DVECTOR* velocity);
    
    // 3D properties
    HRESULT SetMinDistance(float minDistance, DWORD applyMode);
    HRESULT GetMinDistance(float* minDistance);
    HRESULT SetMaxDistance(float maxDistance, DWORD applyMode);
    HRESULT GetMaxDistance(float* maxDistance);
    HRESULT SetMode(DWORD mode, DWORD applyMode);
    HRESULT GetMode(DWORD* mode);

private:
    DirectSoundBuffer* m_buffer;
    D3DVECTOR m_position;
    D3DVECTOR m_velocity;
    float m_minDistance;
    float m_maxDistance;
    DWORD m_mode;
    
    void applyChanges();
};

class DirectSound3DListener
{
public:
    DirectSound3DListener();
    ~DirectSound3DListener() = default;
    
    // Listener positioning
    HRESULT SetPosition(float x, float y, float z, DWORD applyMode);
    HRESULT GetPosition(D3DVECTOR* position);
    HRESULT SetVelocity(float x, float y, float z, DWORD applyMode);
    HRESULT GetVelocity(D3DVECTOR* velocity);
    HRESULT SetOrientation(float xFront, float yFront, float zFront,
                          float xTop, float yTop, float zTop, DWORD applyMode);
    HRESULT GetOrientation(D3DVECTOR* orientFront, D3DVECTOR* orientTop);
    
    // Environmental properties
    HRESULT SetDistanceFactor(float factor, DWORD applyMode);
    HRESULT GetDistanceFactor(float* factor);
    HRESULT SetDopplerFactor(float factor, DWORD applyMode);
    HRESULT GetDopplerFactor(float* factor);
    HRESULT SetRolloffFactor(float factor, DWORD applyMode);
    HRESULT GetRolloffFactor(float* factor);

private:
    D3DVECTOR m_position;
    D3DVECTOR m_velocity;
    D3DVECTOR m_orientFront;
    D3DVECTOR m_orientTop;
    float m_distanceFactor;
    float m_dopplerFactor;
    float m_rolloffFactor;
    
    void applyChanges();
};

//=============================================================================
// DirectSound Interface Implementations
//=============================================================================

struct IDirectSoundBuffer8
{
    virtual ~IDirectSoundBuffer8() = default;
    
    // IUnknown methods
    virtual HRESULT QueryInterface(const void* riid, void** ppvObj) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    
    // IDirectSoundBuffer methods
    virtual HRESULT GetCaps(void* caps) = 0;
    virtual HRESULT GetCurrentPosition(DWORD* playPos, DWORD* writePos) = 0;
    virtual HRESULT GetFormat(void* wfxFormat, DWORD sizeAllocated, DWORD* sizeWritten) = 0;
    virtual HRESULT GetVolume(LONG* volume) = 0;
    virtual HRESULT GetPan(LONG* pan) = 0;
    virtual HRESULT GetFrequency(DWORD* frequency) = 0;
    virtual HRESULT GetStatus(DWORD* status) = 0;
    virtual HRESULT Initialize(IDirectSound8* directSound, const DSBUFFERDESC* desc) = 0;
    virtual HRESULT Lock(DWORD offset, DWORD bytes, void** audioPtr1, DWORD* audioBytes1,
                        void** audioPtr2, DWORD* audioBytes2, DWORD flags) = 0;
    virtual HRESULT Play(DWORD reserved1, DWORD priority, DWORD flags) = 0;
    virtual HRESULT SetCurrentPosition(DWORD position) = 0;
    virtual HRESULT SetFormat(const void* wfxFormat) = 0;
    virtual HRESULT SetVolume(LONG volume) = 0;
    virtual HRESULT SetPan(LONG pan) = 0;
    virtual HRESULT SetFrequency(DWORD frequency) = 0;
    virtual HRESULT Stop() = 0;
    virtual HRESULT Unlock(void* audioPtr1, DWORD audioBytes1, void* audioPtr2, DWORD audioBytes2) = 0;
    virtual HRESULT Restore() = 0;
    
    // IDirectSoundBuffer8 specific methods
    virtual HRESULT SetFX(DWORD effectsCount, void* fxDesc, DWORD* resultCodes) = 0;
    virtual HRESULT AcquireResources(DWORD flags, DWORD effectsCount, DWORD* resultCodes) = 0;
    virtual HRESULT GetObjectInPath(const void* guidObject, DWORD index, const void* guidInterface, void** ppvObj) = 0;
};

struct IDirectSound8
{
    virtual ~IDirectSound8() = default;
    
    // IUnknown methods
    virtual HRESULT QueryInterface(const void* riid, void** ppvObj) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    
    // IDirectSound methods
    virtual HRESULT CreateSoundBuffer(const DSBUFFERDESC* desc, IDirectSoundBuffer8** buffer, void* outer) = 0;
    virtual HRESULT GetCaps(DSCAPS* caps) = 0;
    virtual HRESULT DuplicateSoundBuffer(IDirectSoundBuffer8* original, IDirectSoundBuffer8** duplicate) = 0;
    virtual HRESULT SetCooperativeLevel(void* hwnd, DWORD level) = 0;
    virtual HRESULT Compact() = 0;
    virtual HRESULT GetSpeakerConfig(DWORD* speakerConfig) = 0;
    virtual HRESULT SetSpeakerConfig(DWORD speakerConfig) = 0;
    virtual HRESULT Initialize(const void* guid) = 0;
    
    // IDirectSound8 specific methods
    virtual HRESULT VerifyCertification(DWORD* certified) = 0;
};

struct IDirectSound3DListener8
{
    virtual ~IDirectSound3DListener8() = default;
    
    // IUnknown methods
    virtual HRESULT QueryInterface(const void* riid, void** ppvObj) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    
    // IDirectSound3DListener methods
    virtual HRESULT GetAllParameters(void* listener) = 0;
    virtual HRESULT GetDistanceFactor(float* factor) = 0;
    virtual HRESULT GetDopplerFactor(float* factor) = 0;
    virtual HRESULT GetOrientation(D3DVECTOR* orientFront, D3DVECTOR* orientTop) = 0;
    virtual HRESULT GetPosition(D3DVECTOR* position) = 0;
    virtual HRESULT GetRolloffFactor(float* factor) = 0;
    virtual HRESULT GetVelocity(D3DVECTOR* velocity) = 0;
    virtual HRESULT SetAllParameters(const void* listener, DWORD applyMode) = 0;
    virtual HRESULT SetDistanceFactor(float factor, DWORD applyMode) = 0;
    virtual HRESULT SetDopplerFactor(float factor, DWORD applyMode) = 0;
    virtual HRESULT SetOrientation(float xFront, float yFront, float zFront,
                                  float xTop, float yTop, float zTop, DWORD applyMode) = 0;
    virtual HRESULT SetPosition(float x, float y, float z, DWORD applyMode) = 0;
    virtual HRESULT SetRolloffFactor(float factor, DWORD applyMode) = 0;
    virtual HRESULT SetVelocity(float x, float y, float z, DWORD applyMode) = 0;
    virtual HRESULT CommitDeferredSettings() = 0;
};

struct IDirectSound3DBuffer8
{
    virtual ~IDirectSound3DBuffer8() = default;
    
    // IUnknown methods
    virtual HRESULT QueryInterface(const void* riid, void** ppvObj) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    
    // IDirectSound3DBuffer methods
    virtual HRESULT GetAllParameters(void* buffer) = 0;
    virtual HRESULT GetConeAngles(DWORD* insideConeAngle, DWORD* outsideConeAngle) = 0;
    virtual HRESULT GetConeOrientation(D3DVECTOR* orientation) = 0;
    virtual HRESULT GetConeOutsideVolume(LONG* coneOutsideVolume) = 0;
    virtual HRESULT GetMaxDistance(float* maxDistance) = 0;
    virtual HRESULT GetMinDistance(float* minDistance) = 0;
    virtual HRESULT GetMode(DWORD* mode) = 0;
    virtual HRESULT GetPosition(D3DVECTOR* position) = 0;
    virtual HRESULT GetVelocity(D3DVECTOR* velocity) = 0;
    virtual HRESULT SetAllParameters(const void* buffer, DWORD applyMode) = 0;
    virtual HRESULT SetConeAngles(DWORD insideConeAngle, DWORD outsideConeAngle, DWORD applyMode) = 0;
    virtual HRESULT SetConeOrientation(float x, float y, float z, DWORD applyMode) = 0;
    virtual HRESULT SetConeOutsideVolume(LONG coneOutsideVolume, DWORD applyMode) = 0;
    virtual HRESULT SetMaxDistance(float maxDistance, DWORD applyMode) = 0;
    virtual HRESULT SetMinDistance(float minDistance, DWORD applyMode) = 0;
    virtual HRESULT SetMode(DWORD mode, DWORD applyMode) = 0;
    virtual HRESULT SetPosition(float x, float y, float z, DWORD applyMode) = 0;
    virtual HRESULT SetVelocity(float x, float y, float z, DWORD applyMode) = 0;
};

//=============================================================================
// DirectSound API Functions
//=============================================================================

// Main DirectSound creation function
HRESULT DirectSoundCreate8(const void* deviceGuid, IDirectSound8** directSound, void* outer);

// DirectSound enumeration (stub for compatibility)
BOOL DirectSoundEnumerate(void* enumCallback, void* context);

#endif // !_WIN32

#endif // DSOUND_COMPAT_H_INCLUDED