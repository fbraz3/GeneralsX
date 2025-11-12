/**
 * @file mss_compat.h
 * @brief Miles Sound System (MSS) stub types for non-Windows platforms
 * @details Provides stub type definitions for mss.h when building on non-Windows platforms
 *
 * Pattern: source_dest_type_compat
 * Source: Miles Sound System (mss.h)
 * Destination: POSIX/Linux/macOS platforms
 * Type: Audio API compatibility stubs
 */

#pragma once

#ifndef MSS_COMPAT_H_INCLUDED
#define MSS_COMPAT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * PLATFORM DETECTION AND CALLING CONVENTION FIXES
 * ============================================================================ */

/**
 * @brief Platform calling convention (_stdcall)
 * 
 * On Windows, _stdcall is a calling convention. On other platforms, it's undefined.
 * We define it as empty to allow compilation on non-Windows platforms.
 */
#ifndef _stdcall
#define _stdcall  /* Empty on non-Windows platforms */
#endif

/**
 * @brief Generic HANDLE type
 * 
 * HANDLE is a Windows generic handle type used for various system resources.
 */
#ifndef HANDLE
typedef void* HANDLE;
#endif

/* ============================================================================
 * MILES SOUND SYSTEM STUB TYPES
 * These are placeholders for mss.h types on non-Windows platforms
 * ============================================================================ */

/**
 * @brief Handle to a 2D sound sample
 * 
 * HSAMPLE is used to reference sound samples in the Miles Sound System.
 * On non-Windows platforms, we define this as a void pointer for compatibility.
 */
typedef void* HSAMPLE;

/**
 * @brief Handle to a 3D sound sample
 * 
 * H3DSAMPLE is used to reference 3D sound samples in the Miles Sound System.
 * On non-Windows platforms, we define this as a void pointer for compatibility.
 */
typedef void* H3DSAMPLE;

/* Callback function pointer types are defined in AudioEvents.h */
/* We don't redefine them here to avoid conflicts */

/**
 * @brief DWORD type definition (if not already defined)
 * 
 * DWORD represents a 32-bit unsigned integer.
 * Note: bittype.h defines this as unsigned long, so we use ifdef to avoid conflict.
 */
#if !defined(DWORD)
typedef unsigned long DWORD;
#endif

/**
 * @brief WORD type definition (if not already defined)
 * 
 * WORD represents a 16-bit unsigned integer.
 */
#if !defined(WORD)
typedef unsigned short WORD;
#endif

/**
 * @brief BYTE type definition (if not already defined)
 * 
 * BYTE represents a 8-bit unsigned integer.
 */
#if !defined(BYTE)
typedef unsigned char BYTE;
#endif

/**
 * @brief U32 type definition (if not already defined)
 * 
 * U32 represents a 32-bit unsigned integer.
 */
#ifndef U32
typedef unsigned int U32;
#endif

/**
 * @brief S32 type definition (if not already defined)
 * 
 * S32 represents a 32-bit signed integer.
 */
#ifndef S32
typedef int S32;
#endif

/**
 * @brief WAVEFORMAT structure stub
 * 
 * LPWAVEFORMAT is a pointer to a WAVEFORMAT structure used in audio initialization.
 * This is a simplified stub version for compatibility.
 */
typedef struct WAVEFORMAT {
    unsigned short wFormatTag;
    unsigned short nChannels;
    unsigned int nSamplesPerSec;
    unsigned int nAvgBytesPerSec;
    unsigned short nBlockAlign;
} WAVEFORMAT;

typedef WAVEFORMAT* LPWAVEFORMAT;

/**
 * @brief Handle to an audio provider
 * 
 * HPROVIDER is used to reference audio providers/drivers in Miles Sound System.
 */
typedef void* HPROVIDER;

/**
 * @brief Handle to a 3D positional object
 * 
 * H3DPOBJECT is used to reference 3D objects (like listeners) in the audio system.
 */
typedef void* H3DPOBJECT;

/**
 * @brief Handle to a digital audio driver (2D or 3D)
 * 
 * HDIGDRIVER is used to reference a digital audio driver in the Miles Sound System.
 * This represents the actual audio device being used.
 */
typedef void* HDIGDRIVER;

/**
 * @brief Handle to a timer object
 * 
 * HTIMER is used to reference timer objects used for audio synchronization.
 */
typedef void* HTIMER;

/**
 * @brief Miles AIL callback macro
 * 
 * AILCALLBACK is used to define callback function signatures in AIL.
 * On most platforms, this is just an empty definition.
 */
#ifndef AILCALLBACK
#define AILCALLBACK
#endif

/**
 * @brief DRIVER_TYPE enumeration
 * 
 * Represents different audio driver types.
 */
typedef enum {
    AUDIO_DRIVER_NONE = 0,
    AUDIO_DRIVER_2D = 1,
    AUDIO_DRIVER_3D = 2
} DRIVER_TYPE;

#ifdef __cplusplus
}
#endif

#endif /* MSS_COMPAT_H_INCLUDED */
