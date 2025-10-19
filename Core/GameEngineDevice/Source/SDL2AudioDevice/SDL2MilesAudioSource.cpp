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

// FILE: SDL2MilesAudioSource.cpp
// SDL2 + Miles Sound System Integration Implementation
// Author: Phase 32.2 - MP3/WAV Playback Support
// Date: October 19, 2025

#include "SDL2AudioDevice/SDL2MilesAudioSource.h"
#include "Common/FileSystem.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace SDL2Audio
{
    // Global loader instance
    static AudioFileLoader* g_audioLoader = nullptr;

    AudioFileLoader& getGlobalLoader()
    {
        if (!g_audioLoader) {
            g_audioLoader = new AudioFileLoader();
        }
        return *g_audioLoader;
    }

    //------------------------------------------------------------------------
    // MilesSampleSource Implementation
    //------------------------------------------------------------------------

    MilesSampleSource::MilesSampleSource(HSAMPLE sample, AudioChannel channel)
        : m_sample(sample)
        , m_channel(channel)
        , m_volume(1.0f)
        , m_paused(false)
        , m_finished(false)
        , m_posX(0.0f), m_posY(0.0f), m_posZ(0.0f)
        , m_listenerX(0.0f), m_listenerY(0.0f), m_listenerZ(0.0f)
        , m_listenerOrientX(0.0f), m_listenerOrientY(1.0f), m_listenerOrientZ(0.0f)
        , m_distanceAttenuation(1.0f)
        , m_stereoPan(0.0f)
        , m_is3DSound(false)
    {
        if (m_sample) {
            printf("SDL2Audio::MilesSampleSource - Created (sample: %p, channel: %d)\n", 
                m_sample, (int)m_channel);
        }
    }

    MilesSampleSource::~MilesSampleSource()
    {
        if (m_sample) {
            stop();
            AIL_release_sample_handle(m_sample);
            m_sample = nullptr;
        }
    }

    int MilesSampleSource::fillBuffer(float* buffer, int sampleCount)
    {
        if (!m_sample || m_finished || m_paused) {
            return 0;
        }

        // Check if sample finished playing (position-based detection)
        S32 total_ms = 0;
        S32 current_ms = 0;
        AIL_sample_ms_position(m_sample, &total_ms, &current_ms);
        
        if (current_ms >= total_ms && total_ms > 0) {
            m_finished = true;
            return 0;
        }

        // Miles handles the actual playback via its own callback system
        // This is a stub for now - Miles will output audio directly
        // In Phase 32.3, we'll implement proper integration
        
        return 0;
    }

    bool MilesSampleSource::isFinished() const
    {
        if (!m_sample) {
            return true;
        }

        if (m_finished) {
            return true;
        }

        // Use position-based finish detection
        S32 total_ms = 0;
        S32 current_ms = 0;
        AIL_sample_ms_position(m_sample, &total_ms, &current_ms);
        
        return (current_ms >= total_ms && total_ms > 0);
    }

    void MilesSampleSource::setVolume(float volume)
    {
        m_volume = volume;
        if (m_volume < 0.0f) m_volume = 0.0f;
        if (m_volume > 1.0f) m_volume = 1.0f;

        if (m_sample) {
            // Miles uses 0-127 range for volume
            S32 milesVolume = (S32)(m_volume * 127.0f);
            AIL_set_sample_volume(m_sample, milesVolume);
        }
    }

    void MilesSampleSource::play()
    {
        if (m_sample) {
            AIL_start_sample(m_sample);
            m_finished = false;
            m_paused = false;
            printf("SDL2Audio::MilesSampleSource - Playing\n");
        }
    }

    void MilesSampleSource::stop()
    {
        if (m_sample && !m_finished) {
            // Use position-based finish detection before stopping
            S32 total_ms = 0;
            S32 current_ms = 0;
            AIL_sample_ms_position(m_sample, &total_ms, &current_ms);
            
            if (current_ms < total_ms) {
                AIL_end_sample(m_sample);
            }
            m_finished = true;
            m_paused = false;
        }
    }

    void MilesSampleSource::pause()
    {
        if (m_sample && !m_paused) {
            AIL_stop_sample(m_sample);
            m_paused = true;
        }
    }

    void MilesSampleSource::resume()
    {
        if (m_sample && m_paused) {
            AIL_resume_sample(m_sample);
            m_paused = false;
        }
    }

    bool MilesSampleSource::isPlaying() const
    {
        if (!m_sample || m_finished || m_paused) {
            return false;
        }
        
        // Position-based playing detection
        S32 total_ms = 0;
        S32 current_ms = 0;
        AIL_sample_ms_position(m_sample, &total_ms, &current_ms);
        
        return (current_ms < total_ms);
    }

    bool MilesSampleSource::isPaused() const
    {
        return m_paused;
    }

    int MilesSampleSource::getSampleLength() const
    {
        if (!m_sample) {
            return 0;
        }
        S32 current, total;
        AIL_sample_ms_position(m_sample, &total, &current);
        return total;
    }

    int MilesSampleSource::getCurrentPosition() const
    {
        if (!m_sample) {
            return 0;
        }
        S32 current, total;
        AIL_sample_ms_position(m_sample, &current, &total);
        return current;
    }

    void MilesSampleSource::setPosition(int position)
    {
        if (m_sample) {
            AIL_set_sample_ms_position(m_sample, position);
        }
    }

    //------------------------------------------------------------------------
    // MilesStreamSource Implementation
    //------------------------------------------------------------------------

    MilesStreamSource::MilesStreamSource(HSTREAM stream, AudioChannel channel)
        : m_stream(stream)
        , m_channel(channel)
        , m_volume(1.0f)
        , m_paused(false)
        , m_finished(false)
        , m_looping(false)
    {
        if (m_stream) {
            printf("SDL2Audio::MilesStreamSource - Created (stream: %p, channel: %d)\n", 
                m_stream, (int)m_channel);
        }
    }

    MilesStreamSource::~MilesStreamSource()
    {
        if (m_stream) {
            stop();
            AIL_close_stream(m_stream);
            m_stream = nullptr;
        }
    }

    int MilesStreamSource::fillBuffer(float* buffer, int sampleCount)
    {
        if (!m_stream || m_finished || m_paused) {
            return 0;
        }

        // Check stream position to detect if finished
        S32 current, total;
        AIL_stream_ms_position(m_stream, &total, &current);
        
        if (current >= total && !m_looping) {
            m_finished = true;
            return 0;
        } else if (current >= total && m_looping) {
            // Restart stream for looping
            AIL_set_stream_ms_position(m_stream, 0);
            AIL_start_stream(m_stream);
        }

        // Miles handles the actual playback via its own callback system
        // This is a stub for now - Miles will output audio directly
        
        return 0;
    }

    bool MilesStreamSource::isFinished() const
    {
        if (!m_stream) {
            return true;
        }

        if (m_looping) {
            return false;  // Looping streams never finish
        }

        if (m_finished) {
            return true;
        }

        // Check position to determine if finished
        S32 current, total;
        AIL_stream_ms_position(m_stream, &total, &current);
        return (current >= total);
    }

    void MilesStreamSource::setVolume(float volume)
    {
        m_volume = volume;
        if (m_volume < 0.0f) m_volume = 0.0f;
        if (m_volume > 1.0f) m_volume = 1.0f;

        if (m_stream) {
            // Miles uses 0-127 range for volume
            S32 milesVolume = (S32)(m_volume * 127.0f);
            AIL_set_stream_volume(m_stream, milesVolume);
        }
    }

    void MilesStreamSource::play()
    {
        if (m_stream) {
            AIL_start_stream(m_stream);
            m_finished = false;
            m_paused = false;
            printf("SDL2Audio::MilesStreamSource - Playing\n");
        }
    }

    void MilesStreamSource::stop()
    {
        if (m_stream) {
            AIL_pause_stream(m_stream, 1);
            AIL_set_stream_ms_position(m_stream, 0);  // Use ms position API
            m_finished = true;
            m_paused = false;
        }
    }

    void MilesStreamSource::pause()
    {
        if (m_stream && !m_paused) {
            AIL_pause_stream(m_stream, 1);
            m_paused = true;
        }
    }

    void MilesStreamSource::resume()
    {
        if (m_stream && m_paused) {
            AIL_pause_stream(m_stream, 0);
            m_paused = false;
        }
    }

    void MilesStreamSource::setLooping(bool loop)
    {
        m_looping = loop;
        if (m_stream) {
            AIL_set_stream_loop_count(m_stream, loop ? 0 : 1);
        }
    }

    bool MilesStreamSource::isPlaying() const
    {
        if (!m_stream || m_finished || m_paused) {
            return false;
        }
        
        // Position-based playing detection
        S32 total_ms = 0;
        S32 current_ms = 0;
        AIL_stream_ms_position(m_stream, &total_ms, &current_ms);
        
        return (current_ms < total_ms || m_looping);
    }

    bool MilesStreamSource::isPaused() const
    {
        return m_paused;
    }

    int MilesStreamSource::getStreamLength() const
    {
        if (!m_stream) {
            return 0;
        }
        S32 total_ms = 0;
        S32 current_ms = 0;
        AIL_stream_ms_position(m_stream, &total_ms, &current_ms);
        return total_ms;
    }

    int MilesStreamSource::getCurrentPosition() const
    {
        if (!m_stream) {
            return 0;
        }
        S32 current, total;
        AIL_stream_ms_position(m_stream, &current, &total);
        return current;
    }

    void MilesStreamSource::setPosition(int position)
    {
        if (m_stream) {
            AIL_set_stream_ms_position(m_stream, position);
        }
    }

    //------------------------------------------------------------------------
    // AudioFileLoader Implementation
    //------------------------------------------------------------------------

    AudioFileLoader::AudioFileLoader()
        : m_initialized(false)
        , m_digitalDriver(nullptr)
    {
    }

    AudioFileLoader::~AudioFileLoader()
    {
        shutdown();
    }

    bool AudioFileLoader::init()
    {
        if (m_initialized) {
            printf("SDL2Audio::AudioFileLoader - Already initialized\n");
            return true;
        }

        // Initialize Miles Sound System
        AIL_startup();

        // Note: AIL_open_digital_driver not available in stub
        // Miles stub will use default audio device
        m_digitalDriver = nullptr;

        m_initialized = true;
        printf("SDL2Audio::AudioFileLoader - Initialized successfully\n");
        printf("  - Using Miles Sound System stub (no digital driver)\n");

        return true;
    }

    void AudioFileLoader::shutdown()
    {
        if (!m_initialized) {
            return;
        }

        unloadAll();

        // Note: AIL_close_digital_driver not available in stub
        // Digital driver cleanup handled by AIL_shutdown()
        m_digitalDriver = nullptr;

        AIL_shutdown();
        m_initialized = false;
        
        printf("SDL2Audio::AudioFileLoader - Shut down\n");
    }

    MilesSampleSource* AudioFileLoader::loadSample(
        const char* filename,
        AudioChannel channel,
        bool use3D)
    {
        if (!m_initialized || !filename) {
            return nullptr;
        }

        // Load file into memory
        void* fileData = nullptr;
        unsigned int fileSize = 0;
        
        if (!loadFileIntoMemory(filename, &fileData, &fileSize)) {
            printf("SDL2Audio::AudioFileLoader - Failed to load file: %s\n", filename);
            return nullptr;
        }

        // Allocate Miles sample handle
        HSAMPLE sample = AIL_allocate_sample_handle(m_digitalDriver);
        if (!sample) {
            printf("SDL2Audio::AudioFileLoader - Failed to allocate sample handle\n");
            freeFileMemory(fileData);
            return nullptr;
        }

        // Initialize sample with file data
        if (AIL_set_named_sample_file(sample, (char*)filename, 
                                       fileData, fileSize, 0) == 0) {
            printf("SDL2Audio::AudioFileLoader - Failed to set sample file: %s\n", filename);
            AIL_release_sample_handle(sample);
            freeFileMemory(fileData);
            return nullptr;
        }

        // Create source wrapper
        MilesSampleSource* source = new MilesSampleSource(sample, channel);
        m_samples.push_back(source);

        printf("SDL2Audio::AudioFileLoader - Loaded sample: %s\n", filename);
        
        return source;
    }

    MilesStreamSource* AudioFileLoader::loadStream(
        const char* filename,
        AudioChannel channel)
    {
        if (!m_initialized || !filename) {
            return nullptr;
        }

        // Open stream from file
        HSTREAM stream = AIL_open_stream(m_digitalDriver, (char*)filename, 0);
        if (!stream) {
            printf("SDL2Audio::AudioFileLoader - Failed to open stream: %s\n", filename);
            return nullptr;
        }

        // Create source wrapper
        MilesStreamSource* source = new MilesStreamSource(stream, channel);
        m_streams.push_back(source);

        printf("SDL2Audio::AudioFileLoader - Loaded stream: %s\n", filename);
        
        return source;
    }

    void AudioFileLoader::unloadSample(MilesSampleSource* source)
    {
        if (!source) {
            return;
        }

        auto it = std::find(m_samples.begin(), m_samples.end(), source);
        if (it != m_samples.end()) {
            delete *it;
            m_samples.erase(it);
        }
    }

    void AudioFileLoader::unloadStream(MilesStreamSource* source)
    {
        if (!source) {
            return;
        }

        auto it = std::find(m_streams.begin(), m_streams.end(), source);
        if (it != m_streams.end()) {
            delete *it;
            m_streams.erase(it);
        }
    }

    void AudioFileLoader::unloadAll()
    {
        // Unload all samples
        for (auto sample : m_samples) {
            delete sample;
        }
        m_samples.clear();

        // Unload all streams
        for (auto stream : m_streams) {
            delete stream;
        }
        m_streams.clear();
    }

    bool AudioFileLoader::isFormatSupported(const char* filename) const
    {
        if (!filename) {
            return false;
        }

        const char* ext = strrchr(filename, '.');
        if (!ext) {
            return false;
        }

        // Miles Sound System supports: WAV, MP3, OGG, FLAC, etc.
        return (strcasecmp(ext, ".wav") == 0 ||
                strcasecmp(ext, ".mp3") == 0 ||
                strcasecmp(ext, ".ogg") == 0 ||
                strcasecmp(ext, ".flac") == 0);
    }

    int AudioFileLoader::getProviderCount() const
    {
        if (!m_initialized) {
            return 0;
        }
        
        // Query Miles for available providers
        // This is a stub - actual implementation would enumerate providers
        return 1;
    }

    const char* AudioFileLoader::getProviderName(int index) const
    {
        if (!m_initialized || index < 0) {
            return nullptr;
        }
        
        // Stub - return default provider name
        return "Miles Digital Driver";
    }

    bool AudioFileLoader::selectProvider(int index)
    {
        // Stub - provider selection would happen during init
        return (index == 0);
    }

    bool AudioFileLoader::loadFileIntoMemory(const char* filename, void** data, unsigned int* size)
    {
        if (!filename || !data || !size) {
            return false;
        }

        // Use TheFileSystem to load file
        // For now, use standard file I/O as fallback
        FILE* file = fopen(filename, "rb");
        if (!file) {
            return false;
        }

        // Get file size
        fseek(file, 0, SEEK_END);
        *size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Allocate memory
        *data = malloc(*size);
        if (!*data) {
            fclose(file);
            return false;
        }

        // Read file
        size_t bytesRead = fread(*data, 1, *size, file);
        fclose(file);

        if (bytesRead != *size) {
            free(*data);
            *data = nullptr;
            return false;
        }

        return true;
    }

    void AudioFileLoader::freeFileMemory(void* data)
    {
        if (data) {
            free(data);
        }
    }

    //------------------------------------------------------------------------
    // MilesSampleSource 3D Audio Implementation (Phase 32.3)
    //------------------------------------------------------------------------

    void MilesSampleSource::set3DPosition(float x, float y, float z)
    {
        m_posX = x;
        m_posY = y;
        m_posZ = z;
        m_is3DSound = true;
        
        // Update spatial audio calculations
        updateSpatialAudio();
    }

    void MilesSampleSource::get3DPosition(float& x, float& y, float& z) const
    {
        x = m_posX;
        y = m_posY;
        z = m_posZ;
    }

    void MilesSampleSource::setListenerPosition(float x, float y, float z)
    {
        m_listenerX = x;
        m_listenerY = y;
        m_listenerZ = z;
        
        if (m_is3DSound) {
            updateSpatialAudio();
        }
    }

    void MilesSampleSource::setListenerOrientation(float x, float y, float z)
    {
        m_listenerOrientX = x;
        m_listenerOrientY = y;
        m_listenerOrientZ = z;
        
        if (m_is3DSound) {
            updateSpatialAudio();
        }
    }

    void MilesSampleSource::updateSpatialAudio()
    {
        if (!m_is3DSound || !m_sample) {
            return;
        }

        // Calculate distance attenuation
        m_distanceAttenuation = calculateDistanceAttenuation();
        
        // Calculate stereo panning
        m_stereoPan = calculateStereoPan();
        
        // Apply volume with distance attenuation
        float effectiveVolume = m_volume * m_distanceAttenuation;
        S32 milesVolume = (S32)(effectiveVolume * 127.0f);
        AIL_set_sample_volume(m_sample, milesVolume);
        
        // Apply stereo panning
        // Miles uses -127 (left) to 127 (right)
        S32 milesPan = (S32)(m_stereoPan * 127.0f);
        AIL_set_sample_pan(m_sample, milesPan);
    }

    float MilesSampleSource::calculateDistanceAttenuation() const
    {
        // Calculate distance from listener to sound source
        float dx = m_posX - m_listenerX;
        float dy = m_posY - m_listenerY;
        float dz = m_posZ - m_listenerZ;
        float distance = sqrtf(dx * dx + dy * dy + dz * dz);
        
        // Distance attenuation parameters (tunable)
        const float MIN_DISTANCE = 10.0f;   // Distance at which sound is at full volume
        const float MAX_DISTANCE = 1000.0f; // Distance at which sound is inaudible
        const float ROLLOFF_FACTOR = 1.0f;  // How quickly sound fades with distance
        
        if (distance <= MIN_DISTANCE) {
            return 1.0f; // Full volume
        }
        
        if (distance >= MAX_DISTANCE) {
            return 0.0f; // Silent
        }
        
        // Inverse distance attenuation with rolloff
        // attenuation = MIN_DISTANCE / (MIN_DISTANCE + ROLLOFF * (distance - MIN_DISTANCE))
        float attenuation = MIN_DISTANCE / (MIN_DISTANCE + ROLLOFF_FACTOR * (distance - MIN_DISTANCE));
        
        // Clamp to valid range
        if (attenuation < 0.0f) attenuation = 0.0f;
        if (attenuation > 1.0f) attenuation = 1.0f;
        
        return attenuation;
    }

    float MilesSampleSource::calculateStereoPan() const
    {
        // Calculate vector from listener to sound source
        float dx = m_posX - m_listenerX;
        float dz = m_posZ - m_listenerZ; // Using Z as the horizontal plane
        
        // Calculate listener's right vector (perpendicular to orientation in XZ plane)
        // Assuming listener orientation is (orientX, orientY, orientZ)
        // Right vector = cross(orientation, up) where up = (0, 1, 0)
        float rightX = m_listenerOrientZ;  // Cross product component
        float rightZ = -m_listenerOrientX; // Cross product component
        
        // Normalize right vector
        float rightLength = sqrtf(rightX * rightX + rightZ * rightZ);
        if (rightLength > 0.0001f) {
            rightX /= rightLength;
            rightZ /= rightLength;
        } else {
            rightX = 1.0f;
            rightZ = 0.0f;
        }
        
        // Project sound position onto listener's right axis
        // Dot product of (sound - listener) with right vector
        float panValue = dx * rightX + dz * rightZ;
        
        // Normalize distance for panning
        float distance = sqrtf(dx * dx + dz * dz);
        if (distance > 0.0001f) {
            panValue /= distance;
        }
        
        // Clamp to [-1, 1] range (-1 = left, 1 = right)
        if (panValue < -1.0f) panValue = -1.0f;
        if (panValue > 1.0f) panValue = 1.0f;
        
        return panValue;
    }

} // namespace SDL2Audio
