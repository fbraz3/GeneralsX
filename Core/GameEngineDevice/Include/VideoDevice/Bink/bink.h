#ifndef BINK_H
#define BINK_H

// Bink Video stub for macOS port
// This is a minimal stub to allow compilation

#ifdef __cplusplus
extern "C" {
#endif

// Basic types - only define what we need to avoid conflicts
typedef struct BINK* HBINK;  // Use pointer to struct instead of void*
typedef void* BINKSND;

// Constants
#define BINKOLDFRAMEFORMAT 0x00008000L
#define BINKPRELOADALL     0x00000001L
#define BINKSURFACE32      0x00000001L
#define BINKSURFACE24      0x00000002L
#define BINKSURFACE565     0x00000004L
#define BINKSURFACE555     0x00000008L

// Bink structure stub
typedef struct BINK {
    unsigned int Width;
    unsigned int Height;
    unsigned int Frames;
    unsigned int FrameNum;
    unsigned int LastFrameNum;
    unsigned int FrameRate;
    unsigned int FrameRateDiv;
} BINK;

// Function stubs - all return success or do nothing
static inline HBINK BinkOpen(const char* name, unsigned int flags) { 
    return nullptr; // Return null to indicate no video 
}
static inline void BinkClose(HBINK bink) {}
static inline int BinkWait(HBINK bink) { return 0; }
static inline void BinkDoFrame(HBINK bink) {}
static inline void BinkNextFrame(HBINK bink) {}
static inline void BinkSetVolume(HBINK bink, unsigned int track, int volume) {}
static inline void BinkSetSoundTrack(unsigned int track, HBINK bink) {}
static inline int BinkSoundUseDirectSound(void* ds) { return 0; }
static inline void BinkCopyToBuffer(HBINK bink, void* dest, int destpitch, unsigned int destheight, unsigned int destx, unsigned int desty, unsigned int flags) {}
static inline void BinkGoto(HBINK bink, unsigned int frame, int flags) {}

#ifdef __cplusplus
}
#endif

#endif // BINK_H