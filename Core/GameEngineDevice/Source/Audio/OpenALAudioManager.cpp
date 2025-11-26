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

#include "Audio/OpenALAudioManager.h"
#include "Audio/OpenALAudioDevice.h"
#include "Common/AudioEventInfo.h"
#include "Common/AudioRequest.h"
#include "Common/AudioAffect.h"
#include "Common/AsciiString.h"
#include "Common/Debug.h"
#include "GameClient/DebugDisplay.h"
#include "Lib/BaseType.h"

/**
 * OpenALAudioManager Constructor
 * 
 * Initializes the OpenAL audio manager with default settings.
 */
OpenALAudioManager::OpenALAudioManager() :
    m_audioDevice(nullptr),
    m_num2DSamples(32),
    m_num3DSamples(32),
    m_numStreams(4),
    m_soundVolume(1.0f),
    m_sound3DVolume(1.0f),
    m_speechVolume(1.0f),
    m_musicVolume(1.0f),
    m_audioOn(true),
    m_sound3DOn(true),
    m_speechOn(true),
    m_musicOn(true),
    m_ambientPaused(false),
    m_selectedProvider(0),
    m_selectedSpeakerType(0)
{
    DEBUG_LOG(("OpenALAudioManager::OpenALAudioManager - Initializing OpenAL audio manager\n"));
}

/**
 * OpenALAudioManager Destructor
 * 
 * Cleans up OpenAL device and resources.
 */
OpenALAudioManager::~OpenALAudioManager()
{
    DEBUG_LOG(("OpenALAudioManager::~OpenALAudioManager - Destroying OpenAL audio manager\n"));
    closeDevice();
    
    if (m_audioDevice != nullptr) {
        OpenALAudioDevice_Destroy(m_audioDevice);
        m_audioDevice = nullptr;
    }
}

/**
 * Initialize the audio subsystem
 * 
 * Called during game initialization to set up OpenAL context and device.
 */
void OpenALAudioManager::init()
{
    DEBUG_LOG(("OpenALAudioManager::init - Initializing audio subsystem\n"));
    
    // Call parent class initialization
    AudioManager::init();
    
    openDevice();
    initializeChannels();
}

/**
 * Post-process loading phase
 * 
 * Called after all game assets are loaded.
 */
void OpenALAudioManager::postProcessLoad()
{
    DEBUG_LOG(("OpenALAudioManager::postProcessLoad - Audio subsystem post-load\n"));
    AudioManager::postProcessLoad();
}

/**
 * Reset the audio system to default state
 */
void OpenALAudioManager::reset()
{
    DEBUG_LOG(("OpenALAudioManager::reset - Resetting audio system\n"));
    
    if (m_audioDevice != nullptr) {
        OpenALAudioDevice_StopAllSources(m_audioDevice);
    }
    
    AudioManager::reset();
}

/**
 * Update the audio system (called each frame)
 * 
 * Handles playback updates, listener position updates, and cleanup.
 */
void OpenALAudioManager::update()
{
    // Process frame updates
    if (m_audioDevice != nullptr) {
        OpenALAudioDevice_Update(m_audioDevice, 0.016f); // ~60 FPS
    }
    
    AudioManager::update();
}

/**
 * Stop audio for a specific category
 * 
 * @param which The category of audio to stop (AudioAffect enum)
 */
void OpenALAudioManager::stopAudio(AudioAffect which)
{
    DEBUG_LOG(("OpenALAudioManager::stopAudio - Stopping audio category: %d\n", which));
    
    if (m_audioDevice == nullptr) {
        return;
    }
    
    switch (which) {
        case AudioAffect_Sound:
            m_audioOn = false;
            OpenALAudioDevice_SetChannelVolume(m_audioDevice, OPENAL_CHANNEL_SFX, 0.0f);
            break;
        case AudioAffect_Sound3D:
            m_sound3DOn = false;
            break;
        case AudioAffect_Speech:
            m_speechOn = false;
            OpenALAudioDevice_SetChannelVolume(m_audioDevice, OPENAL_CHANNEL_VOICE, 0.0f);
            break;
        case AudioAffect_Music:
            m_musicOn = false;
            OpenALAudioDevice_SetChannelVolume(m_audioDevice, OPENAL_CHANNEL_MUSIC, 0.0f);
            break;
        default:
            break;
    }
}

