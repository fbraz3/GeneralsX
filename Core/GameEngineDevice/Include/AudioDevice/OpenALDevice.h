/*
 * Phase 07: OpenALDevice - OpenAL Audio Backend Implementation
 * 
 * Complete OpenAL-soft implementation of the AudioDevice interface
 * Provides cross-platform 3D audio using OpenAL
 * 
 * Supported features:
 * - Music streaming (background tracks)
 * - Sound effects (2D positional)
 * - 3D audio with distance attenuation and panning
 * - Voice/speech playback
 * - Real-time position updates for moving sounds
 * - Doppler effects for fast-moving sources
 * 
 * Thread Safety: NOT thread-safe - must be called from single thread
 * Typical Usage:
 *   AudioDevice* device = new OpenALDevice();
 *   device->init();
 *   device->setListenerPosition(camera.x, camera.y, camera.z);
 *   int sound = device->play3DSound("unit_voice.wav", unit.x, unit.y, unit.z);
 *   device->update(deltaTime); // Called every frame
 *   device->shutdown();
 *   delete device;
 */

#ifndef OPENALDEVICE_H
#define OPENALDEVICE_H

#include "AudioDevice.h"
#include <map>
#include <string>

// Forward declarations for OpenAL types
typedef struct ALCdevice ALCdevice;
typedef struct ALCcontext ALCcontext;
typedef unsigned int ALuint;

namespace GeneralsX {
namespace Audio {

/**
 * OpenAL-based audio device implementation
 */
class OpenALDevice : public AudioDevice {
public:
    /**
     * Construct OpenAL audio device
     * Does not initialize OpenAL - call init() separately
     */
    OpenALDevice();

    /**
     * Destructor - automatically calls shutdown if not already called
     */
    virtual ~OpenALDevice();

    // ============================================================
    // Lifecycle Methods (AudioDevice interface)
    // ============================================================
    
    virtual bool init();
    virtual void shutdown();
    virtual void update(float deltaTime);
    virtual bool isInitialized() const;
    virtual const char* getLastError() const;

    // ============================================================
    // Music Playback (AudioDevice interface)
    // ============================================================
    
    virtual int playMusic(const char* filename, bool loop);
    virtual void stopMusic(int musicId);
    virtual bool isMusicPlaying(int musicId) const;
    virtual void setMusicVolume(int musicId, float volume);
    virtual float getMusicVolume(int musicId) const;
    virtual void pauseMusic(int musicId);
    virtual void resumeMusic(int musicId);

    // ============================================================
    // Sound Effects (AudioDevice interface)
    // ============================================================
    
    virtual int playSound(const char* filename, float volume = 1.0f);
    virtual void stopSound(int soundId);
    virtual bool isSoundPlaying(int soundId) const;
    virtual void setSoundVolume(int soundId, float volume);
    virtual void setSoundPitch(int soundId, float pitch);

    // ============================================================
    // 3D Positional Audio (AudioDevice interface)
    // ============================================================
    
    virtual int play3DSound(const char* filename,
                           float worldX, float worldY, float worldZ,
                           float volume = 1.0f, float pitch = 1.0f);
    virtual void set3DSoundPosition(int soundId,
                                   float worldX, float worldY, float worldZ);
    virtual void set3DSoundVelocity(int soundId,
                                   float velX, float velY, float velZ);
    virtual void stop3DSound(int soundId);
    virtual void get3DSoundPosition(int soundId,
                                   float& outX, float& outY, float& outZ) const;
    virtual float get3DSoundDistance(int soundId) const;

    // ============================================================
    // Listener Positioning (AudioDevice interface)
    // ============================================================
    
    virtual void setListenerPosition(float worldX, float worldY, float worldZ);
    virtual void getListenerPosition(float& outX, float& outY, float& outZ) const;
    virtual void setListenerOrientation(float forwardX, float forwardY, float forwardZ,
                                       float upX, float upY, float upZ);
    virtual void setListenerVelocity(float velX, float velY, float velZ);
    virtual void getListenerOrientation(float& outForwardX, float& outForwardY, float& outForwardZ,
                                       float& outUpX, float& outUpY, float& outUpZ) const;

    // ============================================================
    // Voice/Speech (AudioDevice interface)
    // ============================================================
    
    virtual int playVoice(const char* filename, float volume = 1.0f);
    virtual void stopVoice(int voiceId);
    virtual bool isVoicePlaying(int voiceId) const;
    virtual void setVoiceVolume(int voiceId, float volume);
    virtual void pauseVoice(int voiceId);
    virtual void resumeVoice(int voiceId);

