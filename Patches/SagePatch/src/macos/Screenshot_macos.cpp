// F11 screenshot. We resolve the SDL3 window's Cocoa window number, then shell
// out to `/usr/sbin/screencapture -l <id> -x <path>` — that binary has been on
// every macOS version since 10.2 and unlike CGWindowListCreateImageFromArray
// is not deprecated on macOS 15+.

#include "SagePatch/Features.h"
#include "SagePatch/Logger.h"

#include <SDL3/SDL.h>
#include <objc/runtime.h>
#include <objc/message.h>

#include <chrono>
#include <ctime>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>

namespace sagepatch {

static void ensureScreenshotDir(char* outPath, size_t outPathLen) {
    const char* home = getenv("HOME");
    if (!home || !*home) {
        const struct passwd* pw = getpwuid(getuid());
        home = pw ? pw->pw_dir : ".";
    }
    snprintf(outPath, outPathLen, "%s/Pictures/GeneralsX", home);
    mkdir(outPath, 0755);
}

static void timestampFilename(char* out, size_t outLen, const char* dir) {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&t, &tm);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    snprintf(out, outLen, "%s/generalsx_%04d-%02d-%02d_%02d-%02d-%02d-%03d.png",
             dir,
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec,
             static_cast<int>(ms.count()));
}

static unsigned long cocoaWindowNumber(void* nsWindow) {
    using SendLong = unsigned long (*)(void*, SEL);
    SEL sel = sel_registerName("windowNumber");
    SendLong send = reinterpret_cast<SendLong>(objc_msgSend);
    return send(nsWindow, sel);
}

void takeScreenshot(SDL_Window* window) {
    if (!window) {
        SAGEPATCH_LOG("Screenshot: no window");
        return;
    }

    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    void* nsWindow = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
    if (!nsWindow) {
        SAGEPATCH_LOG("Screenshot: SDL3 window has no Cocoa NSWindow handle");
        return;
    }

    unsigned long winId = cocoaWindowNumber(nsWindow);
    if (winId == 0) {
        SAGEPATCH_LOG("Screenshot: window number == 0");
        return;
    }

    char dir[1024], path[2048], cmd[4096];
    ensureScreenshotDir(dir, sizeof(dir));
    timestampFilename(path, sizeof(path), dir);

    // -l <id>  : capture only the window with that ID
    // -x       : no shutter sound
    // -o       : do not capture the window's shadow (cleaner crop)
    // -t png   : png output
    snprintf(cmd, sizeof(cmd),
             "/usr/sbin/screencapture -l%lu -x -o -t png \"%s\" 2>/dev/null",
             winId, path);

    int rc = system(cmd);
    if (rc == 0) {
        struct stat st;
        if (stat(path, &st) == 0 && st.st_size > 0) {
            SAGEPATCH_LOG("Screenshot saved: %s (%lld bytes)", path, (long long)st.st_size);
            return;
        }
    }
    SAGEPATCH_LOG("Screenshot: screencapture failed (rc=%d)", rc);
}

}
