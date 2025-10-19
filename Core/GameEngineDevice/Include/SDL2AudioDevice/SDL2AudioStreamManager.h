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

// FILE: SDL2AudioStreamManager.h
// Audio Stream Management System - Background music and ambient sound management
// Author: Phase 32.2 - Audio Stream Management
// Date: October 19, 2025

#ifndef SDL2AUDIOSTREAMMANAGER_H
#define SDL2AUDIOSTREAMMANAGER_H

#include "SDL2AudioDevice/SDL2AudioMixer.h"
#include "SDL2AudioDevice/SDL2MilesAudioSource.h"
#include "Common/AsciiString.h"
#include <vector>
#include <map>

namespace SDL2Audio
{
    // Stream playback state
    enum StreamState
    {
        STREAM_STATE_STOPPED = 0,
        STREAM_STATE_PLAYING,
        STREAM_STATE_PAUSED,
        STREAM_STATE_LOADING
    };

    // Music track information
    struct MusicTrack
    {
        AsciiString filename;
        AsciiString trackName;
        int durationMS;
        bool isLooping;
        
        MusicTrack()
            : durationMS(0)
            , isLooping(false)
        {}
    };

    // Music playlist
    class MusicPlaylist
    {
    public:
        MusicPlaylist();
        ~MusicPlaylist();

        // Playlist management
        void addTrack(const AsciiString& filename, const AsciiString& trackName, bool loop = false);
        void removeTrack(const AsciiString& trackName);
        void clearPlaylist();
        
        int getTrackCount() const { return (int)m_tracks.size(); }
        const MusicTrack* getTrack(int index) const;
        const MusicTrack* findTrack(const AsciiString& trackName) const;

        // Playback order
        void setShuffled(bool shuffled);
        bool isShuffled() const { return m_shuffled; }
        
        void setRepeat(bool repeat);
        bool isRepeat() const { return m_repeat; }

        // Navigation
        const MusicTrack* getNextTrack();
        const MusicTrack* getPreviousTrack();
        const MusicTrack* getCurrentTrack() const;
        
        void reset();

    private:
        std::vector<MusicTrack> m_tracks;
        int m_currentTrackIndex;
        bool m_shuffled;
        bool m_repeat;
    };

    // Audio stream manager - manages background music and ambient sounds
    class AudioStreamManager
    {
    public:
        AudioStreamManager();
        ~AudioStreamManager();

        // Initialization
        void init(AudioMixer* mixer, AudioFileLoader* loader);
        void shutdown();
        bool isInitialized() const { return m_initialized; }

        // Music playback
        bool playMusic(const char* filename, bool loop = false);
        bool playMusicTrack(const AsciiString& trackName);
        void stopMusic();
        void pauseMusic();
        void resumeMusic();
        
        bool isMusicPlaying() const;
        bool isMusicPaused() const;
        
        // Music navigation (playlist)
        void nextTrack();
        void previousTrack();
        AsciiString getCurrentTrackName() const;
        int getCurrentTrackPosition() const;  // In milliseconds
        int getCurrentTrackDuration() const;  // In milliseconds

        // Music volume (0.0 to 1.0)
        void setMusicVolume(float volume);
        float getMusicVolume() const { return m_musicVolume; }

        // Ambient sound management
        bool playAmbient(const char* filename, bool loop = true);
        void stopAmbient();
        void pauseAmbient();
        void resumeAmbient();
        
        bool isAmbientPlaying() const;
        
        // Ambient volume (0.0 to 1.0)
        void setAmbientVolume(float volume);
        float getAmbientVolume() const { return m_ambientVolume; }

        // Playlist management
        MusicPlaylist& getPlaylist() { return m_playlist; }
        const MusicPlaylist& getPlaylist() const { return m_playlist; }

        // Fade effects
        void fadeMusicIn(int durationMS);
        void fadeMusicOut(int durationMS);
        void crossfadeToTrack(const char* filename, int durationMS);

        // Update (called each frame)
        void update(float deltaTimeMS);

        // Debug information
        void printState();

    private:
        bool m_initialized;
        
        AudioMixer* m_mixer;
        AudioFileLoader* m_loader;
        
        // Current music stream
        MilesStreamSource* m_musicStream;
        StreamState m_musicState;
        float m_musicVolume;
        
        // Current ambient stream
        MilesStreamSource* m_ambientStream;
        StreamState m_ambientState;
        float m_ambientVolume;
        
        // Music playlist
        MusicPlaylist m_playlist;
        
        // Fade state
        bool m_fadingMusic;
        float m_fadeStartVolume;
        float m_fadeTargetVolume;
        float m_fadeTimeElapsed;
        float m_fadeDuration;
        
        // Helper methods
        void updateFade(float deltaTimeMS);
        void cleanupMusicStream();
        void cleanupAmbientStream();
    };

    // Global stream manager instance
    AudioStreamManager& getGlobalStreamManager();

} // namespace SDL2Audio

#endif // SDL2AUDIOSTREAMMANAGER_H
