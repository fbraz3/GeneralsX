#pragma once
#ifndef _OBJBASE_H_
#define _OBJBASE_H_

// Object base compatibility for macOS port
#ifdef __APPLE__

#include "windows.h"
#include <stddef.h>

// Include our compatibility first to avoid conflicts
#include "../../Source/WWVegas/WW3D2/win32_compat.h"

// Basic types
typedef size_t SIZE_T;

// OLE string types
typedef wchar_t OLECHAR;
typedef OLECHAR* LPOLESTR;
typedef const OLECHAR* LPCOLESTR;

// COM basic types - check if already defined and avoid conflicts
#ifndef _GUID_DEFINED
#define _GUID_DEFINED
// GUID is already defined in win32_compat.h, so skip redefinition
/*
typedef struct _GUID {
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID;
*/
#endif

#ifndef IID
typedef GUID IID;
#endif
#ifndef CLSID
typedef GUID CLSID;
#endif
#ifndef LPGUID
typedef GUID* LPGUID;
#endif
#ifndef LPIID
typedef IID* LPIID;
#endif
#ifndef LPCLSID
typedef CLSID* LPCLSID;
#endif

// Reference IID
#define REFIID const IID&
#define REFCLSID const CLSID&
#define REFGUID const GUID&

// COM result codes
#define S_OK                    0x00000000L
#define S_FALSE                 0x00000001L
#define E_NOTIMPL               0x80004001L
#define E_NOINTERFACE           0x80004002L
#define E_POINTER               0x80004003L
#define E_ABORT                 0x80004004L
#define E_FAIL                  0x80004005L
#define E_UNEXPECTED            0x8000FFFFL

// SUCCEEDED and FAILED macros
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)

// COM initialization
typedef enum tagCOINIT {
    COINIT_APARTMENTTHREADED  = 0x2,
    COINIT_MULTITHREADED      = 0x0,
    COINIT_DISABLE_OLE1DDE    = 0x4,
    COINIT_SPEED_OVER_MEMORY  = 0x8
} COINIT;

// COM function stubs
inline HRESULT CoInitialize(LPVOID pvReserved) {
    return S_OK; // Stub
}

inline HRESULT CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit) {
    return S_OK; // Stub
}

// CoUninitialize is already defined as macro in win32_compat.h, so skip it here
/*
inline void CoUninitialize() {
    // Stub
}
*/

inline HRESULT CoCreateInstance(REFCLSID rclsid, IUnknown* pUnkOuter, 
                               DWORD dwClsContext, REFIID riid, LPVOID* ppv) {
    return E_NOINTERFACE; // Stub - no COM objects available
}

inline HRESULT CoGetMalloc(DWORD dwMemContext, void** ppMalloc) {
    return E_NOINTERFACE; // Stub
}

// Memory allocation (use standard malloc/free)
inline LPVOID CoTaskMemAlloc(SIZE_T cb) {
    return malloc(cb);
}

inline void CoTaskMemFree(LPVOID pv) {
    free(pv);
}

// GUID comparison
inline BOOL IsEqualGUID(REFGUID rguid1, REFGUID rguid2) {
    return memcmp(&rguid1, &rguid2, sizeof(GUID)) == 0;
}

#define IsEqualIID(riid1, riid2) IsEqualGUID(riid1, riid2)
#define IsEqualCLSID(rclsid1, rclsid2) IsEqualGUID(rclsid1, rclsid2)

// Class context
#define CLSCTX_INPROC_SERVER    0x1
#define CLSCTX_INPROC_HANDLER   0x2
#define CLSCTX_LOCAL_SERVER     0x4
#define CLSCTX_REMOTE_SERVER    0x10
#define CLSCTX_ALL              (CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER)

#endif // __APPLE__
#endif // _OBJBASE_H_
