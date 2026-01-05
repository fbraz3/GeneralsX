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

// dinput_compat.h - DirectInput keyboard code compatibility layer for cross-platform builds
// This file provides DIK_* keyboard scan codes for non-Windows platforms where dinput.h is not available
// The scan codes are based on the DirectInput keyboard scan code definitions

#ifndef DINPUT_COMPAT_H
#define DINPUT_COMPAT_H

// DIK_* keyboard scan codes (DirectInput format)
// Based on Windows DirectInput scan codes

#ifndef DIK_ESCAPE
#define DIK_ESCAPE			0x01	/* Esc */
#endif

#ifndef DIK_1
#define DIK_1				0x02	/* 1 */
#endif

#ifndef DIK_2
#define DIK_2				0x03	/* 2 */
#endif

#ifndef DIK_3
#define DIK_3				0x04	/* 3 */
#endif

#ifndef DIK_4
#define DIK_4				0x05	/* 4 */
#endif

#ifndef DIK_5
#define DIK_5				0x06	/* 5 */
#endif

#ifndef DIK_6
#define DIK_6				0x07	/* 6 */
#endif

#ifndef DIK_7
#define DIK_7				0x08	/* 7 */
#endif

#ifndef DIK_8
#define DIK_8				0x09	/* 8 */
#endif

#ifndef DIK_9
#define DIK_9				0x0A	/* 9 */
#endif

#ifndef DIK_0
#define DIK_0				0x0B	/* 0 */
#endif

#ifndef DIK_MINUS
#define DIK_MINUS			0x0C	/* - */
#endif

#ifndef DIK_EQUALS
#define DIK_EQUALS			0x0D	/* = */
#endif

#ifndef DIK_BACK
#define DIK_BACK			0x0E	/* Backspace */
#endif

#ifndef DIK_TAB
#define DIK_TAB				0x0F	/* Tab */
#endif

#ifndef DIK_Q
#define DIK_Q				0x10	/* Q */
#endif

#ifndef DIK_W
#define DIK_W				0x11	/* W */
#endif

#ifndef DIK_E
#define DIK_E				0x12	/* E */
#endif

#ifndef DIK_R
#define DIK_R				0x13	/* R */
#endif

#ifndef DIK_T
#define DIK_T				0x14	/* T */
#endif

#ifndef DIK_Y
#define DIK_Y				0x15	/* Y */
#endif

#ifndef DIK_U
#define DIK_U				0x16	/* U */
#endif

#ifndef DIK_I
#define DIK_I				0x17	/* I */
#endif

#ifndef DIK_O
#define DIK_O				0x18	/* O */
#endif

#ifndef DIK_P
#define DIK_P				0x19	/* P */
#endif

#ifndef DIK_LBRACKET
#define DIK_LBRACKET		0x1A	/* [ */
#endif

#ifndef DIK_RBRACKET
#define DIK_RBRACKET		0x1B	/* ] */
#endif

#ifndef DIK_RETURN
#define DIK_RETURN			0x1C	/* Enter */
#endif

#ifndef DIK_LCONTROL
#define DIK_LCONTROL		0x1D	/* Left Ctrl */
#endif

#ifndef DIK_A
#define DIK_A				0x1E	/* A */
#endif

#ifndef DIK_S
#define DIK_S				0x1F	/* S */
#endif

#ifndef DIK_D
#define DIK_D				0x20	/* D */
#endif

#ifndef DIK_F
#define DIK_F				0x21	/* F */
#endif

#ifndef DIK_G
#define DIK_G				0x22	/* G */
#endif

#ifndef DIK_H
#define DIK_H				0x23	/* H */
#endif

#ifndef DIK_J
#define DIK_J				0x24	/* J */
#endif

#ifndef DIK_K
#define DIK_K				0x25	/* K */
#endif

#ifndef DIK_L
#define DIK_L				0x26	/* L */
#endif

#ifndef DIK_SEMICOLON
#define DIK_SEMICOLON		0x27	/* ; */
#endif

#ifndef DIK_APOSTROPHE
#define DIK_APOSTROPHE		0x28	/* ' */
#endif

#ifndef DIK_GRAVE
#define DIK_GRAVE			0x29	/* ` */
#endif

#ifndef DIK_LSHIFT
#define DIK_LSHIFT			0x2A	/* Left Shift */
#endif

#ifndef DIK_BACKSLASH
#define DIK_BACKSLASH		0x2B	/* \ */
#endif

#ifndef DIK_Z
#define DIK_Z				0x2C	/* Z */
#endif

#ifndef DIK_X
#define DIK_X				0x2D	/* X */
#endif

#ifndef DIK_C
#define DIK_C				0x2E	/* C */
#endif

#ifndef DIK_V
#define DIK_V				0x2F	/* V */
#endif

#ifndef DIK_B
#define DIK_B				0x30	/* B */
#endif

#ifndef DIK_N
#define DIK_N				0x31	/* N */
#endif

#ifndef DIK_M
#define DIK_M				0x32	/* M */
#endif

#ifndef DIK_COMMA
#define DIK_COMMA			0x33	/* , */
#endif

