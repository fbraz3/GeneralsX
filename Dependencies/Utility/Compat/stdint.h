/*
 * stdint.h - Compatibility shim for VC6 to provide C99 standard integer types
 * This file provides SDL2 compatibility for systems using stdint_adapter.h
 */

#pragma once

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

/* Include the existing stdint_adapter.h which provides all standard integer types */
#include "../Utility/stdint_adapter.h"
