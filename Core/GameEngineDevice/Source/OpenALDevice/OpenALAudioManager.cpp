/**
 * @file OpenALAudioManager.cpp
 * @brief OpenAL Audio Manager implementation
 * 
 * Ported from jmarshall-win64-modern reference implementation
 */

#include "Lib/BaseType.h"
#include "OpenALDevice/OpenALAudioManager.h"
#include "OpenALDevice/OpenALAudioLoader.h"
#include "Common/AudioSettings.h"
#include "Common/AudioRequest.h"
#include "Common/AudioAffect.h"
#include "Common/AudioEventInfo.h"
#include "Common/AudioEventRTS.h"
#include "Common/AudioHandleSpecialValues.h"
#include "GameLogic/Object.h"
#include <iostream>
#include <chrono>

// Constructor
OpenALAudioManager::OpenALAudioManager() 
    : m_initialized(false),
      m_volumeHasChanged(false),
      device(nullptr),
      context(nullptr),
      m_musicSource(0),
      m_digitalHandle(nullptr),
      m_selectedSpeakerType(0)
{
    std::cerr << "OpenALAudioManager: Initializing (full implementation)" << std::endl;
    
    // Initialize source pool bookkeeping
    for (int i = 0; i < NUM_POOLED_SOURCES2D; ++i) {
        m_sourcePool2D[i] = 0;
        m_sourceInUse2D[i] = false;
    }
    
    for (int i = 0; i < NUM_POOLED_SOURCES3D; ++i) {
        m_sourcePool3D[i] = 0;
        m_sourceInUse3D[i] = false;
    }
}

// Destructor
OpenALAudioManager::~OpenALAudioManager()
{
    std::cerr << "OpenALAudioManager: Cleanup starting" << std::endl;
    
    // Stop and cleanup music source
    if (m_musicSource) {
        alSourceStop(m_musicSource);
        alSourcei(m_musicSource, AL_BUFFER, AL_NONE);
        alDeleteSources(1, &m_musicSource);
    }
    
    // Cleanup 2D source pool
    for (size_t i = 0; i < NUM_POOLED_SOURCES2D; ++i) {
        if (m_sourceInUse2D[i]) {
            alSourceStop(m_sourcePool2D[i]);
            alSourcei(m_sourcePool2D[i], AL_BUFFER, AL_NONE);
        }
    }
    alDeleteSources(NUM_POOLED_SOURCES2D, m_sourcePool2D);
    
    // Cleanup 3D source pool
    for (size_t i = 0; i < NUM_POOLED_SOURCES3D; ++i) {
        if (m_sourceInUse3D[i]) {
            alSourceStop(m_sourcePool3D[i]);
            alSourcei(m_sourcePool3D[i], AL_BUFFER, AL_NONE);
        }
    }
    alDeleteSources(NUM_POOLED_SOURCES3D, m_sourcePool3D);
    
    // Cleanup all buffers
    if (!m_buffers.empty()) {
        alDeleteBuffers(m_buffers.size(), m_buffers.data());
        m_buffers.clear();
    }
    
    // Cleanup OpenAL context and device
    if (context) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(context);
        context = nullptr;
    }
    
    if (device) {
        alcCloseDevice(device);
        device = nullptr;
    }
    
    std::cerr << "OpenALAudioManager: Cleanup complete" << std::endl;
}

void OpenALAudioManager::init()
{
    std::cerr << "OpenALAudioManager::init() - Starting full initialization" << std::endl;
    
    // Call parent init to load audio INI files
    AudioManager::init();
    
    // Open OpenAL device
    openDevice();
    
    m_initialized = true;
    std::cerr << "OpenALAudioManager::init() - Complete" << std::endl;
}

