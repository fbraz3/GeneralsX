/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// FILE: SDL2AudioBackend.cpp
// SDL2 Audio Backend Implementation
// Author: Phase 32.1 - SDL2 Audio Initialization
// Date: October 19, 2025

#include "SDL2AudioDevice/SDL2AudioBackend.h"
#include <cstring>
#include <cstdio>

namespace SDL2Audio
{
    // Global backend instance
    static AudioBackend* g_audioBackend = nullptr;

    AudioBackend& getGlobalBackend()
    {
        if (!g_audioBackend) {
            g_audioBackend = new AudioBackend();
        }
        return *g_audioBackend;
    }

    //------------------------------------------------------------------------
    // AudioBackend Implementation
    //------------------------------------------------------------------------

    AudioBackend::AudioBackend()
        : m_initialized(false)
        , m_paused(false)
        , m_deviceID(0)
        , m_currentDeviceIndex(-1)
        , m_userCallback(nullptr)
        , m_userCallbackData(nullptr)
        , m_masterVolume(1.0f)
        , m_sdlAudioOwned(false)
    {
        memset(&m_spec, 0, sizeof(m_spec));
        memset(&m_wantedSpec, 0, sizeof(m_wantedSpec));
    }

    AudioBackend::~AudioBackend()
    {
        shutdown();
    }

    bool AudioBackend::init(const char* deviceName)
    {
        if (m_initialized) {
            printf("SDL2Audio::AudioBackend - Already initialized\n");
            return true;
        }

        // Initialize SDL Audio subsystem if not already initialized
        if (!SDL_WasInit(SDL_INIT_AUDIO)) {
            if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
                printf("SDL2Audio::AudioBackend - Failed to initialize SDL Audio: %s\n", SDL_GetError());
                return false;
            }
            m_sdlAudioOwned = true;
            printf("SDL2Audio::AudioBackend - SDL Audio subsystem initialized\n");
        } else {
            printf("SDL2Audio::AudioBackend - SDL Audio already initialized\n");
        }

        // Enumerate available audio devices
        enumerateDevices();

        // Setup wanted audio specification
        m_wantedSpec.freq = DEFAULT_SAMPLE_RATE;
        m_wantedSpec.format = DEFAULT_FORMAT;
        m_wantedSpec.channels = DEFAULT_CHANNELS;
        m_wantedSpec.samples = DEFAULT_BUFFER_SIZE;
        m_wantedSpec.callback = audioCallbackWrapper;
        m_wantedSpec.userdata = this;

        m_initialized = true;
        
        printf("SDL2Audio::AudioBackend - Initialized successfully\n");
        printf("  - Sample Rate: %d Hz\n", m_wantedSpec.freq);
        printf("  - Channels: %d\n", m_wantedSpec.channels);
        printf("  - Buffer Size: %d samples\n", m_wantedSpec.samples);
        printf("  - Format: F32 (32-bit float)\n");

