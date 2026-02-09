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
** OpenALAudioManager.cpp
**
** OpenAL audio backend implementation for Linux builds.
**
** TheSuperHackers @feature CnC_Generals_Linux 07/02/2026
** Provides OpenAL-based audio playback for sound effects, music, and voices.
** Based on fighter19 reference implementation.
*/

#ifndef _WIN32
#ifdef SAGE_USE_OPENAL

#include "OpenALAudioManager.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>

/**
 * Constructor: Initialize OpenAL manager state
 */
OpenALAudioManager::OpenALAudioManager()
	: m_alcDevice(nullptr),
	  m_alcContext(nullptr),
	  m_isInitialized(false),
	  m_isMusicPlaying(false),
	  m_isPaused(false),
	  m_isAmbientPaused(false)
{
	m_currentMusicTrack = AsciiString::TheEmptyString;
	
	// Pre-allocate source vectors
	m_sources2D.reserve(OPENAL_SOURCES_2D);
	m_sources3D.reserve(OPENAL_SOURCES_3D);
	m_streamSources.reserve(OPENAL_STREAMS);
	
	fprintf(stderr, "DEBUG: OpenALAudioManager::OpenALAudioManager() created\n");
}

/**
 * Destructor: Cleanup OpenAL resources
 */
OpenALAudioManager::~OpenALAudioManager()
{
	if (m_isInitialized) {
		closeDevice();
	}
	fprintf(stderr, "DEBUG: OpenALAudioManager::~OpenALAudioManager() destroyed\n");
}

/**
 * Initialize OpenAL device and context
 */
bool OpenALAudioManager::initializeALContext()
{
	// Open default OpenAL device
	m_alcDevice = alcOpenDevice(nullptr);
	if (!m_alcDevice) {
		fprintf(stderr, "ERROR: Failed to open OpenAL device\n");
		return false;
	}

	// Create OpenAL context
	m_alcContext = alcCreateContext(m_alcDevice, nullptr);
	if (!m_alcContext) {
		fprintf(stderr, "ERROR: Failed to create OpenAL context\n");
		alcCloseDevice(m_alcDevice);
		m_alcDevice = nullptr;
		return false;
	}

	// Make context current (required before AL calls)
	if (!alcMakeContextCurrent(m_alcContext)) {
		fprintf(stderr, "ERROR: Failed to make OpenAL context current\n");
		alcDestroyContext(m_alcContext);
		alcCloseDevice(m_alcDevice);
		m_alcContext = nullptr;
		m_alcDevice = nullptr;
		return false;
	}

	// Enable distance model for 3D audio
	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

	// Log OpenAL info
	fprintf(stderr, "DEBUG: OpenAL initialized successfully\n");
	fprintf(stderr, "  Device: %s\n", alcGetString(m_alcDevice, ALC_DEVICE_SPECIFIER));
	fprintf(stderr, "  Vendor: %s\n", alGetString(AL_VENDOR));
	fprintf(stderr, "  Renderer: %s\n", alGetString(AL_RENDERER));
	fprintf(stderr, "  Version: %s\n", alGetString(AL_VERSION));

	return true;
}

/**
 * Shutdown OpenAL context
 */
void OpenALAudioManager::shutdownALContext()
{
	if (m_alcContext) {
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(m_alcContext);
		m_alcContext = nullptr;
	}

	if (m_alcDevice) {
		alcCloseDevice(m_alcDevice);
		m_alcDevice = nullptr;
	}

	fprintf(stderr, "DEBUG: OpenAL context shutdown\n");
}

/**
 * Allocate an OpenAL source from pool
 */
ALuint OpenALAudioManager::allocateSource(Bool is3D)
{
	std::vector<ALuint>& sourcePool = is3D ? m_sources3D : m_sources2D;

	// If pool has available sources, return one
	if (!sourcePool.empty()) {
		ALuint source = sourcePool.back();
		sourcePool.pop_back();
		return source;
	}

	// Otherwise, generate a new source
	ALuint source;
	alGenSources(1, &source);

	if (alGetError() != AL_NO_ERROR) {
		fprintf(stderr, "ERROR: Failed to generate OpenAL source\n");
		return 0;
	}

	// Configure source as 2D or 3D
	if (is3D) {
		alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);
	} else {
		alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
	}

	return source;
}

