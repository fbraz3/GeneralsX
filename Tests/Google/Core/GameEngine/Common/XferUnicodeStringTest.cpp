/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

#include "Common/Debug.h"
#include "Common/UnicodeString.h"
#include "Common/XferLoad.h"
#include "Common/XferSave.h"

namespace
{

bool g_crashCaptured = false;
std::string g_crashMessage;

void captureCrash(const char *message)
{
	g_crashCaptured = true;
	g_crashMessage = message != nullptr ? message : "";
}

// XferSave reports a string it refuses with DEBUG_CRASH before it throws, and the test executable
// turns every crash into a test failure, so the expected one is taken out of the way here.
class ScopedCrashCapture
{
public:
	ScopedCrashCapture() : m_previous(DebugGetCrashHandler())
	{
		g_crashCaptured = false;
		g_crashMessage.clear();
		DebugSetCrashHandler(captureCrash);
	}

	~ScopedCrashCapture()
	{
		DebugSetCrashHandler(m_previous);
	}

private:
	DebugCrashHandler m_previous;
};

}  // namespace

// The save file format stores a unicode string as a length byte followed by 2 byte UTF-16 code
// units, the width of wchar_t on Windows. The port writes and reads that width on every platform,
// including the ones where wchar_t is 4 bytes, so that save files stay interchangeable between
// platforms and with the retail game.
class XferUnicodeStringTest : public ::testing::Test
{
protected:
	virtual void SetUp() override
	{
		m_directory = std::filesystem::temp_directory_path()
			/ ("XferUnicodeStringTest_" + std::to_string(std::random_device()()));
		std::filesystem::remove_all(m_directory);
		std::filesystem::create_directories(m_directory);
		m_path = (m_directory / "unicode.sav").string();
	}

	virtual void TearDown() override
	{
		std::filesystem::remove_all(m_directory);
	}

	// Writes the string to the test file and returns the bytes that ended up in it.
	std::vector<UnsignedByte> writeString(UnicodeString text)
	{
		XferSave xfer;
		xfer.open(AsciiString(m_path.c_str()));
		xfer.xferUnicodeString(&text);
		xfer.close();

		std::ifstream file(m_path, std::ios::binary);
		return std::vector<UnsignedByte>(std::istreambuf_iterator<char>(file),
			std::istreambuf_iterator<char>());
	}

	// Puts bytes in the test file as they would come from a save written elsewhere, so that the
	// reader is exercised on input the writer did not produce.
	void writeBytes(const std::vector<UnsignedByte> &bytes)
	{
		std::ofstream file(m_path, std::ios::binary | std::ios::trunc);
		file.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
	}

	// Reads a string back from the test file.
	UnicodeString readString()
	{
		XferLoad xfer;
		xfer.open(AsciiString(m_path.c_str()));
		UnicodeString text;
		xfer.xferUnicodeString(&text);
		xfer.close();
		return text;
	}

	std::filesystem::path m_directory;
	std::string m_path;
};

TEST_F(XferUnicodeStringTest, WritesTwoBytesPerCharacter)
{
	const std::vector<UnsignedByte> bytes = writeString(UnicodeString(L"Début"));

	// one length byte, then one 2 byte code unit per character
	ASSERT_EQ(bytes.size(), 1u + 2u * 5u);
	EXPECT_EQ(bytes[0], 5u);

	const UnsignedByte expected[] = { 'D', 0x00, 0xE9, 0x00, 'b', 0x00, 'u', 0x00, 't', 0x00 };
	EXPECT_TRUE(std::equal(std::begin(expected), std::end(expected), bytes.begin() + 1));
}

TEST_F(XferUnicodeStringTest, ReadsBackWhatItWrote)
{
	writeString(UnicodeString(L"Début de mission : USA 2"));

	EXPECT_STREQ(readString().str(), L"Début de mission : USA 2");
}