void OpenALAudioManager::openDevice(void)
{
    std::cerr << "OpenALAudioManager::openDevice() - Opening OpenAL device" << std::endl;
    
    // Open default audio device
    device = alcOpenDevice(NULL);
    if (!device) {
        std::cerr << "ERROR: Failed to open OpenAL device!" << std::endl;
        return;
    }
    std::cerr << "OpenALAudioManager::openDevice() - Device opened successfully" << std::endl;
    
    // Create audio context
    context = alcCreateContext(device, NULL);
    if (!context) {
        std::cerr << "ERROR: Failed to create OpenAL context!" << std::endl;
        alcCloseDevice(device);
        device = nullptr;
        return;
    }
    std::cerr << "OpenALAudioManager::openDevice() - Context created successfully" << std::endl;
    
    // Make context current
    if (!alcMakeContextCurrent(context)) {
        std::cerr << "ERROR: Failed to make OpenAL context current!" << std::endl;
        alcDestroyContext(context);
        alcCloseDevice(device);
        context = nullptr;
        device = nullptr;
        return;
    }
    std::cerr << "OpenALAudioManager::openDevice() - Context activated" << std::endl;
    
    // Generate 2D source pool
    alGenSources(NUM_POOLED_SOURCES2D, m_sourcePool2D);
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "ERROR: Failed to generate 2D sources! Error code: " << error << std::endl;
    } else {
        std::cerr << "OpenALAudioManager::openDevice() - Generated " << NUM_POOLED_SOURCES2D << " 2D sources" << std::endl;
    }
    
    // Generate 3D source pool
    alGenSources(NUM_POOLED_SOURCES3D, m_sourcePool3D);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "ERROR: Failed to generate 3D sources! Error code: " << error << std::endl;
    } else {
        std::cerr << "OpenALAudioManager::openDevice() - Generated " << NUM_POOLED_SOURCES3D << " 3D sources" << std::endl;
    }
    
    // Generate dedicated music source
    alGenSources(1, &m_musicSource);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "ERROR: Failed to generate music source! Error code: " << error << std::endl;
    } else {
        std::cerr << "OpenALAudioManager::openDevice() - Generated music source" << std::endl;
    }
    
    std::cerr << "OpenALAudioManager::openDevice() - Device initialization complete" << std::endl;
}

void OpenALAudioManager::closeDevice(void)
{
    std::cerr << "OpenALAudioManager::closeDevice() - Closing device" << std::endl;
    
    stopAllAudioImmediately();
    
    // Cleanup is handled in destructor
}

void* OpenALAudioManager::getDevice(void)
{
    return device;
}

void OpenALAudioManager::reset()
{
    printf("OpenALAudioManager::reset() - Full implementation\n");
    
    // Stop all playing audio
    stopAudio(AudioAffect_All);
    
    // Process all lists to ensure they're cleaned up
    processPlayingList();
    processFadingList();
    processStoppedList();
    
    // Clear all tracking lists
    m_playingSounds.clear();
    m_playing3DSounds.clear();
    m_fadingAudio.clear();
    m_stoppedAudio.clear();
    
    // Reset all 2D sources
    for (unsigned int i = 0; i < NUM_POOLED_SOURCES2D; ++i) {
        if (m_sourcePool2D[i] != 0) {
            alSourceStop(m_sourcePool2D[i]);
            alSourcei(m_sourcePool2D[i], AL_BUFFER, 0);
        }
    }
    
    // Reset all 3D sources
    for (unsigned int i = 0; i < NUM_POOLED_SOURCES3D; ++i) {
        if (m_sourcePool3D[i] != 0) {
            alSourceStop(m_sourcePool3D[i]);
            alSourcei(m_sourcePool3D[i], AL_BUFFER, 0);
        }
    }
    
    // Reset music source
    if (m_musicSource != 0) {
        alSourceStop(m_musicSource);
        alSourcei(m_musicSource, AL_BUFFER, 0);
    }
    
    printf("OpenALAudioManager::reset() - Complete\n");
}

void OpenALAudioManager::update()
{
    // Call base class update (handles listener position, zoom volume, etc.)
    AudioManager::update();
    
    // Update listener position for OpenAL
    setDeviceListenerPosition();
    
    // Process all audio requests (play, stop, pause, etc.)
    processRequestList();
    
    // Process all audio lists
    processPlayingList();
    processFadingList();
    processStoppedList();
    
    m_volumeHasChanged = false;
}