    // ============================================================
    // Global Audio Control (AudioDevice interface)
    // ============================================================
    
    virtual void setMasterVolume(float volume);
    virtual float getMasterVolume() const;
    virtual void pauseAll();
    virtual void resumeAll();
    virtual void stopAll();

    // ============================================================
    // Audio File Management (AudioDevice interface)
    // ============================================================
    
    virtual bool preloadAudio(const char* filename);
    virtual void unloadAudio(const char* filename);
    virtual bool isAudioPreloaded(const char* filename) const;
    virtual void clearAudioCache();
    virtual size_t getAudioCacheSize() const;

private:
    // ============================================================
    // Internal Helper Structures
    // ============================================================

    /**
     * Sound source information
     */
    struct SoundSource {
        ALuint alSource;              // OpenAL source handle
        ALuint alBuffer;              // OpenAL buffer (audio data)
        std::string filename;         // Source filename
        float posX, posY, posZ;       // 3D position
        float velX, velY, velZ;       // Velocity
        float volume;                 // Base volume
        float pitch;                  // Playback pitch
        bool is3D;                    // Is 3D sound?
        bool isPaused;                // Is paused?
    };

    /**
     * Cached audio buffer
     */
    struct AudioBuffer {
        ALuint alBuffer;              // OpenAL buffer
        size_t dataSize;              // Size in bytes
        int refCount;                 // Number of sources using this buffer
    };

    // ============================================================
    // Internal Methods
    // ============================================================

    /**
     * Load audio file and get or create OpenAL buffer
     * Caches buffers to avoid redundant loading
     * @param filename Path to audio file
     * @return OpenAL buffer handle, 0 on failure
     */
    ALuint loadAudioBuffer(const char* filename);

    /**
     * Create new OpenAL source with loaded buffer
     * @param filename Path to audio file
     * @param volume Base volume [0.0, 1.0]
     * @param pitch Playback pitch [0.5, 2.0]
     * @param is3D Is 3D source?
     * @return Handle to new sound source, INVALID_AUDIO_HANDLE on failure
     */
    int createSoundSource(const char* filename, float volume, float pitch, bool is3D);

    /**
     * Get sound source from handle
     * @param handle Sound handle
     * @return Pointer to SoundSource, nullptr if not found
     */
    SoundSource* getSoundSource(int handle);
    const SoundSource* getSoundSource(int handle) const;

    /**
     * Allocate new sound handle
     * @return New unique handle
     */
    int allocateSoundHandle();

    /**
     * Load WAV file into OpenAL buffer
     * @param filename Path to WAV file
     * @param outBuffer [out] OpenAL buffer handle
     * @param outFrequency [out] Sample rate
     * @param outChannels [out] Channel count (1=mono, 2=stereo)
     * @return true if successful
     */
    bool loadWAVFile(const char* filename, ALuint& outBuffer, 
                     int& outFrequency, int& outChannels);

    /**
     * Log error for debugging
     * @param context Description of what failed
     * @param alError OpenAL error code
     */
    void logALError(const char* context, int alError);

    // ============================================================
    // Member Variables
    // ============================================================

    bool m_initialized;                                   // Is OpenAL initialized?
    std::string m_lastError;                              // Last error message
    ALCdevice* m_alDevice;                                // OpenAL device
    ALCcontext* m_alContext;                              // OpenAL context

    std::map<int, SoundSource> m_soundSources;  // Active sound sources
    std::map<std::string, AudioBuffer> m_audioCache; // Cached buffers
    int m_nextHandle;                                     // Next sound handle ID
    int m_currentMusicId;                                 // Current music handle
    int m_currentVoiceId;                                 // Current voice handle

    float m_masterVolume;                                 // Master volume [0.0, 1.0]
    bool m_allPaused;                                     // Are all sounds paused?

    // Listener state
    float m_listenerX, m_listenerY, m_listenerZ;         // Position
    float m_listenerVelX, m_listenerVelY, m_listenerVelZ; // Velocity
    float m_listenerForwardX, m_listenerForwardY, m_listenerForwardZ; // Forward
    float m_listenerUpX, m_listenerUpY, m_listenerUpZ;   // Up
};

} // namespace Audio
} // namespace GeneralsX

#endif // OPENALDEVICE_H
