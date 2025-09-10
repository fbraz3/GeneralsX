#ifndef BROWSERENGINE_H
#define BROWSERENGINE_H

// Stub header for EABrowserEngine compatibility on non-Windows platforms
// This is a placeholder implementation for cross-platform compilation

#ifndef _WIN32

// Stub types and classes for the BrowserEngine
class IBrowserEngineHost {
public:
    virtual ~IBrowserEngineHost() {}
};

class IBrowserEngine {
public:
    virtual ~IBrowserEngine() {}
    virtual bool Initialize() { return false; }
    virtual void Shutdown() {}
    virtual void Update() {}
    virtual bool LoadURL(const char* url) { return false; }
};

// Stub factory function
inline IBrowserEngine* CreateBrowserEngine(IBrowserEngineHost* host) {
    return nullptr;
}

#else
// On Windows, this would include the actual EABrowserEngine header
// #include "actual_browserengine.h"
#endif

#endif // BROWSERENGINE_H
