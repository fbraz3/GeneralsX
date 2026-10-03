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

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  (c) 2001-2003 Electronic Arts Inc.                                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// FILE: Win32DIKeyboardStub.cpp //////////////////////////////////////////////

#include "Common/Debug.h"
#include "GameClient/KeyDefs.h"
#include "Win32Device/GameClient/Win32DIKeyboard.h"

// GeneralsX @build Copilot 20/05/2026 Keep Windows MinGW buildable with a
// temporary non-DI keyboard stub while DX8/DI compatibility is stabilized.

void DirectInputKeyboard::openKeyboard()
{
	// No-op in stub mode.
}

void DirectInputKeyboard::closeKeyboard()
{
	// No-op in stub mode.
}

void DirectInputKeyboard::getKey( KeyboardIO *key )
{
	if( key )
	{
		key->key = KEY_NONE;
		key->status = KeyboardIO::STATUS_UNUSED;
		key->state = 0;
		key->keyDownTimeMsec = 0;
	}
}

DirectInputKeyboard::DirectInputKeyboard()
{
	m_pDirectInput = nullptr;
	m_pKeyboardDevice = nullptr;
	m_modifiers = 0;
}

DirectInputKeyboard::~DirectInputKeyboard()
{
	closeKeyboard();
}

void DirectInputKeyboard::init()
{
	Keyboard::init();
	openKeyboard();
}

void DirectInputKeyboard::reset()
{
	Keyboard::reset();
}

void DirectInputKeyboard::update()
{
	Keyboard::update();
}

Bool DirectInputKeyboard::getCapsState()
{
	return FALSE;
}