/**
 * Release an OpenAL source back to pool
 */
void OpenALAudioManager::releaseSource(ALuint source)
{
	if (source == 0) {
		return;
	}

	// Stop source
	alSourceStop(source);

	// Clear source properties
	alSourcei(source, AL_BUFFER, 0);

	// Return to appropriate pool
	// TODO: Determine if 2D or 3D and return to correct pool
	m_sources2D.push_back(source);
}

/**
 * From SubsystemInterface: init() - calls openDevice()
 */
void OpenALAudioManager::init()
{
	openDevice();
}

/**
 * From SubsystemInterface: postProcessLoad() - stub for Phase 2
 */
void OpenALAudioManager::postProcessLoad()
{
	fprintf(stderr, "DEBUG: OpenALAudioManager::postProcessLoad() stub\n");
}

/**
 * From SubsystemInterface: reset() - reset all audio state
 */
void OpenALAudioManager::reset()
{
	stopAudio(AudioAffect_All);
	m_isMusicPlaying = false;
	m_currentMusicTrack = AsciiString::TheEmptyString;
	fprintf(stderr, "DEBUG: OpenALAudioManager::reset()\n");
}

/**
 * From SubsystemInterface: update() - per-frame update
 */
void OpenALAudioManager::update()
{
	// TODO: Phase 2 - Poll finished sources, update 3D listener, process audio events
	// For now, just a no-op
}

/**
 * Open OpenAL device and initialize context
 */
void OpenALAudioManager::openDevice(void)
{
	if (m_isInitialized) {
		return;
	}

	if (!initializeALContext()) {
		fprintf(stderr, "ERROR: Failed to initialize OpenAL context\n");
		return;
	}

	// Pre-allocate source pools
	for (int i = 0; i < OPENAL_SOURCES_2D; i++) {
		ALuint source;
		alGenSources(1, &source);
		if (alGetError() == AL_NO_ERROR) {
			m_sources2D.push_back(source);
		}
	}

	for (int i = 0; i < OPENAL_SOURCES_3D; i++) {
		ALuint source;
		alGenSources(1, &source);
		if (alGetError() == AL_NO_ERROR) {
			m_sources3D.push_back(source);
		}
	}

	for (int i = 0; i < OPENAL_STREAMS; i++) {
		ALuint source;
		alGenSources(1, &source);
		if (alGetError() == AL_NO_ERROR) {
			m_streamSources.push_back(source);
		}
	}

	m_isInitialized = true;
	fprintf(stderr, "DEBUG: OpenALAudioManager::openDevice() - allocated %zd 2D, %zd 3D, %zd stream sources\n",
			m_sources2D.size(), m_sources3D.size(), m_streamSources.size());
}

/**
 * Close OpenAL device and cleanup resources
 */
void OpenALAudioManager::closeDevice(void)
{
	if (!m_isInitialized) {
		return;
	}

	// Delete all sources
	for (ALuint source : m_sources2D) {
		alDeleteSources(1, &source);
	}
	m_sources2D.clear();

	for (ALuint source : m_sources3D) {
		alDeleteSources(1, &source);
	}
	m_sources3D.clear();

	for (ALuint source : m_streamSources) {
		alDeleteSources(1, &source);
	}
	m_streamSources.clear();

	shutdownALContext();

	m_isInitialized = false;
	fprintf(stderr, "DEBUG: OpenALAudioManager::closeDevice()\n");
}

/**
 * Stop all audio playback (AudioAffect_All, AudioAffect_Music, etc.)
 */
void OpenALAudioManager::stopAudio(AudioAffect which)
{
	if (!m_isInitialized) {
		return;
	}

	// Stop all sources in both pools
	for (ALuint source : m_sources2D) {
		alSourceStop(source);
	}
	for (ALuint source : m_sources3D) {
		alSourceStop(source);
	}
	for (ALuint source : m_streamSources) {
		alSourceStop(source);
	}

	fprintf(stderr, "DEBUG: OpenALAudioManager::stopAudio(%d)\n", which);
}

