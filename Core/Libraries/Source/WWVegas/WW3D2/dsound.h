#pragma once

// DirectSound compatibility header for cross-platform audio
// This header provides DirectSound API compatibility using OpenAL backend

#ifndef DSOUND_H_INCLUDED
#define DSOUND_H_INCLUDED

#ifdef _WIN32
// On Windows, include the real DirectSound headers
#include <dsound.h>
#else
// On non-Windows systems, use our OpenAL-based compatibility layer
#include "dsound_compat.h"

// Additional DirectSound types not covered in dsound_compat.h
#ifndef DIRECT_SOUND_VERSION
#define DIRECT_SOUND_VERSION 0x0800
#endif

// Additional constants
#define DSSCL_NORMAL            0x00000001
#define DSSCL_PRIORITY          0x00000002
#define DSSCL_EXCLUSIVE         0x00000003
#define DSSCL_WRITEPRIMARY      0x00000004

// Additional flags
#define DSBPLAY_LOOPING         0x00000001
#define DSBPLAY_LOCHARDWARE     0x00000002
#define DSBPLAY_LOCSOFTWARE     0x00000004
#define DSBPLAY_TERMINATEBY_TIME      0x00000008
#define DSBPLAY_TERMINATEBY_DISTANCE  0x00000010
#define DSBPLAY_TERMINATEBY_PRIORITY  0x00000020

// Lock flags
#define DSBLOCK_FROMWRITECURSOR 0x00000001
#define DSBLOCK_ENTIREBUFFER    0x00000002

// Apply mode for 3D settings
#define DS3D_IMMEDIATE          0x00000000
#define DS3D_DEFERRED           0x00000001

// Additional error codes
#define DSERR_PRIOLEVELNEEDED   0x88780046
#define DSERR_BADFORMAT         0x88780064

// Additional speaker configurations
#define DSSPEAKER_COMBINED(c, g)    ((DWORD)(((BYTE)(c)) | ((DWORD)((BYTE)(g))) << 16))
#define DSSPEAKER_CONFIG(a)         ((BYTE)(a))
#define DSSPEAKER_GEOMETRY(a)       ((BYTE)(((DWORD)(a) >> 16) & 0x00FF))

#define DSSPEAKER_GEOMETRY_MIN      0x00000005
#define DSSPEAKER_GEOMETRY_NARROW   0x0000000A
#define DSSPEAKER_GEOMETRY_WIDE     0x00000014
#define DSSPEAKER_GEOMETRY_MAX      0x000000B4

// GUID definitions (simplified for compatibility)
typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;

// Common GUIDs used by DirectSound (stub definitions)
static const GUID DSDEVID_DefaultPlayback = {0xDEF00000, 0x9C6D, 0x47ED, {0xAA, 0xF1, 0x4D, 0xDA, 0x8F, 0x2B, 0x5C, 0x03}};
static const GUID IID_IDirectSound8 = {0xC50A7E93, 0xF395, 0x4834, {0x9E, 0xF6, 0x7F, 0xA9, 0x9D, 0xE5, 0x09, 0x66}};
static const GUID IID_IDirectSoundBuffer8 = {0x6825A449, 0x7524, 0x4D82, {0x92, 0x0F, 0x50, 0xE3, 0x6A, 0xB3, 0xAB, 0x1E}};
static const GUID IID_IDirectSound3DListener = {0x279AFA84, 0x4981, 0x11CE, {0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60}};
static const GUID IID_IDirectSound3DBuffer = {0x279AFA86, 0x4981, 0x11CE, {0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60}};

// Additional buffer description flags
#define DSBCAPS_GETCURRENTPOSITION2 0x00010000

// 3D algorithm GUIDs (stubs)
static const GUID DS3DALG_DEFAULT = {0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
static const GUID DS3DALG_NO_VIRTUALIZATION = {0xC241333F, 0x1C1B, 0x11D2, {0x94, 0xF5, 0x00, 0xC0, 0x4F, 0xC2, 0x8A, 0xCA}};
static const GUID DS3DALG_HRTF_FULL = {0xC2413340, 0x1C1B, 0x11D2, {0x94, 0xF5, 0x00, 0xC0, 0x4F, 0xC2, 0x8A, 0xCA}};
static const GUID DS3DALG_HRTF_LIGHT = {0xC2413342, 0x1C1B, 0x11D2, {0x94, 0xF5, 0x00, 0xC0, 0x4F, 0xC2, 0x8A, 0xCA}};

#endif // _WIN32

#endif // DSOUND_H_INCLUDED