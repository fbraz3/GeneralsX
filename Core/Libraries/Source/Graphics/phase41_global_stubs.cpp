/**
 * Phase 43.6: Critical Runtime Symbols
 * 
 * These are ONLY the symbols that dyld cannot find in any linked library.
 * All other stubs have been removed to avoid conflicts.
 */

#include <cstdint>

// NOTE: These symbols MUST use C linkage (extern "C") to avoid name mangling.
// We define BOTH underscored and non-underscored versions where they don't conflict.

#if defined(__clang__) || defined(__GNUC__)
#define WEAK_GLOBAL __attribute__((weak))
#else
#define WEAK_GLOBAL
#endif

extern "C" {
    // FPU control
    int DX8Wrapper_PreserveFPU WEAK_GLOBAL = 0;
    int _DX8Wrapper_PreserveFPU WEAK_GLOBAL = 0;
    
    // Menu UI state
    unsigned char DontShowMainMenu WEAK_GLOBAL = 0;
    unsigned char _DontShowMainMenu WEAK_GLOBAL = 0;
    
    // GameSpy globals - only define non-conflicting versions
    void* TheGameResultsQueue WEAK_GLOBAL = nullptr;
    void* _TheGameResultsQueue WEAK_GLOBAL = nullptr;
    void* TheGameSpyBuddyMessageQueue WEAK_GLOBAL = nullptr;
    void* _TheGameSpyBuddyMessageQueue WEAK_GLOBAL = nullptr;
    void* TheGameSpyConfig WEAK_GLOBAL = nullptr;
    void* _TheGameSpyConfig WEAK_GLOBAL = nullptr;
    void* TheGameSpyPeerMessageQueue WEAK_GLOBAL = nullptr;
    void* _TheGameSpyPeerMessageQueue WEAK_GLOBAL = nullptr;
    void* ThePinger WEAK_GLOBAL = nullptr;
    void* _ThePinger WEAK_GLOBAL = nullptr;
    void* IMEManager WEAK_GLOBAL = nullptr;
    void* _IMEManager WEAK_GLOBAL = nullptr;
    void* g_LastErrorDump WEAK_GLOBAL = nullptr;
    void* _g_LastErrorDump WEAK_GLOBAL = nullptr;
    void* TheFunctionLexicon WEAK_GLOBAL = nullptr;
    void* _TheFunctionLexicon WEAK_GLOBAL = nullptr;
}

#undef WEAK_GLOBAL