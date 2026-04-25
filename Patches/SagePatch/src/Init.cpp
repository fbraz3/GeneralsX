#include "SagePatch/Hooks.h"
#include "SagePatch/Logger.h"

namespace sagepatch {

__attribute__((constructor))
static void onLoad() {
    SAGEPATCH_LOG("Loaded (version %s) via DYLD_INSERT_LIBRARIES", SAGE_PATCH_VERSION);
    init();
}

__attribute__((destructor))
static void onUnload() {
    shutdown();
    SAGEPATCH_LOG("Unloaded");
}

void init() {
    SAGEPATCH_LOG("Active features: screenshot(F11), cursor-lock(Scroll Lock), brightness(Page+/Page-)");
}

void shutdown() {}

}
