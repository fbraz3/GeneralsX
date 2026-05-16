// Brightness adjustment via macOS CGSetDisplayTransferByFormula. SDL3 deprecated
// the gamma APIs in 3.0; CoreGraphics still works for the active display.
//
// Range: 0.5 (very dark) .. 2.0 (very bright). Internal counter goes -128..+128
// for parity with GenTool's brightness slider semantics, then maps to a gamma
// curve via gamma = 1.0 + (value / 256.0).

#include "SagePatch/Features.h"
#include "SagePatch/Logger.h"

#include <ApplicationServices/ApplicationServices.h>

namespace sagepatch {

static int g_brightness = 0; // -128 .. +128

static int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static void applyGamma() {
    float gamma = 1.0f + static_cast<float>(g_brightness) / 256.0f;
    if (gamma < 0.5f) gamma = 0.5f;
    if (gamma > 2.0f) gamma = 2.0f;

    CGDirectDisplayID displays[8];
    uint32_t count = 0;
    if (CGGetActiveDisplayList(8, displays, &count) != kCGErrorSuccess) {
        SAGEPATCH_LOG("Brightness: cannot enumerate displays");
        return;
    }

    for (uint32_t i = 0; i < count; ++i) {
        CGSetDisplayTransferByFormula(
            displays[i],
            0.0f, 1.0f, 1.0f / gamma,
            0.0f, 1.0f, 1.0f / gamma,
            0.0f, 1.0f, 1.0f / gamma);
    }
}

void adjustBrightness(int delta) {
    g_brightness = clampi(g_brightness + delta, -128, +128);
    applyGamma();
    SAGEPATCH_LOG("Brightness: %+d (gamma %.3f)", g_brightness,
                  1.0 + g_brightness / 256.0);
}

}