#ifndef DIK_PERIOD
#define DIK_PERIOD			0x34	/* . */
#endif

#ifndef DIK_SLASH
#define DIK_SLASH			0x35	/* / */
#endif

#ifndef DIK_RSHIFT
#define DIK_RSHIFT			0x36	/* Right Shift */
#endif

#ifndef DIK_MULTIPLY
#define DIK_MULTIPLY		0x37	/* * (on numeric keypad) */
#endif

#ifndef DIK_LMENU
#define DIK_LMENU			0x38	/* Left Alt */
#endif

#ifndef DIK_SPACE
#define DIK_SPACE			0x39	/* Space */
#endif

#ifndef DIK_CAPITAL
#define DIK_CAPITAL			0x3A	/* Caps Lock */
#endif

#ifndef DIK_F1
#define DIK_F1				0x3B	/* F1 */
#endif

#ifndef DIK_F2
#define DIK_F2				0x3C	/* F2 */
#endif

#ifndef DIK_F3
#define DIK_F3				0x3D	/* F3 */
#endif

#ifndef DIK_F4
#define DIK_F4				0x3E	/* F4 */
#endif

#ifndef DIK_F5
#define DIK_F5				0x3F	/* F5 */
#endif

#ifndef DIK_F6
#define DIK_F6				0x40	/* F6 */
#endif

#ifndef DIK_F7
#define DIK_F7				0x41	/* F7 */
#endif

#ifndef DIK_F8
#define DIK_F8				0x42	/* F8 */
#endif

#ifndef DIK_F9
#define DIK_F9				0x43	/* F9 */
#endif

#ifndef DIK_F10
#define DIK_F10				0x44	/* F10 */
#endif

#ifndef DIK_NUMLOCK
#define DIK_NUMLOCK			0x45	/* Num Lock */
#endif

#ifndef DIK_SCROLL
#define DIK_SCROLL			0x46	/* Scroll Lock */
#endif

#ifndef DIK_NUMPAD7
#define DIK_NUMPAD7			0x47	/* 7 (numeric keypad) */
#endif

#ifndef DIK_NUMPAD8
#define DIK_NUMPAD8			0x48	/* 8 (numeric keypad) */
#endif

#ifndef DIK_NUMPAD9
#define DIK_NUMPAD9			0x49	/* 9 (numeric keypad) */
#endif

#ifndef DIK_SUBTRACT
#define DIK_SUBTRACT		0x4A	/* - (on numeric keypad) */
#endif

#ifndef DIK_NUMPAD4
#define DIK_NUMPAD4			0x4B	/* 4 (numeric keypad) */
#endif

#ifndef DIK_NUMPAD5
#define DIK_NUMPAD5			0x4C	/* 5 (numeric keypad) */
#endif

#ifndef DIK_NUMPAD6
#define DIK_NUMPAD6			0x4D	/* 6 (numeric keypad) */
#endif

#ifndef DIK_ADD
#define DIK_ADD				0x4E	/* + (on numeric keypad) */
#endif

#ifndef DIK_NUMPAD1
#define DIK_NUMPAD1			0x4F	/* 1 (numeric keypad) */
#endif

#ifndef DIK_NUMPAD2
#define DIK_NUMPAD2			0x50	/* 2 (numeric keypad) */
#endif

#ifndef DIK_NUMPAD3
#define DIK_NUMPAD3			0x51	/* 3 (numeric keypad) */
#endif

#ifndef DIK_NUMPAD0
#define DIK_NUMPAD0			0x52	/* 0 (numeric keypad) */
#endif

#ifndef DIK_NUMPADPERIOD
#define DIK_NUMPADPERIOD	0x53	/* . (numeric keypad) */
#endif

#ifndef DIK_OEM_102
#define DIK_OEM_102			0x56	/* < > | on UK/Germany keyboards */
#endif

#ifndef DIK_F11
#define DIK_F11				0x57	/* F11 */
#endif

#ifndef DIK_F12
#define DIK_F12				0x58	/* F12 */
#endif

#ifndef DIK_F13
#define DIK_F13				0x64	/* F13 */
#endif

#ifndef DIK_F14
#define DIK_F14				0x65	/* F14 */
#endif

#ifndef DIK_F15
#define DIK_F15				0x66	/* F15 */
#endif

#ifndef DIK_KANA
#define DIK_KANA			0x70	/* (Japanese keyboard) */
#endif

#ifndef DIK_CONVERT
#define DIK_CONVERT			0x79	/* (Japanese keyboard) */
#endif

#ifndef DIK_NOCONVERT
#define DIK_NOCONVERT		0x7B	/* (Japanese keyboard) */
#endif

#ifndef DIK_YEN
#define DIK_YEN				0x7D	/* (Japanese keyboard) */
#endif

#ifndef DIK_NUMPADEQUALS
#define DIK_NUMPADEQUALS	0x8D	/* = (on numeric keypad, NEC PC98) */
#endif

#ifndef DIK_CIRCUMFLEX
#define DIK_CIRCUMFLEX		0x90	/* (Japanese keyboard) */
#endif

