/* Minimal OpenAL audio stream wrapper used by FFmpeg video playback */
#pragma once

#include <cstdint>
#include <vector>

#ifdef RTS_HAS_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#endif

class OpenALAudioStream {
public:
    OpenALAudioStream();
    ~OpenALAudioStream();

    void reset();
    void play();
    void update();

    // Queue raw PCM data to the stream. "format" is an OpenAL format (eg AL_FORMAT_STEREO16)
    void bufferData(const uint8_t* data, int dataSize, unsigned int format, int sampleRate);

private:
#ifdef RTS_HAS_OPENAL
    ALuint m_source = 0;
     std::vector<ALuint> m_buffers; // pool of buffers
     std::vector<int> m_freeBufferIndices; // indices into m_buffers available for use
     int m_queuedBuffers = 0;
    bool m_playing = false;
#endif
};
