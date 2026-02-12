#pragma once
// COM/DirectX Interface Compatibility Macros
// Include this ONLY before including DirectX headers (d3d8.h, d3d9.h, etc.)
// Do NOT include before GLI/GLM or other template-heavy headers

#include <string.h>  // for memcmp

// GUID structure (Component Object Model)
// Use DXVK-compatible guard name (GUID_DEFINED, not _GUID_DEFINED)
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct GUID {  // DXVK uses "struct GUID", not "struct _GUID"
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;
#endif

// GUID comparison helpers
#ifndef IsEqualGUID
inline int IsEqualGUID(const GUID &a, const GUID &b) {
    return memcmp(&a, &b, sizeof(GUID)) == 0;
}
#endif

// GUID reference types
#ifndef REFGUID_DEFINED
#define REFGUID_DEFINED
#ifdef __cplusplus
typedef const GUID &REFGUID;
typedef const GUID &REFIID;
typedef const GUID &REFCLSID;
#else
typedef const GUID *REFGUID;
typedef const GUID *REFIID;
typedef const GUID *REFCLSID;
#endif
#endif

// IID/CLSID types
typedef GUID IID;
typedef GUID CLSID;

// GUID definition macro (from guiddef.h)
#ifndef DEFINE_GUID
#ifdef __cplusplus
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    extern "C" const GUID name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }
#else
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    const GUID name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }
#endif
#endif

// COM interface keyword (Windows-only)
#ifndef interface
#define interface struct
#endif

// COM/DirectX interface declaration macros (from objbase.h/unknwn.h)
#ifndef DECLARE_INTERFACE
#define DECLARE_INTERFACE(iface)    interface DECLSPEC_NOVTABLE iface
#endif

#ifndef DECLARE_INTERFACE_
#define DECLARE_INTERFACE_(iface, baseiface)    interface DECLSPEC_NOVTABLE iface : public baseiface
#endif

#ifndef DECLSPEC_NOVTABLE
#if defined(__GNUC__) || defined(__clang__)
#define DECLSPEC_NOVTABLE
#else
#define DECLSPEC_NOVTABLE __declspec(novtable)
#endif
#endif

// COM method declarations
#ifndef PURE
#define PURE = 0
#endif

#ifndef THIS_
#define THIS_
#endif

#ifndef THIS
#define THIS void
#endif

#ifndef STDMETHOD
#define STDMETHOD(method) virtual HRESULT STDMETHODCALLTYPE method
#endif

#ifndef STDMETHOD_
#define STDMETHOD_(type,method) virtual type STDMETHODCALLTYPE method
#endif

#ifndef STDMETHODCALLTYPE
#ifdef _MSC_VER
#define STDMETHODCALLTYPE __stdcall
#else
#define STDMETHODCALLTYPE
#endif
#endif

// IUnknown - Base COM interface (all DirectX interfaces inherit from this)
// TheSuperHackers @build BenderAI 11/02/2026 Added DXVK guard to prevent redefinition
#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__
#define _IUNKNOWN_DEFINED  // Prevent DXVK's unknwn.h from redefining
DECLARE_INTERFACE(IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
};
#endif /* __IUnknown_INTERFACE_DEFINED__ */
