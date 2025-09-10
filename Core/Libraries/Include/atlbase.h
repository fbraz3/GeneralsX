#pragma once
#ifndef _ATLBASE_H_
#define _ATLBASE_H_

// ATL compatibility for macOS port
#ifdef __APPLE__

#include "windows.h"
#include <wchar.h>
#include <stdlib.h>

// Common HRESULT values (moved before use)
#define S_OK 0
#define S_FALSE 1
#define E_FAIL 0x80004005L
#define E_NOINTERFACE 0x80004002L

// COM interface stubs
struct IUnknown {
    virtual ~IUnknown() = default;
    virtual HRESULT QueryInterface(const void* riid, void** ppvObject) { return 0; }
    virtual ULONG AddRef() { return 1; }
    virtual ULONG Release() { return 0; }
};

// ATL smart pointer template
template<class T>
class CComPtr {
private:
    T* p;

public:
    CComPtr() : p(nullptr) {}
    CComPtr(T* ptr) : p(ptr) { if (p) p->AddRef(); }
    ~CComPtr() { if (p) p->Release(); }
    
    T* operator->() const { return p; }
    T& operator*() const { return *p; }
    operator T*() const { return p; }
    T** operator&() { return &p; }
    
    CComPtr& operator=(T* ptr) {
        if (p) p->Release();
        p = ptr;
        if (p) p->AddRef();
        return *this;
    }
    
    void Release() {
        if (p) {
            p->Release();
            p = nullptr;
        }
    }
    
    HRESULT CoCreateInstance(const void* clsid, IUnknown* outer = nullptr, DWORD context = 1) {
        return 0x80004005; // E_FAIL
    }
};

// ATL module base
class CComModule {
public:
    HRESULT Init(void*, HINSTANCE, const void*) { return 0; }
    void Term() {}
    HRESULT RegisterServer(BOOL = FALSE) { return 0; }
    HRESULT UnregisterServer(BOOL = FALSE) { return 0; }
};

extern CComModule _Module;

// ATL string class stub
class CComBSTR {
private:
    wchar_t* m_str;
    
public:
    CComBSTR() : m_str(nullptr) {}
    CComBSTR(const wchar_t* str) : m_str(nullptr) {
        if (str) {
            size_t len = wcslen(str);
            m_str = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
            wcscpy(m_str, str);
        }
    }
    ~CComBSTR() { if (m_str) free(m_str); }
    
    operator const wchar_t*() const { return m_str; }
    wchar_t** operator&() { return &m_str; }
};

// ATL variant stub
struct CComVariant {
    int vt;
    union {
        int intVal;
        float fltVal;
        wchar_t* bstrVal;
    };
    
    CComVariant() : vt(0), intVal(0) {}
    void Clear() { vt = 0; intVal = 0; }
};

#endif // __APPLE__
#endif // _ATLBASE_H_
