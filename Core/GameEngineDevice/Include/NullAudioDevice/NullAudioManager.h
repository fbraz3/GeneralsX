/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
*/

#pragma once

#include "Common/AsciiString.h"
#include "Common/GameAudio.h"

// GeneralsX @feature Codex 04/08/2026 Provide an explicit audio-disabled device contract for browser builds.
class NullAudioManager final : public AudioManager
{
public:
#if defined(RTS_DEBUG)
	void audioDebugDisplay(DebugDisplayInterface *, void *, FILE * = nullptr) override {}
#endif

	void stopAudio(AudioAffect) override {}
	void pauseAudio(AudioAffect) override {}
	void resumeAudio(AudioAffect) override {}
	void pauseAmbient(Bool) override {}
	void killAudioEventImmediately(AudioHandle) override {}

	AsciiString nextMusicTrack() override { return AsciiString(); }
	AsciiString prevMusicTrack() override { return AsciiString(); }
	Bool isMusicPlaying() const override { return false; }
	Bool hasMusicTrackCompleted(const AsciiString &, Int) const override { return false; }
	// GeneralsX @port Codex 04/08/2026 Treat intentional silence as a ready audio contract.
	Bool isMusicAlreadyLoaded() const override { return true; }

	void openDevice() override {}
	void closeDevice() override {}
	void *getDevice() override { return nullptr; }
	void notifyOfAudioCompletion(UnsignedInt, UnsignedInt) override {}

	UnsignedInt getProviderCount() const override { return 0; }
	AsciiString getProviderName(UnsignedInt) const override { return AsciiString(); }
	UnsignedInt getProviderIndex(AsciiString) const override { return PROVIDER_ERROR; }
	void selectProvider(UnsignedInt) override {}
	void unselectProvider() override {}
	UnsignedInt getSelectedProvider() const override { return PROVIDER_ERROR; }
	void setSpeakerType(UnsignedInt) override {}
	UnsignedInt getSpeakerType() override { return 0; }

	UnsignedInt getNum2DSamples() const override { return 0; }
	UnsignedInt getNum3DSamples() const override { return 0; }
	UnsignedInt getNumStreams() const override { return 0; }
	UnsignedInt getNumAvailable2DSamples() const override { return 0; }
	UnsignedInt getNumAvailable3DSamples() const override { return 0; }
	Bool doesViolateLimit(AudioEventRTS *) const override { return false; }
	Bool isPlayingLowerPriority(AudioEventRTS *) const override { return false; }
	Bool isPlayingAlready(AudioEventRTS *) const override { return false; }
	Bool isObjectPlayingVoice(UnsignedInt) const override { return false; }

	void adjustVolumeOfPlayingAudio(AsciiString, Real) override {}
	void removePlayingAudio(AsciiString) override {}
	void removeAllDisabledAudio() override {}
	Bool has3DSensitiveStreamsPlaying() const { return false; }

	void *getHandleForBink() override { return nullptr; }
	void releaseHandleForBink() override {}
	void friend_forcePlayAudioEventRTS(const AudioEventRTS *) override {}
	void setPreferredProvider(AsciiString) override {}
	void setPreferredSpeaker(AsciiString) override {}
	Real getFileLengthMS(AsciiString) const override { return 0.0f; }
	void closeAnySamplesUsingFile(const void *) override {}

protected:
	void setDeviceListenerPosition() override {}
};
