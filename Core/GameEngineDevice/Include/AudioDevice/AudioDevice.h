/*
 * Phase 07: AudioDevice - Cross-Platform Audio Abstraction
 * 
 * This abstraction layer provides a unified interface for all audio operations,
 * replacing the legacy Miles Audio library with OpenAL (phase 1) and enabling
 * cross-platform support.
 * 
 * Supported operations:
 * - Music playback (background tracks, loops)
 * - Sound effects (2D audio, variable volume/pitch)
 * - 3D positional audio (unit voices, spatial SFX)
 * - Voice/speech playback (campaign dialogue, narration)
 * - Listener positioning (camera/observer location)
 * - Audio effects (gain/volume, pitch, attenuation)
 * 
 * Thread Safety: Safe for single-threaded access from main game thread
 * Platform Support: Windows (OpenAL), macOS (native + OpenAL), Linux (OpenAL)
 */

#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

namespace GeneralsX {
namespace Audio {

/// Invalid sound/music handle returned by operations
const int INVALID_AUDIO_HANDLE = -1;

/**
 * @brief Abstract audio device interface for all game audio operations
 * 
 * All audio subsystems in the game route through this interface.
 * Implementations provide actual backend (OpenAL, Miles, native, etc.)
 */
class AudioDevice {
public:
    virtual ~AudioDevice() {}  // Empty destructor for pure virtual base class

    // ============================================================
    // Lifecycle
    // ============================================================

    /**
     * Initialize the audio device and create audio context
     * @return true if initialization successful
     */
    virtual bool init() = 0;

    /**
     * Shutdown audio device, release all resources
     * Safe to call multiple times
     */
    virtual void shutdown() = 0;

    /**
     * Update audio system (process pending commands, clean up stopped sounds)
     * Should be called every frame from main game loop
     * @param deltaTime Frame delta time in seconds
     */
    virtual void update(float deltaTime) = 0;

    /**
     * Check if device is currently initialized and ready
     * @return true if device is initialized
     */
    virtual bool isInitialized() const = 0;

    /**
     * Get last error message (for debugging)
     * @return Error string, empty if no error
     */
    virtual const char* getLastError() const = 0;

    // ============================================================
    // Music Playback (Single-Stream, High Priority)
    // ============================================================

    /**
     * Start playing background music
     * Only one music track plays at a time; starting new music stops previous
     * @param filename Path to music file (relative to game data directory)
     * @param loop Whether to loop the track
     * @return Handle for music track (use to stop/control), INVALID_AUDIO_HANDLE on failure
     */
    virtual int playMusic(const char* filename, bool loop) = 0;

    /**
     * Stop music playback
     * @param musicId Handle returned by playMusic()
     */
    virtual void stopMusic(int musicId) = 0;

    /**
     * Get whether music is currently playing
     * @param musicId Handle returned by playMusic()
     * @return true if music is playing
     */
    virtual bool isMusicPlaying(int musicId) const = 0;

    /**
     * Set music volume (0.0 = silent, 1.0 = full volume)
     * @param musicId Handle returned by playMusic()
     * @param volume Volume level [0.0, 1.0]
     */
    virtual void setMusicVolume(int musicId, float volume) = 0;

    /**
     * Get current music volume
     * @param musicId Handle returned by playMusic()
     * @return Current volume [0.0, 1.0]
     */
    virtual float getMusicVolume(int musicId) const = 0;

    /**
     * Pause music (can resume from same position)
     * @param musicId Handle returned by playMusic()
     */
    virtual void pauseMusic(int musicId) = 0;

    /**
     * Resume paused music
     * @param musicId Handle returned by playMusic()
     */
    virtual void resumeMusic(int musicId) = 0;

    // ============================================================
    // Sound Effects (2D Audio, Fire and Forget)
    // ============================================================

    /**
     * Play a 2D sound effect (no spatial positioning)
     * Fire-and-forget: sound plays until completion
     * @param filename Path to sound file
     * @param volume Volume level [0.0, 1.0]
     * @return Handle for sound (use to stop early if needed), INVALID_AUDIO_HANDLE on failure
     */
    virtual int playSound(const char* filename, float volume = 1.0f) = 0;

