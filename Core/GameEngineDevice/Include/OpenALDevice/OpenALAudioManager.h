/**
 * @file OpenALAudioManager.h
 * @brief OpenAL Audio Manager stub for cross-platform audio support
 */

#pragma once

#ifndef __OPENALAUDIOMANAGER_H_
#define __OPENALAUDIOMANAGER_H_

#include "Common/GameAudio.h"

/**
 * @class OpenALAudioManager
 * @brief Stub implementation of AudioManager using OpenAL
 * 
 * This is a minimal stub to allow compilation. Full implementation
 * should be copied from reference repository when audio functionality is needed.
 */
class OpenALAudioManager : public AudioManager
{
public:
    OpenALAudioManager();
    virtual ~OpenALAudioManager() override;

    // AudioDevice interface
    virtual void init() override;
    virtual void reset() override;
    virtual void update() override;
    virtual void postProcessLoad() override;

    // AudioManager interface stubs (all pure virtual methods)
    virtual void stopAudio(AudioAffect which) override;
    virtual void pauseAudio(AudioAffect which) override;
    virtual void resumeAudio(AudioAffect which) override;
    virtual void pauseAmbient(Bool shouldPause) override;
    virtual void killAudioEventImmediately(AudioHandle audioEvent) override;
    virtual void nextMusicTrack(void) override;
    virtual void prevMusicTrack(void) override;
    virtual Bool isMusicPlaying(void) const override;
    virtual Bool hasMusicTrackCompleted(const AsciiString& trackName, Int numberOfTimes) const override;
    virtual AsciiString getMusicTrackName(void) const override;
    virtual void openDevice(void) override;
    virtual void closeDevice(void) override;
    virtual void* getDevice(void) override;
    virtual void notifyOfAudioCompletion(UnsignedInt audioCompleted, UnsignedInt flags) override;
    virtual UnsignedInt getProviderCount(void) const override;
    virtual AsciiString getProviderName(UnsignedInt providerNum) const override;
    virtual UnsignedInt getProviderIndex(AsciiString providerName) const override;
    virtual void selectProvider(UnsignedInt providerNdx) override;
    virtual void unselectProvider(void) override;
    virtual UnsignedInt getSelectedProvider(void) const override;
    virtual void setSpeakerType(UnsignedInt speakerType) override;
    virtual UnsignedInt getSpeakerType(void) override;
    virtual UnsignedInt getNum2DSamples(void) const override;
    virtual UnsignedInt getNum3DSamples(void) const override;
    virtual UnsignedInt getNumStreams(void) const override;
    virtual Bool doesViolateLimit(AudioEventRTS* event) const override;
    virtual Bool isPlayingLowerPriority(AudioEventRTS* event) const override;
    virtual Bool isPlayingAlready(AudioEventRTS* event) const override;
    virtual Bool isObjectPlayingVoice(UnsignedInt objID) const override;
    virtual void adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume) override;
    virtual void removePlayingAudio(AsciiString eventName) override;
    virtual void removeAllDisabledAudio() override;
    virtual Bool has3DSensitiveStreamsPlaying(void) const override;
    virtual void* getHandleForBink(void) override;
    virtual void releaseHandleForBink(void) override;
    virtual void friend_forcePlayAudioEventRTS(const AudioEventRTS* eventToPlay) override;
    virtual void setPreferredProvider(AsciiString providerNdx) override;
    virtual void setPreferredSpeaker(AsciiString speakerType) override;
    virtual Real getFileLengthMS(AsciiString strToLoad) const override;
    virtual void closeAnySamplesUsingFile(const void* fileToClose) override;
    virtual void setDeviceListenerPosition(void) override;

private:
    Bool m_initialized;
};

#endif // __OPENALAUDIOMANAGER_H_
