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
 * Phase 07: OpenALAudioManager Implementation
 * 
 * Wraps OpenALDevice to provide GameAudio.h AudioManager interface
 */

#include "Common/GameAudio.h"
#include "Common/AudioEventRTS.h"
#include "AudioDevice/OpenALAudioManager.h"
#include "Common/Debug.h"
#include <stdio.h>

#pragma comment(lib, "OpenAL32.lib")

//-------------------------------------------------------------------------------------------------
/** Constructor */
//-------------------------------------------------------------------------------------------------
OpenALAudioManager::OpenALAudioManager()
	: m_audioDevice(nullptr)
	, m_isInitialized(false)
{
	fprintf(stderr, "[OpenALAudioManager] Constructor called\n");
	fflush(stderr);
}

//-------------------------------------------------------------------------------------------------
/** Destructor */
//-------------------------------------------------------------------------------------------------
OpenALAudioManager::~OpenALAudioManager()
{
	fprintf(stderr, "[OpenALAudioManager] Destructor - cleaning up\n");
	fflush(stderr);
	
	if (m_audioDevice) {
		m_audioDevice->shutdown();
		delete m_audioDevice;
		m_audioDevice = nullptr;
	}
}

//-------------------------------------------------------------------------------------------------
/** Initialize the audio manager */
//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::init()
{
	fprintf(stderr, "[OpenALAudioManager] init() called\n");
	fflush(stderr);
	
	if (m_isInitialized) {
		fprintf(stderr, "[OpenALAudioManager] Already initialized, skipping\n");
		fflush(stderr);
		return;
	}

	// Create and initialize OpenAL device
	m_audioDevice = new GeneralsX::Audio::OpenALDevice();
	
	if (m_audioDevice && m_audioDevice->init()) {
		fprintf(stderr, "[OpenALAudioManager] OpenAL device initialized successfully\n");
		fflush(stderr);
		m_isInitialized = true;
	} else {
		fprintf(stderr, "[OpenALAudioManager] FAILED to initialize OpenAL device\n");
		fflush(stderr);
		if (m_audioDevice) {
			delete m_audioDevice;
			m_audioDevice = nullptr;
		}
	}
}

//-------------------------------------------------------------------------------------------------
/** Post-process load */
//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::postProcessLoad()
{
	if (m_audioDevice) {
		fprintf(stderr, "[OpenALAudioManager] Post-processing load\n");
		fflush(stderr);
	}
}

//-------------------------------------------------------------------------------------------------
/** Reset the audio manager */
//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::reset()
{
	fprintf(stderr, "[OpenALAudioManager] reset() called\n");
	fflush(stderr);
	
	if (m_audioDevice) {
		m_audioDevice->stopAll();
	}
}

//-------------------------------------------------------------------------------------------------
/** Update the audio manager */
//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::update()
{
	if (m_audioDevice && m_isInitialized) {
		m_audioDevice->update(0.016f); // Approximate 60 FPS update
	}
}

//-------------------------------------------------------------------------------------------------
/** Open the audio device */
//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::openDevice(void)
{
	fprintf(stderr, "[OpenALAudioManager] openDevice() called\n");
	fflush(stderr);
	
	if (!m_audioDevice || !m_isInitialized) {
		init();
	}
}

//-------------------------------------------------------------------------------------------------
/** Close the audio device */
//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::closeDevice(void)
{
	fprintf(stderr, "[OpenALAudioManager] closeDevice() called\n");
	fflush(stderr);
	
	if (m_audioDevice) {
		m_audioDevice->shutdown();
		m_isInitialized = false;
	}
}

//-------------------------------------------------------------------------------------------------
/** Get provider name */
//-------------------------------------------------------------------------------------------------
AsciiString OpenALAudioManager::getProviderName(UnsignedInt providerNum) const
{
	if (providerNum == 0) {
		return "OpenAL-soft";
	}
	return "Unknown";
}

//-------------------------------------------------------------------------------------------------
/** Play an audio event */
//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::friend_forcePlayAudioEventRTS(const AudioEventRTS* eventToPlay)
{
	if (!m_audioDevice || !m_isInitialized || !eventToPlay) {
		return;
	}

	// Generate the filename to load
	AudioEventRTS* nonConstEvent = const_cast<AudioEventRTS*>(eventToPlay);
	nonConstEvent->generateFilename();
	
	AsciiString filename = nonConstEvent->getFilename();
	if (filename.isEmpty()) {
		fprintf(stderr, "[OpenALAudioManager] WARNING: Empty filename for event '%s'\n", 
				eventToPlay->getEventName().str());
		fflush(stderr);
		return;
	}

	// Get volume from event (or use default)
	float volume = eventToPlay->getVolume();
	if (volume < 0.0f) {
		volume = 1.0f;  // Default volume
	}

	fprintf(stderr, "[OpenALAudioManager] Playing audio: '%s' (vol=%.2f)\n", 
			filename.str(), volume);
	fflush(stderr);

	// Determine if this is 3D audio based on event properties
	// For now, treat all as 2D (non-positional)
	int playHandle = m_audioDevice->playSound(filename.str(), volume);
	
	if (playHandle >= 0) {
		fprintf(stderr, "[OpenALAudioManager] Successfully started audio playback (handle=%d)\n", playHandle);
		fflush(stderr);
	} else {
		fprintf(stderr, "[OpenALAudioManager] ERROR: Failed to play audio '%s'\n", filename.str());
		fprintf(stderr, "[OpenALAudioManager] Device error: %s\n", m_audioDevice->getLastError());
		fflush(stderr);
	}
}

//-------------------------------------------------------------------------------------------------
/** Update listener position from AudioManager */
//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::setDeviceListenerPosition(void)
{
	if (!m_audioDevice || !m_isInitialized) {
		return;
	}

	// Get listener position from the base AudioManager class
	// The game's AudioManager stores this in m_listenerPosition
	// We need to forward it to the OpenAL device
	
	// Note: This is called by AudioManager::update() after it calculates listener position
	// We get the position via getListenerPosition() which is set by AudioManager
	const Coord3D* listenerPos = getListenerPosition();
	if (listenerPos) {
		m_audioDevice->setListenerPosition(listenerPos->x, listenerPos->y, listenerPos->z);
		
		// Also set listener orientation (forward/up vectors)
		// Defaults: looking down -Z (forward) with +Y as up
		m_audioDevice->setListenerOrientation(0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
		
		fprintf(stderr, "[OpenALAudioManager] Updated listener position: (%.2f, %.2f, %.2f)\n", 
				listenerPos->x, listenerPos->y, listenerPos->z);
		fflush(stderr);
	}
}

