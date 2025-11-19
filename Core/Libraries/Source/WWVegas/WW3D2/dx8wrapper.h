/*
 * dx8wrapper.h (Compatibility Redirect)
 * 
 * Phase 39.4 Compatibility Layer
 * This file provides a compatibility redirect from the old DirectX 8 wrapper
 * to the new stub implementation.
 * 
 * Purpose: Allow legacy #include "dx8wrapper.h" directives to work without
 * modification while using the new stub implementations.
 * 
 * Created: November 18, 2025
 */

#ifndef __DX8WRAPPER_H__
#define __DX8WRAPPER_H__

// Redirect to the stub implementation
#include "DX8Wrapper_Stubs.h"

#endif // __DX8WRAPPER_H__
