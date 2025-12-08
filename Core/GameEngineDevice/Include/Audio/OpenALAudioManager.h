/*
**  Command & Conquer Generals Zero Hour(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Common/GameAudio.h"
#include "Common/AsciiString.h"
// Bink handle stream
#include "OpenALAudioDevice/OpenALAudioStream.h"

/* Forward declarations */
struct OpenALAudioDevice;
class AudioEventRTS;
class DebugDisplayInterface;

/**
 * OpenAL-based audio manager implementation
 * 
 * Provides cross-platform audio support using OpenAL backend.
 * Manages audio playback, 3D positioning, effects, and channel prioritization.
 * 
 * Phase 33: Initial implementation with basic playback and volume control.
 */
class OpenALAudioManager : public AudioManager
{
public:
    OpenALAudioManager();
    virtual ~OpenALAudioManager();

#if defined(RTS_DEBUG)
    virtual void audioDebugDisplay(DebugDisplayInterface *dd, void *userData, FILE *fp = NULL);
#endif

    // From SubsystemInterface
    virtual void init();
    virtual void postProcessLoad();
    virtual void reset();
    virtual void update();

    // Audio control
    virtual void stopAudio(AudioAffect which);
    virtual void pauseAudio(AudioAffect which);
    virtual void resumeAudio(AudioAffect which);
    virtual void pauseAmbient(Bool shouldPause);

    // Focus handling
    virtual void loseFocus(void);
    virtual void regainFocus(void);

    // Audio event management
    virtual void killAudioEventImmediately(AudioHandle audioEvent);

    // Music track control
    virtual void nextMusicTrack(void);
    virtual void prevMusicTrack(void);
    virtual Bool isMusicPlaying(void) const;
    virtual Bool hasMusicTrackCompleted(const AsciiString& trackName, Int numberOfTimes) const;
    virtual AsciiString getMusicTrackName(void) const;

    // Device management
    virtual void openDevice(void);
    virtual void closeDevice(void);
    virtual void *getDevice(void);

    // Audio completion notifications
    virtual void notifyOfAudioCompletion(UnsignedInt audioCompleted, UnsignedInt flags);

    // Provider management (audio devices)
    virtual UnsignedInt getProviderCount(void) const;
    virtual AsciiString getProviderName(UnsignedInt providerNum) const;
    virtual UnsignedInt getProviderIndex(AsciiString providerName) const;
    virtual void selectProvider(UnsignedInt providerNdx);
    virtual void unselectProvider(void);
    virtual UnsignedInt getSelectedProvider(void) const;

    // Speaker type management
    virtual void setSpeakerType(UnsignedInt speakerType);
    virtual UnsignedInt getSpeakerType(void);

    // Channel information
    virtual UnsignedInt getNum2DSamples(void) const;
    virtual UnsignedInt getNum3DSamples(void) const;
    virtual UnsignedInt getNumStreams(void) const;

    // Audio prioritization
    virtual Bool doesViolateLimit(AudioEventRTS *event) const;
    virtual Bool isPlayingLowerPriority(AudioEventRTS *event) const;
    virtual Bool isPlayingAlready(AudioEventRTS *event) const;
    virtual Bool isObjectPlayingVoice(UnsignedInt objID) const;

    // Playing audio manipulation
    virtual void adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume);
    virtual void removePlayingAudio(AsciiString eventName);
    virtual void removeAllDisabledAudio();

    // 3D streaming
    virtual Bool has3DSensitiveStreamsPlaying(void) const;

    // Bink video support
    virtual void *getHandleForBink(void);
    virtual void releaseHandleForBink(void);

    // Audio event forced playback
    virtual void friend_forcePlayAudioEventRTS(const AudioEventRTS* eventToPlay);

    // File management
    virtual Real getFileLengthMS(AsciiString strToLoad) const;
    virtual void closeAnySamplesUsingFile(const void *fileToClose);

    // Provider preferences
    virtual void setPreferredProvider(AsciiString providerNdx);
    virtual void setPreferredSpeaker(AsciiString speakerType);

    // Device listener positioning
    virtual void setDeviceListenerPosition(void);

private:
    OpenALAudioDevice *m_audioDevice;
    OpenALAudioStream *m_binkStream = nullptr;
    
    // Channel tracking
    UnsignedInt m_num2DSamples;
    UnsignedInt m_num3DSamples;
    UnsignedInt m_numStreams;

    // Volume settings for each audio category
    Real m_soundVolume;
    Real m_sound3DVolume;
    Real m_speechVolume;
    Real m_musicVolume;

    // Audio state
    Bool m_audioOn;
    Bool m_sound3DOn;
    Bool m_speechOn;
    Bool m_musicOn;
    Bool m_ambientPaused;

    // Provider/device tracking
    UnsignedInt m_selectedProvider;
    UnsignedInt m_selectedSpeakerType;

    // Helper functions
    void initializeChannels();
    void updateMasterVolume();
};
