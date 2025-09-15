#pragma once
#ifndef _IMAGEHLP_H_
#define _IMAGEHLP_H_

// ImageHlp compatibility for macOS port
#ifdef __APPLE__

#include "../Source/WWVegas/WW3D2/win32_compat.h"

// 64-bit types
typedef uint64_t DWORD64;

// Image help constants
#define SYMOPT_UNDNAME 0x00000002
#define SYMOPT_DEFERRED_LOADS 0x00000004
#define SYMOPT_NO_CPP 0x00000008
#define SYMOPT_LOAD_LINES 0x00000010
#define SYMOPT_DEBUG 0x80000000

// Symbol types
#define SymTagFunction 5
#define SymTagData 7

// Symbol info structure
typedef struct _SYMBOL_INFO {
    ULONG SizeOfStruct;
    ULONG TypeIndex;
    ULONG Reserved[2];
    ULONG Index;
    ULONG Size;
    ULONG ModBase;
    ULONG Flags;
    ULONG Value;
    ULONG Address;
    ULONG Register;
    ULONG Scope;
    ULONG Tag;
    ULONG NameLen;
    ULONG MaxNameLen;
    CHAR Name[1];
} SYMBOL_INFO, *PSYMBOL_INFO;

// Stack frame structure
typedef struct _STACKFRAME64 {
    ULONG AddrPC;
    ULONG AddrReturn;
    ULONG AddrFrame;
    ULONG AddrStack;
    ULONG AddrBStore;
    LPVOID FuncTableEntry;
    ULONG Params[4];
    BOOL Far;
    BOOL Virtual;
    ULONG Reserved[3];
} STACKFRAME64, *LPSTACKFRAME64;

// Context stub
typedef struct _CONTEXT64 {
    ULONG dummy;
} CONTEXT64;

// Image help function stubs
inline BOOL SymInitialize(HANDLE hProcess, LPCSTR UserSearchPath, BOOL fInvadeProcess) {
    return FALSE; // Stub
}

inline BOOL SymCleanup(HANDLE hProcess) {
    return FALSE; // Stub
}

inline DWORD SymSetOptions(DWORD SymOptions) {
    return 0; // Stub
}

inline BOOL SymFromAddr(HANDLE hProcess, DWORD64 Address, DWORD64* Displacement, PSYMBOL_INFO Symbol) {
    return FALSE; // Stub
}

inline BOOL StackWalk64(DWORD MachineType, HANDLE hProcess, HANDLE hThread, 
                       LPSTACKFRAME64 StackFrame, LPVOID ContextRecord,
                       void* ReadMemoryRoutine, void* FunctionTableAccessRoutine,
                       void* GetModuleBaseRoutine, void* TranslateAddress) {
    return FALSE; // Stub
}

// Machine types
#define IMAGE_FILE_MACHINE_I386 0x014c
#define IMAGE_FILE_MACHINE_AMD64 0x8664

#endif // __APPLE__
#endif // _IMAGEHLP_H_
