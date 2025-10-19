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

// FILE: SDL2AudioStreamManager.cpp
// Audio Stream Management System Implementation
// Author: Phase 32.2 - Audio Stream Management
// Date: October 19, 2025

#include "SDL2AudioDevice/SDL2AudioStreamManager.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>

namespace SDL2Audio
{
    // Global stream manager instance
    static AudioStreamManager* g_streamManager = nullptr;

    AudioStreamManager& getGlobalStreamManager()
    {
        if (!g_streamManager) {
            g_streamManager = new AudioStreamManager();
        }
        return *g_streamManager;
    }

    //------------------------------------------------------------------------
    // MusicPlaylist Implementation
    //------------------------------------------------------------------------

    MusicPlaylist::MusicPlaylist()
        : m_currentTrackIndex(-1)
        , m_shuffled(false)
        , m_repeat(false)
    {
        srand((unsigned int)time(nullptr));
    }

    MusicPlaylist::~MusicPlaylist()
    {
        clearPlaylist();
    }

    void MusicPlaylist::addTrack(const AsciiString& filename, const AsciiString& trackName, bool loop)
    {
        MusicTrack track;
        track.filename = filename;
        track.trackName = trackName;
        track.isLooping = loop;
        track.durationMS = 0;  // Will be filled when track is loaded
        
        m_tracks.push_back(track);
        
        printf("SDL2Audio::MusicPlaylist - Added track: %s (%s)\n", 
            trackName.str(), filename.str());
    }

    void MusicPlaylist::removeTrack(const AsciiString& trackName)
    {
        auto it = m_tracks.begin();
        while (it != m_tracks.end()) {
            if (it->trackName == trackName) {
                it = m_tracks.erase(it);
            } else {
                ++it;
            }
        }
    }

    void MusicPlaylist::clearPlaylist()
    {
        m_tracks.clear();
        m_currentTrackIndex = -1;
    }

    const MusicTrack* MusicPlaylist::getTrack(int index) const
    {
        if (index < 0 || index >= (int)m_tracks.size()) {
            return nullptr;
        }
        return &m_tracks[index];
    }

    const MusicTrack* MusicPlaylist::findTrack(const AsciiString& trackName) const
    {
        for (const auto& track : m_tracks) {
            if (track.trackName == trackName) {
                return &track;
            }
        }
        return nullptr;
    }

    void MusicPlaylist::setShuffled(bool shuffled)
    {
        m_shuffled = shuffled;
    }

    void MusicPlaylist::setRepeat(bool repeat)
    {
        m_repeat = repeat;
    }

    const MusicTrack* MusicPlaylist::getNextTrack()
    {
        if (m_tracks.empty()) {
            return nullptr;
        }

        if (m_shuffled) {
            // Random track
            m_currentTrackIndex = rand() % m_tracks.size();
        } else {
            // Sequential
            m_currentTrackIndex++;
            if (m_currentTrackIndex >= (int)m_tracks.size()) {
                if (m_repeat) {
                    m_currentTrackIndex = 0;
                } else {
                    m_currentTrackIndex = -1;
                    return nullptr;
                }
            }
        }

        return &m_tracks[m_currentTrackIndex];
    }

    const MusicTrack* MusicPlaylist::getPreviousTrack()
    {
        if (m_tracks.empty()) {
            return nullptr;
        }

        if (m_shuffled) {
            // Random track
            m_currentTrackIndex = rand() % m_tracks.size();
        } else {
            // Sequential
            m_currentTrackIndex--;
            if (m_currentTrackIndex < 0) {
                if (m_repeat) {
                    m_currentTrackIndex = (int)m_tracks.size() - 1;
                } else {
                    m_currentTrackIndex = -1;
                    return nullptr;
                }
            }
        }

        return &m_tracks[m_currentTrackIndex];
    }

    const MusicTrack* MusicPlaylist::getCurrentTrack() const
    {
        if (m_currentTrackIndex < 0 || m_currentTrackIndex >= (int)m_tracks.size()) {
            return nullptr;
        }
        return &m_tracks[m_currentTrackIndex];
    }

