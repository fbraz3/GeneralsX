/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "Lib/BaseType.h"
#include "MiniAudioDevice/MiniAudioMusicStream.h"
#include "Common/file.h"

#include <chrono>
#include <system_error>
extern "C" {
#include <libavcodec/avcodec.h>
}

static ma_format musicSampleFormat(AVSampleFormat format)
{
	switch (av_get_packed_sample_fmt(format)) {
		case AV_SAMPLE_FMT_U8:  return ma_format_u8;
		case AV_SAMPLE_FMT_S16: return ma_format_s16;
		case AV_SAMPLE_FMT_S32: return ma_format_s32;
		case AV_SAMPLE_FMT_FLT: return ma_format_f32;
		default:                return ma_format_unknown;
	}
}

bool MiniAudioMusicStream::open(File *file)
{
	if (!m_decoder.open(file, FFmpegFile::LoopingAudio)) {
		return false;
	}
	const ma_format format = musicSampleFormat(static_cast<AVSampleFormat>(m_decoder.getAudioSampleFormat()));
	const int channels = m_decoder.getNumChannels();
	const int rate = m_decoder.getSampleRate();
	if (format == ma_format_unknown || channels <= 0 || channels > MA_MAX_CHANNELS || rate <= 0) {
		return false;
	}
	if (ma_pcm_rb_init(format, channels, rate / 2 + 1, nullptr, nullptr, &m_ring) != MA_SUCCESS) {
		return false;
	}
	m_ringInitialized = true;
	ma_pcm_rb_set_sample_rate(&m_ring, rate);
	m_decoder.setFrameCallback([this](AVFrame *frame, int, int, void *) {
		writeFrame(frame);
	});
	try {
		m_thread = std::thread(&MiniAudioMusicStream::decode, this);
	} catch (const std::system_error &) {
		return false;
	}
	return true;
}

MiniAudioMusicStream::~MiniAudioMusicStream()
{
	if (m_thread.joinable()) {
		stop();
		m_thread.join();
	}
	if (m_ringInitialized) {
		ma_pcm_rb_uninit(&m_ring);
	}
}

void MiniAudioMusicStream::stop()
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_stop = true;
	}
	m_wake.notify_one();
}

void MiniAudioMusicStream::decode()
{
	while (!m_stop && !m_failed) {
		Bool endOfInput = false;
		if (m_decoder.decodePacket(&endOfInput)) {
			continue;
		}
		// Music repeats until the sound is stopped. Keep the next pass queued behind
		// the previous one, including while playback is paused or fading.
		if (!endOfInput || !m_wroteFrames || !m_decoder.rewind()) {
			m_failed = true;
		}
		m_wroteFrames = false;
	}
	m_finished = true;
}

void MiniAudioMusicStream::writeFrame(AVFrame *frame)
{
	const auto avFormat = static_cast<AVSampleFormat>(frame->format);
	const ma_format format = musicSampleFormat(avFormat);
	const int channels = frame->ch_layout.nb_channels;
	if (format != m_ring.format || channels != static_cast<int>(m_ring.channels) ||
		frame->sample_rate != static_cast<int>(ma_pcm_rb_get_sample_rate(&m_ring))) {
		m_failed = true;
		return;
	}
	const unsigned sampleBytes = ma_get_bytes_per_sample(format);
	for (int offset = 0; offset < frame->nb_samples && !m_stop && !m_failed; ) {
		ma_uint32 count = frame->nb_samples - offset;
		void *output = nullptr;
		if (ma_pcm_rb_acquire_write(&m_ring, &count, &output) != MA_SUCCESS) {
			m_failed = true;
			return;
		}
		if (!count) {
			// The audio callback never takes this lock. Stop wakes the producer immediately.
			std::unique_lock<std::mutex> lock(m_mutex);
			m_wake.wait_for(lock, std::chrono::milliseconds(20), [this] { return m_stop.load(); });
			continue;
		}
		if (av_sample_fmt_is_planar(avFormat)) {
			const void *planes[MA_MAX_CHANNELS];
			for (int i = 0; i < channels; ++i) {
				planes[i] = frame->extended_data[i] + offset * sampleBytes;
			}
			ma_interleave_pcm_frames(format, channels, count, planes, output);
		} else {
			const unsigned char *input = frame->extended_data[0] + offset * channels * sampleBytes;
			ma_copy_pcm_frames(output, input, count, format, channels);
		}
		ma_pcm_rb_commit_write(&m_ring, count);
		m_wroteFrames = true;
		offset += count;
	}
}
