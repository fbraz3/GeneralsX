#pragma once

#ifndef D3D8_AUDIO_COMPAT_H_INCLUDED
#define D3D8_AUDIO_COMPAT_H_INCLUDED

#ifndef _WIN32

// ============================================================================
// DirectSound Audio Compatibility Layer
// 
// PHASE 51 COMPONENT: This layer provides DirectSound→OpenAL mapping for
// audio playback on non-Windows platforms (macOS, Linux).
// 
// STATUS: Deferred to Phase 51 (audio implementation)
// Currently: Not included in Phase 50 (graphics-only refactor)
// ============================================================================

// DirectSound result codes
#define DS_OK 0x00000000L
#define DSERR_NODRIVER 0x88780078L
#define DSERR_OUTOFMEMORY 0x88780001L
#define DSERR_GENERIC 0x80004005L

// DirectSound device creation flags
#define DSSCL_NORMAL 0x00000001L
#define DSSCL_PRIORITY 0x00000002L
#define DSSCL_EXCLUSIVE 0x00000003L
#define DSSCL_WRITEPRIMARY 0x00000004L

// DirectSound buffer capabilities
#define DSBCAPS_PRIMARYBUFFER 0x00000001L
#define DSBCAPS_STATIC 0x00000002L
#define DSBCAPS_LOCHARDWARE 0x00000004L
#define DSBCAPS_LOCSOFTWARE 0x00000008L
#define DSBCAPS_CTRL3D 0x00000010L
#define DSBCAPS_CTRLFREQUENCY 0x00000020L
#define DSBCAPS_CTRLPAN 0x00000040L
#define DSBCAPS_CTRLVOLUME 0x00000080L
#define DSBCAPS_CTRLPOSITIONNOTIFY 0x00000100L
#define DSBCAPS_CTRLFX 0x00000200L

// ============================================================================
// Forward Declarations (Placeholder for Phase 51)
// ============================================================================

struct IDirectSound8;
struct IDirectSoundBuffer8;
struct IDirectSoundBuffer3D;
struct IDirectSound3DListener8;

// ============================================================================
// WAVEFORMATEX Structure (Audio Format Specification)
// 
// Phase 51: Will map to OpenAL audio format specifications
// ============================================================================

#ifndef WAVEFORMATEX_DEFINED
#define WAVEFORMATEX_DEFINED

typedef struct {
    unsigned short wFormatTag;      // Audio format (1 = PCM)
    unsigned short nChannels;       // Number of channels (1=mono, 2=stereo)
    unsigned long nSamplesPerSec;   // Sample rate (e.g., 44100, 48000)
    unsigned long nAvgBytesPerSec;  // Average bytes per second
    unsigned short nBlockAlign;     // Block alignment
    unsigned short wBitsPerSample;  // Bits per sample
    unsigned short cbSize;          // Extra format bytes
} WAVEFORMATEX;

// WAVEFORMATEX format tags
#define WAVE_FORMAT_PCM 0x0001
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#define WAVE_FORMAT_ALAW 0x0006
#define WAVE_FORMAT_MULAW 0x0007

#endif // WAVEFORMATEX_DEFINED

// ============================================================================
// DSBUFFERDESC Structure (Buffer Description)
// 
// Phase 51: Describes audio buffer parameters for OpenAL buffer creation
// ============================================================================

#ifndef DSBUFFERDESC_DEFINED
#define DSBUFFERDESC_DEFINED

typedef struct {
    unsigned long dwSize;           // Size of this structure
    unsigned long dwFlags;          // Flags (DSBCAPS_*)
    unsigned long dwBufferBytes;    // Buffer size in bytes
    unsigned long dwReserved;       // Reserved
    WAVEFORMATEX* lpwfxFormat;      // Pointer to WAVEFORMATEX structure
    unsigned long dwFX;             // Number of effects
    void* pdwFxAlgo;                // Array of effect GUIDs
} DSBUFFERDESC;

#endif // DSBUFFERDESC_DEFINED

// ============================================================================
// DS3DBUFFER Structure (3D Audio Buffer Parameters)
// 
// Phase 51: Maps to OpenAL 3D positioning and attenuation
// ============================================================================

#ifndef DS3DBUFFER_DEFINED
#define DS3DBUFFER_DEFINED

typedef struct {
    unsigned long dwSize;           // Size of this structure
    float vPosition[3];             // 3D position (x, y, z)
    float vVelocity[3];             // 3D velocity for Doppler effect
    float flMinDistance;            // Minimum distance for attenuation
    float flMaxDistance;            // Maximum distance for attenuation
    unsigned long dwMode;           // 3D mode (normal or headrelative)
    float flConeOutsideVolume;      // Volume outside cone
    float vConeOrientation[3];      // Cone direction
    float flConeInnerAngle;         // Inner cone angle
    float flConeOuterAngle;         // Outer cone angle
} DS3DBUFFER;

#endif // DS3DBUFFER_DEFINED

// ============================================================================
// DS3DLISTENER Structure (3D Listener Parameters)
// 
// Phase 51: Maps to OpenAL listener position and orientation
// ============================================================================

#ifndef DS3DLISTENER_DEFINED
#define DS3DLISTENER_DEFINED

typedef struct {
    unsigned long dwSize;           // Size of this structure
    float vPosition[3];             // Listener position
    float vVelocity[3];             // Listener velocity
    float vOrientFront[3];          // Listener look-at direction
    float vOrientTop[3];            // Listener up direction
    float flDistanceFactor;         // Scaling factor for distances
    float flRolloffFactor;          // Rolloff factor for attenuation
    float flDopplerFactor;          // Doppler effect scaling
} DS3DLISTENER;

#endif // DS3DLISTENER_DEFINED

// ============================================================================
// 3D Audio Mode Flags
// ============================================================================

#define DS3DMODE_NORMAL 0x00000000L         // Normal 3D audio
#define DS3DMODE_HEADRELATIVE 0x00000001L   // Relative to listener head
#define DS3DMODE_DISABLE 0x00000002L        // Disable 3D processing

// ============================================================================
// Note on Phase 51 Implementation
// ============================================================================

// This compatibility layer will be populated during Phase 51:
// - Map IDirectSound8 interface methods to OpenAL function calls
// - Implement DSBUFFERDESC → OpenAL buffer creation
// - Implement DS3DBUFFER → OpenAL source positioning
// - Implement DS3DLISTENER → OpenAL listener setup
// 
// Reference implementation patterns:
// 1. Create OpenAL context on IDirectSound8::Initialize()
// 2. Map WAVEFORMATEX to OpenAL format enums (AL_FORMAT_MONO16, AL_FORMAT_STEREO16, etc.)
// 3. Create OpenAL sources for each IDirectSoundBuffer8
// 4. Implement 3D positional audio via alSource3f(source, AL_POSITION, ...)
// 5. Map DirectSound volume/pan to OpenAL gain/panning

#endif // !_WIN32

#endif // D3D8_AUDIO_COMPAT_H_INCLUDED
