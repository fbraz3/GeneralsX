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

#include "MiniAudioDevice/MiniAudioStream.h"
#include "Lib/BaseType.h"
#include "Common/GameAudio.h"

MiniAudioStream::MiniAudioStream() :
    m_engine(NULL),
    m_sound(NULL),
    m_sampleRate(0),
    m_channels(0),
    m_format(ma_format_unknown),
    m_initialized(false)
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
        m_channels = channels;
        m_format = format;
        m_initialized = true;
    }

    m_buffer.insert(m_buffer.end(), data, data + data_size);
    return true;
}

bool MiniAudioStream::isPlaying()
{
    if (!m_sound) return false;
    return ma_sound_is_playing(m_sound) == MA_TRUE;
}

void MiniAudioStream::update()
{
}

void MiniAudioStream::reset()
{
    if (m_sound) {
        ma_sound_stop(m_sound);
        ma_sound_uninit(m_sound);
        free(m_sound);
        m_sound = NULL;
    }

    m_initialized = false;
    m_buffer.clear();
    m_sampleRate = 0;
    m_channels = 0;
    m_format = ma_format_unknown;
    m_engine = NULL;
}

void MiniAudioStream::play()
{
    if (!m_initialized || m_buffer.empty()) {
        DEBUG_LOG(("MiniAudioStream: play() ignored (no data)\n"));
        return;
    }

    // Get the engine from TheAudio
    void *device = TheAudio->getDevice();
    m_engine = (ma_engine *)device;
    if (!m_engine) {
        DEBUG_LOG(("MiniAudioStream: play() failed (no engine)\n"));
        return;
    }

    // Stop any previous sound
    if (m_sound) {
        ma_sound_stop(m_sound);
        ma_sound_uninit(m_sound);
        free(m_sound);
    }

    m_sound = (ma_sound *)malloc(sizeof(ma_sound));

    // Create a memory decoder from accumulated audio data
    ma_decoder decoder;
    ma_decoder_config decConfig = ma_decoder_config_init(m_format, m_channels, m_sampleRate);

    ma_result result = ma_decoder_init_memory(m_buffer.data(), m_buffer.size(), &decConfig, &decoder);
    if (result != MA_SUCCESS) {
        DEBUG_LOG(("MiniAudioStream: decoder init failed: %d\n", result));
        free(m_sound);
        m_sound = NULL;
        return;
    }

    // Create sound from decoder (no spatialization, no pitch)
    result = ma_sound_init_from_data_source(m_engine, &decoder,
        MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_NO_PITCH,
        NULL, m_sound);

    if (result != MA_SUCCESS) {
        DEBUG_LOG(("MiniAudioStream: sound init failed: %d\n", result));
        ma_decoder_uninit(&decoder);
        free(m_sound);
        m_sound = NULL;
        return;
    }

    ma_sound_start(m_sound);
    DEBUG_LOG(("MiniAudioStream: playing %zu bytes, %d Hz, %d ch\n",
        m_buffer.size(), m_sampleRate, m_channels));
}

void MiniAudioStream::pause()
{
    if (m_sound) {
        ma_sound_stop(m_sound);
    }
}

void MiniAudioStream::stop()
{
    pause();
    reset();
}

void MiniAudioStream::setVolume(float vol)
{
    if (m_sound) {
        ma_sound_set_volume(m_sound, vol);
    }
}