        return true;
    }

    void AudioBackend::shutdown()
    {
        if (!m_initialized) {
            return;
        }

        closeDevice();

        m_devices.clear();
        m_currentDeviceIndex = -1;
        
        if (m_sdlAudioOwned) {
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            m_sdlAudioOwned = false;
            printf("SDL2Audio::AudioBackend - SDL Audio subsystem shut down\n");
        }

        m_initialized = false;
    }

    int AudioBackend::getDeviceCount(bool isCapture)
    {
        if (!m_initialized) {
            return 0;
        }

        return SDL_GetNumAudioDevices(isCapture ? 1 : 0);
    }

    const AudioDeviceInfo& AudioBackend::getDeviceInfo(int index) const
    {
        static AudioDeviceInfo invalidDevice;
        
        if (index < 0 || index >= (int)m_devices.size()) {
            return invalidDevice;
        }

        return m_devices[index];
    }

    bool AudioBackend::openDevice(const char* deviceName)
    {
        if (!m_initialized) {
            printf("SDL2Audio::AudioBackend - Cannot open device: backend not initialized\n");
            return false;
        }

        if (isDeviceOpen()) {
            printf("SDL2Audio::AudioBackend - Device already open, closing first\n");
            closeDevice();
        }

        // Open audio device
        m_deviceID = SDL_OpenAudioDevice(
            deviceName,
            0,  // Not capture device
            &m_wantedSpec,
            &m_spec,
            SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE
        );

        if (m_deviceID == 0) {
            printf("SDL2Audio::AudioBackend - Failed to open audio device: %s\n", SDL_GetError());
            return false;
        }

        printf("SDL2Audio::AudioBackend - Audio device opened successfully\n");
        printf("  - Device ID: %d\n", m_deviceID);
        printf("  - Actual Sample Rate: %d Hz\n", m_spec.freq);
        printf("  - Actual Channels: %d\n", m_spec.channels);
        printf("  - Actual Buffer Size: %d samples\n", m_spec.samples);

        // Start audio playback (unpaused)
        SDL_PauseAudioDevice(m_deviceID, 0);
        m_paused = false;

        return true;
    }

    void AudioBackend::closeDevice()
    {
        if (m_deviceID > 0) {
            SDL_CloseAudioDevice(m_deviceID);
            printf("SDL2Audio::AudioBackend - Audio device closed (ID: %d)\n", m_deviceID);
            m_deviceID = 0;
        }
        m_paused = false;
    }

    void AudioBackend::pauseAudio()
    {
        if (m_deviceID > 0 && !m_paused) {
            SDL_PauseAudioDevice(m_deviceID, 1);
            m_paused = true;
            printf("SDL2Audio::AudioBackend - Audio paused\n");
        }
    }

    void AudioBackend::resumeAudio()
    {
        if (m_deviceID > 0 && m_paused) {
            SDL_PauseAudioDevice(m_deviceID, 0);
            m_paused = false;
            printf("SDL2Audio::AudioBackend - Audio resumed\n");
        }
    }

    void AudioBackend::setCallback(AudioCallbackFunc callback, void* userdata)
    {
        m_userCallback = callback;
        m_userCallbackData = userdata;
        printf("SDL2Audio::AudioBackend - Audio callback registered\n");
    }

    void AudioBackend::setMasterVolume(float volume)
    {
        m_masterVolume = volume;
        if (m_masterVolume < 0.0f) m_masterVolume = 0.0f;
        if (m_masterVolume > 1.0f) m_masterVolume = 1.0f;
    }

    void AudioBackend::printDeviceInfo()
    {
        if (!m_initialized) {
            printf("SDL2Audio::AudioBackend - Not initialized\n");
            return;
        }

        int outputCount = SDL_GetNumAudioDevices(0);
        int inputCount = SDL_GetNumAudioDevices(1);

        printf("\n=== SDL2 Audio Devices ===\n");
        printf("Output Devices (%d):\n", outputCount);
        for (int i = 0; i < outputCount; ++i) {
            const char* name = SDL_GetAudioDeviceName(i, 0);
            printf("  [%d] %s\n", i, name ? name : "(unknown)");
        }

        printf("\nInput Devices (%d):\n", inputCount);
        for (int i = 0; i < inputCount; ++i) {
            const char* name = SDL_GetAudioDeviceName(i, 1);
            printf("  [%d] %s\n", i, name ? name : "(unknown)");
        }

        if (m_deviceID > 0) {
            printf("\nCurrent Device:\n");
            printf("  - ID: %d\n", m_deviceID);
            printf("  - Sample Rate: %d Hz\n", m_spec.freq);
            printf("  - Channels: %d\n", m_spec.channels);
            printf("  - Buffer Size: %d samples\n", m_spec.samples);
            printf("  - Format: ");
            switch (m_spec.format) {
                case AUDIO_F32SYS: printf("F32 (32-bit float)\n"); break;
                case AUDIO_S16SYS: printf("S16 (16-bit signed)\n"); break;
                case AUDIO_S32SYS: printf("S32 (32-bit signed)\n"); break;
                default: printf("0x%04X\n", m_spec.format); break;
            }
            printf("  - Status: %s\n", m_paused ? "Paused" : "Playing");
            printf("  - Master Volume: %.2f\n", m_masterVolume);
        } else {
            printf("\nNo device currently open\n");
        }
        printf("==========================\n\n");
    }

    AsciiString AudioBackend::getDeviceName() const
    {
        if (m_deviceID > 0) {
            // SDL2 doesn't provide a way to get the device name from ID
            // Return a generic name for now
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "SDL2 Audio Device %d", m_deviceID);
            return AsciiString(buffer);
        }
        return AsciiString("No Device");
    }

    void AudioBackend::audioCallbackWrapper(void* userdata, Uint8* stream, int len)
    {
        AudioBackend* backend = static_cast<AudioBackend*>(userdata);
        
        if (backend && backend->m_userCallback) {
            // Call user callback
            backend->m_userCallback(backend->m_userCallbackData, stream, len);

            // Apply master volume
            if (backend->m_masterVolume < 1.0f) {
                float* samples = reinterpret_cast<float*>(stream);
                int sampleCount = len / sizeof(float);
                
                for (int i = 0; i < sampleCount; ++i) {
                    samples[i] *= backend->m_masterVolume;
                }
            }
        } else {
            // No callback - fill with silence
            memset(stream, 0, len);
        }
    }

    void AudioBackend::enumerateDevices()
    {
        m_devices.clear();

        int count = SDL_GetNumAudioDevices(0); // Output devices
        for (int i = 0; i < count; ++i) {
            AudioDeviceInfo info;
            const char* name = SDL_GetAudioDeviceName(i, 0);
            info.name = name ? AsciiString(name) : AsciiString("(unknown)");
            info.deviceID = i;
            info.isCapture = false;
            m_devices.push_back(info);
        }

        printf("SDL2Audio::AudioBackend - Enumerated %d output devices\n", count);
    }

} // namespace SDL2Audio
