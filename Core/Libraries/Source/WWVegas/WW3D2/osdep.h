#pragma once

/*
 * Operating System Dependency Header for macOS/Unix
 * This file provides compatibility definitions for Unix-like systems
 */

#ifdef _UNIX

// Basic type definitions
#ifndef __cplusplus
typedef enum { false = 0, true = 1 } bool;
#endif

// Function calling conventions (not needed on Unix)
#define __cdecl
#define __stdcall
#define __fastcall

// Windows-specific types mapped to Unix equivalents
#ifndef DWORD
typedef unsigned long DWORD;
#endif

#ifndef WORD
typedef unsigned short WORD;
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// Memory alignment macros
#ifdef __APPLE__
#define ALIGN(n) __attribute__((aligned(n)))
#else
#define ALIGN(n) __attribute__((aligned(n)))
#endif

// Inline assembly compatibility
#define __asm asm

#endif // _UNIX