    /**
     * Stop sound effect playback
     * @param soundId Handle returned by playSound()
     */
    virtual void stopSound(int soundId) = 0;

    /**
     * Check if sound is still playing
     * @param soundId Handle returned by playSound()
     * @return true if sound is playing
     */
    virtual bool isSoundPlaying(int soundId) const = 0;

    /**
     * Set sound volume
     * @param soundId Handle returned by playSound()
     * @param volume Volume level [0.0, 1.0]
     */
    virtual void setSoundVolume(int soundId, float volume) = 0;

    /**
     * Set sound pitch/speed
     * @param soundId Handle returned by playSound()
     * @param pitch Playback speed (1.0 = normal, 0.5 = half speed, 2.0 = double speed)
     */
    virtual void setSoundPitch(int soundId, float pitch) = 0;

    // ============================================================
    // 3D Positional Audio (Unit Voices, Spatial SFX)
    // Critical for RTS gameplay audio immersion
    // ============================================================

    /**
     * Play a 3D sound at specific world position
     * Sound will attenuate with distance and pan based on listener position
     * @param filename Path to sound file
     * @param worldX X coordinate in game world
     * @param worldY Y coordinate in game world
     * @param worldZ Z coordinate in game world (altitude)
     * @param volume Base volume [0.0, 1.0] before distance attenuation
     * @param pitch Playback speed [0.5, 2.0] (1.0 = normal)
     * @return Handle for 3D sound, INVALID_AUDIO_HANDLE on failure
     */
    virtual int play3DSound(const char* filename,
                           float worldX, float worldY, float worldZ,
                           float volume = 1.0f, float pitch = 1.0f) = 0;

    /**
     * Update 3D sound position (for moving sounds like unit voices)
     * @param soundId Handle returned by play3DSound()
     * @param worldX New X coordinate
     * @param worldY New Y coordinate
     * @param worldZ New Z coordinate
     */
    virtual void set3DSoundPosition(int soundId,
                                   float worldX, float worldY, float worldZ) = 0;

    /**
     * Update 3D sound velocity (for Doppler effect)
     * @param soundId Handle returned by play3DSound()
     * @param velX Velocity in X axis
     * @param velY Velocity in Y axis
     * @param velZ Velocity in Z axis
     */
    virtual void set3DSoundVelocity(int soundId,
                                   float velX, float velY, float velZ) = 0;

    /**
     * Stop 3D sound playback
     * @param soundId Handle returned by play3DSound()
     */
    virtual void stop3DSound(int soundId) = 0;

    /**
     * Get 3D sound position
     * @param soundId Handle returned by play3DSound()
     * @param outX [out] Current X position
     * @param outY [out] Current Y position
     * @param outZ [out] Current Z position
     */
    virtual void get3DSoundPosition(int soundId,
                                   float& outX, float& outY, float& outZ) const = 0;

    /**
     * Get distance from listener to 3D sound
     * @param soundId Handle returned by play3DSound()
     * @return Distance in game units, -1 if sound not found
     */
    virtual float get3DSoundDistance(int soundId) const = 0;

    // ============================================================
    // Listener Positioning (Camera/Observer Location)
    // Updates where "ear" is in 3D space for spatial audio processing
    // ============================================================

    /**
     * Set listener (camera/observer) position in world
     * All 3D sounds are positioned relative to the listener
     * @param worldX X coordinate of listener
     * @param worldY Y coordinate of listener
     * @param worldZ Z coordinate of listener (altitude)
     */
    virtual void setListenerPosition(float worldX, float worldY, float worldZ) = 0;

    /**
     * Get current listener position
     * @param outX [out] Current X position
     * @param outY [out] Current Y position
     * @param outZ [out] Current Z position
     */
    virtual void getListenerPosition(float& outX, float& outY, float& outZ) const = 0;

