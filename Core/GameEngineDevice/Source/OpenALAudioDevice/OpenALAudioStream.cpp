#include "OpenALAudioDevice/OpenALAudioStream.h"
#include "OpenALAudioDevice/OpenALAudioManager.h"

OpenALAudioStream::OpenALAudioStream()
{ 
    alGenSources(1, &m_source);
    alGenBuffers(AL_STREAM_BUFFER_COUNT, m_buffers);

    // Make stream ignore positioning
    alSourcei(m_source, AL_SOURCE_RELATIVE, AL_TRUE);

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
    alBufferData(current_buffer, format, data, data_size, samplerate);
    alSourceQueueBuffers(m_source, 1, &current_buffer);
    m_current_buffer_idx++;

    if (m_current_buffer_idx >= AL_STREAM_BUFFER_COUNT)
        m_current_buffer_idx = 0;

    return true;
}

void OpenALAudioStream::update()
{
    ALint sourceState;
    alGetSourcei(m_source, AL_SOURCE_STATE, &sourceState);

    ALint processed;
    alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);
    DEBUG_LOG(("%i buffers have been processed\n", processed));
    while (processed > 0) {
        ALuint buffer;
        alSourceUnqueueBuffers(m_source, 1, &buffer);
        processed--;
    }

    ALint num_queued;
    alGetSourcei(m_source, AL_BUFFERS_QUEUED, &num_queued);
    DEBUG_LOG(("Having %i buffers queued\n", num_queued));
    if (num_queued < AL_STREAM_BUFFER_COUNT/2 && m_requireDataCallback) {
        // Ask for more data to be buffered
        // Only fill up to the half, because some formats can output
        // more than one buffer per decoded frame
        while (num_queued < AL_STREAM_BUFFER_COUNT/2) {
            m_requireDataCallback();
        	num_queued++;
        }
    }

    // GeneralsX @bugfix fbraz3 20/04/2026 Restart source if it is not playing:
    // - AL_STOPPED: source ran out of buffers and stopped.
    // - AL_INITIAL: freshly created / reset()ed source (alSourceStop on AL_INITIAL is a no-op,
    //   so reset() leaves the source in AL_INITIAL rather than AL_STOPPED).
    // - AL_PAUSED: OpenAL-Soft on macOS auto-pauses all sources when the application loses
    //   window focus (Core Audio session suspension). Without this, a momentary focus loss
    //   during the shell-map / loading-screen transition permanently silences video audio
    //   because update() never called play() again.
    // Issue: https://github.com/fbraz3/GeneralsX/issues/38
    if (sourceState == AL_STOPPED || sourceState == AL_INITIAL || sourceState == AL_PAUSED) {
        ALint num_remaining;
        alGetSourcei(m_source, AL_BUFFERS_QUEUED, &num_remaining);
        if (num_remaining > 0) {
            play();
        }
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