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
 * Phase 07: OpenALAudioManager - AudioManager Wrapper for OpenAL Backend
 * 
 * Adapts the OpenALDevice (AudioDevice interface) to the AudioManager interface
 * This allows the game engine to use OpenAL audio via the AudioManager abstraction
 */

#pragma once

#include "Common/GameAudio.h"
#include "AudioDevice/OpenALDevice.h"

class AudioEventRTS;

/**
 * OpenAL-based audio manager
 * Wraps OpenALDevice to provide AudioManager interface
 */
class OpenALAudioManager : public AudioManager
{
public:
	OpenALAudioManager();
	virtual ~OpenALAudioManager();

	// AudioManager interface implementation
	virtual void init();
	virtual void postProcessLoad();
	virtual void reset();
	virtual void update();

	virtual void nextMusicTrack(void) {}
	virtual void prevMusicTrack(void) {}
	virtual Bool isMusicPlaying(void) const { return false; }
	virtual Bool hasMusicTrackCompleted(const AsciiString& trackName, Int numberOfTimes) const { return false; }
	virtual AsciiString getMusicTrackName(void) const { return AsciiString::TheEmptyString; }

	virtual void openDevice(void);
	virtual void closeDevice(void);
	virtual void *getDevice(void) { return m_audioDevice; }

	virtual void stopAudio(AudioAffect which) {}
	virtual void pauseAudio(AudioAffect which) {}
	virtual void resumeAudio(AudioAffect which) {}
	virtual void pauseAmbient(Bool shouldPause) {}

	virtual void killAudioEventImmediately(AudioHandle audioEvent) {}

	virtual Bool isCurrentlyPlaying(AudioHandle handle) { return false; }

	virtual void notifyOfAudioCompletion(UnsignedInt audioCompleted, UnsignedInt flags) {}
	virtual PlayingAudio *findPlayingAudioFrom(UnsignedInt audioCompleted, UnsignedInt flags) { return nullptr; }

	virtual UnsignedInt getProviderCount(void) const { return 1; }
	virtual AsciiString getProviderName(UnsignedInt providerNum) const;
	virtual UnsignedInt getProviderIndex(AsciiString providerName) const { return 0; }
	virtual void selectProvider(UnsignedInt providerNdx) {}
	virtual void unselectProvider(void) {}
	virtual UnsignedInt getSelectedProvider(void) const { return 0; }
	virtual void setSpeakerType(UnsignedInt speakerType) {}
	virtual UnsignedInt getSpeakerType(void) { return 0; }

	virtual void *getHandleForBink(void) { return nullptr; }
	virtual void releaseHandleForBink(void) {}

	virtual void friend_forcePlayAudioEventRTS(const AudioEventRTS* eventToPlay) {}

	virtual UnsignedInt getNum2DSamples(void) const { return 64; }
	virtual UnsignedInt getNum3DSamples(void) const { return 32; }
	virtual UnsignedInt getNumStreams(void) const { return 4; }

	virtual Bool doesViolateLimit(AudioEventRTS *event) const { return false; }
	virtual Bool isPlayingLowerPriority(AudioEventRTS *event) const { return false; }
	virtual Bool isPlayingAlready(AudioEventRTS *event) const { return false; }
	virtual Bool isObjectPlayingVoice(UnsignedInt objID) const { return false; }

	virtual void adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume) {}

	virtual void removePlayingAudio(AsciiString eventName) {}
	virtual void removeAllDisabledAudio() {}

	virtual void processRequestList(void) {}
	virtual void processPlayingList(void) {}
	virtual void processFadingList(void) {}
	virtual void processStoppedList(void) {}

	virtual void setHardwareAccelerated(Bool accel) {}
	virtual void setSpeakerSurround(Bool surround) {}

	virtual void setPreferredProvider(AsciiString provider) {}
	virtual void setPreferredSpeaker(AsciiString speakerType) {}

	virtual Real getFileLengthMS(AsciiString strToLoad) const { return 0.0f; }

	virtual void closeAnySamplesUsingFile(const void *fileToClose) {}

	virtual Bool has3DSensitiveStreamsPlaying(void) const { return false; }

protected:
	GeneralsX::Audio::OpenALDevice *m_audioDevice;
	Bool m_isInitialized;
};

