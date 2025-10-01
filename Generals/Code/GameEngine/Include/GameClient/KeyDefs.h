/*
**	Command & Conquer Generals(tm)
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
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: KeyDefs.h ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  KeyDefs.h
//
// Created:    Mike Morrison, 1995
//						 Colin Day, June 2001
//
// Desc:       Basic keyboard key definitions.
//
/** @todo NOTE: These key definitions are currently tied directly to the
*		Direct Input key codes, therefore making these definitions device
*		dependent even though this code lives on the device INdependent side
*		of the engine.  In the future to be truly device independent we
*		need to define our own key codes, and have a translation between
*		what we read from the device to our own system*/
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __KEYDEFS_H_
#define __KEYDEFS_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>
#ifdef _WIN32
#ifndef DIRECTINPUT_VERSION
#	define DIRECTINPUT_VERSION	0x800
#endif
#include <dinput.h>
#else
// macOS DirectInput key code compatibility - using common key code values
#define DIK_NUMPAD0 0x52
#define DIK_NUMPAD1 0x4F
#define DIK_NUMPAD2 0x50
#define DIK_NUMPAD3 0x51
#define DIK_NUMPAD4 0x4B
#define DIK_NUMPAD5 0x4C
#define DIK_NUMPAD6 0x4D
#define DIK_NUMPAD7 0x47
#define DIK_NUMPAD8 0x48
#define DIK_NUMPAD9 0x49
#define DIK_NUMPADPERIOD 0x53
#define DIK_NUMPADSTAR 0x55
#define DIK_NUMPADMINUS 0x4E
#define DIK_NUMPADPLUS 0x45
#define DIK_ESCAPE 0x01
#define DIK_1 0x02
#define DIK_2 0x03
#define DIK_3 0x04
#define DIK_4 0x05
#define DIK_5 0x06
#define DIK_6 0x07
#define DIK_7 0x08
#define DIK_8 0x09
#define DIK_9 0x0A
#define DIK_0 0x0B
#define DIK_Q 0x10
#define DIK_W 0x11
#define DIK_E 0x12
#define DIK_R 0x13
#define DIK_T 0x14
#define DIK_Y 0x15
#define DIK_U 0x16
#define DIK_I 0x17
#define DIK_O 0x18
#define DIK_P 0x19
#define DIK_A 0x1E
#define DIK_S 0x1F
#define DIK_D 0x20
#define DIK_F 0x21
#define DIK_G 0x22
#define DIK_H 0x23
#define DIK_J 0x24
#define DIK_K 0x25
#define DIK_L 0x26
#define DIK_Z 0x2C
#define DIK_X 0x2D
#define DIK_C 0x2E
#define DIK_V 0x2F
#define DIK_B 0x30
#define DIK_N 0x31
#define DIK_M 0x32
#define DIK_SPACE 0x39
#define DIK_RETURN 0x1C
#define DIK_LSHIFT 0x2A
#define DIK_RSHIFT 0x36
#define DIK_LCONTROL 0x1D
#define DIK_RCONTROL 0x9D
#define DIK_LALT 0x38
#define DIK_LMENU 0x38
#define DIK_RMENU 0xB8
#define DIK_TAB 0x0F
#define DIK_CAPSLOCK 0x3A
#define DIK_BACKSPACE 0x0E
#define DIK_DELETE 0xD3
#define DIK_UP 0xC8
#define DIK_UPARROW 0xC8
#define DIK_DOWN 0xD0
#define DIK_DOWNARROW 0xD0
#define DIK_LEFT 0xCB
#define DIK_LEFTARROW 0xCB
#define DIK_RIGHT 0xCD
#define DIK_RIGHTARROW 0xCD
#define DIK_HOME 0xC7
#define DIK_END 0xCF
#define DIK_PRIOR 0xC9
#define DIK_NEXT 0xD1
#define DIK_INSERT 0xD2
#define DIK_F1 0x3B
#define DIK_F2 0x3C
#define DIK_F3 0x3D
#define DIK_F4 0x3E
#define DIK_F5 0x3F
#define DIK_F6 0x40
#define DIK_F7 0x41
#define DIK_F8 0x42
#define DIK_F9 0x43
#define DIK_F10 0x44
#define DIK_F11 0x57
#define DIK_F12 0x58
#define DIK_BACK 0x0E
#define DIK_MINUS 0x0C
#define DIK_EQUALS 0x0D
#define DIK_LBRACKET 0x1A
#define DIK_RBRACKET 0x1B
#define DIK_BACKSLASH 0x2B
#define DIK_SEMICOLON 0x27
#define DIK_APOSTROPHE 0x28
#define DIK_GRAVE 0x29
#define DIK_COMMA 0x33
#define DIK_PERIOD 0x34
#define DIK_SLASH 0x35
#define DIK_NUMLOCK 0x45
#define DIK_SCROLL 0x46
#define DIK_NUMPADENTER 0x9C
#define DIK_NUMPADSLASH 0xB5
#define DIK_NUMPADEQUALS 0x8D
#define DIK_PAUSE 0xC5
#define DIK_SYSRQ 0xB7
#define DIK_PGUP 0xC9
#define DIK_PGDN 0xD1
#define DIK_RALT 0xB8
#endif
#include <Lib/BaseType.h>