#ifndef DIK_AT
#define DIK_AT				0x91	/* (NEC PC98) */
#endif

#ifndef DIK_KANJI
#define DIK_KANJI			0x94	/* (Japanese keyboard) */
#endif

#ifndef DIK_STOP
#define DIK_STOP			0x95	/* (NEC PC98) */
#endif

#ifndef DIK_AX
#define DIK_AX				0x96	/* (Japan AX) */
#endif

#ifndef DIK_UNLABELED
#define DIK_UNLABELED		0x97	/* (J3100) */
#endif

#ifndef DIK_NUMPADENTER
#define DIK_NUMPADENTER		0x9C	/* Enter (numeric keypad) */
#endif

#ifndef DIK_RCONTROL
#define DIK_RCONTROL		0x9D	/* Right Ctrl */
#endif

#ifndef DIK_NUMPADCOMMA
#define DIK_NUMPADCOMMA		0xB3	/* , (on numeric keypad, NEC PC98) */
#endif

#ifndef DIK_DIVIDE
#define DIK_DIVIDE			0xB5	/* / (on numeric keypad) */
#endif

#ifndef DIK_SYSRQ
#define DIK_SYSRQ			0xB7	/* Sys Req / Print Screen */
#endif

#ifndef DIK_RMENU
#define DIK_RMENU			0xB8	/* Right Alt */
#endif

#ifndef DIK_HOME
#define DIK_HOME			0xC7	/* Home */
#endif

#ifndef DIK_UP
#define DIK_UP				0xC8	/* Up Arrow */
#endif

#ifndef DIK_PRIOR
#define DIK_PRIOR			0xC9	/* Page Up */
#endif

#ifndef DIK_LEFT
#define DIK_LEFT			0xCB	/* Left Arrow */
#endif

#ifndef DIK_RIGHT
#define DIK_RIGHT			0xCD	/* Right Arrow */
#endif

#ifndef DIK_END
#define DIK_END				0xCF	/* End */
#endif

#ifndef DIK_DOWN
#define DIK_DOWN			0xD0	/* Down Arrow */
#endif

#ifndef DIK_NEXT
#define DIK_NEXT			0xD1	/* Page Down */
#endif

#ifndef DIK_INSERT
#define DIK_INSERT			0xD2	/* Insert */
#endif

#ifndef DIK_DELETE
#define DIK_DELETE			0xD3	/* Delete */
#endif

#ifndef DIK_LWIN
#define DIK_LWIN			0xDB	/* Left Windows key */
#endif

#ifndef DIK_RWIN
#define DIK_RWIN			0xDC	/* Right Windows key */
#endif

#ifndef DIK_APPS
#define DIK_APPS			0xDD	/* Application key */
#endif

#ifndef DIK_POWER
#define DIK_POWER			0xDE	/* Power key */
#endif

#ifndef DIK_SLEEP
#define DIK_SLEEP			0xDF	/* Sleep key */
#endif

// Aliases for backward compatibility / alternative names
#ifndef DIK_NUMPADSTAR
#define DIK_NUMPADSTAR		DIK_MULTIPLY	/* * (on numeric keypad) */
#endif

#ifndef DIK_NUMPADMINUS
#define DIK_NUMPADMINUS		DIK_SUBTRACT	/* - (on numeric keypad) */
#endif

#ifndef DIK_NUMPADPLUS
#define DIK_NUMPADPLUS		DIK_ADD			/* + (on numeric keypad) */
#endif

#ifndef DIK_NUMPADSLASH
#define DIK_NUMPADSLASH		DIK_DIVIDE		/* / (on numeric keypad) */
#endif

// Alternative names for some keys
#ifndef DIK_CAPSLOCK
#define DIK_CAPSLOCK		DIK_CAPITAL		/* Caps Lock (alternative name) */
#endif

#ifndef DIK_LALT
#define DIK_LALT			DIK_LMENU		/* Left Alt (alternative name) */
#endif

#ifndef DIK_RALT
#define DIK_RALT			DIK_RMENU		/* Right Alt (alternative name) */
#endif

#ifndef DIK_UPARROW
#define DIK_UPARROW			DIK_UP			/* Up Arrow (alternative name) */
#endif

#ifndef DIK_DOWNARROW
#define DIK_DOWNARROW		DIK_DOWN		/* Down Arrow (alternative name) */
#endif

#ifndef DIK_LEFTARROW
#define DIK_LEFTARROW		DIK_LEFT		/* Left Arrow (alternative name) */
#endif

#ifndef DIK_RIGHTARROW
#define DIK_RIGHTARROW		DIK_RIGHT		/* Right Arrow (alternative name) */
#endif

#ifndef DIK_ENTER
#define DIK_ENTER			DIK_RETURN		/* Enter (alternative name) */
#endif

#ifndef DIK_PAGEUP
#define DIK_PAGEUP			DIK_PRIOR		/* Page Up (alternative name) */
#endif

#ifndef DIK_PAGEDOWN
#define DIK_PAGEDOWN		DIK_NEXT		/* Page Down (alternative name) */
#endif

#endif // DINPUT_COMPAT_H