TEST_F(XferUnicodeStringTest, LeavesTheFilePositionOnTheNextValue)
{
	// A string that is read too wide swallows the data behind it, which is what garbled the save
	// game descriptions and broke the reads that follow them.
	{
		XferSave xfer;
		xfer.open(AsciiString(m_path.c_str()));
		UnicodeString text(L"Début");
		xfer.xferUnicodeString(&text);
		AsciiString mapLabel("MD_USA01.map");
		xfer.xferAsciiString(&mapLabel);
		xfer.close();
	}

	XferLoad xfer;
	xfer.open(AsciiString(m_path.c_str()));
	UnicodeString text;
	xfer.xferUnicodeString(&text);
	AsciiString mapLabel;
	xfer.xferAsciiString(&mapLabel);
	xfer.close();

	EXPECT_STREQ(text.str(), L"Début");
	EXPECT_STREQ(mapLabel.str(), "MD_USA01.map");
}

TEST_F(XferUnicodeStringTest, WritesAnEmptyStringAsItsLengthAlone)
{
	const std::vector<UnsignedByte> bytes = writeString(UnicodeString(L""));

	ASSERT_EQ(bytes.size(), 1u);
	EXPECT_EQ(bytes[0], 0u);
	EXPECT_STREQ(readString().str(), L"");
}

// A character outside the basic multilingual plane is one wchar_t where wchar_t is 4 bytes and a
// surrogate pair on Windows. The file holds the surrogate pair either way, so the length header
// counts two code units for it and the saves stay interchangeable.
TEST_F(XferUnicodeStringTest, WritesASupplementaryCharacterAsASurrogatePair)
{
	const std::vector<UnsignedByte> bytes = writeString(UnicodeString(L"\U0001F600"));

	ASSERT_EQ(bytes.size(), 1u + 2u * 2u);
	EXPECT_EQ(bytes[0], 2u);

	const UnsignedByte expected[] = { 0x3D, 0xD8, 0x00, 0xDE };
	EXPECT_TRUE(std::equal(std::begin(expected), std::end(expected), bytes.begin() + 1));
}

TEST_F(XferUnicodeStringTest, ReadsBackASupplementaryCharacter)
{
	writeString(UnicodeString(L"Mission \U0001F600 ok"));

	EXPECT_STREQ(readString().str(), L"Mission \U0001F600 ok");
}

// The reader has to rebuild the character from a pair it did not write itself, which is what a save
// coming from the retail game or from another platform looks like.
TEST_F(XferUnicodeStringTest, DecodesASurrogatePairFromAForeignSave)
{
	writeBytes({ 3u, 0x3D, 0xD8, 0x00, 0xDE, 'A', 0x00 });

	EXPECT_STREQ(readString().str(), L"\U0001F600A");
}

// An unpaired surrogate has no character behind it, so it goes through as it is rather than
// swallowing the code unit that follows it.
TEST_F(XferUnicodeStringTest, KeepsAnUnpairedSurrogateAsItIs)
{
	writeBytes({ 2u, 0x3D, 0xD8, 'A', 0x00 });

	const UnicodeString text = readString();

	ASSERT_EQ(text.getLength(), 2);
	EXPECT_EQ((UnsignedInt)text.getCharAt(0), 0xD83Du);
	EXPECT_EQ((UnsignedInt)text.getCharAt(1), (UnsignedInt)L'A');
}

// The length header is a single byte, so the limit is 255 code units once encoded, not 255
// characters: 128 supplementary characters already need 256 of them.
TEST_F(XferUnicodeStringTest, RefusesAStringThatEncodesToMoreThan255CodeUnits)
{
	UnicodeString text;
	for (int i = 0; i < 128; ++i)
		text.concat(L"\U0001F600");

	XferSave xfer;
	xfer.open(AsciiString(m_path.c_str()));
	{
		ScopedCrashCapture crash;
		EXPECT_THROW(xfer.xferUnicodeString(&text), XferStatus);
	}
	xfer.close();

#ifdef DEBUG_CRASHING
	EXPECT_TRUE(g_crashCaptured);
	EXPECT_NE(g_crashMessage.find("too long"), std::string::npos);
#endif
}

TEST_F(XferUnicodeStringTest, WritesTheLargestStringThatStillFits)
{
	UnicodeString text;
	for (int i = 0; i < 127; ++i)
		text.concat(L"\U0001F600");
	text.concat(L"A");

	const std::vector<UnsignedByte> bytes = writeString(text);

	ASSERT_EQ(bytes.size(), 1u + 2u * 255u);
	EXPECT_EQ(bytes[0], 255u);
	EXPECT_STREQ(readString().str(), text.str());
}

