// Linux brightness via XF86VidMode (X11 only). Loaded lazily via dlopen so the
// patch links cleanly on systems without libXxf86vm or running under Wayland.
// Under Wayland we log a one-time warning and no-op.

#include "SagePatch/Features.h"
#include "SagePatch/Logger.h"

#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>

namespace sagepatch {

static int g_brightness = 0; // -128 .. +128

static int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// Minimal X11 + XF86VidMode forward decls so we do not need their headers.
typedef struct _XDisplay Display;
typedef Display* (*XOpenDisplay_fn)(const char*);
typedef int (*XCloseDisplay_fn)(Display*);
typedef int (*XDefaultScreen_fn)(Display*);
typedef int (*XF86VidModeGetGammaRampSize_fn)(Display*, int, int*);
typedef int (*XF86VidModeSetGammaRamp_fn)(Display*, int, int, unsigned short*, unsigned short*, unsigned short*);

static bool tryGamma(float gamma) {
    void* libX11 = dlopen("libX11.so.6", RTLD_NOW | RTLD_GLOBAL);
    if (!libX11) libX11 = dlopen("libX11.so", RTLD_NOW | RTLD_GLOBAL);
    void* libXxf86vm = dlopen("libXxf86vm.so.1", RTLD_NOW);
    if (!libXxf86vm) libXxf86vm = dlopen("libXxf86vm.so", RTLD_NOW);

    if (!libX11 || !libXxf86vm) {
        if (libX11) dlclose(libX11);
        if (libXxf86vm) dlclose(libXxf86vm);
        return false;
    }

    auto XOpenDisplay = (XOpenDisplay_fn)dlsym(libX11, "XOpenDisplay");
    auto XCloseDisplay = (XCloseDisplay_fn)dlsym(libX11, "XCloseDisplay");
    auto XDefaultScreen = (XDefaultScreen_fn)dlsym(libX11, "XDefaultScreen");
    auto XF86VidModeGetGammaRampSize = (XF86VidModeGetGammaRampSize_fn)dlsym(libXxf86vm, "XF86VidModeGetGammaRampSize");
    auto XF86VidModeSetGammaRamp = (XF86VidModeSetGammaRamp_fn)dlsym(libXxf86vm, "XF86VidModeSetGammaRamp");

    if (!XOpenDisplay || !XCloseDisplay || !XDefaultScreen ||
        !XF86VidModeGetGammaRampSize || !XF86VidModeSetGammaRamp) {
        dlclose(libX11);
        dlclose(libXxf86vm);
        return false;
    }

    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        dlclose(libX11);
        dlclose(libXxf86vm);
        return false;
    }

    int screen = XDefaultScreen(dpy);
    int rampSize = 0;
    if (!XF86VidModeGetGammaRampSize(dpy, screen, &rampSize) || rampSize <= 0) {
        XCloseDisplay(dpy);
        dlclose(libX11);
        dlclose(libXxf86vm);
        return false;
    }

    std::vector<unsigned short> r(rampSize), g(rampSize), b(rampSize);
    for (int i = 0; i < rampSize; ++i) {
        double pos = static_cast<double>(i) / (rampSize - 1);
        double v = 65535.0 * pow(pos, 1.0 / gamma);
        if (v < 0.0) v = 0.0;
        if (v > 65535.0) v = 65535.0;
        unsigned short val = static_cast<unsigned short>(v);
        r[i] = g[i] = b[i] = val;
    }
    XF86VidModeSetGammaRamp(dpy, screen, rampSize, r.data(), g.data(), b.data());

    XCloseDisplay(dpy);
    // Intentionally leak the dlopens — we want them to stay loaded for the
    // process lifetime to avoid re-resolving on every keypress.
    return true;
}

void adjustBrightness(int delta) {
    g_brightness = clampi(g_brightness + delta, -128, +128);
    float gamma = 1.0f + static_cast<float>(g_brightness) / 256.0f;
    if (gamma < 0.5f) gamma = 0.5f;
    if (gamma > 2.0f) gamma = 2.0f;

    static bool warned = false;
    if (!tryGamma(gamma)) {
        if (!warned) {
            SAGEPATCH_LOG("Brightness: XF86VidMode unavailable (Wayland or missing libXxf86vm) — feature disabled.");
            warned = true;
        }
        return;
    }
    SAGEPATCH_LOG("Brightness: %+d (gamma %.3f)", g_brightness, gamma);
}

}
