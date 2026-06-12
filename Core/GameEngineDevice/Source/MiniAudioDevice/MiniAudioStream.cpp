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

MiniAudioStream::MiniAudioStream() :
    m_initialized(false)
{
    memset(&m_sound, 0, sizeof(m_sound));
}

MiniAudioStream::~MiniAudioStream()
{
    reset();
}

bool MiniAudioStream::bufferData(uint8_t *data, size_t data_size, ma_format format, int samplerate, int channels)
{
    // For video audio streaming, we use a simple approach:
    // Initialize the sound from memory on first buffer, then update with new data.
    // miniaudio handles the internal buffering.

    if (!m_initialized) {
        // First call - we need an engine to initialize the sound.
        // This will be set up when getHandleForBink() is called from MiniAudioManager.
        // For now, just store the data.
        DEBUG_LOG(("MiniAudioStream::bufferData - first buffer, %zu bytes\n", data_size));
    }

    // miniaudio doesn't have a direct buffer queue like OpenAL.
    // For video audio, we'll use a different approach - the sound is initialized
    // from a memory buffer when the video starts, and we feed data through a decoder.
    // For now, this is a placeholder that will be enhanced when integrating with FFmpeg.

    DEBUG_LOG(("MiniAudioStream::bufferData - %zu bytes (format=%d, rate=%d, ch=%d)\n",
        data_size, format, samplerate, channels));

    return true;
}

bool MiniAudioStream::isPlaying()
{
    if (!m_initialized) return false;
    return ma_sound_is_playing(&m_sound) == MA_TRUE;
}

void MiniAudioStream::update()
{
    // miniaudio handles internal buffer management automatically.
    // No manual buffer queue management needed unlike OpenAL.
}

void MiniAudioStream::reset()
{
    if (m_initialized) {
        ma_sound_stop(&m_sound);
        ma_sound_uninit(&m_sound);
        m_initialized = false;
        memset(&m_sound, 0, sizeof(m_sound));
    }
}
