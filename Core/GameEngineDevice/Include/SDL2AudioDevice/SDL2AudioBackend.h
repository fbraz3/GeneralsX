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

// FILE: SDL2AudioBackend.h
// SDL2 Audio Backend for Cross-Platform Audio Support
// Author: Phase 32.1 - SDL2 Audio Initialization
// Date: October 19, 2025

#ifndef SDL2AUDIOBACKEND_H
#define SDL2AUDIOBACKEND_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include "Lib/BaseType.h"
#include "Common/AsciiString.h"
#include <vector>

namespace SDL2Audio
{
    // Audio Configuration Constants
    const int DEFAULT_SAMPLE_RATE = 44100;      // CD-quality audio
    const int DEFAULT_BUFFER_SIZE = 4096;       // Buffer size in samples
    const int DEFAULT_CHANNELS = 2;              // Stereo output
    const SDL_AudioFormat DEFAULT_FORMAT = AUDIO_F32SYS; // 32-bit float format

    // Audio Device Information
    struct AudioDeviceInfo
    {
        AsciiString name;
        int deviceID;
        int sampleRate;
        int channels;
        bool isCapture;  // true = input device, false = output device

        AudioDeviceInfo() 
            : deviceID(-1)
            , sampleRate(DEFAULT_SAMPLE_RATE)
            , channels(DEFAULT_CHANNELS)
            , isCapture(false)
        {}
    };

    // Audio Backend State
    class AudioBackend
    {
    public:
        AudioBackend();
        ~AudioBackend();

        // Initialization and cleanup
        bool init(const char* deviceName = nullptr);
        void shutdown();
        bool isInitialized() const { return m_initialized; }

        // Device enumeration
        int getDeviceCount(bool isCapture = false);
        const AudioDeviceInfo& getDeviceInfo(int index) const;
        const std::vector<AudioDeviceInfo>& getDevices() const { return m_devices; }
        
        // Device selection
        bool openDevice(const char* deviceName = nullptr);
        void closeDevice();
        bool isDeviceOpen() const { return m_deviceID > 0; }

        // Audio control
        void pauseAudio();
        void resumeAudio();
        bool isPaused() const { return m_paused; }

        // Audio callback registration
        typedef void (*AudioCallbackFunc)(void* userdata, Uint8* stream, int len);
        void setCallback(AudioCallbackFunc callback, void* userdata);

        // Audio properties
        int getSampleRate() const { return m_spec.freq; }
        int getBufferSize() const { return m_spec.samples; }
        int getChannels() const { return m_spec.channels; }
        SDL_AudioFormat getFormat() const { return m_spec.format; }

        // Volume control (0.0 to 1.0)
        void setMasterVolume(float volume);
        float getMasterVolume() const { return m_masterVolume; }

        // Debug information
        void printDeviceInfo();
        AsciiString getDeviceName() const;

    private:
        // Audio callback wrapper
        static void audioCallbackWrapper(void* userdata, Uint8* stream, int len);

        // Device enumeration
        void enumerateDevices();

        // Members
        bool m_initialized;
        bool m_paused;
        SDL_AudioDeviceID m_deviceID;
        SDL_AudioSpec m_spec;        // Actual spec obtained
        SDL_AudioSpec m_wantedSpec;  // Requested spec
        
        std::vector<AudioDeviceInfo> m_devices;
        int m_currentDeviceIndex;

        // Audio callback
        AudioCallbackFunc m_userCallback;
        void* m_userCallbackData;

        // Volume control
        float m_masterVolume;

        // SDL2 subsystem state
        bool m_sdlAudioOwned;  // true if we initialized SDL_Audio subsystem
    };

    // Global backend instance accessor
    AudioBackend& getGlobalBackend();
    
} // namespace SDL2Audio

#endif // SDL2AUDIOBACKEND_H
