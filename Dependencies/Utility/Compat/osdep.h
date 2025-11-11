#pragma once

// Minimal osdep stub for non-Windows builds.
// Add platform-specific helpers here if needed by legacy code.

#ifndef _OSDEP_H_STUB
#define _OSDEP_H_STUB

#include <unistd.h>
#include <sys/types.h>

// Provide common macros expected by legacy headers
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#endif // _OSDEP_H_STUB
