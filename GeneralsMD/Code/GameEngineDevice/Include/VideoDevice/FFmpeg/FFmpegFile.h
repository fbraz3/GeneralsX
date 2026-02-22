/**
 * @file
 *
 * @author feliwir
 *
 * @brief Class for opening FFmpeg contexts from a file.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"

#include <functional>
#include <vector>

struct AVFormatContext;
struct AVIOContext;
struct AVCodec;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct File;

using FFmpegFrameCallback = std::function<void(AVFrame *, int, int, void *)>;

class FFmpegFile
{
public:
    FFmpegFile();
    FFmpegFile(File *file);
    ~FFmpegFile();

    bool open(File *file);
    void close();
    void setFrameCallback(FFmpegFrameCallback callback) { m_frameCallback = callback; }
    void setUserData(void *user_data) { m_userData = user_data; }
    // Read & decode a packet from the container. Note that we could/should split this step
    bool decodePacket();
    void seekFrame(int frame_idx);
    bool hasAudio() const;

    // Audio specific
    int getSizeForSamples(int numSamples) const;
    int getNumChannels() const;
    int getSampleRate() const;
    int getBytesPerSample() const;

    // Video specific
    int getWidth() const;
    int getHeight() const;
    int getNumFrames() const;
    int getCurrentFrame() const;
    int getPixelFormat() const;
    unsigned int getFrameTime() const;

private:
    struct FFmpegStream
    {
        AVCodecContext *codec_ctx = nullptr;
        const AVCodec *codec = nullptr;
        int stream_idx = -1;
        int stream_type = -1;
        AVFrame *frame = nullptr;
    };

    static int readPacket(void *opaque, uint8_t *buf, int buf_size);
    const FFmpegStream *findMatch(int type) const;

    FFmpegFrameCallback m_frameCallback = nullptr;
    AVFormatContext *m_fmtCtx = nullptr;
    AVIOContext *m_avioCtx = nullptr;
    AVPacket *m_packet = nullptr;
    std::vector<FFmpegStream> m_streams;
    File *m_file = nullptr;
    void *m_userData = nullptr;
};