/**
 * Pause audio for a specific category
 * 
 * @param which The category of audio to pause
 */
void OpenALAudioManager::pauseAudio(AudioAffect which)
{
    DEBUG_LOG(("OpenALAudioManager::pauseAudio - Pausing audio category: %d\n", which));
    
    if (m_audioDevice == nullptr) {
        return;
    }
    
    OpenALAudioDevice_PauseAllSources(m_audioDevice);
}

/**
 * Resume audio for a specific category
 * 
 * @param which The category of audio to resume
 */
void OpenALAudioManager::resumeAudio(AudioAffect which)
{
    DEBUG_LOG(("OpenALAudioManager::resumeAudio - Resuming audio category: %d\n", which));
    
    if (m_audioDevice == nullptr) {
        return;
    }
    
    // Re-enable the category
    switch (which) {
        case AudioAffect_Sound:
            m_audioOn = true;
            OpenALAudioDevice_SetChannelVolume(m_audioDevice, OPENAL_CHANNEL_SFX, m_soundVolume);
            break;
        case AudioAffect_Sound3D:
            m_sound3DOn = true;
            break;
        case AudioAffect_Speech:
            m_speechOn = true;
            OpenALAudioDevice_SetChannelVolume(m_audioDevice, OPENAL_CHANNEL_VOICE, m_speechVolume);
            break;
        case AudioAffect_Music:
            m_musicOn = true;
            OpenALAudioDevice_SetChannelVolume(m_audioDevice, OPENAL_CHANNEL_MUSIC, m_musicVolume);
            break;
        default:
            break;
    }
    
    OpenALAudioDevice_ResumeAllSources(m_audioDevice);
}

/**
 * Pause ambient sounds
 * 
 * @param shouldPause True to pause, false to resume
 */
void OpenALAudioManager::pauseAmbient(Bool shouldPause)
{
    DEBUG_LOG(("OpenALAudioManager::pauseAmbient - %s ambient audio\n", 
               shouldPause ? "Pausing" : "Resuming"));
    
    m_ambientPaused = shouldPause;
    
    if (m_audioDevice == nullptr) {
        return;
    }
    
    if (shouldPause) {
        OpenALAudioDevice_PauseAllSources(m_audioDevice);
    } else {
        OpenALAudioDevice_ResumeAllSources(m_audioDevice);
    }
}

/**
 * Handle loss of audio focus (e.g., app loses focus)
 */
void OpenALAudioManager::loseFocus(void)
{
    DEBUG_LOG(("OpenALAudioManager::loseFocus - Audio focus lost\n"));
    // Pause all audio when losing focus
    pauseAudio(AudioAffect_Sound);
}

/**
 * Handle regaining audio focus
 */
void OpenALAudioManager::regainFocus(void)
{
    DEBUG_LOG(("OpenALAudioManager::regainFocus - Audio focus regained\n"));
    // Resume audio when focus regained
    resumeAudio(AudioAffect_Sound);
}

/**
 * Kill an audio event immediately without fading
 * 
 * @param audioEvent The audio handle to kill
 */
void OpenALAudioManager::killAudioEventImmediately(AudioHandle audioEvent)
{
    DEBUG_LOG(("OpenALAudioManager::killAudioEventImmediately - Killing audio event: %u\n", audioEvent));
    
    if (m_audioDevice == nullptr) {
        return;
    }
    
    // Stop the corresponding OpenAL source
    OpenALAudioDevice_Stop(m_audioDevice, (AudioSourceHandle)audioEvent);
}

/**
 * Go to next music track
 */
void OpenALAudioManager::nextMusicTrack(void)
{
    DEBUG_LOG(("OpenALAudioManager::nextMusicTrack - Moving to next music track\n"));
    // TODO: Implement music track advancement
}