    void MusicPlaylist::reset()
    {
        m_currentTrackIndex = -1;
    }

    //------------------------------------------------------------------------
    // AudioStreamManager Implementation
    //------------------------------------------------------------------------

    AudioStreamManager::AudioStreamManager()
        : m_initialized(false)
        , m_mixer(nullptr)
        , m_loader(nullptr)
        , m_musicStream(nullptr)
        , m_musicState(STREAM_STATE_STOPPED)
        , m_musicVolume(1.0f)
        , m_ambientStream(nullptr)
        , m_ambientState(STREAM_STATE_STOPPED)
        , m_ambientVolume(0.5f)
        , m_fadingMusic(false)
        , m_fadeStartVolume(0.0f)
        , m_fadeTargetVolume(1.0f)
        , m_fadeTimeElapsed(0.0f)
        , m_fadeDuration(1000.0f)
    {
    }

    AudioStreamManager::~AudioStreamManager()
    {
        shutdown();
    }

    void AudioStreamManager::init(AudioMixer* mixer, AudioFileLoader* loader)
    {
        if (m_initialized) {
            return;
        }

        m_mixer = mixer;
        m_loader = loader;
        m_initialized = true;

        printf("SDL2Audio::AudioStreamManager - Initialized\n");
    }

    void AudioStreamManager::shutdown()
    {
        if (!m_initialized) {
            return;
        }

        stopMusic();
        stopAmbient();
        
        m_playlist.clearPlaylist();
        
        m_mixer = nullptr;
        m_loader = nullptr;
        m_initialized = false;

        printf("SDL2Audio::AudioStreamManager - Shut down\n");
    }

    bool AudioStreamManager::playMusic(const char* filename, bool loop)
    {
        if (!m_initialized || !filename) {
            return false;
        }

        // Stop current music
        cleanupMusicStream();

        // Load new music stream
        m_musicStream = m_loader->loadStream(filename, CHANNEL_MUSIC);
        if (!m_musicStream) {
            printf("SDL2Audio::AudioStreamManager - Failed to load music: %s\n", filename);
            return false;
        }

        // Configure stream
        m_musicStream->setLooping(loop);
        m_musicStream->setVolume(m_musicVolume);

        // Add to mixer and play
        m_mixer->addSource(m_musicStream);
        m_musicStream->play();
        
        m_musicState = STREAM_STATE_PLAYING;

        printf("SDL2Audio::AudioStreamManager - Playing music: %s\n", filename);
        return true;
    }

    bool AudioStreamManager::playMusicTrack(const AsciiString& trackName)
    {
        const MusicTrack* track = m_playlist.findTrack(trackName);
        if (!track) {
            printf("SDL2Audio::AudioStreamManager - Track not found: %s\n", trackName.str());
            return false;
        }

        return playMusic(track->filename.str(), track->isLooping);
    }

    void AudioStreamManager::stopMusic()
    {
        if (m_musicStream) {
            m_musicStream->stop();
            m_musicState = STREAM_STATE_STOPPED;
        }
        cleanupMusicStream();
    }

    void AudioStreamManager::pauseMusic()
    {
        if (m_musicStream && m_musicState == STREAM_STATE_PLAYING) {
            m_musicStream->pause();
            m_musicState = STREAM_STATE_PAUSED;
        }
    }

    void AudioStreamManager::resumeMusic()
    {
        if (m_musicStream && m_musicState == STREAM_STATE_PAUSED) {
            m_musicStream->resume();
            m_musicState = STREAM_STATE_PLAYING;
        }
    }

    bool AudioStreamManager::isMusicPlaying() const
    {
        return (m_musicState == STREAM_STATE_PLAYING && 
                m_musicStream && 
                m_musicStream->isPlaying());
    }

    bool AudioStreamManager::isMusicPaused() const
    {
        return (m_musicState == STREAM_STATE_PAUSED);
    }

    void AudioStreamManager::nextTrack()
    {
        const MusicTrack* track = m_playlist.getNextTrack();
        if (track) {
            playMusic(track->filename.str(), track->isLooping);
        }
    }

