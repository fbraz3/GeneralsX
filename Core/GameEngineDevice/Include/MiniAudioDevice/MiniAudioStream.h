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
#include <vector>

class MiniAudioStream final
{
public:
    MiniAudioStream();
    ~MiniAudioStream();

    // Called by MiniAudioManager to provide engine reference at construction
    void setEngine(ma_engine *engine) { m_engine = engine; }

    // Push decoded PCM data (called by FFmpegVideoStream::onFrame)
    bool bufferData(uint8_t *data, size_t data_size, ma_format format, int samplerate, int channels);

    bool isPlaying();
    // Must be called every frame from FFmpegVideoStream::update()
    void update();
    void reset();

    void play();
    void pause();
    void stop();
    void setVolume(float vol);

protected:
    void createSound();

    std::vector<uint8_t> m_buffer;
    ma_engine *m_engine;
    ma_sound *m_sound;
    ma_audio_buffer *m_audioBuffer;
    int m_sampleRate;
    int m_channels;
    ma_format m_format;
    bool m_initialized;
    bool m_playing;
    bool m_soundCreated;
    size_t m_lastBufferSize;
    int m_stableFrameCount;  // Frames with no new data — used to detect buffer completion
};
