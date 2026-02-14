/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
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

// BinkVideoPlayerStub.cpp - Linux stub for Bink video codec (not available)
// TheSuperHackers @build felipebraz 13/02/2026 - Phase 3

#ifndef _WIN32

#include "VideoDevice/Bink/BinkVideoPlayer.h"
#include <cstdio>

// Stub constructor
BinkVideoPlayer::BinkVideoPlayer()
{
	fprintf(stderr, "INFO: BinkVideoPlayer() - Linux stub (Windows Bink SDK not available)\n");
}

// Stub destructor
BinkVideoPlayer::~BinkVideoPlayer()
{
	fprintf(stderr, "INFO: ~BinkVideoPlayer() - Linux stub\n");
}

// Subsystem requirements - all stubs for Linux
void BinkVideoPlayer::init(void)
{
	fprintf(stderr, "WARNING: BinkVideoPlayer::init() - Linux stub (video playback not available)\n");
}

void BinkVideoPlayer::reset(void)
{
	fprintf(stderr, "DEBUG: BinkVideoPlayer::reset() - Linux stub\n");
}

void BinkVideoPlayer::update(void)
{
	// Silent no-op
}

void BinkVideoPlayer::deinit(void)
{
	fprintf(stderr, "DEBUG: BinkVideoPlayer::deinit() - Linux stub\n");
}

// Focus management - stubs for Linux
void BinkVideoPlayer::loseFocus(void)
{
	fprintf(stderr, "DEBUG: BinkVideoPlayer::loseFocus() - Linux stub\n");
}

void BinkVideoPlayer::regainFocus(void)
{
	fprintf(stderr, "DEBUG: BinkVideoPlayer::regainFocus() - Linux stub\n");
}

// Video file operations - stubs for Linux
VideoStreamInterface* BinkVideoPlayer::open(AsciiString movieTitle)
{
	fprintf(stderr, "WARNING: BinkVideoPlayer::open('%s') - Linux stub (video playback not available)\n", 
		movieTitle.str() ? movieTitle.str() : "(null)");
	return nullptr;  // Video playback not supported on Linux
}

VideoStreamInterface* BinkVideoPlayer::load(AsciiString movieTitle)
{
	fprintf(stderr, "WARNING: BinkVideoPlayer::load('%s') - Linux stub (video playback not available)\n", 
		movieTitle.str() ? movieTitle.str() : "(null)");
	return nullptr;  // Video loading not supported on Linux
}

void BinkVideoPlayer::notifyVideoPlayerOfNewProvider(Bool nowHasValid)
{
	fprintf(stderr, "DEBUG: BinkVideoPlayer::notifyVideoPlayerOfNewProvider(%d) - Linux stub\n", nowHasValid);
}

void BinkVideoPlayer::initializeBinkWithMiles(void)
{
	fprintf(stderr, "DEBUG: BinkVideoPlayer::initializeBinkWithMiles() - Linux stub\n");
}

#endif // !_WIN32