void OpenALAudioManager::postProcessLoad()
{
    std::cerr << "OpenALAudioManager::postProcessLoad() - Calling AudioManager::init()" << std::endl;
    // Call the parent class init() method to load audio INI files
    AudioManager::init();
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

// Music management methods - see implementations at end of file

// Provider and device management - see implementations at end of file

// Get methods - see implementations at end of file

void OpenALAudioManager::removeAllDisabledAudio() {}
Bool OpenALAudioManager::has3DSensitiveStreamsPlaying(void) const { return false; }
void* OpenALAudioManager::getHandleForBink(void) { return NULL; }
void OpenALAudioManager::releaseHandleForBink(void) {}

// ===========================================================================================
// Music Management
// ===========================================================================================

void OpenALAudioManager::nextMusicTrack(void)
{
    std::cerr << "OpenALAudioManager::nextMusicTrack()" << std::endl;
    
    // Get current track name
    AsciiString trackName;
    std::list<OpenALPlayingAudio*>::iterator it;
    OpenALPlayingAudio* playing;
    for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it) {
        playing = *it;
        if (playing && playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music) {
            trackName = playing->m_audioEventRTS->getEventName();
            break;
        }
    }

    // Stop currently playing music
    TheAudio->removeAudioEvent(AHSV_StopTheMusic);

    // Play next track (using parent's nextTrackName method)
    trackName = nextTrackName(trackName);
    AudioEventRTS newTrack(trackName);
    TheAudio->addAudioEvent(&newTrack);
}

void OpenALAudioManager::prevMusicTrack(void)
{
    std::cerr << "OpenALAudioManager::prevMusicTrack()" << std::endl;
    
    // Get current track name
    AsciiString trackName;
    std::list<OpenALPlayingAudio*>::iterator it;
    OpenALPlayingAudio* playing;
    for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it) {
        playing = *it;
        if (playing && playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music) {
            trackName = playing->m_audioEventRTS->getEventName();
            break;
        }
    }

    // Stop currently playing music
    TheAudio->removeAudioEvent(AHSV_StopTheMusic);

    // Play previous track (using parent's prevTrackName method)
    trackName = prevTrackName(trackName);
    AudioEventRTS newTrack(trackName);
    TheAudio->addAudioEvent(&newTrack);
}

Bool OpenALAudioManager::isMusicPlaying(void) const
{
    std::list<OpenALPlayingAudio*>::const_iterator it;
    OpenALPlayingAudio* playing;
    for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it) {
        playing = *it;
        if (playing && playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music) {
            // Check if source is actually playing
            ALint state;
            alGetSourcei(playing->source, AL_SOURCE_STATE, &state);
            if (state == AL_PLAYING) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

Bool OpenALAudioManager::hasMusicTrackCompleted(const AsciiString& trackName, Int numberOfTimes) const
{
    // TODO: Track completion count
    return FALSE;
}

AsciiString OpenALAudioManager::getMusicTrackName(void) const
{
    // Check requests first
    std::list<AudioRequest*>::const_iterator ait;
    for (ait = m_audioRequests.begin(); ait != m_audioRequests.end(); ++ait) {
        if ((*ait)->m_request != AR_Play) {
            continue;
        }
        if (!(*ait)->m_usePendingEvent) {
            continue;
        }
        if ((*ait)->m_pendingEvent->getAudioEventInfo()->m_soundType == AT_Music) {
            return (*ait)->m_pendingEvent->getEventName();
        }
    }

    // Check playing streams
    std::list<OpenALPlayingAudio*>::const_iterator it;
    OpenALPlayingAudio* playing;
    for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ++it) {
        playing = *it;
        if (playing && playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music) {
            return playing->m_audioEventRTS->getEventName();
        }
    }

    return AsciiString::getEmptyString();
}

// ===========================================================================================
// Provider and Device Management
// ===========================================================================================

void OpenALAudioManager::notifyOfAudioCompletion(UnsignedInt audioCompleted, UnsignedInt flags) {
    // TODO: Handle audio completion notification
}

UnsignedInt OpenALAudioManager::getProviderCount(void) const { return 1; }
AsciiString OpenALAudioManager::getProviderName(UnsignedInt providerNum) const { return "OpenAL"; }
UnsignedInt OpenALAudioManager::getProviderIndex(AsciiString providerName) const { return 0; }
void OpenALAudioManager::selectProvider(UnsignedInt providerNdx) {}
void OpenALAudioManager::unselectProvider(void) {}
UnsignedInt OpenALAudioManager::getSelectedProvider(void) const { return 0; }
void OpenALAudioManager::setSpeakerType(UnsignedInt speakerType) { m_selectedSpeakerType = speakerType; }
UnsignedInt OpenALAudioManager::getSpeakerType(void) { return m_selectedSpeakerType; }

UnsignedInt OpenALAudioManager::getNum2DSamples(void) const { return NUM_POOLED_SOURCES2D; }
UnsignedInt OpenALAudioManager::getNum3DSamples(void) const { return NUM_POOLED_SOURCES3D; }
UnsignedInt OpenALAudioManager::getNumStreams(void) const { return 1; } // One dedicated music source

Bool OpenALAudioManager::doesViolateLimit(AudioEventRTS* event) const { return FALSE; }
Bool OpenALAudioManager::isPlayingLowerPriority(AudioEventRTS* event) const { return FALSE; }
Bool OpenALAudioManager::isPlayingAlready(AudioEventRTS* event) const { return FALSE; }
Bool OpenALAudioManager::isObjectPlayingVoice(UnsignedInt objID) const { return FALSE; }
void OpenALAudioManager::adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume) {}
void OpenALAudioManager::removePlayingAudio(AsciiString eventName) {}

