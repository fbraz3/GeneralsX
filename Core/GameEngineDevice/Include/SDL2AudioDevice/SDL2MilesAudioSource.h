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

// FILE: SDL2MilesAudioSource.h
// SDL2 + Miles Sound System Integration - Audio Source Implementations
// Author: Phase 32.2 - MP3/WAV Playback Support
// Date: October 19, 2025

#ifndef SDL2MILESAUDIOSOURCE_H
#define SDL2MILESAUDIOSOURCE_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

// Define sentinel macro before including Miles SDK to prevent typedef conflicts
#define MILES_SOUND_SYSTEM_TYPES_DEFINED
#include "mss/mss.h"  // Miles FIRST to avoid typedef conflicts
#include "SDL2AudioDevice/SDL2AudioMixer.h"
#include "Common/AsciiString.h"
#include <vector>

namespace SDL2Audio
{
    // Forward declarations
    class FileSystem;

    // Miles-based audio source for samples (sound effects, short sounds)
    class MilesSampleSource : public IAudioSource
    {
    public:
        MilesSampleSource(HSAMPLE sample, AudioChannel channel);
        virtual ~MilesSampleSource();

        // IAudioSource interface
        virtual int fillBuffer(float* buffer, int sampleCount) override;
        virtual bool isFinished() const override;
        virtual float getVolume() const override { return m_volume; }
        virtual void setVolume(float volume) override;
        virtual AudioChannel getChannel() const override { return m_channel; }

        // Miles-specific control
        void play();
        void stop();
        void pause();
        void resume();
        
        bool isPlaying() const;
        bool isPaused() const;

        // Sample properties
        int getSampleLength() const;
        int getCurrentPosition() const;
        void setPosition(int position);

        // 3D Audio positioning (Phase 32.3)
        void set3DPosition(float x, float y, float z);
        void get3DPosition(float& x, float& y, float& z) const;
        void setListenerPosition(float x, float y, float z);
        void setListenerOrientation(float x, float y, float z);
        
        // Calculate spatial audio effects
        void updateSpatialAudio();
        float calculateDistanceAttenuation() const;
        float calculateStereoPan() const;

        HSAMPLE getMilesSample() const { return m_sample; }

    private:
        HSAMPLE m_sample;
        AudioChannel m_channel;
        float m_volume;
        bool m_paused;
        bool m_finished;
        
        // 3D Audio state (Phase 32.3)
        float m_posX, m_posY, m_posZ;           // Sound source position
        float m_listenerX, m_listenerY, m_listenerZ;  // Listener position
        float m_listenerOrientX, m_listenerOrientY, m_listenerOrientZ;  // Listener orientation
        float m_distanceAttenuation;            // Volume attenuation based on distance
        float m_stereoPan;                      // Left/right panning (-1.0 to 1.0)
        bool m_is3DSound;                       // Whether this is a 3D positioned sound
    };

    // Miles-based audio source for streams (music, long audio files)
    class MilesStreamSource : public IAudioSource
    {
    public:
        MilesStreamSource(HSTREAM stream, AudioChannel channel);
        virtual ~MilesStreamSource();

        // IAudioSource interface
        virtual int fillBuffer(float* buffer, int sampleCount) override;
        virtual bool isFinished() const override;
        virtual float getVolume() const override { return m_volume; }
        virtual void setVolume(float volume) override;
        virtual AudioChannel getChannel() const override { return m_channel; }

        // Miles-specific control
        void play();
        void stop();
        void pause();
        void resume();
        void setLooping(bool loop);
        
        bool isPlaying() const;
        bool isPaused() const;
        bool isLooping() const { return m_looping; }

        // Stream properties
        int getStreamLength() const;
        int getCurrentPosition() const;
        void setPosition(int position);

        HSTREAM getMilesStream() const { return m_stream; }

    private:
        HSTREAM m_stream;
        AudioChannel m_channel;
        float m_volume;
        bool m_paused;
        bool m_finished;
        bool m_looping;
    };

    // Audio file loader - loads MP3/WAV files via Miles Sound System
    class AudioFileLoader
    {
    public:
        AudioFileLoader();
        ~AudioFileLoader();

        // Initialize/shutdown
        bool init();
        void shutdown();
        bool isInitialized() const { return m_initialized; }

        // Sample loading (sound effects - loaded into memory)
        MilesSampleSource* loadSample(
            const char* filename,
            AudioChannel channel = CHANNEL_SOUND_EFFECTS,
            bool use3D = false
        );

        // Stream loading (music - streamed from disk)
        MilesStreamSource* loadStream(
            const char* filename,
            AudioChannel channel = CHANNEL_MUSIC
        );

        // Resource management
        void unloadSample(MilesSampleSource* source);
        void unloadStream(MilesStreamSource* source);
        void unloadAll();

        // File format support check
        bool isFormatSupported(const char* filename) const;
        
        // Miles provider selection
        int getProviderCount() const;
        const char* getProviderName(int index) const;
        bool selectProvider(int index);
        
    private:
        bool m_initialized;
        HDIGDRIVER m_digitalDriver;
        
        std::vector<MilesSampleSource*> m_samples;
        std::vector<MilesStreamSource*> m_streams;
        
        // Helper methods
        bool loadFileIntoMemory(const char* filename, void** data, unsigned int* size);
        void freeFileMemory(void* data);
    };

    // Global audio file loader
    AudioFileLoader& getGlobalLoader();

} // namespace SDL2Audio

#endif // SDL2MILESAUDIOSOURCE_H
