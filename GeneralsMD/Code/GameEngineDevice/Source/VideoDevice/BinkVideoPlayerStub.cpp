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

// BinkVideoPlayerStub.cpp - Portable stub for Bink video codec (not available)
// TheSuperHackers @build felipebraz 13/02/2026 - Phase 3
// GeneralsX @build Copilot 20/05/2026 Keep Bink stub active on Windows MinGW when proprietary SDK is unavailable.

#include "VideoDevice/Bink/BinkVideoPlayer.h"
#include <cstdio>

// Stub constructor
BinkVideoPlayer::BinkVideoPlayer()
{
	fprintf(stderr, "INFO: BinkVideoPlayer() - stub (Bink SDK not available)\n");
}

// Stub destructor
BinkVideoPlayer::~BinkVideoPlayer()
{
	fprintf(stderr, "INFO: ~BinkVideoPlayer() - stub\n");
}

// Subsystem requirements - all stubs
void BinkVideoPlayer::init(void)
{
	fprintf(stderr, "WARNING: BinkVideoPlayer::init() - stub (video playback not available)\n");
}

void BinkVideoPlayer::reset(void)
{
	fprintf(stderr, "DEBUG: BinkVideoPlayer::reset() - stub\n");
}

void BinkVideoPlayer::update(void)
{
	// Silent no-op
}

void BinkVideoPlayer::deinit(void)
{
	fprintf(stderr, "DEBUG: BinkVideoPlayer::deinit() - stub\n");
}

// Focus management - stubs
void BinkVideoPlayer::loseFocus(void)
{
	fprintf(stderr, "DEBUG: BinkVideoPlayer::loseFocus() - stub\n");
}

void BinkVideoPlayer::regainFocus(void)
{
	fprintf(stderr, "DEBUG: BinkVideoPlayer::regainFocus() - stub\n");
}

// Video file operations - stubs
VideoStreamInterface* BinkVideoPlayer::open(AsciiString movieTitle)
{
	fprintf(stderr, "WARNING: BinkVideoPlayer::open('%s') - stub (video playback not available)\n", 
		movieTitle.str() ? movieTitle.str() : "(null)");
	return nullptr;  // Video playback not supported in stub mode
}

VideoStreamInterface* BinkVideoPlayer::load(AsciiString movieTitle)
{
	fprintf(stderr, "WARNING: BinkVideoPlayer::load('%s') - stub (video playback not available)\n", 
		movieTitle.str() ? movieTitle.str() : "(null)");
	return nullptr;  // Video loading not supported in stub mode
}

void BinkVideoPlayer::notifyVideoPlayerOfNewProvider(Bool nowHasValid)
{
	fprintf(stderr, "DEBUG: BinkVideoPlayer::notifyVideoPlayerOfNewProvider(%d) - stub\n", nowHasValid);
}

void BinkVideoPlayer::initializeBinkWithMiles(void)
{
	fprintf(stderr, "DEBUG: BinkVideoPlayer::initializeBinkWithMiles() - stub\n");
}
