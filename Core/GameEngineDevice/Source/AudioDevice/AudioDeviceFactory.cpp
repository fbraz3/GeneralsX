/*
 * Phase 07: AudioDeviceFactory Implementation
 * 
 * Factory for creating appropriate AudioDevice backend
 * Selects best available implementation for current platform
 */

#include "../Include/AudioDevice/AudioDeviceFactory.h"
#include "../Include/AudioDevice/OpenALDevice.h"

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
    // Try backends in order of preference
    if (isBackendAvailable(BackendType::OPENAL)) {
        return createAudioDevice(BackendType::OPENAL);
    }

    // If no backends available, return nullptr
    return nullptr;
}

AudioDevice* AudioDeviceFactory::createAudioDevice(BackendType backend) {
    switch (backend) {
        case BackendType::OPENAL: {
            OpenALDevice* device = new OpenALDevice();
            if (device->init()) {
                s_activeBackend = BackendType::OPENAL;
                return device;
            } else {
                delete device;
                return nullptr;
            }
        }

        case BackendType::MILES:
        case BackendType::NATIVE:
        case BackendType::UNKNOWN:
        default:
            // Not yet implemented
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