/**
 * Go to previous music track
 */
void OpenALAudioManager::prevMusicTrack(void)
{
    DEBUG_LOG(("OpenALAudioManager::prevMusicTrack - Moving to previous music track\n"));
    // TODO: Implement music track rewind
}

/**
 * Check if music is currently playing
 * 
 * @return True if music is playing, false otherwise
 */
Bool OpenALAudioManager::isMusicPlaying(void) const
{
    if (m_audioDevice == nullptr) {
        return false;
    }
    
    // Query music channel state
    OpenAL_AudioState state = OpenALAudioDevice_GetSourceState(m_audioDevice, OPENAL_HANDLE_MUSIC_MIN);
    return (state == OPENAL_AUDIO_STATE_PLAYING);
}

/**
 * Check if a music track has completed a specified number of times
 * 
 * @param trackName Name of the track to check
 * @param numberOfTimes Number of times to check for completion
 * @return True if track completed the specified number of times
 */
Bool OpenALAudioManager::hasMusicTrackCompleted(const AsciiString& trackName, Int numberOfTimes) const
{
    // TODO: Track completion counter
    return false;
}

/**
 * Get the current music track name
 * 
 * @return The name of the currently playing track
 */
AsciiString OpenALAudioManager::getMusicTrackName(void) const
{
    // TODO: Implement track name tracking
    return AsciiString::TheEmptyString;
}

/**
 * Open the audio device and initialize OpenAL context
 */
void OpenALAudioManager::openDevice(void)
{
    DEBUG_LOG(("OpenALAudioManager::openDevice - Opening OpenAL device\n"));
    
    if (m_audioDevice != nullptr) {
        return; // Already open
    }
    
    m_audioDevice = OpenALAudioDevice_Create();
    if (m_audioDevice != nullptr) {
        OpenALAudioDevice_Initialize(m_audioDevice);
        DEBUG_LOG(("OpenALAudioManager::openDevice - OpenAL device initialized successfully\n"));
    } else {
        DEBUG_LOG(("OpenALAudioManager::openDevice - Failed to create OpenAL device\n"));
    }
}

/**
 * Close the audio device and release OpenAL context
 */
void OpenALAudioManager::closeDevice(void)
{
    DEBUG_LOG(("OpenALAudioManager::closeDevice - Closing OpenAL device\n"));
    
    if (m_audioDevice != nullptr) {
        OpenALAudioDevice_StopAllSources(m_audioDevice);
        OpenALAudioDevice_Shutdown(m_audioDevice);
    }
}

/**
 * Get the raw audio device pointer
 * 
 * @return Pointer to the OpenAL device
 */
void *OpenALAudioManager::getDevice(void)
{
    return m_audioDevice;
}

/**
 * Handle audio completion notifications from OpenAL
 * 
 * @param audioCompleted The handle of the completed audio
 * @param flags Completion flags
 */
void OpenALAudioManager::notifyOfAudioCompletion(UnsignedInt audioCompleted, UnsignedInt flags)
{
    DEBUG_LOG(("OpenALAudioManager::notifyOfAudioCompletion - Audio completion: %u\n", audioCompleted));
    // TODO: Handle completion callbacks
}

/**
 * Get the number of available audio providers (devices)
 * 
 * @return Number of available audio devices
 */
UnsignedInt OpenALAudioManager::getProviderCount(void) const
{
    // OpenAL typically has at least one provider (default device)
    return 1;
}

/**
 * Get the name of an audio provider
 * 
 * @param providerNum Index of the provider
 * @return Name of the provider
 */
AsciiString OpenALAudioManager::getProviderName(UnsignedInt providerNum) const
{
    if (providerNum == 0) {
        return AsciiString("OpenAL Default Device");
    }
    return AsciiString::TheEmptyString;
}

/**
 * Get the index of a provider by name
 * 
 * @param providerName Name of the provider to find
 * @return Index of the provider, or invalid index if not found
 */
