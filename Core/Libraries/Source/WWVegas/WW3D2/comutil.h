#pragma once

// Windows COM utility compatibility header
// This header provides COM-related functionality for non-Windows systems

#ifndef _WIN32

#include "win32_compat.h"

// COM error codes
#define S_OK 0
#define S_FALSE 1
#define E_FAIL 0x80004005L
#define E_NOTIMPL 0x80004001L
#define E_OUTOFMEMORY 0x8007000EL
#define E_INVALIDARG 0x80070057L
#define E_NOINTERFACE 0x80004002L
#define E_POINTER 0x80004003L

// VARIANT types for COM compatibility
typedef struct tagVARIANT {
    WORD vt;
    WORD wReserved1;
    WORD wReserved2;
    WORD wReserved3;
    union {
        long lVal;
        unsigned char bVal;
        short iVal;
        float fltVal;
        double dblVal;
        BOOL boolVal;
        WCHAR* bstrVal;
        void* pVal;
    };
} VARIANT;

// _bstr_t equivalent class for basic string handling
class _bstr_t {
private:
    WCHAR* m_data;
    
public:
    _bstr_t() : m_data(nullptr) {}
    _bstr_t(const char* str) : m_data(nullptr) {
        if (str) {
            // Simple ASCII to WCHAR conversion
            size_t len = strlen(str);
            m_data = new WCHAR[len + 1];
            for (size_t i = 0; i <= len; ++i) {
                m_data[i] = static_cast<WCHAR>(str[i]);
            }
        }
    }
    
    ~_bstr_t() {
        delete[] m_data;
    }
    
    operator const WCHAR*() const { return m_data; }
    const WCHAR* GetBSTR() const { return m_data; }
};

// _variant_t equivalent class
class _variant_t {
private:
    VARIANT m_var;
    
public:
    _variant_t() { memset(&m_var, 0, sizeof(m_var)); }
    _variant_t(const _bstr_t& str) {
        memset(&m_var, 0, sizeof(m_var));
        m_var.vt = 8; // VT_BSTR
        m_var.bstrVal = const_cast<WCHAR*>(str.GetBSTR());
    }
    
    operator VARIANT&() { return m_var; }
    operator const VARIANT&() const { return m_var; }
};

#else
// On Windows, include the real comutil.h
#include <comutil.h>
#endif
