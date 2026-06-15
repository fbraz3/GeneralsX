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

// FILE: MiniAudioStream.cpp ////////////////////////////////////////////////////////////////////////
// MiniAudioStream - Streaming audio for video playback via miniaudio
// Author: GeneralsX Contributors, June 2026
//
// Design: FFmpegVideoStream decodes video packets every frame via decodePacket().
// Each audio packet decoded calls bufferData() to accumulate PCM data.
// update() is called every frame and triggers createSound() once the buffer
// stops growing (all audio decoded for this playback cycle).

#include "MiniAudioDevice/MiniAudioStream.h"
#include "Lib/BaseType.h"

MiniAudioStream::MiniAudioStream() :
    m_engine(NULL),
    m_sound(NULL),
    m_audioBuffer(NULL),
    m_sampleRate(0),
    m_channels(0),
    m_format(ma_format_unknown),
    m_initialized(false),
    m_playing(false),
    m_soundCreated(false),
    m_lastBufferSize(0),
    m_stableFrameCount(0)
{
}

MiniAudioStream::~MiniAudioStream()
{
    reset();
}

bool MiniAudioStream::bufferData(uint8_t *data, size_t data_size, ma_format format, int samplerate, int channels)
{
    if (data_size == 0) return false;

    if (!m_initialized) {
        m_sampleRate = samplerate;
        m_channels   = channels;
        m_format     = format;
        m_initialized = true;
    }

    m_buffer.insert(m_buffer.end(), data, data + data_size);
    return true;
}

bool MiniAudioStream::isPlaying()
{
    if (!m_sound) return m_playing;  // playing flag set but sound not yet created
    return ma_sound_is_playing(m_sound) == MA_TRUE;
}

void MiniAudioStream::update()
{
    // Only act when play() was called and sound hasn't been created yet
    if (!m_playing || !m_initialized || m_soundCreated) return;

    size_t currentSize = m_buffer.size();

    if (currentSize == m_lastBufferSize && currentSize > 0) {
        // Buffer hasn't grown this frame — may be complete
        m_stableFrameCount++;
        // Wait 2 stable frames to be safe (handles 1-frame decode bursts)
        if (m_stableFrameCount >= 2) {
            createSound();
        }
    } else {
        m_stableFrameCount = 0;
    }

    m_lastBufferSize = currentSize;
}

void MiniAudioStream::reset()
{
    if (m_sound) {
        ma_sound_stop(m_sound);
        ma_sound_uninit(m_sound);
        free(m_sound);
        m_sound = NULL;
    }

    if (m_audioBuffer) {
        ma_audio_buffer_uninit(m_audioBuffer);
        free(m_audioBuffer);
        m_audioBuffer = NULL;
    }

    m_initialized    = false;
    m_playing        = false;
    m_soundCreated   = false;
    m_buffer.clear();
    m_sampleRate     = 0;
    m_channels       = 0;
    m_format         = ma_format_unknown;
    m_lastBufferSize = 0;
    m_stableFrameCount = 0;
    // Don't clear m_engine — it's provided externally and persists
}

void MiniAudioStream::play()
{
    m_playing        = true;
    m_soundCreated   = false;
    m_lastBufferSize = 0;
    m_stableFrameCount = 0;
}

void MiniAudioStream::createSound()
{
    if (!m_engine || m_buffer.empty() || m_format == ma_format_unknown) return;

    // Cleanup previous sound/buffer if any
    if (m_sound) {
        ma_sound_stop(m_sound);
        ma_sound_uninit(m_sound);
        free(m_sound);
        m_sound = NULL;
    }
    if (m_audioBuffer) {
        ma_audio_buffer_uninit(m_audioBuffer);
        free(m_audioBuffer);
        m_audioBuffer = NULL;
    }

    ma_uint64 frameCount = m_buffer.size() / ma_get_bytes_per_frame(m_format, m_channels);
    if (frameCount == 0) return;

    // init_copy copies data into internal storage — safe after m_buffer is freed
    ma_audio_buffer_config abConfig = ma_audio_buffer_config_init(
        m_format, m_channels, frameCount, m_buffer.data(), NULL);

    m_audioBuffer = (ma_audio_buffer *)malloc(sizeof(ma_audio_buffer));
    ma_result result = ma_audio_buffer_init_copy(&abConfig, m_audioBuffer);
    if (result != MA_SUCCESS) {
        free(m_audioBuffer);
        m_audioBuffer = NULL;
        return;
    }
    m_audioBuffer->ref.sampleRate = m_sampleRate;

    m_sound = (ma_sound *)malloc(sizeof(ma_sound));
    result = ma_sound_init_from_data_source(m_engine, m_audioBuffer,
        MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_NO_PITCH,
        NULL, m_sound);

    if (result != MA_SUCCESS) {
        ma_audio_buffer_uninit(m_audioBuffer);
        free(m_audioBuffer);
        m_audioBuffer = NULL;
        free(m_sound);
        m_sound = NULL;
        return;
    }

    ma_sound_set_volume(m_sound, 1.0f);
    ma_sound_start(m_sound);
    m_soundCreated = true;
}

void MiniAudioStream::pause()
{
    m_playing = false;
    if (m_sound) ma_sound_stop(m_sound);
}

void MiniAudioStream::stop()
{
    pause();
    reset();
}

void MiniAudioStream::setVolume(float vol)
{
    if (m_sound) ma_sound_set_volume(m_sound, vol);
}