/**
 * Pause all audio playback
 */
void OpenALAudioManager::pauseAudio(AudioAffect which)
{
	if (!m_isInitialized || m_isPaused) {
		return;
	}

	for (ALuint source : m_sources2D) {
		alSourcePause(source);
	}
	for (ALuint source : m_sources3D) {
		alSourcePause(source);
	}
	for (ALuint source : m_streamSources) {
		alSourcePause(source);
	}

	m_isPaused = true;
	fprintf(stderr, "DEBUG: OpenALAudioManager::pauseAudio(%d)\n", which);
}

/**
 * Resume all audio playback
 */
void OpenALAudioManager::resumeAudio(AudioAffect which)
{
	if (!m_isInitialized || !m_isPaused) {
		return;
	}

	for (ALuint source : m_sources2D) {
		ALenum state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		if (state == AL_PAUSED) {
			alSourcePlay(source);
		}
	}
	for (ALuint source : m_sources3D) {
		ALenum state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		if (state == AL_PAUSED) {
			alSourcePlay(source);
		}
	}
	for (ALuint source : m_streamSources) {
		ALenum state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		if (state == AL_PAUSED) {
			alSourcePlay(source);
		}
	}

	m_isPaused = false;
	fprintf(stderr, "DEBUG: OpenALAudioManager::resumeAudio(%d)\n", which);
}

/**
 * Pause/unpause ambient sounds only
 */
void OpenALAudioManager::pauseAmbient(Bool shouldPause)
{
	m_isAmbientPaused = shouldPause;
	fprintf(stderr, "DEBUG: OpenALAudioManager::pauseAmbient(%d)\n", shouldPause);
}

/**
 * Immediately kill an audio event by handle
 * TODO: Phase 2 - Track audio handles and implement real kill logic
 */
void OpenALAudioManager::killAudioEventImmediately(AudioHandle audioEvent)
{
	fprintf(stderr, "DEBUG: OpenALAudioManager::killAudioEventImmediately(0x%x) - stub\n", audioEvent);
}

/**
 * Add an audio event to the playback queue
 * TODO: Phase 2 - Implement full audio event system
 */
AudioHandle OpenALAudioManager::addAudioEvent(const AudioEventRTS *eventToAdd)
{
	if (!m_isInitialized) {
		return AUDIO_HANDLE_INVALID;
	}

	fprintf(stderr, "DEBUG: OpenALAudioManager::addAudioEvent() - stub (Phase 2)\n");
	return AUDIO_HANDLE_INVALID;
}

/**
 * Remove a previously added audio event
 */
void OpenALAudioManager::removeAudioEvent(AudioHandle audioEvent)
{
	fprintf(stderr, "DEBUG: OpenALAudioManager::removeAudioEvent(0x%x) - stub\n", audioEvent);
}

/**
 * Check if an audio event is currently playing
 */
Bool OpenALAudioManager::isCurrentlyPlaying(AudioHandle handle)
{
	// TODO: Phase 2 - Track playing audio by handle
	return false;
}

/**
 * Music playback - next track
 */
void OpenALAudioManager::nextMusicTrack(void)
{
	fprintf(stderr, "DEBUG: OpenALAudioManager::nextMusicTrack() - stub\n");
}

/**
 * Music playback - previous track
 */
void OpenALAudioManager::prevMusicTrack(void)
{
	fprintf(stderr, "DEBUG: OpenALAudioManager::prevMusicTrack() - stub\n");
}

/**
 * Check if music is playing
 */
Bool OpenALAudioManager::isMusicPlaying(void) const
{
	return m_isMusicPlaying;
}

/**
 * Check if a music track has completed N times
 */
Bool OpenALAudioManager::hasMusicTrackCompleted(const AsciiString &trackName, Int numberOfTimes) const
{
	// TODO: Phase 2 - Track music completion count
	return false;
}

/**
 * Get current music track name
 */
AsciiString OpenALAudioManager::getMusicTrackName(void) const
{
	return m_currentMusicTrack;
}

#endif // SAGE_USE_OPENAL
#endif // !_WIN32

