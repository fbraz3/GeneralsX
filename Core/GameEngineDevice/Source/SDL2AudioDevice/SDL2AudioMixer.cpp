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

// FILE: SDL2AudioMixer.cpp
// SDL2 Audio Mixing System Implementation
// Author: Phase 32.1 - Audio Callback System
// Date: October 19, 2025

#include "SDL2AudioDevice/SDL2AudioMixer.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace SDL2Audio
{
    // Global mixer instance
    static AudioMixer* g_audioMixer = nullptr;

    AudioMixer& getGlobalMixer()
    {
        if (!g_audioMixer) {
            g_audioMixer = new AudioMixer();
        }
        return *g_audioMixer;
    }

    //------------------------------------------------------------------------
    // AudioMixer Implementation
    //------------------------------------------------------------------------

    AudioMixer::AudioMixer()
        : m_sampleRate(44100)
        , m_channels(2)
        , m_masterVolume(1.0f)
        , m_masterMuted(false)
    {
        // Initialize channel volumes to 1.0 (full volume)
        for (int i = 0; i < CHANNEL_COUNT; ++i) {
            m_channelVolumes[i] = 1.0f;
            m_channelMuted[i] = false;
        }
    }

    AudioMixer::~AudioMixer()
    {
        shutdown();
    }

    void AudioMixer::init(int sampleRate, int channels)
    {
        m_sampleRate = sampleRate;
        m_channels = channels;

        // Pre-allocate buffers (1 second worth of audio)
        int bufferSize = m_sampleRate * m_channels;
        m_mixBuffer.resize(bufferSize);
        m_sourceBuffer.resize(bufferSize);

        printf("SDL2Audio::AudioMixer - Initialized\n");
        printf("  - Sample Rate: %d Hz\n", m_sampleRate);
        printf("  - Channels: %d\n", m_channels);
        printf("  - Buffer Size: %d samples\n", bufferSize);
    }

    void AudioMixer::shutdown()
    {
        clearAllSources();
        m_mixBuffer.clear();
        m_sourceBuffer.clear();
    }

    void AudioMixer::addSource(IAudioSource* source)
    {
        if (!source) {
            return;
        }

        // Check if source already exists
        auto it = std::find(m_sources.begin(), m_sources.end(), source);
        if (it == m_sources.end()) {
            m_sources.push_back(source);
        }
    }

    void AudioMixer::removeSource(IAudioSource* source)
    {
        if (!source) {
            return;
        }

        auto it = std::find(m_sources.begin(), m_sources.end(), source);
        if (it != m_sources.end()) {
            m_sources.erase(it);
        }
    }

    void AudioMixer::removeFinishedSources()
    {
        auto it = m_sources.begin();
        while (it != m_sources.end()) {
            if ((*it)->isFinished()) {
                it = m_sources.erase(it);
            } else {
                ++it;
            }
        }
    }

    void AudioMixer::clearAllSources()
    {
        m_sources.clear();
    }

    void AudioMixer::mixAudio(float* outputBuffer, int sampleCount)
    {
        // Clear output buffer
        memset(outputBuffer, 0, sampleCount * sizeof(float));

        // If muted or no sources, return silence
        if (m_masterMuted || m_sources.empty()) {
            return;
        }

        // Resize source buffer if needed
        if ((int)m_sourceBuffer.size() < sampleCount) {
            m_sourceBuffer.resize(sampleCount);
        }

        // Mix each source
        for (auto source : m_sources) {
            if (!source || source->isFinished()) {
                continue;
            }

            // Calculate effective volume
            float effectiveVolume = calculateEffectiveVolume(source);
            if (effectiveVolume <= 0.0f) {
                continue;
            }

            // Get audio data from source
            int samplesWritten = source->fillBuffer(m_sourceBuffer.data(), sampleCount);
            if (samplesWritten <= 0) {
                continue;
            }

            // Mix into output buffer with volume applied
            for (int i = 0; i < samplesWritten; ++i) {
                outputBuffer[i] += m_sourceBuffer[i] * effectiveVolume;
            }
        }

        // Clamp output to prevent clipping
        for (int i = 0; i < sampleCount; ++i) {
            if (outputBuffer[i] > 1.0f) {
                outputBuffer[i] = 1.0f;
            } else if (outputBuffer[i] < -1.0f) {
                outputBuffer[i] = -1.0f;
            }
        }
    }

    void AudioMixer::setChannelVolume(AudioChannel channel, float volume)
    {
        if (channel < 0 || channel >= CHANNEL_COUNT) {
            return;
        }
        m_channelVolumes[channel] = clampVolume(volume);
    }

    float AudioMixer::getChannelVolume(AudioChannel channel) const
    {
        if (channel < 0 || channel >= CHANNEL_COUNT) {
            return 0.0f;
        }
        return m_channelVolumes[channel];
    }

    void AudioMixer::setMasterVolume(float volume)
    {
        m_masterVolume = clampVolume(volume);
    }

    void AudioMixer::setChannelMuted(AudioChannel channel, bool muted)
    {
        if (channel < 0 || channel >= CHANNEL_COUNT) {
            return;
        }
        m_channelMuted[channel] = muted;
    }

    bool AudioMixer::isChannelMuted(AudioChannel channel) const
    {
        if (channel < 0 || channel >= CHANNEL_COUNT) {
            return true;
        }
        return m_channelMuted[channel];
    }

    void AudioMixer::setMasterMuted(bool muted)
    {
        m_masterMuted = muted;
    }

    void AudioMixer::printMixerState()
    {
        printf("\n=== SDL2 Audio Mixer State ===\n");
        printf("Active Sources: %d\n", (int)m_sources.size());
        printf("Sample Rate: %d Hz\n", m_sampleRate);
        printf("Channels: %d\n", m_channels);
        printf("Master Volume: %.2f %s\n", m_masterVolume, m_masterMuted ? "(MUTED)" : "");
        
        printf("\nChannel Volumes:\n");
        const char* channelNames[] = {
            "Music", "Sound Effects", "Voice", "Ambient", "UI"
        };
        for (int i = 0; i < CHANNEL_COUNT; ++i) {
            printf("  %s: %.2f %s\n", 
                channelNames[i], 
                m_channelVolumes[i],
                m_channelMuted[i] ? "(MUTED)" : "");
        }
        printf("==============================\n\n");
    }

    float AudioMixer::clampVolume(float volume)
    {
        if (volume < 0.0f) return 0.0f;
        if (volume > 1.0f) return 1.0f;
        return volume;
    }

    float AudioMixer::calculateEffectiveVolume(IAudioSource* source) const
    {
        if (!source) {
            return 0.0f;
        }

        AudioChannel channel = source->getChannel();
        
        // Check if channel is muted
        if (channel >= 0 && channel < CHANNEL_COUNT && m_channelMuted[channel]) {
            return 0.0f;
        }

        float volume = source->getVolume();
        
        // Apply channel volume
        if (channel >= 0 && channel < CHANNEL_COUNT) {
            volume *= m_channelVolumes[channel];
        }
        
        // Apply master volume
        volume *= m_masterVolume;
        
        return clampVolume(volume);
    }

    //------------------------------------------------------------------------
    // SimpleAudioSource Implementation (for testing)
    //------------------------------------------------------------------------

    SimpleAudioSource::SimpleAudioSource(AudioChannel channel, float frequency, float duration)
        : m_channel(channel)
        , m_frequency(frequency)
        , m_duration(duration)
        , m_volume(1.0f)
        , m_sampleRate(44100)
        , m_samplesGenerated(0)
        , m_totalSamples(0)
        , m_phase(0.0f)
    {
        m_totalSamples = (int)(m_duration * m_sampleRate);
    }

    SimpleAudioSource::~SimpleAudioSource()
    {
    }

    int SimpleAudioSource::fillBuffer(float* buffer, int sampleCount)
    {
        if (isFinished()) {
            return 0;
        }

        int samplesToGenerate = (sampleCount < (m_totalSamples - m_samplesGenerated)) ? sampleCount : (m_totalSamples - m_samplesGenerated);
        float phaseIncrement = (2.0f * M_PI * m_frequency) / m_sampleRate;

        for (int i = 0; i < samplesToGenerate; ++i) {
            // Generate sine wave
            buffer[i] = sinf(m_phase);
            m_phase += phaseIncrement;
            
            // Wrap phase to avoid precision loss
            if (m_phase >= 2.0f * M_PI) {
                m_phase -= 2.0f * M_PI;
            }
        }

        m_samplesGenerated += samplesToGenerate;
        return samplesToGenerate;
    }

    bool SimpleAudioSource::isFinished() const
    {
        return m_samplesGenerated >= m_totalSamples;
    }

    void SimpleAudioSource::reset()
    {
        m_samplesGenerated = 0;
        m_phase = 0.0f;
    }

    float SimpleAudioSource::clampVolume(float volume)
    {
        if (volume < 0.0f) return 0.0f;
        if (volume > 1.0f) return 1.0f;
        return volume;
    }

} // namespace SDL2Audio
