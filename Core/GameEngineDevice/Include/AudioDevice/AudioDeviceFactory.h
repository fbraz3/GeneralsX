/*
 * Phase 07: AudioDeviceFactory - Audio Device Creation
 * 
 * Factory for creating appropriate AudioDevice implementation
 * Supports multiple backends (OpenAL primary, Miles fallback for legacy)
 */

#ifndef AUDIODEVICE_FACTORY_H
#define AUDIODEVICE_FACTORY_H

#include "AudioDevice.h"

namespace GeneralsX {
namespace Audio {

/**
 * @brief Factory for creating AudioDevice implementations
 * 
 * Determines and creates the appropriate backend based on platform
 * and available libraries
 */
class AudioDeviceFactory {
public:
    /**
     * Audio backend type enumeration
     */
    enum BackendType {
        UNKNOWN = 0,
        OPENAL = 1,        // OpenAL-soft (cross-platform, primary)
        MILES = 2,         // Legacy Miles Audio (Windows only, fallback)
        NATIVE = 3,        // Platform-native (CoreAudio on macOS, WASAPI on Windows)
    };

    /**
     * Create audio device instance
     * Automatically selects best available backend
     * @return Pointer to AudioDevice implementation, nullptr on failure
     * 
     * Caller takes ownership and must delete the returned pointer
     */
    static AudioDevice* createAudioDevice();

    /**
     * Create audio device with specific backend
     * @param backend Backend type to request
     * @return Pointer to AudioDevice implementation, nullptr if backend unavailable
     * 
     * Caller takes ownership and must delete the returned pointer
     */
    static AudioDevice* createAudioDevice(BackendType backend);

    /**
     * Get the currently active backend type
     * @return Backend type of the created device, or UNKNOWN if not created
     */
    static BackendType getActiveBackend();

    /**
     * Get human-readable name for backend type
     * @param backend Backend type
     * @return String name (e.g., "OpenAL", "Miles Audio")
     */
    static const char* getBackendName(BackendType backend);

    /**
     * Check if specific backend is available
     * @param backend Backend type to check
     * @return true if backend is available on this platform
     */
    static bool isBackendAvailable(BackendType backend);
};

} // namespace Audio
} // namespace GeneralsX

#endif // AUDIODEVICE_FACTORY_H
