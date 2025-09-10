#ifndef MMSYSTEM_H
#define MMSYSTEM_H

#ifdef _WIN32
    #include <mmsystem.h>
#else
    // Unix/macOS multimedia system compatibility
    
    // Time functions
    typedef unsigned long DWORD;
    typedef unsigned int UINT;
    
    inline DWORD timeGetTime() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
    }
    
    inline UINT timeBeginPeriod(UINT period) { return 0; }
    inline UINT timeEndPeriod(UINT period) { return 0; }
    
    // Multimedia constants
    #define TIMERR_NOERROR 0
    #define TIMERR_NOCANDO 97
    
    #include <sys/time.h>
    
#endif

#endif // MMSYSTEM_H
