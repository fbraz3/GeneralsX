/**
 * @file OpenALAudioManager.h
 * @brief OpenAL Audio Manager for cross-platform audio support
 */

#pragma once

#ifndef __OPENALAUDIOMANAGER_H_
#define __OPENALAUDIOMANAGER_H_

#include "Common/GameAudio.h"
#include "Common/AsciiString.h"

#ifdef __APPLE__
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
#else
    #include <AL/al.h>
    #include <AL/alc.h>
#endif

#include <list>
#include <vector>

#undef stopAudioEvent

#define NUM_POOLED_SOURCES2D 32
#define NUM_POOLED_SOURCES3D 128

enum OpenALPlayingAudioType
{
    PAT_Sample,
    PAT_3DSample,
    PAT_Stream,
    PAT_INVALID
};

enum OpenALPlayingStatus
{
    PS_Playing,
    PS_Stopped,
    PS_Paused
};

enum OpenALPlayingWhich
{
    PW_Attack,
    PW_Sound,
    PW_Decay,
    PW_INVALID
};

struct OpenALPlayingAudio
{
    ALuint source;
    ALuint buffer;
    ALuint poolIndex;

    OpenALPlayingAudioType m_type;
    OpenALPlayingStatus m_status;
    AudioEventRTS* m_audioEventRTS;
    Bool m_requestStop;
    Bool m_cleanupAudioEventRTS;
    Int m_framesFaded;
    
    // Volume fading support
    float m_originalVolume;    // Volume before fading started
    float m_currentVolume;     // Current volume during fade
    float m_fadeStartTime;     // Time when fade started (in seconds)
    float m_fadeDuration;      // Total fade duration (in seconds)

    OpenALPlayingAudio() :
        source(0),
        buffer(0),
        poolIndex(0),
        m_type(PAT_INVALID),
        m_status(PS_Stopped),
        m_audioEventRTS(NULL),
        m_requestStop(false),
        m_cleanupAudioEventRTS(true),
        m_framesFaded(0),
        m_originalVolume(1.0f),
        m_currentVolume(1.0f),
        m_fadeStartTime(0.0f),
        m_fadeDuration(1.0f)  // Default 1 second fade
    {
    }
};

/**
 * @class OpenALAudioManager
 * @brief Full implementation of AudioManager using OpenAL
 * 
 * Ported from jmarshall-win64-modern reference implementation
 * for cross-platform audio support (macOS/Linux/Windows)
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

protected:
    void processRequest(AudioRequest* req);
    void playAudioEvent(AudioEventRTS* event);
    void stopAudioEvent(AudioHandle handle);
    void pauseAudioEvent(AudioHandle handle);

    OpenALPlayingAudio* allocatePlayingAudio(void);
    void releasePlayingAudio(OpenALPlayingAudio* release);
    
    void recycleSource(ALuint poolIndex, bool is3D);
    ALuint getFreeSource(ALuint& poolIndex, bool is3D);

    const Coord3D* getCurrentPositionFromEvent(AudioEventRTS* event);

    void playSample(AudioEventRTS* event, OpenALPlayingAudio* audio, bool isMusic = false);
    bool playSample3D(AudioEventRTS* event, OpenALPlayingAudio* audio);
    ALuint openFile(AudioEventRTS* eventToOpenFrom);
    float getEffectiveVolume(AudioEventRTS* event) const;

    void adjustPlayingVolume(OpenALPlayingAudio* audio);
    void stopAllSpeech(void);
    void stopAllAudioImmediately(void);
    
    // Fading helper
    void startFade(OpenALPlayingAudio* audio, float duration = 1.0f);
    
    // Processing methods
    void processPlayingList(void);
    void processFadingList(void);
    void processStoppedList(void);

private:
    Bool m_initialized;
    bool m_volumeHasChanged;
    
    // OpenAL device and context
    ALCdevice* device;
    ALCcontext* context;
    
    // Source pools for 2D and 3D audio
    ALuint m_sourcePool2D[NUM_POOLED_SOURCES2D];
    bool m_sourceInUse2D[NUM_POOLED_SOURCES2D];
    
    ALuint m_sourcePool3D[NUM_POOLED_SOURCES3D];
    bool m_sourceInUse3D[NUM_POOLED_SOURCES3D];
    
    // Dedicated music source
    ALuint m_musicSource;
    
    // Buffer management
    std::vector<ALuint> m_buffers;
    
    // Playing audio lists
    std::list<OpenALPlayingAudio*> m_playingSounds;     // 2D sounds
    std::list<OpenALPlayingAudio*> m_playing3DSounds;   // 3D positioned sounds
    std::list<OpenALPlayingAudio*> m_playingStreams;    // Music and streaming audio
    std::list<OpenALPlayingAudio*> m_fadingAudio;       // Audio in fade transition
    std::list<OpenALPlayingAudio*> m_stoppedAudio;      // Completed audio for cleanup
    
    // Settings
    void* m_digitalHandle;
    AsciiString m_pref3DProvider;
    AsciiString m_prefSpeaker;
    UnsignedInt m_selectedSpeakerType;
};

#endif // __OPENALAUDIOMANAGER_H_
