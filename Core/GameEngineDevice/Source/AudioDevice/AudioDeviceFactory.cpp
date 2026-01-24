/*
 * Phase 07: AudioDeviceFactory Implementation
 * 
 * Factory for creating appropriate AudioDevice backend
 * Selects best available implementation for current platform
 */

#include "../Include/AudioDevice/AudioDeviceFactory.h"
#include "../Include/AudioDevice/OpenALDevice.h"
#include <stdio.h>

namespace GeneralsX {
namespace Audio {

// Static variables for singleton pattern
static AudioDeviceFactory::BackendType s_activeBackend = AudioDeviceFactory::BackendType::UNKNOWN;

/**
 * Check if OpenAL is available
 */
static bool isOpenALAvailable() {
    // Try to create a test context
    // If we can, OpenAL is available
    // For now, assume it's available if we compiled with OpenAL support
#ifdef __has_include
#if __has_include(<AL/al.h>)
    return true;
#endif
#endif
    return false;
}

AudioDevice* AudioDeviceFactory::createAudioDevice() {
    fprintf(stderr, "[AUDIO] Audio Device Factory: Attempting to initialize audio backends...\n");
    fflush(stderr);
    
    // Try backends in order of preference
    if (isBackendAvailable(BackendType::OPENAL)) {
        fprintf(stderr, "[AUDIO] OpenAL backend is available, attempting initialization...\n");
        fflush(stderr);
        return createAudioDevice(BackendType::OPENAL);
    }

    // If no backends available, return nullptr
    fprintf(stderr, "[AUDIO] ERROR: No audio backends available! Audio will be disabled.\n");
    fflush(stderr);
    return nullptr;
}

AudioDevice* AudioDeviceFactory::createAudioDevice(BackendType backend) {
    switch (backend) {
        case BackendType::OPENAL: {
            fprintf(stderr, "[AUDIO] Creating OpenAL audio device...\n");
            fflush(stderr);
            
            OpenALDevice* device = new OpenALDevice();
            if (device->init()) {
                s_activeBackend = BackendType::OPENAL;
                fprintf(stderr, "[AUDIO] SUCCESS: OpenAL audio device initialized and ready\n");
                fprintf(stderr, "[AUDIO] Audio Backend: %s\n", getBackendName(BackendType::OPENAL));
                fflush(stderr);
                return device;
            } else {
                fprintf(stderr, "[AUDIO] FAILED: OpenAL device initialization failed - %s\n", device->getLastError());
                fflush(stderr);
                delete device;
                return nullptr;
            }
        }

        case BackendType::MILES: {
            fprintf(stderr, "[AUDIO] Miles Audio backend requested but not yet implemented\n");
            fflush(stderr);
            return nullptr;
        }

        case BackendType::NATIVE:
        case BackendType::UNKNOWN:
        default:
            fprintf(stderr, "[AUDIO] ERROR: Unsupported audio backend type: %d\n", (int)backend);
            fflush(stderr);
            return nullptr;
    }
}

AudioDeviceFactory::BackendType AudioDeviceFactory::getActiveBackend() {
    return s_activeBackend;
}

const char* AudioDeviceFactory::getBackendName(BackendType backend) {
    switch (backend) {
        case BackendType::OPENAL:  return "OpenAL-soft";
        case BackendType::MILES:   return "Miles Audio";
        case BackendType::NATIVE:  return "Native Audio";
        case BackendType::UNKNOWN:
        default:                   return "Unknown";
    }
}

bool AudioDeviceFactory::isBackendAvailable(BackendType backend) {
    switch (backend) {
        case BackendType::OPENAL:
            return isOpenALAvailable();

        case BackendType::MILES:
            // Miles Audio is only available on Windows
#if defined(_WIN32) || defined(WIN32)
            return true;
#else
            return false;
#endif

        case BackendType::NATIVE:
            // Native audio available on all platforms
            return true;

        case BackendType::UNKNOWN:
        default:
            return false;
    }
}

} // namespace Audio
} // namespace GeneralsX