UnsignedInt OpenALAudioManager::getProviderIndex(AsciiString providerName) const
{
    if (providerName.compareNoCase(AsciiString("OpenAL Default Device")) == 0) {
        return 0;
    }
    return PROVIDER_ERROR;
}

/**
 * Select an audio provider
 * 
 * @param providerNdx Index of the provider to select
 */
void OpenALAudioManager::selectProvider(UnsignedInt providerNdx)
{
    DEBUG_LOG(("OpenALAudioManager::selectProvider - Selecting provider: %u\n", providerNdx));
    m_selectedProvider = providerNdx;
}

/**
 * Unselect the current audio provider
 */
void OpenALAudioManager::unselectProvider(void)
{
    DEBUG_LOG(("OpenALAudioManager::unselectProvider - Unselecting provider\n"));
    m_selectedProvider = PROVIDER_ERROR;
}

/**
 * Get the currently selected provider
 * 
 * @return Index of the selected provider
 */
UnsignedInt OpenALAudioManager::getSelectedProvider(void) const
{
    return m_selectedProvider;
}

/**
 * Set the speaker type/configuration
 * 
 * @param speakerType The speaker type to set
 */
void OpenALAudioManager::setSpeakerType(UnsignedInt speakerType)
{
    DEBUG_LOG(("OpenALAudioManager::setSpeakerType - Setting speaker type: %u\n", speakerType));
    m_selectedSpeakerType = speakerType;
}

/**
 * Get the current speaker type/configuration
 * 
 * @return The current speaker type
 */
UnsignedInt OpenALAudioManager::getSpeakerType(void)
{
    return m_selectedSpeakerType;
}

/**
 * Get the number of available 2D (mono) audio channels
 * 
 * @return Number of 2D channels
 */
UnsignedInt OpenALAudioManager::getNum2DSamples(void) const
{
    return m_num2DSamples;
}

/**
 * Get the number of available 3D (positional) audio channels
 * 
 * @return Number of 3D channels
 */
UnsignedInt OpenALAudioManager::getNum3DSamples(void) const
{
    return m_num3DSamples;
}

/**
 * Get the number of available audio stream channels
 * 
 * @return Number of stream channels
 */
UnsignedInt OpenALAudioManager::getNumStreams(void) const
{
    return m_numStreams;
}

/**
 * Check if a new audio event would violate channel limits
 * 
 * @param event The event to check
 * @return True if playing this event would exceed limits
 */
Bool OpenALAudioManager::doesViolateLimit(AudioEventRTS *event) const
{
    // TODO: Implement limit checking logic
    return false;
}

/**
 * Check if there's a lower priority audio event currently playing
 * 
 * @param event The event to check against
 * @return True if a lower priority event is playing
 */
Bool OpenALAudioManager::isPlayingLowerPriority(AudioEventRTS *event) const
{
    // TODO: Implement priority checking
    return false;
}

/**
 * Check if an audio event is already playing
 * 
 * @param event The event to check
 * @return True if the event is currently playing
 */
Bool OpenALAudioManager::isPlayingAlready(AudioEventRTS *event) const
{
    // TODO: Implement duplicate checking
    return false;
}

/**
 * Check if an object is currently playing voice audio
 * 
 * @param objID The object ID to check
 * @return True if the object is playing voice audio
 */
Bool OpenALAudioManager::isObjectPlayingVoice(UnsignedInt objID) const
{
    // TODO: Implement object voice tracking
    return false;
}

/**
 * Adjust the volume of playing audio by event name
 * 
 * @param eventName Name of the event to adjust
 * @param newVolume New volume level
 */
void OpenALAudioManager::adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume)
{
    DEBUG_LOG(("OpenALAudioManager::adjustVolumeOfPlayingAudio - Event: %s, Volume: %.2f\n", 
               eventName.str(), newVolume));
    // TODO: Implement volume adjustment for playing audio
}

/**
 * Remove playing audio by event name
 * 
 * @param eventName Name of the event to remove
 */
