/**
 * @file OpenALAudioManager.cpp
 * @brief OpenAL Audio Manager stub implementation
 */

#include "Lib/BaseType.h"
#include "OpenALDevice/OpenALAudioManager.h"
#include <iostream>

OpenALAudioManager::OpenALAudioManager() : m_initialized(false)
{
    std::cerr << "OpenALAudioManager: Stub constructor" << std::endl;
}

OpenALAudioManager::~OpenALAudioManager()
{
    std::cerr << "OpenALAudioManager: Stub destructor" << std::endl;
}

void OpenALAudioManager::init()
{
    std::cerr << "OpenALAudioManager::init() - Stub implementation" << std::endl;
    m_initialized = true;
}

void OpenALAudioManager::reset()
{
    std::cerr << "OpenALAudioManager::reset() - Stub implementation" << std::endl;
}

void OpenALAudioManager::update()
{
    // Called frequently, so don't log
}

void OpenALAudioManager::postProcessLoad()
{
    std::cerr << "OpenALAudioManager::postProcessLoad() - Stub implementation" << std::endl;
}

void OpenALAudioManager::stopAudio(AudioAffect which)
{
    // Called frequently, so don't log unless debugging
}

void OpenALAudioManager::pauseAudio(AudioAffect which)
{
    // Called frequently, so don't log unless debugging
}

void OpenALAudioManager::resumeAudio(AudioAffect which)
{
    // Called frequently, so don't log unless debugging
}

void OpenALAudioManager::pauseAmbient(Bool shouldPause)
{
    // Called frequently, so don't log unless debugging
}

void OpenALAudioManager::killAudioEventImmediately(AudioHandle audioEvent)
{
    // Called frequently, so don't log unless debugging
}

void OpenALAudioManager::nextMusicTrack(void)
{
    std::cerr << "OpenALAudioManager::nextMusicTrack() - Stub implementation" << std::endl;
}

void OpenALAudioManager::prevMusicTrack(void)
{
    std::cerr << "OpenALAudioManager::prevMusicTrack() - Stub implementation" << std::endl;
}

Bool OpenALAudioManager::isMusicPlaying(void) const
{
    return false;
}

Bool OpenALAudioManager::hasMusicTrackCompleted(const AsciiString& trackName, Int numberOfTimes) const
{
    return false;
}

AsciiString OpenALAudioManager::getMusicTrackName(void) const
{
    return AsciiString::getEmptyString();
}

void OpenALAudioManager::openDevice(void) {}
void OpenALAudioManager::closeDevice(void) {}
void* OpenALAudioManager::getDevice(void) { return NULL; }
void OpenALAudioManager::notifyOfAudioCompletion(UnsignedInt audioCompleted, UnsignedInt flags) {}
UnsignedInt OpenALAudioManager::getProviderCount(void) const { return 0; }
AsciiString OpenALAudioManager::getProviderName(UnsignedInt providerNum) const { return "OpenAL"; }
UnsignedInt OpenALAudioManager::getProviderIndex(AsciiString providerName) const { return 0; }
void OpenALAudioManager::selectProvider(UnsignedInt providerNdx) {}
void OpenALAudioManager::unselectProvider(void) {}
UnsignedInt OpenALAudioManager::getSelectedProvider(void) const { return 0; }
void OpenALAudioManager::setSpeakerType(UnsignedInt speakerType) {}
UnsignedInt OpenALAudioManager::getSpeakerType(void) { return 0; }
UnsignedInt OpenALAudioManager::getNum2DSamples(void) const { return 0; }
UnsignedInt OpenALAudioManager::getNum3DSamples(void) const { return 0; }
UnsignedInt OpenALAudioManager::getNumStreams(void) const { return 0; }
Bool OpenALAudioManager::doesViolateLimit(AudioEventRTS* event) const { return false; }
Bool OpenALAudioManager::isPlayingLowerPriority(AudioEventRTS* event) const { return false; }
Bool OpenALAudioManager::isPlayingAlready(AudioEventRTS* event) const { return false; }
Bool OpenALAudioManager::isObjectPlayingVoice(UnsignedInt objID) const { return false; }
void OpenALAudioManager::adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume) {}
void OpenALAudioManager::removePlayingAudio(AsciiString eventName) {}
void OpenALAudioManager::removeAllDisabledAudio() {}
Bool OpenALAudioManager::has3DSensitiveStreamsPlaying(void) const { return false; }
void* OpenALAudioManager::getHandleForBink(void) { return NULL; }
void OpenALAudioManager::releaseHandleForBink(void) {}
void OpenALAudioManager::friend_forcePlayAudioEventRTS(const AudioEventRTS* eventToPlay) {}
void OpenALAudioManager::setPreferredProvider(AsciiString providerNdx) {}
void OpenALAudioManager::setPreferredSpeaker(AsciiString speakerType) {}
Real OpenALAudioManager::getFileLengthMS(AsciiString strToLoad) const { return -1.0f; }
void OpenALAudioManager::closeAnySamplesUsingFile(const void* fileToClose) {}
void OpenALAudioManager::setDeviceListenerPosition(void) {}
