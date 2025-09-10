#ifndef BROWSERENGINE_H
#define BROWSERENGINE_H

// Stub header for EABrowserEngine compatibility on non-Windows platforms
// This is a placeholder implementation for cross-platform compilation

#ifndef _WIN32

// Forward declarations and COM compatibility
// Note: _bstr_t is defined in comutil.h

typedef long HRESULT;
typedef unsigned long ULONG;

// COM compatibility
#define STDMETHOD(method) virtual HRESULT method
#define STDMETHOD_(type, method) virtual type method

// Stub types and classes for the BrowserEngine
class IBrowserEngineHost {
public:
    virtual ~IBrowserEngineHost() {}
};

class IBrowserEngine {
public:
    virtual ~IBrowserEngine() {}
    
    // COM interface methods
    virtual ULONG AddRef() { return 1; }
    virtual ULONG Release() { return 1; }
    virtual HRESULT QueryInterface(const void* riid, void** ppv) { return 0; }
    
    // Core browser methods
    virtual bool Initialize(long* device) { return false; }
    virtual bool Initialize() { return false; }
    virtual void Shutdown() {}
    virtual void Update() {}
    virtual void D3DUpdate() {}
    virtual void D3DRender(int backbufferindex) {}
    virtual bool LoadURL(const char* url) { return false; }
    
    // Property setters
    virtual HRESULT put_BadPageURL(const _bstr_t& url) { return 0; }
    virtual HRESULT put_LoadingPageURL(const _bstr_t& url) { return 0; }
    virtual HRESULT put_MouseFileName(const _bstr_t& filename) { return 0; }
    virtual HRESULT put_MouseBusyFileName(const _bstr_t& filename) { return 0; }
    
    // Browser management
    virtual HRESULT CreateBrowser(const _bstr_t& name, const _bstr_t& url, long hwnd, int x, int y, int w, int h, long options, void* dispatch) { return 0; }
    virtual HRESULT DestroyBrowser(const _bstr_t& name) { return 0; }
    virtual HRESULT SetUpdateRate(const _bstr_t& name, int ticks) { return 0; }
    virtual HRESULT Navigate(const _bstr_t& name, const _bstr_t& url) { return 0; }
    virtual HRESULT IsOpen(const _bstr_t& name, long* isopen) { if (isopen) *isopen = 0; return 0; }
    
    // COM-style CreateInstance method
    virtual HRESULT CreateInstance(const void* clsid) { return 0; }
};

// Smart pointer compatibility 
typedef IBrowserEngine* IFEBrowserEngine2Ptr;

// COM creation stubs
struct FEBrowserEngine2 {};

inline void* __uuidof(const FEBrowserEngine2&) { return nullptr; }

// Stub factory function
inline IBrowserEngine* CreateBrowserEngine(IBrowserEngineHost* host) {
    return nullptr; 
}

#else
// On Windows, this would include the actual EABrowserEngine header
// #include "actual_browserengine.h"
#endif

#endif // BROWSERENGINE_H
