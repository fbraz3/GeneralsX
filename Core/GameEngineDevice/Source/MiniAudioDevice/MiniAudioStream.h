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

// FILE: MiniAudioStream.h //////////////////////////////////////////////////////////////////////////
// MiniAudioStream - Streaming audio for video playback via miniaudio
// Author: GeneralsX Contributors, June 2026
#pragma once

#include "always.h"
#include <miniaudio.h>
#include <stdint.h>

#define MA_STREAM_BUFFER_COUNT 32

class MiniAudioStream final
{
public:
    MiniAudioStream();
    ~MiniAudioStream();

    bool bufferData(uint8_t *data, size_t data_size, ma_format format, int samplerate, int channels);
    bool isPlaying();
    void update();
    void reset();

    void play() { ma_sound_start(&m_sound); }
    void pause() { ma_sound_stop(&m_sound); }
    void stop() { ma_sound_stop(&m_sound); }

    void setVolume(float vol) { ma_sound_set_volume(&m_sound, vol); }

    ma_sound *getSound() { return &m_sound; }

protected:
    ma_sound m_sound;
    bool m_initialized;
};