    void AudioStreamManager::previousTrack()
    {
        const MusicTrack* track = m_playlist.getPreviousTrack();
        if (track) {
            playMusic(track->filename.str(), track->isLooping);
        }
    }

    AsciiString AudioStreamManager::getCurrentTrackName() const
    {
        const MusicTrack* track = m_playlist.getCurrentTrack();
        if (track) {
            return track->trackName;
        }
        return AsciiString();
    }

    int AudioStreamManager::getCurrentTrackPosition() const
    {
        if (m_musicStream) {
            return m_musicStream->getCurrentPosition();
        }
        return 0;
    }

    int AudioStreamManager::getCurrentTrackDuration() const
    {
        if (m_musicStream) {
            return m_musicStream->getStreamLength();
        }
        return 0;
    }

    void AudioStreamManager::setMusicVolume(float volume)
    {
        m_musicVolume = volume;
        if (m_musicVolume < 0.0f) m_musicVolume = 0.0f;
        if (m_musicVolume > 1.0f) m_musicVolume = 1.0f;

        if (m_musicStream) {
            m_musicStream->setVolume(m_musicVolume);
        }
    }

    bool AudioStreamManager::playAmbient(const char* filename, bool loop)
    {
        if (!m_initialized || !filename) {
            return false;
        }

        // Stop current ambient
        cleanupAmbientStream();

        // Load new ambient stream
        m_ambientStream = m_loader->loadStream(filename, CHANNEL_AMBIENT);
        if (!m_ambientStream) {
            printf("SDL2Audio::AudioStreamManager - Failed to load ambient: %s\n", filename);
            return false;
        }

        // Configure stream
        m_ambientStream->setLooping(loop);
        m_ambientStream->setVolume(m_ambientVolume);

        // Add to mixer and play
        m_mixer->addSource(m_ambientStream);
        m_ambientStream->play();
        
        m_ambientState = STREAM_STATE_PLAYING;

        printf("SDL2Audio::AudioStreamManager - Playing ambient: %s\n", filename);
        return true;
    }

    void AudioStreamManager::stopAmbient()
    {
        if (m_ambientStream) {
            m_ambientStream->stop();
            m_ambientState = STREAM_STATE_STOPPED;
        }
        cleanupAmbientStream();
    }

    void AudioStreamManager::pauseAmbient()
    {
        if (m_ambientStream && m_ambientState == STREAM_STATE_PLAYING) {
            m_ambientStream->pause();
            m_ambientState = STREAM_STATE_PAUSED;
        }
    }

    void AudioStreamManager::resumeAmbient()
    {
        if (m_ambientStream && m_ambientState == STREAM_STATE_PAUSED) {
            m_ambientStream->resume();
            m_ambientState = STREAM_STATE_PLAYING;
        }
    }

    bool AudioStreamManager::isAmbientPlaying() const
    {
        return (m_ambientState == STREAM_STATE_PLAYING && 
                m_ambientStream && 
                m_ambientStream->isPlaying());
    }

    void AudioStreamManager::setAmbientVolume(float volume)
    {
        m_ambientVolume = volume;
        if (m_ambientVolume < 0.0f) m_ambientVolume = 0.0f;
        if (m_ambientVolume > 1.0f) m_ambientVolume = 1.0f;

        if (m_ambientStream) {
            m_ambientStream->setVolume(m_ambientVolume);
        }
    }

    void AudioStreamManager::fadeMusicIn(int durationMS)
    {
        m_fadingMusic = true;
        m_fadeStartVolume = 0.0f;
        m_fadeTargetVolume = m_musicVolume;
        m_fadeTimeElapsed = 0.0f;
        m_fadeDuration = (float)durationMS;

        if (m_musicStream) {
            m_musicStream->setVolume(0.0f);
        }
    }

    void AudioStreamManager::fadeMusicOut(int durationMS)
    {
        m_fadingMusic = true;
        m_fadeStartVolume = m_musicVolume;
        m_fadeTargetVolume = 0.0f;
        m_fadeTimeElapsed = 0.0f;
        m_fadeDuration = (float)durationMS;
    }

