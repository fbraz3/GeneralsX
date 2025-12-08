#include "OpenALAudioDevice/OpenALAudioStream.h"
#include <cstdio>
#include <cstring>

#ifdef RTS_HAS_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#include <thread>
#include <chrono>

static void print_al_error(const char* context) {
    ALenum err = alGetError();
    if (err != AL_NO_ERROR) {
        printf("OpenAL error (%s): 0x%X\n", context, (unsigned int)err);
    }
}

OpenALAudioStream::OpenALAudioStream()
{
    // m_buffers.reserve(1);
    alGenSources(1, &m_source);
    print_al_error("alGenSources");
    // Create an initial pool of buffers to reuse and avoid create/delete churn
    // const int initialPool = 64;
    // m_buffers.resize(initialPool);
    alGenBuffers((ALsizei)m_buffers.size(), m_buffers.data());
    print_al_error("alGenBuffers (init pool)");
    // for (int i = 0; i < initialPool; ++i) m_freeBufferIndices.push_back(i);
    // Runtime debug: confirm stream created
    printf("OpenALAudioStream::OpenALAudioStream - created source=%u pool=%d\n", (unsigned int)m_source, (int)m_buffers.size());
}

OpenALAudioStream::~OpenALAudioStream()
{
    printf("OpenALAudioStream::~OpenALAudioStream - destroying source=%u\n", (unsigned int)m_source);
    reset();
    if (m_source) {
        alDeleteSources(1, &m_source);
        m_source = 0;
    }
    // if (!m_buffers.empty()) {
        alDeleteBuffers((ALsizei)m_buffers.size(), m_buffers.data());
        m_buffers.clear();
    // }
}

void OpenALAudioStream::reset()
{
    if (!m_source) return;
    printf("OpenALAudioStream::reset - source=%u\n", (unsigned int)m_source);
    alSourceStop(m_source);
    print_al_error("alSourceStop");
    // unqueue all
    ALint queued = 0;
    alGetSourcei(m_source, AL_BUFFERS_QUEUED, &queued);
    while (queued-- > 0) {
        ALuint b = 0;
        alSourceUnqueueBuffers(m_source, 1, &b);
        print_al_error("alSourceUnqueueBuffers");
        // Return buffer to free pool for reuse (don't delete)
        if (b != 0) {
            // find index
            for (size_t i = 0; i < m_buffers.size(); ++i) {
                if (m_buffers[i] == b) { m_freeBufferIndices.push_back((int)i); break; }
            }
        }
    }
    m_queuedBuffers = 0;
    m_playing = false;
}

void OpenALAudioStream::play()
{
    if (!m_source) return;
    printf("OpenALAudioStream::play - source=%u\n", (unsigned int)m_source);
    alSourcePlay(m_source);
    print_al_error("alSourcePlay");
    m_playing = true;
    // Report source state after play
    ALint state = 0;
    alGetSourcei(m_source, AL_SOURCE_STATE, &state);
    printf("OpenALAudioStream::play - source=%u state=%d\n", (unsigned int)m_source, state);
}

void OpenALAudioStream::update()
{
    if (!m_source) return;
    // Debug: report queued/processed counts
    printf("OpenALAudioStream::update - source=%u queued=%d processed? check log\n", (unsigned int)m_source, m_queuedBuffers);
    // Unqueue processed buffers and keep internal count
    ALint processed = 0;
    alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);
    while (processed-- > 0) {
        ALuint b = 0;
        alSourceUnqueueBuffers(m_source, 1, &b);
        print_al_error("alSourceUnqueueBuffers (update)");
        if (m_queuedBuffers > 0) m_queuedBuffers--;
        // Return buffer to free pool for reuse
        if (b != 0) {
            for (size_t i = 0; i < m_buffers.size(); ++i) {
                if (m_buffers[i] == b) { m_freeBufferIndices.push_back((int)i); break; }
            }
        }
    }
    // If source stopped but still has queued buffers, restart
    ALint state = 0;
    alGetSourcei(m_source, AL_SOURCE_STATE, &state);
    // Report source state in update
    printf("OpenALAudioStream::update - source=%u AL_SOURCE_STATE=%d queuedBuffers=%d processed=%d\n", (unsigned int)m_source, state, m_queuedBuffers, (int)processed);
    if (state != AL_PLAYING && m_queuedBuffers > 0) {
        alSourcePlay(m_source);
        print_al_error("alSourcePlay (update)");
        m_playing = true;
    }
}

void OpenALAudioStream::bufferData(const uint8_t* data, int dataSize, unsigned int format, int sampleRate)
{
    if (!m_source) return;
    if (data == nullptr || dataSize <= 0) return;

    // Debug: log buffer details
    printf("OpenALAudioStream::bufferData - source=%u dataSize=%d format=0x%X sampleRate=%d\n", (unsigned int)m_source, dataSize, (unsigned int)format, sampleRate);

    // Acquire a buffer index from the pool (or expand pool)
    int bufIndex = -1;
    if (!m_freeBufferIndices.empty()) {
        bufIndex = m_freeBufferIndices.back();
        m_freeBufferIndices.pop_back();
    } else {
        // expand pool by one
        ALuint newBuf = 0;
        alGenBuffers(1, &newBuf);
        print_al_error("alGenBuffers (expand)");
        bufIndex = (int)m_buffers.size();
        m_buffers.push_back(newBuf);
    }
    ALuint buf = m_buffers[bufIndex];
    alBufferData(buf, (ALenum)format, data, (ALsizei)dataSize, sampleRate);
    print_al_error("alBufferData");

    // queue and update counters
    alSourceQueueBuffers(m_source, 1, &buf);
    print_al_error("alSourceQueueBuffers");
    m_queuedBuffers++;

    // If we're not already playing, start playback immediately
    if (!m_playing) {
        alSourcePlay(m_source);
        print_al_error("alSourcePlay (queued)");
        m_playing = true;
        // Report source state after starting from bufferData
        ALint state = 0;
        alGetSourcei(m_source, AL_SOURCE_STATE, &state);
        printf("OpenALAudioStream::bufferData - after play source=%u state=%d queued=%d free=%d\n", (unsigned int)m_source, state, m_queuedBuffers, (int)m_freeBufferIndices.size());
    }
}

#else

OpenALAudioStream::OpenALAudioStream() {}
OpenALAudioStream::~OpenALAudioStream() {}
void OpenALAudioStream::reset() {}
void OpenALAudioStream::play() {}
void OpenALAudioStream::update() {}
void OpenALAudioStream::bufferData(const uint8_t*, int, unsigned int, int) {}

#endif