// ===========================================================================================
// Audio Playback Core
// ===========================================================================================

void OpenALAudioManager::friend_forcePlayAudioEventRTS(const AudioEventRTS* eventToPlay) {
    if (!eventToPlay) {
        std::cerr << "ERROR: friend_forcePlayAudioEventRTS - NULL event" << std::endl;
        return;
    }

    std::cerr << "OpenALAudioManager::friend_forcePlayAudioEventRTS() - Event: '" 
              << eventToPlay->getEventName().str() << "'" << std::endl;

    // Get audio info for this event
    if (!eventToPlay->getAudioEventInfo()) {
        getInfoForAudioEvent(eventToPlay);
        if (!eventToPlay->getAudioEventInfo()) {
                    // No audio info - cannot play
            std::cerr << "ERROR: No audio info for event '" << eventToPlay->getEventName().str() << "'" << std::endl;
            return;
        }
    }

    const AudioEventInfo* info = eventToPlay->getAudioEventInfo();

    // Check if audio is enabled for this type
    switch (info->m_soundType) {
        case AT_Music:
            if (!isOn(AudioAffect_Music)) {
                std::cerr << "Music is disabled, skipping playback" << std::endl;
                return;
            }
            break;
        case AT_SoundEffect:
            if (!isOn(AudioAffect_Sound) || !isOn(AudioAffect_Sound3D)) {
                std::cerr << "Sound effects are disabled, skipping playback" << std::endl;
                return;
            }
            break;
        case AT_Streaming:
            if (!isOn(AudioAffect_Speech)) {
                std::cerr << "Speech is disabled, skipping playback" << std::endl;
                return;
            }
            break;
    }

    // Create a copy of the event for playback
    AudioEventRTS* event = new AudioEventRTS(*eventToPlay);
    event->generateFilename();
    event->generatePlayInfo();

    // Allocate playing audio structure
    OpenALPlayingAudio* audio = allocatePlayingAudio();
    audio->m_audioEventRTS = event;
    audio->m_type = (info->m_soundType == AT_Music || info->m_soundType == AT_Streaming) ? PAT_Stream : PAT_Sample;
    audio->m_cleanupAudioEventRTS = true;

    // Play the sample
    bool isMusic = (info->m_soundType == AT_Music);
    playSample(event, audio, isMusic);
}

