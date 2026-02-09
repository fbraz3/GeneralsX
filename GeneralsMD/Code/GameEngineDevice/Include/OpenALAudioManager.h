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

/*
** OpenALAudioManager.h
**
** OpenAL audio backend implementation for Linux builds.
**
** TheSuperHackers @feature CnC_Generals_Linux 07/02/2026
** Provides OpenAL-based audio playback for sound effects, music, and voices.
** Based on fighter19 reference implementation.
*/

#pragma once

#ifndef _WIN32
#ifdef SAGE_USE_OPENAL

#include "Common/GameAudio.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <vector>

// Forward declarations
class AudioEventRTS;

// OpenAL source pools (jmarshall pattern)
#define OPENAL_SOURCES_2D   32
#define OPENAL_SOURCES_3D   128
#define OPENAL_STREAMS      4

/**
 * OpenALAudioManager
 *
 * Implements AudioManager interface using OpenAL for Linux builds.
 * Manages ALCdevice/ALCcontext lifecycle, audio source pooling, and playback.
 *
 * Implementation pattern:
 * - ALCdevice: OpenAL device handle (usually default)
 * - ALCcontext: OpenAL context attached to device
 * - Source pooling: Pre-allocated ALuint sources for playback
 * - Event processing: Queues audio requests and processes them per frame
 */
class OpenALAudioManager : public AudioManager
{
public:
	OpenALAudioManager();
	virtual ~OpenALAudioManager();

	// AudioManager interface - lifecycle
	virtual void init();
	virtual void postProcessLoad();
	virtual void reset();
	virtual void update();

	// AudioManager interface - playback control
	virtual void openDevice(void);
	virtual void closeDevice(void);
	virtual void stopAudio(AudioAffect which);
	virtual void pauseAudio(AudioAffect which);
	virtual void resumeAudio(AudioAffect which);
	virtual void pauseAmbient(Bool shouldPause);
	virtual void killAudioEventImmediately(AudioHandle audioEvent);

	// AudioManager interface - music
	virtual void nextMusicTrack(void);
	virtual void prevMusicTrack(void);
	virtual Bool isMusicPlaying(void) const;
	virtual Bool hasMusicTrackCompleted(const AsciiString &trackName, Int numberOfTimes) const;
	virtual AsciiString getMusicTrackName(void) const;

	// AudioManager interface - audio events (placeholder for Phase 2)
	virtual AudioHandle addAudioEvent(const AudioEventRTS *eventToAdd);
	virtual void removeAudioEvent(AudioHandle audioEvent);
	virtual Bool isCurrentlyPlaying(AudioHandle handle);

	// Query state
	virtual Bool isInitialized() const { return m_isInitialized; }

protected:
	// Device management
	ALCdevice*		m_alcDevice;
	ALCcontext*		m_alcContext;
	Bool			m_isInitialized;

	// Source pooling
	std::vector<ALuint>	m_sources2D;
	std::vector<ALuint>	m_sources3D;
	std::vector<ALuint>	m_streamSources;

	// State tracking
	Bool			m_isMusicPlaying;
	AsciiString		m_currentMusicTrack;
	Bool			m_isPaused;
	Bool			m_isAmbientPaused;

	// Helper methods
	ALuint allocateSource(Bool is3D);
	void releaseSource(ALuint source);
	Bool initializeALContext();
	void shutdownALContext();
};

#endif // SAGE_USE_OPENAL
#endif // !_WIN32
