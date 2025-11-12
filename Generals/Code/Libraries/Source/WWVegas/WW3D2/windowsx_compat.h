/*
** Phase 03: Windows Extra API Compatibility Stub
** windowsx.h - Windows message macro compatibility
**
** Pattern: source_dest_type_compat
** Source: Windows windowsx.h (message macros)
** Destination: POSIX/Linux/macOS platforms
** Type: Windows API compatibility stubs
**
** windowsx.h provides utility macros for handling Windows messages.
** On non-Windows platforms, this is largely empty as we don't have Windows messages.
*/

#pragma once

#ifndef WINDOWSX_COMPAT_H_INCLUDED
#define WINDOWSX_COMPAT_H_INCLUDED

/* 
 * windowsx.h provides various macros for extracting parameters from Windows messages,
 * particularly WM_* messages. On non-Windows platforms, these macros are not needed
 * since we don't have Windows message handling.
 * 
 * The file is essentially a no-op on non-Windows platforms.
 */

#endif /* WINDOWSX_COMPAT_H_INCLUDED */