void OpenALAudioManager::removePlayingAudio(AsciiString eventName)
{
    DEBUG_LOG(("OpenALAudioManager::removePlayingAudio - Event: %s\n", eventName.str()));
    // TODO: Implement removal of playing audio
}

/**
 * Remove all disabled audio events
 */
void OpenALAudioManager::removeAllDisabledAudio()
{
    DEBUG_LOG(("OpenALAudioManager::removeAllDisabledAudio - Removing disabled audio\n"));
    // TODO: Implement cleanup of disabled audio
}

/**
 * Check if there are 3D sensitive streams currently playing
 * 
 * @return True if 3D streams are playing
 */
Bool OpenALAudioManager::has3DSensitiveStreamsPlaying(void) const
{
    // Check if any 3D positional audio is actively playing
    // For now, return false as a conservative estimate
    return false;
}

/**
 * Get a handle for Bink video playback
 * 
 * @return Handle for Bink video (nullptr if not supported)
 */
void *OpenALAudioManager::getHandleForBink(void)
{
    DEBUG_LOG(("OpenALAudioManager::getHandleForBink - Bink video support requested\n"));
    // Bink support is Windows-only and deprecated
    // Return nullptr to indicate not supported
    return nullptr;
}

/**
 * Release the Bink video handle
 */
void OpenALAudioManager::releaseHandleForBink(void)
{
    DEBUG_LOG(("OpenALAudioManager::releaseHandleForBink - Releasing Bink handle\n"));
    // No-op: Bink is not supported on this platform
}

/**
 * Force play an audio event (used for load screens)
 * 
 * @param eventToPlay The audio event to force play
 */
void OpenALAudioManager::friend_forcePlayAudioEventRTS(const AudioEventRTS* eventToPlay)
{
    DEBUG_LOG(("OpenALAudioManager::friend_forcePlayAudioEventRTS - Force playing audio event\n"));
    // TODO: Implement forced playback for load screens
}

/**
 * Get the length of an audio file in milliseconds
 * 
 * @param strToLoad Filename of the audio file
 * @return Duration in milliseconds
 */
Real OpenALAudioManager::getFileLengthMS(AsciiString strToLoad) const
{
    DEBUG_LOG(("OpenALAudioManager::getFileLengthMS - Getting file length for: %s\n", strToLoad.str()));
    // TODO: Query OpenAL for audio file duration
    return 0.0f;
}

/**
 * Close any samples using a specific file
 * 
 * @param fileToClose Pointer to the file handle to close
 */
void OpenALAudioManager::closeAnySamplesUsingFile(const void *fileToClose)
{
    DEBUG_LOG(("OpenALAudioManager::closeAnySamplesUsingFile - Closing samples for file\n"));
    // TODO: Track and close samples tied to this file
}

/**
 * Set the preferred audio provider/device
 * 
 * @param providerNdx Name of the provider to prefer
 */
void OpenALAudioManager::setPreferredProvider(AsciiString providerNdx)
{
    DEBUG_LOG(("OpenALAudioManager::setPreferredProvider - Setting preferred provider: %s\n", providerNdx.str()));
    m_selectedProvider = getProviderIndex(providerNdx);
}

/**
 * Set the preferred speaker configuration
 * 
 * @param speakerType Name of the speaker configuration to prefer
 */
void OpenALAudioManager::setPreferredSpeaker(AsciiString speakerType)
{
    DEBUG_LOG(("OpenALAudioManager::setPreferredSpeaker - Setting preferred speaker: %s\n", speakerType.str()));
    m_selectedSpeakerType = translateSpeakerTypeToUnsignedInt(speakerType);
}

/**
 * Set the device listener position
 * 
 * Updates the OpenAL listener position based on the current listener coordinates.
 */
