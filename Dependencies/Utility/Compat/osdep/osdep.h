#pragma once

// Minimal osdep stub for non-Windows builds (placed under Compat/osdep/ to satisfy
// includes like "osdep/osdep.h").

#ifndef _OSDEP_H_STUB
#define _OSDEP_H_STUB

#include <unistd.h>
#include <sys/types.h>

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#endif // _OSDEP_H_STUB
