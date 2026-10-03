#include "framgrab.h"

// GeneralsX @bugfix GitHub Copilot 20/05/2026 Stubbed frame capture for MinGW modern builds.
FrameGrabClass::FrameGrabClass(const char *filename, MODE mode, int width, int height, int bitdepth, float framerate)
{
    Filename = filename;
    Mode = mode;
    FrameRate = framerate;
    Counter = 0;
#ifdef _WIN32
    AVIFile = nullptr;
    Bitmap = nullptr;
    Stream = nullptr;
#endif
    (void)width;
    (void)height;
    (void)bitdepth;
}

FrameGrabClass::~FrameGrabClass() {}

void FrameGrabClass::ConvertGrab(void *BitmapPointer)
{
    Grab(BitmapPointer);
}

void FrameGrabClass::Grab(void *BitmapPointer)
{
    (void)BitmapPointer;
}

void FrameGrabClass::GrabAVI(void *BitmapPointer)
{
    (void)BitmapPointer;
}

void FrameGrabClass::GrabRawFrame(void *BitmapPointer)
{
    (void)BitmapPointer;
}

void FrameGrabClass::CleanupAVI() {}

void FrameGrabClass::ConvertFrame(void *BitmapPointer)
{
    (void)BitmapPointer;
}