void OpenALAudioManager::setDeviceListenerPosition(void)
{
    // This is called by parent class to update the 3D listener position
    // The actual position is set via setListenerPosition() from AudioManager
    if (m_audioDevice != nullptr) {
        // Update OpenAL listener position from parent class m_listenerPosition
        // Note: Parent class manages m_listenerPosition and m_listenerOrientation
        const Coord3D *pos = getListenerPosition();
        if (pos != nullptr) {
            OpenAL_Vector3 oalPos = { pos->x, pos->y, pos->z };
            OpenALAudioDevice_SetListenerPosition(m_audioDevice, oalPos);
        }
    }
}

/**
 * Initialize audio channel configuration
 * 
 * Sets up default channel counts and volume levels.
 */
void OpenALAudioManager::initializeChannels()
{
    DEBUG_LOG(("OpenALAudioManager::initializeChannels - Initializing audio channels\n"));
    
    if (m_audioDevice == nullptr) {
        return;
    }
    
    // Set default volumes for each channel
    OpenALAudioDevice_SetChannelVolume(m_audioDevice, OPENAL_CHANNEL_MUSIC, m_musicVolume);
    OpenALAudioDevice_SetChannelVolume(m_audioDevice, OPENAL_CHANNEL_SFX, m_soundVolume);
    OpenALAudioDevice_SetChannelVolume(m_audioDevice, OPENAL_CHANNEL_VOICE, m_speechVolume);
    OpenALAudioDevice_SetChannelVolume(m_audioDevice, OPENAL_CHANNEL_AMBIENT, m_soundVolume);
}

/**
 * Update master volume based on category volumes
 */
void OpenALAudioManager::updateMasterVolume()
{
    if (m_audioDevice == nullptr) {
        return;
    }
    
    // Calculate overall master volume as average of all categories
    float masterVolume = (m_soundVolume + m_sound3DVolume + m_speechVolume + m_musicVolume) / 4.0f;
    OpenALAudioDevice_SetMasterVolume(m_audioDevice, masterVolume);
}

#if defined(RTS_DEBUG)
/**
 * Display audio debug information
 * 
 * @param dd Debug display interface
 * @param userData User data pointer
 * @param fp File pointer for logging (optional)
 */
void OpenALAudioManager::audioDebugDisplay(DebugDisplayInterface *dd, void *userData, FILE *fp)
{
    if (dd) {
        dd->printf("OpenAL Audio Manager Status\n");
        dd->printf("---------------------------\n");
        dd->printf("Device: %s\n", m_audioDevice ? "Initialized" : "Not Initialized");
        dd->printf("Provider: %s\n", getProviderName(m_selectedProvider).str());
        dd->printf("2D Channels: %u/%u\n", 0, m_num2DSamples);
        dd->printf("3D Channels: %u/%u\n", 0, m_num3DSamples);
        dd->printf("Stream Channels: %u/%u\n", 0, m_numStreams);
        dd->printf("Sound Volume: %.2f\n", m_soundVolume);
        dd->printf("3D Sound Volume: %.2f\n", m_sound3DVolume);
        dd->printf("Speech Volume: %.2f\n", m_speechVolume);
        dd->printf("Music Volume: %.2f\n", m_musicVolume);
    }
    
    if (fp) {
        fprintf(fp, "OpenAL Audio Manager Status\n");
        fprintf(fp, "---------------------------\n");
        fprintf(fp, "Device: %s\n", m_audioDevice ? "Initialized" : "Not Initialized");
        fprintf(fp, "Provider: %s\n", getProviderName(m_selectedProvider).str());
        fprintf(fp, "2D Channels: %u/%u\n", 0, m_num2DSamples);
        fprintf(fp, "3D Channels: %u/%u\n", 0, m_num3DSamples);
        fprintf(fp, "Stream Channels: %u/%u\n", 0, m_numStreams);
        fprintf(fp, "Sound Volume: %.2f\n", m_soundVolume);
        fprintf(fp, "3D Sound Volume: %.2f\n", m_sound3DVolume);
        fprintf(fp, "Speech Volume: %.2f\n", m_speechVolume);
        fprintf(fp, "Music Volume: %.2f\n", m_musicVolume);
    }
}
#endif