// USER INCLUDES //////////////////////////////////////////////////////////////

// FORWARD REFERENCES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef DIK_OEM_102
#define DIK_OEM_102         0x56    /* < > | on UK/Germany keyboards  */
#endif

#ifndef DIK_KANA
#define DIK_KANA            0x70    /* (Japanese keyboard)            */
#endif

#ifndef DIK_CONVERT
#define DIK_CONVERT         0x79    /* (Japanese keyboard)            */
#endif

#ifndef DIK_NOCONVERT
#define DIK_NOCONVERT       0x7B    /* (Japanese keyboard)            */
#endif

#ifndef DIK_YEN
#define DIK_YEN             0x7D    /* (Japanese keyboard)            */
#endif

#ifndef DIK_CIRCUMFLEX
#define DIK_CIRCUMFLEX      0x90    /* (Japanese keyboard)            */
#endif

#ifndef DIK_KANJI
#define DIK_KANJI           0x94    /* (Japanese keyboard)            */
#endif

//=============================================================================
/** The key tables */
//=============================================================================

enum KeyDefType CPP_11(: Int)
{
	// keypad keys ----------------------------------------------------------------
	KEY_KP0 								= DIK_NUMPAD0,
	KEY_KP1 								= DIK_NUMPAD1,
	KEY_KP2 								= DIK_NUMPAD2,
	KEY_KP3 								= DIK_NUMPAD3,
	KEY_KP4 								= DIK_NUMPAD4,
	KEY_KP5 								= DIK_NUMPAD5,
	KEY_KP6 								= DIK_NUMPAD6,
	KEY_KP7 								= DIK_NUMPAD7,
	KEY_KP8 								= DIK_NUMPAD8,
	KEY_KP9 								= DIK_NUMPAD9,
	KEY_KPDEL 							= DIK_NUMPADPERIOD,
	KEY_KPSTAR 							= DIK_NUMPADSTAR,
	KEY_KPMINUS 						= DIK_NUMPADMINUS,
	KEY_KPPLUS 							= DIK_NUMPADPLUS,

	// regular keys ---------------------------------------------------------------
	KEY_ESC 								= DIK_ESCAPE,
	KEY_BACKSPACE 					= DIK_BACK,
	KEY_ENTER 							= DIK_RETURN,
	KEY_SPACE 							= DIK_SPACE,
	KEY_TAB 								= DIK_TAB,
	KEY_F1 									= DIK_F1,
	KEY_F2 									= DIK_F2,
	KEY_F3 									= DIK_F3,
	KEY_F4 									= DIK_F4,
	KEY_F5 									= DIK_F5,
	KEY_F6 									= DIK_F6,
	KEY_F7 									= DIK_F7,
	KEY_F8 									= DIK_F8,
	KEY_F9 									= DIK_F9,
	KEY_F10 								= DIK_F10,
	KEY_F11 								= DIK_F11,
	KEY_F12 								= DIK_F12,
	KEY_A 									= DIK_A,
	KEY_B 									= DIK_B,
	KEY_C 									= DIK_C,
	KEY_D 									= DIK_D,
	KEY_E 									= DIK_E,
	KEY_F 									= DIK_F,
	KEY_G 									= DIK_G,
	KEY_H 									= DIK_H,
	KEY_I 									= DIK_I,
	KEY_J 									= DIK_J,
	KEY_K 									= DIK_K,
	KEY_L 									= DIK_L,
	KEY_M 									= DIK_M,
	KEY_N 									= DIK_N,
	KEY_O 									= DIK_O,
	KEY_P 									= DIK_P,
	KEY_Q 									= DIK_Q,
	KEY_R 									= DIK_R,
	KEY_S 									= DIK_S,
	KEY_T 									= DIK_T,
	KEY_U 									= DIK_U,
	KEY_V 									= DIK_V,
	KEY_W 									= DIK_W,
	KEY_X 									= DIK_X,
	KEY_Y 									= DIK_Y,
	KEY_Z 									= DIK_Z,
	KEY_1 									= DIK_1,
	KEY_2 									= DIK_2,
	KEY_3 									= DIK_3,
	KEY_4 									= DIK_4,
	KEY_5 									= DIK_5,
	KEY_6 									= DIK_6,
	KEY_7 									= DIK_7,
	KEY_8 									= DIK_8,
	KEY_9 									= DIK_9,
	KEY_0 									= DIK_0,
	KEY_MINUS 							= DIK_MINUS,
	KEY_EQUAL 							= DIK_EQUALS,
	KEY_LBRACKET 						= DIK_LBRACKET,
	KEY_RBRACKET 						= DIK_RBRACKET,
	KEY_SEMICOLON 					= DIK_SEMICOLON,
	KEY_APOSTROPHE 					= DIK_APOSTROPHE,
	KEY_TICK 								= DIK_GRAVE,
	KEY_BACKSLASH 					= DIK_BACKSLASH,
	KEY_COMMA 							= DIK_COMMA,
	KEY_PERIOD 							= DIK_PERIOD,
	KEY_SLASH 							= DIK_SLASH,

