#include "OpenALAudioDevice/OpenALAudioStream.h"
#include "OpenALAudioDevice/OpenALAudioManager.h"
#include <AL/alext.h>

OpenALAudioStream::OpenALAudioStream()
{ 
    alGenSources(1, &m_source);
    alGenBuffers(AL_STREAM_BUFFER_COUNT, m_buffers);

    // GeneralsX @bugfix BenderAI 22/04/2026 Force video stream source to non-positional direct playback.
    alSourcei(m_source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(m_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcef(m_source, AL_ROLLOFF_FACTOR, 0.0f);
    alSourcef(m_source, AL_GAIN, 1.0f);
    alSourcei(m_source, AL_LOOPING, AL_FALSE);
#ifdef AL_DIRECT_CHANNELS_SOFT
    alSourcei(m_source, AL_DIRECT_CHANNELS_SOFT, AL_TRUE);
#endif
#ifdef AL_SOURCE_SPATIALIZE_SOFT
    alSourcei(m_source, AL_SOURCE_SPATIALIZE_SOFT, AL_FALSE);
#endif

    DEBUG_LOG(("OpenALAudioStream created: %i\n", m_source));
}

OpenALAudioStream::~OpenALAudioStream()
{
    DEBUG_LOG(("OpenALAudioStream freed: %i\n", m_source));
    // Unbind the buffers first
    alSourceStop(m_source);
    alSourcei(m_source, AL_BUFFER, 0);
    alDeleteSources(1, &m_source);
    // Now delete the buffers
    alDeleteBuffers(AL_STREAM_BUFFER_COUNT, m_buffers);
}

bool OpenALAudioStream::bufferData(uint8_t *data, size_t data_size, ALenum format, int samplerate)
{
    DEBUG_LOG(("Buffering %zu bytes of data (samplerate: %i, format: %i)\n", data_size, samplerate, format));
    ALint num_queued;
    alGetSourcei(m_source, AL_BUFFERS_QUEUED, &num_queued);
    if (num_queued >= AL_STREAM_BUFFER_COUNT) {
        DEBUG_LOG(("Having too many buffers already queued: %i", num_queued));
        return false;
    }

    ALuint &current_buffer = m_buffers[m_current_buffer_idx];
    // GeneralsX @bugfix BenderAI 22/04/2026 Detect and reject invalid OpenAL buffer/queue operations.
    while (alGetError() != AL_NO_ERROR) {}
    alBufferData(current_buffer, format, data, data_size, samplerate);
    ALenum err = alGetError();
    if (err != AL_NO_ERROR) {
        DEBUG_LOG(("OpenALAudioStream::bufferData alBufferData failed: err=0x%x format=0x%x size=%zu rate=%d\n",
            (unsigned int)err, (unsigned int)format, data_size, samplerate));
        return false;
    }

    alSourceQueueBuffers(m_source, 1, &current_buffer);
    err = alGetError();
    if (err != AL_NO_ERROR) {
        DEBUG_LOG(("OpenALAudioStream::bufferData alSourceQueueBuffers failed: err=0x%x source=%u buffer=%u\n",
            (unsigned int)err, (unsigned int)m_source, (unsigned int)current_buffer));
        return false;
    }

    m_current_buffer_idx++;

    if (m_current_buffer_idx >= AL_STREAM_BUFFER_COUNT)
        m_current_buffer_idx = 0;

    return true;
}

void OpenALAudioStream::update()
{
    ALint sourceState;
    alGetSourcei(m_source, AL_SOURCE_STATE, &sourceState);

    ALint num_queued;
    alGetSourcei(m_source, AL_BUFFERS_QUEUED, &num_queued);

    // GeneralsX @bugfix BenderAI 22/04/2026 Restart before unqueue to avoid dropping freshly queued
    // briefing buffers when OpenAL reports AL_STOPPED with processed buffers.
    if ((sourceState == AL_STOPPED || sourceState == AL_INITIAL || sourceState == AL_PAUSED) && num_queued > 0) {
        play();
        alGetSourcei(m_source, AL_SOURCE_STATE, &sourceState);
    }

    ALint processedBeforeUnqueue = 0;
    alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processedBeforeUnqueue);
    DEBUG_LOG(("%i buffers have been processed\n", processedBeforeUnqueue));

    // GeneralsX @bugfix BenderAI 22/04/2026 Only unqueue processed data in active playback states.
    ALint processedToUnqueue = ((sourceState == AL_PLAYING || sourceState == AL_PAUSED) ? processedBeforeUnqueue : 0);
    while (processedToUnqueue > 0) {
        ALuint buffer;
        alSourceUnqueueBuffers(m_source, 1, &buffer);
        processedToUnqueue--;
    }

    alGetSourcei(m_source, AL_BUFFERS_QUEUED, &num_queued);
    DEBUG_LOG(("Having %i buffers queued\n", num_queued));

    if (num_queued < AL_STREAM_BUFFER_COUNT / 2 && m_requireDataCallback) {
        // GeneralsX @bugfix BenderAI 22/04/2026 Do not fake queue growth when callback fails to enqueue data.
        // Ask for more data to be buffered.
        // Only fill up to the half, because some formats can output
        // more than one buffer per decoded frame.
        while (num_queued < AL_STREAM_BUFFER_COUNT / 2) {
            m_requireDataCallback();

            ALint refreshedQueued = 0;
            alGetSourcei(m_source, AL_BUFFERS_QUEUED, &refreshedQueued);
            if (refreshedQueued <= num_queued) {
                break;
            }
            num_queued = refreshedQueued;
        }
    }

    // GeneralsX @bugfix fbraz3 27/04/2026 Restart after refill when a generic speech stream
    // began the frame with an empty queue; otherwise processPlayingList() can release it as
    // stopped before the newly buffered narrator audio ever starts playing.
    alGetSourcei(m_source, AL_SOURCE_STATE, &sourceState);
    if ((sourceState == AL_STOPPED || sourceState == AL_INITIAL || sourceState == AL_PAUSED) && num_queued > 0) {
        play();
    }
}

void OpenALAudioStream::reset()
{
    DEBUG_LOG(("Resetting stream\n"));
    // alSourceStop() marks all queued buffers as processed so they can be
    // unqueued. alSourceRewind() transitions to AL_INITIAL but does NOT move
    // unprocessed buffers to processed state, so the subsequent
    // alSourcei(AL_BUFFER, 0) would fail with AL_INVALID_OPERATION if any
    // buffers were still pending.
    alSourceStop(m_source);
    ALint num_queued;
    alGetSourcei(m_source, AL_BUFFERS_QUEUED, &num_queued);
    while (num_queued > 0) {
        ALuint buf;
        alSourceUnqueueBuffers(m_source, 1, &buf);
        num_queued--;
    }
    m_current_buffer_idx = 0;
}

bool OpenALAudioStream::isPlaying()
{
    ALint state;
    alGetSourcei(m_source, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}