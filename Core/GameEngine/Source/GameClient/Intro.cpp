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

#include "PreRTS.h"
#include "GameClient/Intro.h"

#include "Common/GameLOD.h"

#include "GameClient/Display.h"
#include "GameClient/GameWindowManager.h"


Intro::Intro()
	: m_currentState(IntroState_Start)
	, m_allowedStateFlags(0)
	, m_waitUntilMs(0)
{
	if (TheGlobalData->m_playIntro)
	{
		m_allowedStateFlags |= 1u << IntroState_EALogoMovie;
	}

	if (TheGlobalData->m_playSizzle)
	{
		if (TheGlobalData->m_playIntro)
			m_allowedStateFlags |= 1u << IntroState_SizzleMovieWait;

		m_allowedStateFlags |= 1u << IntroState_SizzleMovie;
	}
}

void Intro::enterNextState()
{
	Int currentState = m_currentState;
	while (currentState < IntroState_Done)
	{
		++currentState;
		if (m_allowedStateFlags & (1u << currentState))
			break;
	}

	m_currentState = static_cast<IntroState>(currentState);
}

void Intro::update()
{
	if (!TheDisplay->isMoviePlaying() && m_waitUntilMs < timeGetTime())
	{
		enterNextState();

		switch (m_currentState)
		{
		case IntroState_EALogoMovie: doEALogoMovie(); break;
		case IntroState_SizzleMovieWait: doAsyncWait(1000); break;
		case IntroState_SizzleMovie: doSizzleMovie(); break;
		case IntroState_Done: doPostIntro(); break;
		}
	}
}

Bool Intro::skipCurrentIntroStage()
{
	// Aborts waits, stops movies and skips waiting states

	Bool isSkipping = !isDone();

	m_waitUntilMs = 0;

	switch (m_currentState)
	{
	case IntroState_EALogoMovie:
	case IntroState_SizzleMovie:
		TheDisplay->stopMovie();
		enterNextState();
		break;
	}

	return isSkipping;
}
void Intro::doEALogoMovie()
{
	if (TheGameLODManager && TheGameLODManager->didMemPass())
		TheDisplay->playMovie("EALogoMovie");
	else
		TheDisplay->playMovie("EALogoMovie640");
}

void Intro::doSizzleMovie()
{
	if (TheGameLODManager && TheGameLODManager->didMemPass())
		TheDisplay->playMovie("Sizzle");
	else
		TheDisplay->playMovie("Sizzle640");
}

void Intro::doPostIntro()
{
	TheWritableGlobalData->m_breakTheMovie = TRUE;
}

void Intro::doAsyncWait(UnsignedInt milliseconds)
{
	m_waitUntilMs = timeGetTime() + milliseconds;
}