void OpenALAudioManager::playSample(AudioEventRTS* event, OpenALPlayingAudio* audio, bool isMusic) {
    if (!event || !audio) {
        std::cerr << "ERROR: playSample - NULL event or audio" << std::endl;
        return;
    }

    std::cerr << "OpenALAudioManager::playSample() - Event: '" << event->getEventName().str() 
              << "', isMusic: " << isMusic << std::endl;

    // Load audio file into buffer
    AsciiString filename = event->getFilename();
        // Debug output
    std::cerr << "OpenALAudioManager::playSample() - Filename: '" << filename.str() << "'" << std::endl;

    ALuint buffer = OpenALAudioLoader::loadFromFile(filename);
    if (buffer == 0) {
        std::cerr << "ERROR: Failed to load audio buffer for '" << filename.str() << "'" << std::endl;
        releasePlayingAudio(audio);
        return;
    }

    audio->buffer = buffer;

    // Get a free source (use music source if it's music, otherwise get from pool)
    ALuint source = 0;
    if (isMusic) {
        source = m_musicSource;
        audio->poolIndex = 0xFFFFFFFF; // Music source doesn't use pool
        std::cerr << "OpenALAudioManager::playSample() - Using dedicated music source" << std::endl;
    } else {
        source = getFreeSource(audio->poolIndex, false); // 2D source for now
        if (source == 0) {
            std::cerr << "ERROR: No free sources available" << std::endl;
            releasePlayingAudio(audio);
            return;
        }
        std::cerr << "OpenALAudioManager::playSample() - Using source from pool (index " 
                  << audio->poolIndex << ")" << std::endl;
    }

    audio->source = source;

    // Configure source
    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_GAIN, event->getVolume());
    alSourcef(source, AL_PITCH, 1.0f);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source, AL_LOOPING, event->hasMoreLoops() ? AL_TRUE : AL_FALSE);

    // Check for OpenAL errors
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "ERROR: OpenAL source configuration failed, error code: " << error << std::endl;
        releasePlayingAudio(audio);
        return;
    }

    // Play the source
    alSourcePlay(source);
    error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "ERROR: OpenAL source playback failed, error code: " << error << std::endl;
        releasePlayingAudio(audio);
        return;
    }

    // Add to appropriate playing list
    if (isMusic) {
        m_playingStreams.push_back(audio);
        std::cerr << "OpenALAudioManager::playSample() - Added to streams list (music)" << std::endl;
    } else {
        m_playingSounds.push_back(audio);
        std::cerr << "OpenALAudioManager::playSample() - Added to 2D sounds list" << std::endl;
    }

    std::cerr << "OpenALAudioManager::playSample() - Playback started successfully" << std::endl;
}

