/**
 * @file OpenALAudioLoader.h
 * @brief Utility class to load audio files (WAV/MP3) into OpenAL buffers
 * 
 * Ported from jmarshall-win64-modern reference implementation
 */

#pragma once

#ifndef __OPENALAUDIOLOADER_H_
#define __OPENALAUDIOLOADER_H_

#include "Common/AsciiString.h"

#ifdef __APPLE__
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
#else
    #include <AL/al.h>
    #include <AL/alc.h>
#endif

#include <string>
#include <unordered_map>

/**
 * @class OpenALAudioLoader
 * @brief Loads audio files from VFS (FileSystemEA) and decodes them into OpenAL buffers
 * 
 * Supports:
 * - WAV files (PCM format)
 * - MP3 files (decoded using dr_mp3)
 * 
 * Features buffer caching to avoid redundant loading of the same file.
 */
class OpenALAudioLoader
{
public:
    /**
     * @brief Load an audio file from VFS and return an OpenAL buffer
     * 
     * @param filename File path relative to VFS root (e.g., "Data/Audio/Shell.mp3")
     * @return ALuint OpenAL buffer ID, or 0 on error
     * 
     * Uses buffer cache to avoid redundant loading.
     */
    static ALuint loadFromFile(const std::string& filename);
    static ALuint loadFromFile(const AsciiString& filename);

private:
    /**
     * @brief Decode WAV data from memory buffer
     * 
     * @param data Pointer to WAV file data
     * @param dataSize Size of data in bytes
     * @return ALuint OpenAL buffer ID, or 0 on error
     */
    static ALuint decodeWav(const char* data, size_t dataSize);

    /**
     * @brief Decode MP3 data from memory buffer using dr_mp3
     * 
     * @param data Pointer to MP3 file data
     * @param dataSize Size of data in bytes
     * @return ALuint OpenAL buffer ID, or 0 on error
     */
    static ALuint decodeMp3(const char* data, size_t dataSize);

    /**
     * @brief Check if filename has a specific extension (case-insensitive)
     * 
     * @param filename Filename to check
     * @param ext Extension to match (e.g., ".mp3", ".wav")
     * @return true if filename ends with extension
     */
    static bool hasExtension(const std::string& filename, const char* ext);

    // Buffer cache: filename -> OpenAL buffer ID
    static bool s_initializedCache;
    static std::unordered_map<std::string, ALuint> s_bufferCache;
};

#endif // __OPENALAUDIOLOADER_H_
