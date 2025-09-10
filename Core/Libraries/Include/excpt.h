#pragma once
#ifndef _EXCPT_H_
#define _EXCPT_H_

// Exception handling compatibility for macOS port
#ifdef __APPLE__

// Exception handling constants
#define EXCEPTION_EXECUTE_HANDLER 1
#define EXCEPTION_CONTINUE_SEARCH 0
#define EXCEPTION_CONTINUE_EXECUTION -1

// Exception filter result type
typedef int EXCEPTION_DISPOSITION;

// Exception record structure stub
typedef struct _EXCEPTION_RECORD {
    unsigned long ExceptionCode;
    unsigned long ExceptionFlags;
    struct _EXCEPTION_RECORD* ExceptionRecord;
    void* ExceptionAddress;
    unsigned long NumberParameters;
    unsigned long ExceptionInformation[15];
} EXCEPTION_RECORD;

// Exception context stub
typedef struct _CONTEXT {
    unsigned long dummy;
} CONTEXT;

// Exception pointers
typedef struct _EXCEPTION_POINTERS {
    EXCEPTION_RECORD* ExceptionRecord;
    CONTEXT* ContextRecord;
} EXCEPTION_POINTERS;

// SEH macros (structured exception handling)
#define __try try
#define __except(x) catch(...)
#define __finally 
#define __leave break

// Exception filter function type
typedef EXCEPTION_DISPOSITION (*PEXCEPTION_ROUTINE)(
    EXCEPTION_RECORD* ExceptionRecord,
    void* EstablisherFrame,
    CONTEXT* ContextRecord,
    void* DispatcherContext
);

// Stub functions
inline unsigned long GetExceptionCode() { return 0; }
inline EXCEPTION_POINTERS* GetExceptionInformation() { return nullptr; }

#endif // __APPLE__
#endif // _EXCPT_H_