    /**
     * Set listener (camera) orientation
     * Determines which direction is "forward" for spatial audio
     * @param forwardX Forward vector X component
     * @param forwardY Forward vector Y component
     * @param forwardZ Forward vector Z component
     * @param upX Up vector X component
     * @param upY Up vector Y component
     * @param upZ Up vector Z component
     */
    virtual void setListenerOrientation(float forwardX, float forwardY, float forwardZ,
                                       float upX, float upY, float upZ) = 0;

    /**
     * Set listener velocity (for Doppler effect)
     * @param velX Velocity in X axis
     * @param velY Velocity in Y axis
     * @param velZ Velocity in Z axis
     */
    virtual void setListenerVelocity(float velX, float velY, float velZ) = 0;

    /**
     * Get current listener orientation
     * @param outForwardX [out] Forward vector X
     * @param outForwardY [out] Forward vector Y
     * @param outForwardZ [out] Forward vector Z
     * @param outUpX [out] Up vector X
     * @param outUpY [out] Up vector Y
     * @param outUpZ [out] Up vector Z
     */
    virtual void getListenerOrientation(float& outForwardX, float& outForwardY, float& outForwardZ,
                                       float& outUpX, float& outUpY, float& outUpZ) const = 0;

    // ============================================================
    // Voice/Speech (Campaign Dialogue, Narration)
    // Single voice channel with high priority
    // ============================================================

    /**
     * Play voice/speech audio (campaign dialogue, unit acknowledgments)
     * Only one voice plays at a time; starting new voice stops previous
     * @param filename Path to voice file
     * @param volume Volume level [0.0, 1.0]
     * @return Handle for voice playback, INVALID_AUDIO_HANDLE on failure
     */
    virtual int playVoice(const char* filename, float volume = 1.0f) = 0;

    /**
     * Stop voice playback
     * @param voiceId Handle returned by playVoice()
     */
    virtual void stopVoice(int voiceId) = 0;

    /**
     * Check if voice is currently playing
     * @param voiceId Handle returned by playVoice()
     * @return true if voice is playing
     */
    virtual bool isVoicePlaying(int voiceId) const = 0;

    /**
     * Set voice volume
     * @param voiceId Handle returned by playVoice()
     * @param volume Volume level [0.0, 1.0]
     */
    virtual void setVoiceVolume(int voiceId, float volume) = 0;

    /**
     * Pause voice playback
     * @param voiceId Handle returned by playVoice()
     */
    virtual void pauseVoice(int voiceId) = 0;

    /**
     * Resume paused voice
     * @param voiceId Handle returned by playVoice()
     */
    virtual void resumeVoice(int voiceId) = 0;

    // ============================================================
    // Global Audio Control
    // ============================================================

    /**
     * Set master volume (applies to all sounds)
     * @param volume Master volume [0.0, 1.0]
     */
    virtual void setMasterVolume(float volume) = 0;

    /**
     * Get master volume
     * @return Current master volume [0.0, 1.0]
     */
    virtual float getMasterVolume() const = 0;

    /**
     * Pause all audio (music, sounds, voice)
     */
    virtual void pauseAll() = 0;

    /**
     * Resume all paused audio
     */
    virtual void resumeAll() = 0;

    /**
     * Stop all audio (music, sounds, voice) - full silence
     */
    virtual void stopAll() = 0;

    // ============================================================
    // Audio File Management
    // ============================================================

    /**
     * Preload audio file into memory cache
     * Improves playback latency for frequently-used sounds
     * @param filename Path to audio file
     * @return true if preload successful
     */
    virtual bool preloadAudio(const char* filename) = 0;

    /**
     * Unload preloaded audio file from cache
     * @param filename Path to audio file
     */
    virtual void unloadAudio(const char* filename) = 0;

    /**
     * Check if audio file is preloaded
     * @param filename Path to audio file
     * @return true if file is in cache
     */
    virtual bool isAudioPreloaded(const char* filename) const = 0;

    /**
     * Clear all preloaded audio from cache
     * Frees all cached audio data
     */
    virtual void clearAudioCache() = 0;

    /**
     * Get current memory usage of audio cache
     * @return Cache memory usage in bytes
     */
    virtual size_t getAudioCacheSize() const = 0;
};

} // namespace Audio
} // namespace GeneralsX

#endif // AUDIODEVICE_H
