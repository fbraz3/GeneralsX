#pragma once

// COM Interface Pointer compatibility header
// This header provides COM smart pointer functionality for non-Windows systems

#ifndef _WIN32

#include "comutil.h"

// COM Interface Pointer templates
template<class T>
class _com_ptr_t {
private:
    T* m_ptr;
    
public:
    _com_ptr_t() : m_ptr(nullptr) {}
    _com_ptr_t(T* ptr) : m_ptr(ptr) {
        if (m_ptr) m_ptr->AddRef();
    }
    
    ~_com_ptr_t() {
        if (m_ptr) m_ptr->Release();
    }
    
    T* operator->() const { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    operator T*() const { return m_ptr; }
    
    T** operator&() { return &m_ptr; }
    
    _com_ptr_t& operator=(T* ptr) {
        if (m_ptr != ptr) {
            if (m_ptr) m_ptr->Release();
            m_ptr = ptr;
            if (m_ptr) m_ptr->AddRef();
        }
        return *this;
    }
    
    void Release() {
        if (m_ptr) {
            m_ptr->Release();
            m_ptr = nullptr;
        }
    }
    
    HRESULT CreateInstance(const char* progID) {
        // Stub implementation
        return E_NOTIMPL;
    }
};

// Common COM interface pointer types
typedef _com_ptr_t<struct IUnknown> IUnknownPtr;
typedef _com_ptr_t<struct IDispatch> IDispatchPtr;

#else
// On Windows, include the real comip.h
#include <comip.h>
#endif
