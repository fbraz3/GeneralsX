// Linux screenshot via ImageMagick `import` — captures by X11 window id.
// Wayland fallback: `grim -g <geom>` if available. We do not depend on either
// at link time; if neither is installed, we log a hint and skip.

#include "SagePatch/Features.h"
#include "SagePatch/Logger.h"

#include <SDL3/SDL.h>
#include <chrono>
#include <ctime>
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

static bool tryImport(unsigned long winId, const char* path) {
    char cmd[4096];
    snprintf(cmd, sizeof(cmd),
             "command -v import >/dev/null 2>&1 && import -window 0x%lx \"%s\" 2>/dev/null",
             winId, path);
    int rc = system(cmd);
    if (rc != 0) return false;
    struct stat st;
    return stat(path, &st) == 0 && st.st_size > 0;
}

static bool tryGnomeScreenshot(const char* path) {
    char cmd[4096];
    snprintf(cmd, sizeof(cmd),
             "command -v gnome-screenshot >/dev/null 2>&1 && gnome-screenshot -w -f \"%s\" 2>/dev/null",
             path);
    int rc = system(cmd);
    if (rc != 0) return false;
    struct stat st;
    return stat(path, &st) == 0 && st.st_size > 0;
}

void takeScreenshot(SDL_Window* window) {
    if (!window) {
        SAGEPATCH_LOG("Screenshot: no window");
        return;
    }

    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    auto winIdLong = static_cast<unsigned long>(
        SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));

    char dir[1024], path[2048];
    ensureScreenshotDir(dir, sizeof(dir));
    timestampFilename(path, sizeof(path), dir);

    bool ok = false;
    if (winIdLong != 0) {
        ok = tryImport(winIdLong, path);
    }
    if (!ok) {
        ok = tryGnomeScreenshot(path);
    }

    if (ok) {
        SAGEPATCH_LOG("Screenshot saved: %s", path);
    } else {
        SAGEPATCH_LOG("Screenshot: install ImageMagick (`import`) or gnome-screenshot to enable.");
        SAGEPATCH_LOG("Screenshot: target was %s", path);
    }
}

}