// Pre-1.0.9 POSIX builds wrote WideChar (4-byte UTF-32LE) directly to disk. Existing savegames
// must remain readable and leave the stream aligned on the following fields.
TEST_F(XferUnicodeStringTest, ReadsLegacyUtf32SaveAndPreservesStreamPosition)
{
	// "Début" in 4-byte UTF-32LE followed by AsciiString "MD_USA05.map"
	std::vector<UnsignedByte> bytes = {
		5u,
		'D', 0x00, 0x00, 0x00,
		0xE9, 0x00, 0x00, 0x00,
		'b', 0x00, 0x00, 0x00,
		'u', 0x00, 0x00, 0x00,
		't', 0x00, 0x00, 0x00,
		12u,
		'M', 'D', '_', 'U', 'S', 'A', '0', '5', '.', 'm', 'a', 'p'
	};
	writeBytes(bytes);

	XferLoad xfer;
	xfer.open(AsciiString(m_path.c_str()));
	UnicodeString text;
	xfer.xferUnicodeString(&text);
	AsciiString mapLabel;
	xfer.xferAsciiString(&mapLabel);
	xfer.close();

	EXPECT_STREQ(text.str(), L"Début");
	EXPECT_STREQ(mapLabel.str(), "MD_USA05.map");
}

TEST_F(XferUnicodeStringTest, ReadsSingleCharLegacyUtf32AndSubsequentStrings)
{
	// First string: 1-char UTF-32LE "A", second string: 2-char UTF-32LE "OK", then AsciiString "end"
	std::vector<UnsignedByte> bytes = {
		1u,
		'A', 0x00, 0x00, 0x00,
		2u,
		'O', 0x00, 0x00, 0x00,
		'K', 0x00, 0x00, 0x00,
		3u,
		'e', 'n', 'd'
	};
	writeBytes(bytes);

	XferLoad xfer;
	xfer.open(AsciiString(m_path.c_str()));
	UnicodeString first;
	xfer.xferUnicodeString(&first);
	UnicodeString second;
	xfer.xferUnicodeString(&second);
	AsciiString tail;
	xfer.xferAsciiString(&tail);
	xfer.close();

	EXPECT_STREQ(first.str(), L"A");
	EXPECT_STREQ(second.str(), L"OK");
	EXPECT_STREQ(tail.str(), "end");
}

TEST_F(XferUnicodeStringTest, ReadsSingleCharUtf16AtEof)
{
	writeBytes({ 1u, 'Z', 0x00 });

	EXPECT_STREQ(readString().str(), L"Z");
}

TEST_F(XferUnicodeStringTest, ReadsSingleCharUtf16FollowedByEmptyAsciiStringAtEof)
{
	// Probe reads the trailing 0x00 byte of the empty AsciiString and hits EOF; position must be restored.
	writeBytes({ 1u, 'Z', 0x00, 0x00 });

	XferLoad xfer;
	xfer.open(AsciiString(m_path.c_str()));
	UnicodeString text;
	xfer.xferUnicodeString(&text);
	AsciiString emptyTail;
	xfer.xferAsciiString(&emptyTail);
	xfer.close();

	EXPECT_STREQ(text.str(), L"Z");
	EXPECT_STREQ(emptyTail.str(), "");
}

TEST_F(XferUnicodeStringTest, ReadsUtf16ControlCharacterWithoutMisclassifyingAsUtf32)
{
	writeBytes({ 2u, 'A', 0x00, '\b', 0x00 });

	EXPECT_STREQ(readString().str(), L"A\b");
}

TEST_F(XferUnicodeStringTest, ReadsUtf16ControlCharacterFollowedByInt)
{
	// UTF-16 "A\b" followed by Int(66) must not be misclassified as UTF-32LE.
	writeBytes({ 2u, 'A', 0x00, '\b', 0x00, 0x42, 0x00, 0x00, 0x00 });

	XferLoad xfer;
	xfer.open(AsciiString(m_path.c_str()));
	UnicodeString text;
	xfer.xferUnicodeString(&text);
	Int value = 0;
	xfer.xferInt(&value);
	xfer.close();

	EXPECT_STREQ(text.str(), L"A\b");
	EXPECT_EQ(value, 66);
}