	// special keys ---------------------------------------------------------------
	KEY_SYSREQ 							= DIK_SYSRQ,

	KEY_CAPS 								= DIK_CAPSLOCK,
	KEY_NUM 								= DIK_NUMLOCK,
	KEY_SCROLL 							= DIK_SCROLL,
	KEY_LCTRL 							= DIK_LCONTROL,
	KEY_LALT 								= DIK_LALT,
	KEY_LSHIFT 							= DIK_LSHIFT,
	KEY_RSHIFT 							= DIK_RSHIFT,

	KEY_UP 									= DIK_UPARROW,
	KEY_DOWN 								= DIK_DOWNARROW,
	KEY_LEFT 								= DIK_LEFTARROW,
	KEY_RIGHT 							= DIK_RIGHTARROW,
	KEY_RALT 								= DIK_RALT,
	KEY_RCTRL 							= DIK_RCONTROL,
	KEY_HOME 								= DIK_HOME,
	KEY_END 								= DIK_END,
	KEY_PGUP 								= DIK_PGUP,
	KEY_PGDN 								= DIK_PGDN,
	KEY_INS 								= DIK_INSERT,
	KEY_DEL 								= DIK_DELETE,
	KEY_KPENTER 						= DIK_NUMPADENTER,
	KEY_KPSLASH 						= DIK_NUMPADSLASH,

	KEY_102 								= DIK_OEM_102,

	// Japanese keyboard keys -----------------------------------------------------
	KEY_KANA 								= DIK_KANA,
	KEY_CONVERT 						= DIK_CONVERT,
	KEY_NOCONVERT 					= DIK_NOCONVERT,
	KEY_YEN 								= DIK_YEN,
	KEY_CIRCUMFLEX 					= DIK_CIRCUMFLEX,
	KEY_KANJI 							= DIK_KANJI,

	// specials -------------------------------------------------------------------
	KEY_NONE								= 0x00,		///< to report end of key stream
	KEY_LOST								= 0xFF		///< to report lost keyboard focus

};

// state for keyboard IO ------------------------------------------------------
enum
{
	KEY_STATE_NONE								= 0x0000, // No modifier state
	KEY_STATE_UP									= 0x0001,	// Key is up (default state)
	KEY_STATE_DOWN								= 0x0002,	// Key is down
	KEY_STATE_LCONTROL						= 0x0004,	// Left control is pressed
	KEY_STATE_RCONTROL						= 0x0008,	// Right control is pressed
	KEY_STATE_LSHIFT							= 0x0010,	// left shift is pressed
	KEY_STATE_RSHIFT							= 0x0020,	// right shift is pressed
	KEY_STATE_LALT								= 0x0040,	// left alt is pressed
	KEY_STATE_RALT								= 0x0080,	// right alt is pressed
	KEY_STATE_AUTOREPEAT					= 0x0100,	// Key is down due to autorepeat (only seen in conjunction with KEY_STATE_DOWN)
	KEY_STATE_CAPSLOCK						= 0x0200, // Caps Lock key is on.
	KEY_STATE_SHIFT2							= 0x0400, // Alternate shift key is pressed (I think this is for foreign keyboards..)

	// modifier combinations when left/right isn't a factor
	KEY_STATE_CONTROL		= (KEY_STATE_LCONTROL | KEY_STATE_RCONTROL),
	KEY_STATE_SHIFT			= (KEY_STATE_LSHIFT | KEY_STATE_RSHIFT | KEY_STATE_SHIFT2 ),
	KEY_STATE_ALT				= (KEY_STATE_LALT | KEY_STATE_RALT)

};

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////

#endif // __KEYDEFS_H_
