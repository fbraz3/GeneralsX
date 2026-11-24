/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once

#include "Lib/BaseType.h"
#include "VideoDevice/FFmpeg/FFmpegFile.h"
#include <miniaudio.h>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

// One decoder writes this ring. The miniaudio mixer reads it without waiting for the decoder.
class MiniAudioMusicStream
{
public:
	~MiniAudioMusicStream();
	// Takes ownership of a reader with its own cursor, including on failure.
	bool open(File *file);
	void stop();
	bool finished() const { return !m_thread.joinable() || m_finished.load(); }
	ma_data_source *source() { return &m_ring; }
	bool failed() const { return m_failed.load(); }

private:
	void decode();
	void writeFrame(AVFrame *frame);

	FFmpegFile m_decoder;
	ma_pcm_rb m_ring{};
	bool m_ringInitialized = false;
	bool m_wroteFrames = false;
	std::thread m_thread;
	std::atomic<bool> m_stop{false};
	std::atomic<bool> m_failed{false};
	std::atomic<bool> m_finished{false};
	std::mutex m_mutex;
	std::condition_variable m_wake;
};