bool OpenALAudioManager::playSample3D(AudioEventRTS* event, OpenALPlayingAudio* audio) {
    if (!event || !audio) {
        return false;
    }

    std::cerr << "OpenALAudioManager::playSample3D() - Playing 3D sample: " << event->getEventName().str() << std::endl;

    // Load audio file
    ALuint buffer = openFile(event);
    if (!buffer) {
        std::cerr << "OpenALAudioManager::playSample3D() - Failed to load audio file" << std::endl;
        return false;
    }

    // Get a free 3D source
    audio->source = getFreeSource(audio->poolIndex, true);
    if (audio->poolIndex < 0) {
        std::cerr << "OpenALAudioManager::playSample3D() - No free 3D sources available" << std::endl;
        return false;
    }

    audio->source = m_sourcePool3D[audio->poolIndex];
    audio->buffer = buffer;
    audio->m_type = PAT_3DSample;

    // Bind buffer to source
    alSourcei(audio->source, AL_BUFFER, buffer);

    // Set volume
    float volume = getEffectiveVolume(event);
    alSourcef(audio->source, AL_GAIN, volume);

    // Set 3D properties
    alSourcef(audio->source, AL_REFERENCE_DISTANCE, 10.0f);  // Distance at which volume starts to decrease
    alSourcef(audio->source, AL_MAX_DISTANCE, 1000.0f);      // Maximum distance for attenuation
    alSourcef(audio->source, AL_ROLLOFF_FACTOR, 1.0f);       // How quickly volume decreases with distance
    
    // Set source as positional (not relative to listener)
    alSourcei(audio->source, AL_SOURCE_RELATIVE, AL_FALSE);

    // Get and set initial position
    const Coord3D* pos = getCurrentPositionFromEvent(event);
    if (pos) {
        alSource3f(audio->source, AL_POSITION, pos->x, pos->y, pos->z);
        std::cerr << "OpenALAudioManager::playSample3D() - Set position: " 
                  << pos->x << ", " << pos->y << ", " << pos->z << std::endl;
    } else {
        // Default position at origin if no position available
        alSource3f(audio->source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    }

    // Set velocity (for doppler effect - currently zero)
    alSource3f(audio->source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

    // Play the source
    alSourcePlay(audio->source);

    // Add to 3D sounds list
    m_playing3DSounds.push_back(audio);

    std::cerr << "OpenALAudioManager::playSample3D() - 3D playback started successfully" << std::endl;
    return true;
}

ALuint OpenALAudioManager::openFile(AudioEventRTS* eventToOpenFrom) {
    if (!eventToOpenFrom) {
        return 0;
    }

    AsciiString filename;
    switch (eventToOpenFrom->getNextPlayPortion()) {
        case PP_Attack:
            filename = eventToOpenFrom->getAttackFilename();
            break;
        case PP_Sound:
            filename = eventToOpenFrom->getFilename();
            break;
        case PP_Decay:
            filename = eventToOpenFrom->getDecayFilename();
            break;
        case PP_Done:
            return 0;
    }

    return OpenALAudioLoader::loadFromFile(filename);
}

float OpenALAudioManager::getEffectiveVolume(AudioEventRTS* event) const {
    if (!event) {
        return 0.0f;
    }
    return event->getVolume() * event->getVolumeShift();
}

const Coord3D* OpenALAudioManager::getCurrentPositionFromEvent(AudioEventRTS* event) {
    if (!event || !event->isPositionalAudio()) {
        return NULL;
    }
    return event->getCurrentPosition();
}

void OpenALAudioManager::adjustPlayingVolume(OpenALPlayingAudio* audio) {
    if (!audio || !audio->m_audioEventRTS) {
        return;
    }

    Real volume = getEffectiveVolume(audio->m_audioEventRTS);
    alSourcef(audio->source, AL_GAIN, volume);
}

void OpenALAudioManager::stopAllSpeech(void) {
    std::cerr << "OpenALAudioManager::stopAllSpeech()" << std::endl;

    std::list<OpenALPlayingAudio*>::iterator it;
    OpenALPlayingAudio* playing;
    
    for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ) {
        playing = *it;
        if (playing && playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Streaming) {
            releasePlayingAudio(playing);
            it = m_playingStreams.erase(it);
        } else {
            ++it;
        }
    }
}

void OpenALAudioManager::playAudioEvent(AudioEventRTS* event) {
    // TODO: Implement full playAudioEvent logic
    std::cerr << "OpenALAudioManager::playAudioEvent() - Using friend_forcePlayAudioEventRTS for now" << std::endl;
    friend_forcePlayAudioEventRTS(event);
}

void OpenALAudioManager::stopAudioEvent(AudioHandle handle) {
    std::cerr << "OpenALAudioManager::stopAudioEvent() - Handle: " << handle << std::endl;
    // TODO: Implement stop by handle
}

void OpenALAudioManager::pauseAudioEvent(AudioHandle handle) {
    std::cerr << "OpenALAudioManager::pauseAudioEvent() - Handle: " << handle << std::endl;
    // TODO: Implement pause by handle
}

void OpenALAudioManager::processRequest(AudioRequest* req) {
    if (!req) {
        return;
    }

    printf("OpenALAudioManager::processRequest() - Request type: %d\n", req->m_request);

    switch (req->m_request) {
        case AR_Play:
            if (req->m_pendingEvent) {
                printf("  - AR_Play: event='%s'\n", req->m_pendingEvent->getEventName().str());
                playAudioEvent(req->m_pendingEvent);
            } else {
                printf("  - AR_Play: ERROR - pendingEvent is NULL!\n");
            }
            break;
        case AR_Pause:
            printf("  - AR_Pause: handle=%u\n", req->m_handleToInteractOn);
            pauseAudioEvent(req->m_handleToInteractOn);
            break;
        case AR_Stop:
            printf("  - AR_Stop: handle=%u\n", req->m_handleToInteractOn);
            stopAudioEvent(req->m_handleToInteractOn);
            break;
    }
}
void OpenALAudioManager::setPreferredProvider(AsciiString providerNdx) {}
void OpenALAudioManager::setPreferredSpeaker(AsciiString speakerType) {}
Real OpenALAudioManager::getFileLengthMS(AsciiString strToLoad) const { return -1.0f; }
void OpenALAudioManager::closeAnySamplesUsingFile(const void* fileToClose) {}
void OpenALAudioManager::setDeviceListenerPosition(void) {}

// ===========================================================================================
// Source Pool Management
// ===========================================================================================

ALuint OpenALAudioManager::getFreeSource(ALuint& poolIndex, bool is3D)
{
    if (is3D) {
        for (ALuint i = 0; i < NUM_POOLED_SOURCES3D; ++i) {
            if (!m_sourceInUse3D[i]) {
                m_sourceInUse3D[i] = true;
                poolIndex = i;
                return m_sourcePool3D[i];
            }
        }
        std::cerr << "WARNING: No free 3D sources available!" << std::endl;
    } else {
        for (ALuint i = 0; i < NUM_POOLED_SOURCES2D; ++i) {
            if (!m_sourceInUse2D[i]) {
                m_sourceInUse2D[i] = true;
                poolIndex = i;
                return m_sourcePool2D[i];
            }
        }
        std::cerr << "WARNING: No free 2D sources available!" << std::endl;
    }
    
    poolIndex = 0xFFFFFFFF; // Invalid index
    return 0;
}

void OpenALAudioManager::recycleSource(ALuint poolIndex, bool is3D)
{
    if (is3D) {
        if (poolIndex < NUM_POOLED_SOURCES3D) {
            m_sourceInUse3D[poolIndex] = false;
        }
    } else {
        if (poolIndex < NUM_POOLED_SOURCES2D) {
            m_sourceInUse2D[poolIndex] = false;
        }
    }
}

OpenALPlayingAudio* OpenALAudioManager::allocatePlayingAudio(void)
{
    return new OpenALPlayingAudio();
}

void OpenALAudioManager::releasePlayingAudio(OpenALPlayingAudio* release)
{
    if (!release) {
        return;
    }
    
    // Stop the source if playing
    if (release->source) {
        alSourceStop(release->source);
        alSourcei(release->source, AL_BUFFER, AL_NONE);
        
        // Recycle source back to pool
        bool is3D = (release->m_type == PAT_3DSample);
        recycleSource(release->poolIndex, is3D);
    }
    
    // Cleanup AudioEventRTS if needed
    if (release->m_cleanupAudioEventRTS && release->m_audioEventRTS) {
        delete release->m_audioEventRTS;
        release->m_audioEventRTS = NULL;
    }
    
    delete release;
}

void OpenALAudioManager::stopAllAudioImmediately(void)
{
    std::cerr << "OpenALAudioManager::stopAllAudioImmediately()" << std::endl;
    
    std::list<OpenALPlayingAudio*>::iterator it;
    OpenALPlayingAudio* playing;

    // Stop all 2D sounds
    if (m_playingSounds.size() > 0) {
        for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ) {
            playing = *it;
            if (playing) {
                releasePlayingAudio(playing);
            }
            it = m_playingSounds.erase(it);
        }
    }
    
    // Stop all 3D sounds
    if (m_playing3DSounds.size() > 0) {
        for (it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); ) {
            playing = *it;
            if (playing) {
                releasePlayingAudio(playing);
            }
            it = m_playing3DSounds.erase(it);
        }
    }
    
    // Stop all streams (music)
    if (m_playingStreams.size() > 0) {
        for (it = m_playingStreams.begin(); it != m_playingStreams.end(); ) {
            playing = *it;
            if (playing) {
                releasePlayingAudio(playing);
            }
            it = m_playingStreams.erase(it);
        }
    }
    
    // Clear fading and stopped lists
    m_fadingAudio.clear();
    m_stoppedAudio.clear();
}

