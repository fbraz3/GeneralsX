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
** File name:  audprof.cpp                                                  **
**                                                                          **
** Created by:  mm/dd/yy <author>                                           **
**                                                                          **
** Description: <description>                                               **
**                                                                          **
*****************************************************************************/

/*****************************************************************************
**            Includes                                                      **
*****************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#include <wpaudio/profiler.h>

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



/*****************************************************************************
**         Public Data                                                      **
*****************************************************************************/



/*****************************************************************************
**         Private Prototypes                                               **
*****************************************************************************/



/*****************************************************************************
**          Private Functions                                               **
*****************************************************************************/



/*****************************************************************************
**          Public Functions                                                **
*****************************************************************************/

#ifndef IG_FINAL_RELEASE

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfCacheInit ( ProfileData *prof, int pages, int pageSize )
{

	memset ( prof, 0, sizeof ( ProfileData ));
	if ( !QueryPerformanceFrequency( (LARGE_INTEGER*) &prof->freq ))
	{
		prof->freq = 0;
	}
	prof->update = prof->freq;
	prof->cacheSize = pages*pageSize;
	prof->numPages = pages;
	prof->pageSize = pageSize;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfCacheDeinit ( ProfileData * )
{

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfCacheNewFrame ( ProfileData *prof )
{
	int total;
	prof->frames++;

	if ( prof->totalTime > prof->longestFrame )
	{
		prof->nextLongestFrame = prof->longestFrame;
		prof->nextLongestFramePages = prof->longestFramePages;
		prof->nextLongestFrameBytes = prof->longestFrameBytes;

		prof->longestFrame = prof->totalTime ;
		prof->longestFrameBytes = prof->totalDataBytes;
		prof->longestFramePages = prof->pageCount;

	}
	else if ( prof->totalTime > prof->nextLongestFrame )
	{
		prof->nextLongestFrame = prof->totalTime ;
		prof->nextLongestFrameBytes = prof->totalDataBytes;
		prof->nextLongestFramePages = prof->pageCount;
	}

	prof->pageCount = 0;

	if ( (total = prof->hits + prof->misses ))
	{
		prof->HitPercent = (prof->hits *100) / total;
	}

	prof->BytesPerFrame = prof->totalFrameBytes / prof->frames;

	if ( prof->frames > 3*30 )
	{
			prof->totalFrameBytes  = 0;
			prof->frames = 0;
	}

	if ( ! (prof->frames % 30) )
	{
		prof->hits = 0;
		prof->misses = 0;
	}

	if ( prof->totalTime > prof->update )
	{
		int ms;

		ms = (int) ( (prof->totalTime *1000 )/prof->freq);
		prof->TotalBytesPerSecond = (int) (((unsigned __int64) prof->totalDataBytes *1000)/ ms);
		prof->totalDataBytes = 0;
		prof->totalTime = 0;

		ms = (int) ((prof->totalDecompTime *1000 )/prof->freq) ;

		if ( ms )
		{
			prof->DecompBytesPerSecond = (int) (((unsigned __int64) prof->totalDecompBytes *1000)/ ms);
		}
		else
		{
			prof->DecompBytesPerSecond = 0;
		}
	
		prof->totalDecompBytes = 0;
		prof->totalDecompTime = 0;

		ms = (int) ((prof->totalLoadTime *1000 )/prof->freq );

		if ( ms )
		{
			prof->LoadBytesPerSecond = (int) (((unsigned __int64)prof->totalLoadBytes *1000)/ ms);
		}
		else
		{
			prof->LoadBytesPerSecond = 0;
		}
		prof->totalLoadBytes = 0;
		prof->totalLoadTime = 0;
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfCacheLoadStart ( ProfileData *prof, int bytes )
{
	QueryPerformanceCounter( (LARGE_INTEGER*)&prof->start );
	prof->dbytes = bytes;
	prof->start_decomp = 0;
	prof->pageCount++;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfCacheAddLoadBytes ( ProfileData *prof, int bytes )
{
	prof->dbytes += bytes;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfCacheDecompress ( ProfileData *prof, int bytes )
{
	QueryPerformanceCounter( (LARGE_INTEGER*)&prof->start_decomp );
	prof->lbytes = bytes;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfCacheLoadEnd ( ProfileData *prof )
{
	QueryPerformanceCounter( (LARGE_INTEGER*)&prof->end );
	prof->totalTime += prof->end - prof->start;
	prof->totalDataBytes += prof->dbytes;
	prof->totalFrameBytes += prof->dbytes;
	if ( prof->start_decomp )
	{
		prof->totalLoadTime += prof->start_decomp - prof->start;
		prof->totalDecompTime += prof->end - prof->start_decomp;
		prof->totalLoadBytes += prof->lbytes;
		prof->totalDecompBytes += prof->dbytes;
	}
	else
	{
		prof->totalLoadTime += prof->end - prof->start;
		prof->totalLoadBytes += prof->dbytes;
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfCacheMiss ( ProfileData *prof )
{
	prof->misses++;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfCacheHit ( ProfileData *prof )
{
	prof->hits++;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfCacheAddPage ( ProfileData *prof )
{
	prof->pagesUsed++;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfCacheRemovePage ( ProfileData *prof )
{
	prof->pagesUsed--;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfCacheFill ( ProfileData *prof, int bytes )
{
	prof->cacheUsed += bytes;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfCacheRemove ( ProfileData *prof, int bytes )
{
	prof->cacheUsed -= bytes;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfCacheText ( ProfileData *prof, void ( *print) ( char *text ) )
{
	char buf[1024];
	int used;
	int filled;

	print ("Audio Cache Stats\n");
	sprintf( buf, "Hits: %d%%\n", prof->HitPercent );
	print ( buf );

	if ( prof->numPages )
	{
		used = (prof->pagesUsed *100)/prof->numPages;
	}
	else
	{
		used = 0;
	}

	if ( prof->pagesUsed * prof->pageSize )
	{
		filled = (prof->cacheUsed *100)/ (prof->pagesUsed * prof->pageSize );
	}
	else
	{
		filled = 0;
	}

	sprintf( buf, "Used: %d%% (%d%%)\n", used, filled );
	print ( buf );

	sprintf( buf, "KbPS: %d.%02d (%d.%02d,%d.%02d)\n", 
							prof->TotalBytesPerSecond/1024, ((prof->TotalBytesPerSecond%1024)*100)/1024,
							prof->LoadBytesPerSecond/1024, ((prof->LoadBytesPerSecond%1024)*100)/1024,
							prof->DecompBytesPerSecond/1024, ((prof->DecompBytesPerSecond%1024)*100)/1024);

	print ( buf );

	sprintf( buf, "KPF: %d.%02d\n", 
							prof->BytesPerFrame/1024, ((prof->BytesPerFrame%1024)*100)/1024 );

	print ( buf );

	sprintf( buf, " LF: %d.%02ds; %d pages; %d Kb\n", 
							(int) (prof->longestFrame/prof->freq), (int)(((prof->longestFrame % prof->freq)*100)/prof->freq),
							prof->longestFramePages, prof->longestFrameBytes/1024 );

	print ( buf );

	sprintf( buf, "NLF: %d.%02ds; %d pages; %d Kb\n", 
							(int) (prof->nextLongestFrame/prof->freq), (int) (((prof->nextLongestFrame % prof->freq)*100)/prof->freq),
							prof->nextLongestFramePages, prof->nextLongestFrameBytes/1024 );

	print ( buf );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfCacheUpdateInterval ( ProfileData *prof, int mseconds )
{
	prof->update = (prof->freq * mseconds )/ 1000;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static ProfStamp calc_ticks ( ProfStamp start, ProfStamp end )
{
	if ( start < end )
	{
		return end - start;
	}

	return ((ProfStamp)0xffffffffffffffff) - start + end ;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static void calc_stats ( ProfileCPU &prof )
{

	if ( prof.lastCPU )
	{
		prof.cpuUsage = (int) ((prof.lastTicks*((ProfStamp)1000))/prof.lastCPU);
	}
	else
	{
		prof.cpuUsage = 0;
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfileCPUInit ( ProfileCPU &prof )
{

	memset ( &prof, 0, sizeof ( ProfileCPU ));

	if ( !QueryPerformanceFrequency( (LARGE_INTEGER*) &prof.freq ))
	{
		prof.freq = 0;
	}
	prof.updateInterval = SECONDS(1);

	if ( prof.freq	)
	{
		prof.overflowInterval = SECONDS ( (((ProfStamp) 0xffffffffffffffff) / prof.freq) );

		if ( prof.overflowInterval < prof.updateInterval )
		{
			prof.updateInterval =  prof.overflowInterval;
		}
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfileCPUDeinit ( ProfileCPU &prof )
{
	prof;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfileCPUStart ( ProfileCPU &prof )
{
	
	if ( prof.state == PROF_STATE_IDLE )
	{
		QueryPerformanceCounter( (LARGE_INTEGER*)&prof.start );
		
		if ( prof.lastStart )
		{
			prof.totalCPUTicks += calc_ticks ( prof.lastStart, prof.start );
		}
		
		prof.lastStart = prof.start;
		prof.state = PROF_STATE_PROFILING;
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfileCPUPause ( ProfileCPU &prof )
{

	if ( prof.state == PROF_STATE_PROFILING )
	{
		ProfStamp end;

		QueryPerformanceCounter( (LARGE_INTEGER*) &end );

		prof.totalTicks += calc_ticks ( prof.start, end );
		prof.state = PROF_STATE_PAUSED;
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfileCPUResume ( ProfileCPU &prof )
{
	if ( prof.state == PROF_STATE_PAUSED )
	{
		QueryPerformanceCounter( (LARGE_INTEGER*) &prof.start );
		prof.state = PROF_STATE_PROFILING;
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfileCPUEnd ( ProfileCPU &prof )
{
	ProfStamp end;

	QueryPerformanceCounter( (LARGE_INTEGER*) &end );

	if ( prof.state != PROF_STATE_IDLE )
	{
		if ( prof.start && prof.totalCPUTicks )
		{
			prof.totalTicks += calc_ticks ( prof.start, end );
		}
		prof.state = PROF_STATE_IDLE;
	}

	TimeStamp now = AudioGetTime();
	TimeStamp lastUpdate = now - prof.lastUpdate;

	if ( lastUpdate > prof.updateInterval )
	{
		prof.lastUpdate = now;

		if ( lastUpdate < prof.overflowInterval )
		{
			// we can use the data
			prof.lastTicks = prof.totalTicks;
			prof.lastCPU = prof.totalCPUTicks;
			calc_stats ( prof );
		}

		prof.totalTicks = 0;
		prof.totalCPUTicks = 0;
		
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int ProfileCPUUsage ( ProfileCPU &prof )
{
	return prof.cpuUsage;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int ProfileCPUTicks ( ProfileCPU &prof )
{
	prof;
	return 0;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfileCPUSetName ( ProfileCPU &prof, const char *name )
{
	strncpy ( prof.name, name, MAX_PROF_NAME );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void ProfileCPUPrint ( ProfileCPU &prof, void ( *print) ( char *text ) )
{
	char buffer[200];

	if ( prof.freq )
	{
		sprintf ( buffer, "%s : CPU %3d.%1d / %I64d", prof.name, prof.cpuUsage/10, prof.cpuUsage%10, prof.lastTicks );
	}
	else
	{
		sprintf ( buffer, "%s : CPU (no timer)" );
	}
	print ( buffer );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/


#endif

