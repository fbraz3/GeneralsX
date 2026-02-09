#pragma once

#ifndef CALLBACK
#define CALLBACK
#endif

#if !defined(__stdcall)
#define __stdcall
#endif

#ifndef WINAPI
#define WINAPI
#endif

#if !defined _MSC_VER
#if !defined(__cdecl)
#if defined __has_attribute && __has_attribute(cdecl) && defined(__i386__)
#define __cdecl __attribute__((cdecl))
#else
#define __cdecl
#endif
#endif /* !defined __cdecl */
#endif

#ifndef __forceinline
#if defined __has_attribute && __has_attribute(always_inline)
#define __forceinline __attribute__((always_inline)) inline
#else
#define __forceinline inline
#endif
#endif

static unsigned int GetDoubleClickTime()
{
  return 500;
}

#include "types_compat.h"

#define HIWORD(value) ((((uint32_t)(value) >> 16) & 0xFFFF))
#define LOWORD(value) (((uint32_t)(value) & 0xFFFF))

// Copied from windows_base.h from DXVK-Native
#ifndef MAKE_HRESULT
#define MAKE_HRESULT(sev, fac, code) \
	((HRESULT)(((unsigned long)(sev) << 31) | ((unsigned long)(fac) << 16) | ((unsigned long)(code))))
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif

#ifndef MAX_PATH
#define MAX_PATH _MAX_PATH
#endif

#ifndef _MAX_DIR
#define _MAX_DIR _MAX_PATH
#endif

#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif

#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif

#include "thread_compat.h"
#include "tchar_compat.h"
#include "time_compat.h"
#include "string_compat.h"
#include "keyboard_compat.h"
#include "memory_compat.h"
#include "module_compat.h"
#include "wchar_compat.h"
#include "gdi_compat.h"
#include "wnd_compat.h"
#include "file_compat.h"
//#include "intrin_compat.h"