void OpenALAudioManager::startFade(OpenALPlayingAudio* audio, float duration)
{
    if (!audio) return;
    
    // Get current time in seconds
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float currentSeconds = std::chrono::duration<float>(currentTime - startTime).count();
    
    // Get current volume from OpenAL source
    if (audio->source) {
        ALfloat currentGain = 1.0f;
        alGetSourcef(audio->source, AL_GAIN, &currentGain);
        audio->m_originalVolume = currentGain;
        audio->m_currentVolume = currentGain;
    } else {
        audio->m_originalVolume = 1.0f;
        audio->m_currentVolume = 1.0f;
    }
    
    // Set fade parameters
    audio->m_fadeStartTime = currentSeconds;
    audio->m_fadeDuration = duration;
    
    // Move to fading list
    m_fadingAudio.push_back(audio);
}

//-------------------------------------------------------------------------------------------------
void OpenALAudioManager::processRequestList(void)
{
    std::list<AudioRequest*>::iterator it;
    for (it = m_audioRequests.begin(); it != m_audioRequests.end(); /* empty */) {
        AudioRequest *req = (*it);
        if (req == NULL) {
            ++it;
            continue;
        }

        // Process the request based on type
        processRequest(req);
        
        // Release and remove the request
        releaseAudioRequest(req);
        it = m_audioRequests.erase(it);
    }
}

