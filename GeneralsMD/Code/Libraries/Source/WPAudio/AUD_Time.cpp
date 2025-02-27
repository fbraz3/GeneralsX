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

/*****************************************************************************
**                                                                          **
**                       Westwood Studios Pacific.                          **
**                                                                          **
**                       Confidential Information					                  **
**                Copyright (C) 2000 - All Rights Reserved                  **
**                                                                          **
******************************************************************************
**                                                                          **
** Project:  Dune Emperor                                                   **
**                                                                          **
** Module:  <module> (<prefix>_)                                            **
**                                                                          **
** Version:  $ID$                                                           **
**                                                                          **
** File name:  audtimer.cpp                                                 **
**                                                                          **
** Created by:  04/??/99 TR                                                 **
**                                                                          **
** Description: <description>                                               **
**                                                                          **
*****************************************************************************/

/*****************************************************************************
**            Includes                                                      **
*****************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <assert.h>

#include <wpaudio/time.h>

/*****************************************************************************
**          Externals                                                       **
*****************************************************************************/



/*****************************************************************************
**           Defines                                                        **
*****************************************************************************/



/*****************************************************************************
**        Private Types                                                     **
*****************************************************************************/



/*****************************************************************************
**         Private Data                                                     **
*****************************************************************************/

static	TimeStamp		lastTime = 0;
static	TimeStamp		interval = 0;
static	TimeStamp		timeOut = 0;
static	TimeStamp		(*timerFunc)( void ) = NULL;// set to either highResGetTime or
																								// failsafeGetTime at initialization
static LONGLONG			timerMilliSecScale;					// res counter millisecond scaling factor

/*****************************************************************************
**         Public Data                                                      **
*****************************************************************************/



/*****************************************************************************
**         Private Prototypes                                               **
*****************************************************************************/

static TimeStamp failsafeGetTime( void );
static TimeStamp highResGetTime( void );

/*****************************************************************************
**          Private Functions                                               **
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static TimeStamp highResGetTime( void )
{
	LARGE_INTEGER count;
	union
	{
		TimeStamp timeStamp;
		LARGE_INTEGER largeInt;
	}
	myTime;

	// read the high res counter
	QueryPerformanceCounter(&count);

	// convert high res ticks to number of milliseconds
	myTime.largeInt.QuadPart = count.QuadPart / timerMilliSecScale;

	return myTime.timeStamp;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static TimeStamp failsafeGetTime( void )
{
	static	volatile TimeStamp	time = (TimeStamp) 0x100000000;					// last time value
	static	unsigned int*	const	pl = (unsigned int* const) &time;		// low word of time
	static	unsigned int*	const	ph = (unsigned int* const) ((unsigned int)&time + 4);		// high word of time
					unsigned int					now, lw, hw, called;
	static	volatile unsigned int	calls = 0;

		
	calls++;

retry:

	called = calls;

	hw = *ph;	// read high word of time stamp
	lw = *pl;	// read low word of time stamp

	if ( called != calls)	
	{
		// AudioGetTime() has been re-entered. Cannot trust lw and lh values
		goto retry;
	}

reread:

	now = timeGetTime ();

	if ( now < lw )
	{
		// wrap round
		hw++; // increment high word by one
	}

	*ph = hw;
	*pl = now;

	if ( called != calls )
	{
		// re-entered. cannot trust *ph and *pl to be correct
		called = calls;
		goto reread;
	}

	return time;
}

/*****************************************************************************
**          Public Functions                                                **
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

//
// Initialize the high resolution timer by querying the system for it's
// availability. If one does exist then we set the game timer function
// to 'highResGetTime' otherwise we use the original code at 'failsafeGetTime'.
// For the hi res counter we precalculate the millisecond scaling factor to
// convert hi res ticks to millisecond usage.

void InitAudioTimer( void )
{
	LARGE_INTEGER freq;

	// does hardware exist for high res counter?
	if (QueryPerformanceFrequency(&freq))
	{
		// calculate timer ticks per second
		timerMilliSecScale = freq.QuadPart / (LONGLONG) SECONDS(1);

		timerFunc = highResGetTime;
	}
	else
	{
		// no high res timer, use old code instead
		timerFunc = failsafeGetTime;
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

TimeStamp AudioGetTime( void )
{
	return timerFunc ? timerFunc() : 0 ;
}