    void AudioStreamManager::crossfadeToTrack(const char* filename, int durationMS)
    {
        // Fade out current track
        fadeMusicOut(durationMS);
        
        // After fade completes, load and fade in new track
        // This would need a callback system - simplified for now
    }

    void AudioStreamManager::update(float deltaTimeMS)
    {
        if (!m_initialized) {
            return;
        }

        // Update fade
        updateFade(deltaTimeMS);

        // Check if music stream finished (for non-looping tracks)
        if (m_musicStream && m_musicState == STREAM_STATE_PLAYING) {
            if (m_musicStream->isFinished()) {
                // Auto-advance to next track in playlist
                const MusicTrack* currentTrack = m_playlist.getCurrentTrack();
                if (currentTrack && !currentTrack->isLooping) {
                    nextTrack();
                }
            }
        }

        // Remove finished sources from mixer
        m_mixer->removeFinishedSources();
    }

    void AudioStreamManager::printState()
    {
        printf("\n=== SDL2 Audio Stream Manager ===\n");
        printf("Music State: ");
        switch (m_musicState) {
            case STREAM_STATE_STOPPED: printf("STOPPED\n"); break;
            case STREAM_STATE_PLAYING: printf("PLAYING\n"); break;
            case STREAM_STATE_PAUSED: printf("PAUSED\n"); break;
            case STREAM_STATE_LOADING: printf("LOADING\n"); break;
        }
        printf("Music Volume: %.2f\n", m_musicVolume);
        
        if (m_musicStream) {
            printf("Music Position: %d / %d ms\n", 
                getCurrentTrackPosition(), 
                getCurrentTrackDuration());
        }

        printf("\nAmbient State: ");
        switch (m_ambientState) {
            case STREAM_STATE_STOPPED: printf("STOPPED\n"); break;
            case STREAM_STATE_PLAYING: printf("PLAYING\n"); break;
            case STREAM_STATE_PAUSED: printf("PAUSED\n"); break;
            case STREAM_STATE_LOADING: printf("LOADING\n"); break;
        }
        printf("Ambient Volume: %.2f\n", m_ambientVolume);

        printf("\nPlaylist: %d tracks\n", m_playlist.getTrackCount());
        printf("Current Track: %s\n", getCurrentTrackName().str());
        printf("Shuffle: %s\n", m_playlist.isShuffled() ? "ON" : "OFF");
        printf("Repeat: %s\n", m_playlist.isRepeat() ? "ON" : "OFF");
        printf("=================================\n\n");
    }

    void AudioStreamManager::updateFade(float deltaTimeMS)
    {
        if (!m_fadingMusic) {
            return;
        }

        m_fadeTimeElapsed += deltaTimeMS;

        if (m_fadeTimeElapsed >= m_fadeDuration) {
            // Fade complete
            m_fadingMusic = false;
            if (m_musicStream) {
                m_musicStream->setVolume(m_fadeTargetVolume);
            }

            // If faded to zero, stop music
            if (m_fadeTargetVolume == 0.0f) {
                stopMusic();
            }
        } else {
            // Interpolate volume
            float t = m_fadeTimeElapsed / m_fadeDuration;
            float currentVolume = m_fadeStartVolume + (m_fadeTargetVolume - m_fadeStartVolume) * t;
            
            if (m_musicStream) {
                m_musicStream->setVolume(currentVolume);
            }
        }
    }

    void AudioStreamManager::cleanupMusicStream()
    {
        if (m_musicStream) {
            m_mixer->removeSource(m_musicStream);
            m_loader->unloadStream(m_musicStream);
            m_musicStream = nullptr;
        }
        m_musicState = STREAM_STATE_STOPPED;
        m_fadingMusic = false;
    }

    void AudioStreamManager::cleanupAmbientStream()
    {
        if (m_ambientStream) {
            m_mixer->removeSource(m_ambientStream);
            m_loader->unloadStream(m_ambientStream);
            m_ambientStream = nullptr;
        }
        m_ambientState = STREAM_STATE_STOPPED;
    }

} // namespace SDL2Audio