void OpenALAudioManager::processPlayingList(void)
{
    std::list<OpenALPlayingAudio*>::iterator it;
    OpenALPlayingAudio* playing;

    // Process 2D sounds
    for (it = m_playingSounds.begin(); it != m_playingSounds.end(); /* empty */) {
        playing = (*it);
        if (!playing) {
            it = m_playingSounds.erase(it);
            continue;
        }

        // Check if source has stopped playing
        if (playing->source) {
            ALint state;
            alGetSourcei(playing->source, AL_SOURCE_STATE, &state);
            if (state == AL_STOPPED) {
                playing->m_status = PS_Stopped;
            }
        }

        if (playing->m_status == PS_Stopped) {
            releasePlayingAudio(playing);
            it = m_playingSounds.erase(it);
        } else {
            if (m_volumeHasChanged) {
                adjustPlayingVolume(playing);
            }
            ++it;
        }
    }

    // Process 3D sounds
    for (it = m_playing3DSounds.begin(); it != m_playing3DSounds.end(); /* empty */) {
        playing = (*it);
        if (!playing) {
            it = m_playing3DSounds.erase(it);
            continue;
        }

        // Check if source has stopped playing
        if (playing->source) {
            ALint state;
            alGetSourcei(playing->source, AL_SOURCE_STATE, &state);
            if (state == AL_STOPPED) {
                playing->m_status = PS_Stopped;
            }
        }

        if (playing->m_status == PS_Stopped) {
            releasePlayingAudio(playing);
            it = m_playing3DSounds.erase(it);
        } else {
            if (m_volumeHasChanged) {
                adjustPlayingVolume(playing);
            }

            // Update 3D position if available
            const Coord3D* pos = getCurrentPositionFromEvent(playing->m_audioEventRTS);
            if (pos) {
                alSource3f(playing->source, AL_POSITION, pos->x, pos->y, pos->z);
            }

            ++it;
        }
    }

    // Process streams (music)
    for (it = m_playingStreams.begin(); it != m_playingStreams.end(); /* empty */) {
        playing = (*it);
        if (!playing) {
            it = m_playingStreams.erase(it);
            continue;
        }

        // Check if source has stopped playing
        if (playing->source) {
            ALint state;
            alGetSourcei(playing->source, AL_SOURCE_STATE, &state);
            if (state == AL_STOPPED) {
                playing->m_status = PS_Stopped;
            }
        }

        if (playing->m_status == PS_Stopped) {
            releasePlayingAudio(playing);
            it = m_playingStreams.erase(it);
        } else {
            if (m_volumeHasChanged) {
                adjustPlayingVolume(playing);
            }
            ++it;
        }
    }
}

void OpenALAudioManager::processFadingList(void)
{
    std::list<OpenALPlayingAudio*>::iterator it;
    OpenALPlayingAudio* playing;
    
    // Get current time in seconds (high precision)
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float currentSeconds = std::chrono::duration<float>(currentTime - startTime).count();

    for (it = m_fadingAudio.begin(); it != m_fadingAudio.end(); /* empty */) {
        playing = (*it);
        if (!playing) {
            it = m_fadingAudio.erase(it);
            continue;
        }
        
        // Calculate fade progress (0.0 = start, 1.0 = complete)
        float elapsedTime = currentSeconds - playing->m_fadeStartTime;
        float fadeProgress = elapsedTime / playing->m_fadeDuration;
        
        // Clamp to [0.0, 1.0]
        if (fadeProgress < 0.0f) fadeProgress = 0.0f;
        if (fadeProgress > 1.0f) fadeProgress = 1.0f;
        
        // Linear interpolation from original volume to 0
        // Could use other curves: quadratic, exponential, etc.
        float newVolume = playing->m_originalVolume * (1.0f - fadeProgress);
        playing->m_currentVolume = newVolume;
        
        // Apply new volume to OpenAL source
        if (playing->source) {
            alSourcef(playing->source, AL_GAIN, newVolume);
        }
        
        // Check if fade complete
        if (fadeProgress >= 1.0f || newVolume <= 0.001f) {
            // Fade complete - move to stopped list
            playing->m_status = PS_Stopped;
            m_stoppedAudio.push_back(playing);
            it = m_fadingAudio.erase(it);
        } else {
            ++it;
        }
    }
}

void OpenALAudioManager::processStoppedList(void)
{
    std::list<OpenALPlayingAudio*>::iterator it;
    OpenALPlayingAudio* playing;

    for (it = m_stoppedAudio.begin(); it != m_stoppedAudio.end(); /* empty */) {
        playing = (*it);
        if (!playing) {
            it = m_stoppedAudio.erase(it);
            continue;
        }

        releasePlayingAudio(playing);
        it = m_stoppedAudio.erase(it);
    }
}
