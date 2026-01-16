/*
 * immintrin.h - Compatibility shim for VC6 to provide Intel intrinsics
 * Provides minimal stubs for SDL2 compatibility on older MSVC versions
 */

#pragma once

/* VC6 doesn't have Intel intrinsics, provide empty stubs for compatibility */
#define _MM_SET_EXCEPTION_MASK(a)
#define _MM_GET_EXCEPTION_MASK() 0
#define _MM_GET_EXCEPTION_STATE() 0
#define _MM_SET_EXCEPTION_STATE(a)
#define _MM_GET_ROUNDING_MODE() 0
#define _MM_SET_ROUNDING_MODE(a)
