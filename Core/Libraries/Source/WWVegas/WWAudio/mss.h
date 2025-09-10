#pragma once

// Miles Sound System compatibility header
// This header provides audio system compatibility for non-Windows systems

#ifndef _WIN32

#include "win32_compat.h"

// Miles Sound System types and constants
typedef void* HDIGDRIVER;
typedef void* HSAMPLE;
typedef void* HSTREAM;
typedef void* H3DSAMPLE;
typedef void* H3DPOBJECT;
typedef void* HPROVIDER;

// Audio format constants
#define DIG_F_MONO_8 0x00000001
#define DIG_F_MONO_16 0x00000002
#define DIG_F_STEREO_8 0x00000004
#define DIG_F_STEREO_16 0x00000008

// Sample status constants
#define SMP_FREE 0
#define SMP_DONE 1
#define SMP_PLAYING 2
#define SMP_STOPPED 3
#define SMP_PLAYINGBUTRELEASED 4

// Stream constants
#define STREAM_SEEK_SET 0
#define STREAM_SEEK_CUR 1
#define STREAM_SEEK_END 2

// Stub functions for Miles Sound System
inline HDIGDRIVER AIL_open_digital_driver(DWORD frequency, DWORD bits, DWORD channels, DWORD flags) { return nullptr; }
inline void AIL_close_digital_driver(HDIGDRIVER drvr) {}
inline HSAMPLE AIL_allocate_sample_handle(HDIGDRIVER drvr) { return nullptr; }
inline void AIL_release_sample_handle(HSAMPLE sample) {}
inline void AIL_init_sample(HSAMPLE sample) {}
inline DWORD AIL_sample_status(HSAMPLE sample) { return SMP_FREE; }
inline void AIL_start_sample(HSAMPLE sample) {}
inline void AIL_stop_sample(HSAMPLE sample) {}
inline void AIL_set_sample_address(HSAMPLE sample, void* start, DWORD len) {}
inline void AIL_set_sample_type(HSAMPLE sample, DWORD format, DWORD flags) {}
inline void AIL_set_sample_playback_rate(HSAMPLE sample, DWORD rate) {}
inline void AIL_set_sample_volume(HSAMPLE sample, DWORD volume) {}
inline void AIL_set_sample_pan(HSAMPLE sample, DWORD pan) {}

// 3D audio functions
inline H3DPOBJECT AIL_open_3D_provider(HPROVIDER lib) { return nullptr; }
inline void AIL_close_3D_provider(H3DPOBJECT obj) {}
inline H3DSAMPLE AIL_allocate_3D_sample_handle(H3DPOBJECT obj) { return nullptr; }
inline void AIL_release_3D_sample_handle(H3DSAMPLE sample) {}

// Stream functions
inline HSTREAM AIL_open_stream(HDIGDRIVER drvr, const char* filename, DWORD stream_mem) { return nullptr; }
inline void AIL_close_stream(HSTREAM stream) {}
inline void AIL_start_stream(HSTREAM stream) {}
inline void AIL_stop_stream(HSTREAM stream) {}
inline DWORD AIL_stream_status(HSTREAM stream) { return SMP_FREE; }

// Initialization functions
inline DWORD AIL_startup() { return 1; }
inline void AIL_shutdown() {}

#else
// On Windows, include the real mss.h
#include <mss.h>
#endif
