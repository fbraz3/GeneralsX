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

// FILE: SDL2AudioMixer.h
// SDL2 Audio Mixing System - Multi-channel audio mixing with volume control
// Author: Phase 32.1 - Audio Callback System
// Date: October 19, 2025

#ifndef SDL2AUDIOMIXER_H
#define SDL2AUDIOMIXER_H

#include "Lib/BaseType.h"
#include <vector>
#include <cstring>

namespace SDL2Audio
{
    // Audio channel types
    enum AudioChannel
    {
        CHANNEL_MUSIC = 0,
        CHANNEL_SOUND_EFFECTS,
        CHANNEL_VOICE,
        CHANNEL_AMBIENT,
        CHANNEL_UI,
        CHANNEL_COUNT
    };

    // Audio source interface - represents a single audio source
    class IAudioSource
    {
    public:
        virtual ~IAudioSource() {}
        
        // Fill buffer with audio data
        // Returns number of samples actually written
        virtual int fillBuffer(float* buffer, int sampleCount) = 0;
        
        // Check if source has finished playing
        virtual bool isFinished() const = 0;
        
        // Get/Set volume (0.0 to 1.0)
        virtual float getVolume() const = 0;
        virtual void setVolume(float volume) = 0;
        
        // Get channel this source belongs to
        virtual AudioChannel getChannel() const = 0;
    };

    // Audio mixer - manages multiple audio sources and mixes them together
    class AudioMixer
    {
    public:
        AudioMixer();
        ~AudioMixer();

        // Initialize mixer
        void init(int sampleRate, int channels);
        void shutdown();

        // Audio source management
        void addSource(IAudioSource* source);
        void removeSource(IAudioSource* source);
        void removeFinishedSources();
        void clearAllSources();
        int getSourceCount() const { return (int)m_sources.size(); }

        // Mixing callback - called by SDL2 backend
        void mixAudio(float* outputBuffer, int sampleCount);

        // Channel volume control (0.0 to 1.0)
        void setChannelVolume(AudioChannel channel, float volume);
        float getChannelVolume(AudioChannel channel) const;

        // Master volume control (0.0 to 1.0)
        void setMasterVolume(float volume);
        float getMasterVolume() const { return m_masterVolume; }

        // Mute/Unmute
        void setChannelMuted(AudioChannel channel, bool muted);
        bool isChannelMuted(AudioChannel channel) const;
        
        void setMasterMuted(bool muted);
        bool isMasterMuted() const { return m_masterMuted; }

        // Audio properties
        int getSampleRate() const { return m_sampleRate; }
        int getChannels() const { return m_channels; }

        // Debug information
        void printMixerState();

    private:
        // Clamp volume to valid range
        static float clampVolume(float volume);
        
        // Calculate effective volume for a source
        float calculateEffectiveVolume(IAudioSource* source) const;

        // Members
        std::vector<IAudioSource*> m_sources;
        
        int m_sampleRate;
        int m_channels;
        
        float m_channelVolumes[CHANNEL_COUNT];
        bool m_channelMuted[CHANNEL_COUNT];
        
        float m_masterVolume;
        bool m_masterMuted;

        // Temporary mixing buffer
        std::vector<float> m_mixBuffer;
        std::vector<float> m_sourceBuffer;
    };

    // Simple audio source implementation for testing
    class SimpleAudioSource : public IAudioSource
    {
    public:
        SimpleAudioSource(AudioChannel channel, float frequency, float duration);
        virtual ~SimpleAudioSource();

        virtual int fillBuffer(float* buffer, int sampleCount) override;
        virtual bool isFinished() const override;
        virtual float getVolume() const override { return m_volume; }
        virtual void setVolume(float volume) override { m_volume = clampVolume(volume); }
        virtual AudioChannel getChannel() const override { return m_channel; }

        void reset();

    private:
        static float clampVolume(float volume);

        AudioChannel m_channel;
        float m_frequency;
        float m_duration;
        float m_volume;
        
        int m_sampleRate;
        int m_samplesGenerated;
        int m_totalSamples;
        float m_phase;
    };

    // Global mixer instance accessor
    AudioMixer& getGlobalMixer();

} // namespace SDL2Audio

#endif // SDL2AUDIOMIXER_H
