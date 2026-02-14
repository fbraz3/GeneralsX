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

/*
** BinkVideoPlayerStub.cpp
**
** Stub implementation of BinkVideoPlayer for Linux
**
** TheSuperHackers @build felipebraz 13/02/2026
** Bink video codec is Windows/proprietary only. On Linux, videos are skipped.
*/

#ifndef _WIN32

#include "../../Core/GameEngineDevice/Include/VideoDevice/Bink/BinkVideoPlayer.h"
#include <cstdio>

/**
 * BinkVideoPlayer constructor - Linux stub
 * TheSuperHackers @build felipebraz 13/02/2026 (P3)
 * Bink codec not available on Linux, video playback will be skipped
 */
BinkVideoPlayer::BinkVideoPlayer()
	: m_binkHandle(nullptr),
	  m_width(0),
	  m_height(0),
	  m_isPlaying(false),
	  m_isPaused(false),
	  m_volume(1.0f),
	  m_currentFrame(0),
	  m_totalFrames(0)
{
	fprintf(stderr, "INFO: BinkVideoPlayer::BinkVideoPlayer() - stub constructor (Bink not available on Linux)\n");
}

#endif // !_WIN32
