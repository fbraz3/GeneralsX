/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "../GameEngine/Common/FileSystemTestFixture.h"
#include "MiniAudioDevice/MiniAudioMusicStream.h"

#include <chrono>
#include <thread>

using MiniAudioMusicStreamTest = FileSystemTestFixture;

// Both fixtures have the eight unsigned PCM samples 129 through 136 on their first track.
static void expectFourPasses(File *file)
{
	ASSERT_NE(file, nullptr);
	MiniAudioMusicStream stream;
	ASSERT_TRUE(stream.open(file));
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
	unsigned samples = 0;
	while (samples < 32 && std::chrono::steady_clock::now() < deadline) {
		unsigned char output[8];
		ma_uint64 read = 0;
		ASSERT_EQ(ma_data_source_read_pcm_frames(stream.source(), output, 8, &read), MA_SUCCESS);
		ASSERT_EQ(read, 8u);
		for (unsigned char sample : output) {
			// The empty ring provides silence while the worker starts.
			if (sample == 128) {
				continue;
			}
			ASSERT_EQ(sample, 129 + samples % 8);
			++samples;
		}
		ASSERT_FALSE(stream.failed());
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	EXPECT_GE(samples, 32u);
}

TEST_F(MiniAudioMusicStreamTest, ContinuesAcrossTrackBoundary)
{
	const unsigned char wave[] = {
		'R','I','F','F', 44,0,0,0, 'W','A','V','E', 'f','m','t',' ',
		16,0,0,0, 1,0, 1,0, 0x40,0x1f,0,0, 0x40,0x1f,0,0, 1,0, 8,0,
		'd','a','t','a', 8,0,0,0, 129,130,131,132,133,134,135,136
	};
	writeLooseFile("loop.wav", std::string(reinterpret_cast<const char *>(wave), sizeof(wave)));
	expectFourPasses(TheFileSystem->openFile("loop.wav"));
}

TEST_F(MiniAudioMusicStreamTest, LoopsOnlyTheSelectedSoundtrack)
{
	// Minimal Matroska container with two mono PCM tracks. The second track has a
	// different rate and waveform. Feeding it into the first track's ring would stop playback.
	const char twoTracks[] =
		"\x1a\x45\xdf\xa3\x93\x42\x82\x88" "matroska" "\x42\x87\x81\x04\x42\x85\x81\x02"
		"\x18\x53\x80\x67\x40\x98" // Segment
		"\x15\x49\xa9\x66\x8e\x4d\x80\x84" "Test" "\x57\x41\x84" "Test"
		"\x16\x54\xae\x6b\xd4" // Tracks
		"\xae\xa8\xd7\x81\x01\x73\xc5\x81\x01\x83\x81\x02\x86\x8d" "A_PCM/INT/LIT"
		"\xe1\x8d\xb5\x84\x45\xfa\x00\x00\x9f\x81\x01\x62\x64\x81\x08" // 8 kHz
		"\xae\xa8\xd7\x81\x02\x73\xc5\x81\x02\x83\x81\x02\x86\x8d" "A_PCM/INT/LIT"
		"\xe1\x8d\xb5\x84\x46\x7a\x00\x00\x9f\x81\x01\x62\x64\x81\x08" // 16 kHz
		"\x1f\x43\xb6\x75\xa7\xe7\x81\x00" // Cluster at time zero
		"\xa3\x8c\x81\x00\x00\x80\x81\x82\x83\x84\x85\x86\x87\x88"
		"\xa3\x94\x82\x00\x00\x80\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0"
		"\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0";
	writeLooseFile("two-tracks.mka", std::string(twoTracks, sizeof(twoTracks) - 1));
	expectFourPasses(TheFileSystem->openFile("two-tracks.mka"));
}

TEST_F(MiniAudioMusicStreamTest, RejectsAnInvalidTrack)
{
	writeLooseFile("invalid.mp3", "Invalid audio");
	File *file = TheFileSystem->openFile("invalid.mp3");
	ASSERT_NE(file, nullptr);
	MiniAudioMusicStream stream;
	EXPECT_FALSE(stream.open(file));
}
