/**
 * @file OpenALAudioLoader.cpp
 * @brief Implementation of OpenAL audio file loader
 * 
 * Ported from jmarshall-win64-modern reference implementation
 */

#include "Lib/BaseType.h"
#include "OpenALDevice/OpenALAudioLoader.h"

#include <stdexcept>
#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iostream>

#define DR_MP3_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION

#include "OpenALDevice/dr_wav.h"
#include "OpenALDevice/dr_mp3.h"
#include "Common/file.h"
#include "Common/FileSystem.h"

/** Static members */
bool OpenALAudioLoader::s_initializedCache = false;
std::unordered_map<std::string, ALuint> OpenALAudioLoader::s_bufferCache;

// ------------------------------------------------------
// Helper: check file extension (case-insensitive)
// ------------------------------------------------------
bool OpenALAudioLoader::hasExtension(const std::string& filename, const char* ext) {
    if (filename.size() < strlen(ext)) {
        return false;
    }
    std::string fileExt = filename.substr(filename.size() - strlen(ext));
    std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::tolower);

    std::string cmpExt(ext);
    std::transform(cmpExt.begin(), cmpExt.end(), cmpExt.begin(), ::tolower);

    return fileExt == cmpExt;
}

// ------------------------------------------------------
// Main load function that reads from FileSystem (VFS)
// ------------------------------------------------------
ALuint OpenALAudioLoader::loadFromFile(const std::string& filename) {
    if (!s_initializedCache) {
        s_bufferCache.clear();
        s_initializedCache = true;
    }

    // Check cache first
    auto it = s_bufferCache.find(filename);
    if (it != s_bufferCache.end()) {
        std::cerr << "OpenALAudioLoader: Using cached buffer for '" << filename << "'" << std::endl;
        return it->second;
    }

    std::cerr << "OpenALAudioLoader: Loading '" << filename << "' from VFS" << std::endl;

    // Open file from VFS
    File* file = TheFileSystem->openFile(filename.c_str());
    if (!file) {
        if (!filename.empty()) {
            std::cerr << "ERROR: Failed to open audio file: '" << filename << "'" << std::endl;
        }
        return 0;
    }

    // Read entire file into memory
    unsigned int fileSize = file->size();
    char* buffer = file->readEntireAndClose();
    if (!buffer || fileSize == 0) {
        std::cerr << "ERROR: Failed to read audio file: '" << filename << "'" << std::endl;
        delete[] buffer;
        return 0;
    }

    std::cerr << "OpenALAudioLoader: Loaded " << fileSize << " bytes from '" << filename << "'" << std::endl;

    // Decode based on file extension
    ALuint alBuffer = 0;
    try {
        if (hasExtension(filename, ".wav")) {
            std::cerr << "OpenALAudioLoader: Decoding WAV file" << std::endl;
            alBuffer = decodeWav(buffer, fileSize);
        }
        else if (hasExtension(filename, ".mp3")) {
            std::cerr << "OpenALAudioLoader: Decoding MP3 file" << std::endl;
            alBuffer = decodeMp3(buffer, fileSize);
        }
        else {
            std::cerr << "ERROR: Unsupported audio format: '" << filename << "'" << std::endl;
            delete[] buffer;
            return 0;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: Audio decoding failed: " << e.what() << std::endl;
        alBuffer = 0;
    }

    delete[] buffer;

    // Cache successful load
    if (alBuffer != 0) {
        s_bufferCache[filename] = alBuffer;
        std::cerr << "OpenALAudioLoader: Successfully loaded and cached buffer " << alBuffer << std::endl;
    }

    return alBuffer;
}

ALuint OpenALAudioLoader::loadFromFile(const AsciiString& filename) {
    return loadFromFile(std::string(filename.str()));
}

// ------------------------------------------------------
// WAV decoding (using dr_wav)
// ------------------------------------------------------
ALuint OpenALAudioLoader::decodeWav(const char* data, size_t dataSize) {
    drwav wav;
    if (!drwav_init_memory(&wav, data, dataSize, nullptr)) {
        throw std::runtime_error("Failed to initialize WAV decoder");
    }

    std::vector<int16_t> pcmData(wav.totalPCMFrameCount * wav.channels);
    drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, pcmData.data());
    drwav_uninit(&wav);

    ALenum format = (wav.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    ALuint bufferID;
    alGenBuffers(1, &bufferID);
    alBufferData(bufferID, format, pcmData.data(), pcmData.size() * sizeof(int16_t), wav.sampleRate);

    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "ERROR: OpenAL buffer creation failed, error code: " << error << std::endl;
        return 0;
    }

    return bufferID;
}

// ------------------------------------------------------
// MP3 decoding (using dr_mp3)
// ------------------------------------------------------
ALuint OpenALAudioLoader::decodeMp3(const char* data, size_t dataSize) {
    drmp3 mp3;
    if (!drmp3_init_memory(&mp3, data, dataSize, nullptr)) {
        throw std::runtime_error("Failed to initialize MP3 decoder");
    }

    std::vector<int16_t> pcmData(drmp3_get_pcm_frame_count(&mp3) * mp3.channels);
    drmp3_read_pcm_frames_s16(&mp3, pcmData.size() / mp3.channels, pcmData.data());
    drmp3_uninit(&mp3);

    ALenum format = (mp3.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    ALuint bufferID;
    alGenBuffers(1, &bufferID);
    alBufferData(bufferID, format, pcmData.data(), pcmData.size() * sizeof(int16_t), mp3.sampleRate);

    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "ERROR: OpenAL buffer creation failed, error code: " << error << std::endl;
        return 0;
    }

    return bufferID;
}
