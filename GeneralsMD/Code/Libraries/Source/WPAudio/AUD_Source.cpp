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
** File name:  <filename>                                                   **
**                                                                          **
** Created by:  04/05/96 TR                                                 **
**                                                                          **
** Description: <description>                                               **
**                                                                          **
*****************************************************************************/

/*****************************************************************************
**            Includes                                                      **
*****************************************************************************/

#include <memory.h>
#include <stdio.h>
#include <string.h>

#include <wpaudio/altypes.h>
#include <wpaudio/memory.h>
#include <wpaudio/time.h>
#include <wpaudio/source.h>
#include <wsys/file.h>

//#include "asimp3\mss.h"
//#include "asimp3\mp3dec.h"

// 'assignment within condition expression'.
#pragma warning(disable : 4706)


DBG_DECLARE_TYPE ( AudioSample )
DBG_DECLARE_TYPE ( AudioFormat )
DBG_DECLARE_TYPE ( AudioFrame )

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

//
// Sample rate in samples/second for [MPEG25][MPEG version][value]
//

static  const int sample_rate[2][2][4] =
   {{
      { 22050L,24000L,16000L,22050L },
      { 44100L,48000L,32000L,44100L }
   },
   {
      { 11025L,12000L, 8000L,11025L },
      { 44100L,48000L,32000L,44100L }
   }};

//
// Bit rate in bits/second for [MPEG version][value]
//

static const int bit_rate[2][15] =
{
   { 0L,8000L,16000L,24000L,32000L,40000L,48000L,56000L,64000L,80000L,96000L,112000L,128000L,144000L,160000L         }
   ,
   { 0L,32000L,40000L,48000L,56000L,64000L,80000L,96000L,112000L,128000L,160000L,192000L,224000L,256000L,320000L     }
};




/*****************************************************************************
**         Public Data                                                      **
*****************************************************************************/

const short MSADPCM_StdCoef[7][2] = {
			{ 256,   0},
			{ 512,-256},
			{   0,   0},
			{ 192,  64},
			{ 240,   0},
			{ 460,-208},
			{ 392,-232}
};


/*****************************************************************************
**         Private Prototypes                                               **
*****************************************************************************/



/*****************************************************************************
**          Private Functions                                               **
*****************************************************************************/


/*****************************************************************************
**          Public Functions                                                **
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioSample* AudioCreateSample( uint bytes )
{
	AudioSample	*sample;


	DBG_ASSERT ( bytes > 0 );


	ALLOC_STRUCT ( sample, AudioSample );

	AudioSampleInit ( sample );

	sample->Bytes = bytes;

	if ( ! ( sample->Data = ( char *) AudioMemAlloc (  sample->Bytes ) ))
	{
		goto error;
	}


    return sample;

error:

	if ( sample )
	{
		AudioSampleDestroy ( sample );
	}

	return NULL;
}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		AudioSampleDestroy ( AudioSample *sample )
{

	DBG_ASSERT_TYPE ( sample, AudioSample );

	if ( sample->Data )
	{
		AudioMemFree ( sample->Data );
	}

	AudioMemFree ( sample );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		AudioSampleInit ( AudioSample *sample )
{

	DBG_ASSERT ( sample != NULL );
	DBG_SET_TYPE ( sample, AudioSample );
	sample->Data = NULL;
	sample->Bytes = 0;
	sample->Format = NULL;
	sample->Attribs = NULL;
	ListInit ( &sample->Frames );

	#ifdef _DEBUG
	sample->name[0] = 0;
	#endif
}

#ifndef IG_FINAL_RELEASE

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void AudioSampleSetName ( AudioSample *sample, const char *orig_name )
{
	int diff;
	char *buffer = NULL;
	char *name;

	DBG_ASSERT_TYPE ( sample, AudioSample );

	buffer = (char *) AudioMemAlloc ( strlen ( orig_name ) + 1);
	name = buffer;
	sample->name[0] = 0;

	if ( name )
	{
		char *ptr;

		strcpy ( name, orig_name );

		ptr  = strrchr ( name, '.' );

		if ( ptr )
		{
			*ptr = 0;
		}
		
		ptr  = strrchr ( name, '\\' );

		if ( ptr )
		{
			name = ptr + 1;
		}
		
		if ( ( diff = (strlen ( name ) - (sizeof(sample->name) - 1))) > 0 )
		{
			strcpy ( sample->name, "...");	
			name = name + diff + 3;
			strcat ( sample->name, name );
		}
		else
		{
			strcpy ( sample->name, name );
		}
	}

	if ( buffer )
	{
		AudioMemFree ( buffer );
	}
}

#endif

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		AudioSampleAddFrame ( AudioSample *sample, AudioFrame *frame )
{


	DBG_ASSERT_TYPE ( sample, AudioSample );
	DBG_ASSERT_TYPE ( frame, AudioFrame );

	ListAddToTail ( &sample->Frames, &frame->nd );

	sample->Bytes += frame->Bytes;
	frame->sample = sample;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioFrame*	AudioSampleFirstFrame ( AudioSample *sample )
{

	DBG_ASSERT_TYPE ( sample, AudioSample );

	return (AudioFrame *) ListNodeNext ( &sample->Frames );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		AudioSampleDeinit ( AudioSample *sample )
{

	DBG_ASSERT_TYPE ( sample, AudioSample );

	DBG_INVALIDATE_TYPE ( sample );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		AudioFormatBytes ( AudioFormat *format, TimeStamp time  )
{

	DBG_ASSERT_TYPE ( format, AudioFormat );
	return (int) ( ((TimeStamp)format->BytesPerSecond*time)/(TimeStamp)SECONDS(1));
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

TimeStamp		AudioFormatTime ( AudioFormat *format, int bytes  )
{

	DBG_ASSERT_TYPE ( format, AudioFormat );

	if ( format->BytesPerSecond )
	{
		return (((TimeStamp)bytes)*(TimeStamp)SECONDS(1))/(TimeStamp)format->BytesPerSecond;
	}

	return 0;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		AudioFormatInit ( AudioFormat *format )
{

	DBG_ASSERT ( format != NULL );

	memset ( format, 0, sizeof ( AudioFormat));

	DBG_SET_TYPE ( format, AudioFormat );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		AudioFormatUpdate ( AudioFormat *format )
{

	DBG_ASSERT_TYPE ( format, AudioFormat );
	DBG_ASSERT ( format->Channels != 0 );
	DBG_ASSERT ( format->SampleWidth >0 );
	DBG_ASSERT ( format->Rate >0 );
	DBG_ASSERT ( format->Compression < AUDIO_COMPRESS_MAX_ID );

	if ( format->Compression != AUDIO_COMPRESS_MP3 )
	{
		format->BytesPerSecond = format->Channels*format->SampleWidth*format->Rate;
		if ( format->Compression == AUDIO_COMPRESS_IMA_ADPCM 
				|| format->Compression == AUDIO_COMPRESS_MS_ADPCM )
		{
			format->BytesPerSecond >>= 2;	//  4:1 compression 
		}
	}
	else
	{
		int mpeg1 =   (W_BitsGet ( format->cdata.mp3.Header, 1, 19 ));
		int bitrateindex = W_BitsGet ( format->cdata.mp3.Header, 4, 12 );
		format->BytesPerSecond = bit_rate[mpeg1][bitrateindex] / 8;
	}	
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		AudioFrameInit ( AudioFrame *frame, void *data, int bytes )
{

	DBG_ASSERT_PTR ( frame );

	DBG_SET_TYPE ( frame, AudioFrame );

	frame->Bytes = bytes;
	frame->Data = data;
	ListNodeInit ( &frame->nd );
	frame->sample = NULL;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioFrameDeinit ( AudioFrame *frame )
{

	DBG_ASSERT_TYPE ( frame, AudioFrame );

	frame->sample = NULL;

	DBG_INVALIDATE_TYPE ( frame );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

#define MAX_SYNC_SEARCH (10*1024)		// max bytes that will be searched for the sync 

int AudioFormatReadMP3File ( File *file, AudioFormat *format, int *datasize )
{
	unsigned char buffer[MAX_SYNC_SEARCH+3];
	int bytes;
	int pos;
	unsigned int	header = 0;
	int bitrateindex = 0;	// initialize to prevent compiler warning
	int sampling_frequency = 0;	// initialize to prevent compiler warning
	int layer;

	int data_start = file->position ();

	
	bytes = file->read ( buffer, sizeof (buffer ));

	pos = 0;

	while ( !header && bytes >= 4 )
	{
		unsigned char  mask = 0xFF;
		// find next sync
		while ( bytes >= 3 )
		{
			if ( (buffer[pos]&mask) == mask )
			{
				if ( mask == 0xE0 )
				{
					pos--;
					bytes++;
					//header = (buffer[pos] << 24) || (buffer[pos+1] << 16) || (buffer[pos+2] << 8) || buffer[pos+3];
					header = MAKEID ( buffer[pos], buffer[pos+1], buffer[pos+2], buffer[pos+3]);
					break;
				}
				mask = 0xE0;
			}
			else
			{
				mask = 0xFF;
			}
			pos++;
			bytes--;
		}

		// validate the header
		bitrateindex = W_BitsGet ( header, 4, 12 );
		sampling_frequency = W_BitsGet ( header, 2, 10 );
		layer =   (W_BitsGet ( header, 2, 17 ));
		if ( bitrateindex == 0x0f || sampling_frequency == 0x03 || layer != 1)
		{
			header = 0;
			pos++;
			bytes--;
			continue;
		}
	}

	if (!header )
	{
		return FALSE;
	}

	int mpeg25 = !(W_BitsGet ( header, 1, 20 ));
	int mpeg1 =   (W_BitsGet ( header, 1, 19 ));
	int mode =    (W_BitsGet ( header, 2, 6 ));

	format->Compression = AUDIO_COMPRESS_MP3;
	format->SampleWidth = 2;
	format->Channels = (mode == 3) ? 1 : 2;
	format->BytesPerSecond = bit_rate[mpeg1][bitrateindex] / 8;
	format->Rate = sample_rate[mpeg25][mpeg1][sampling_frequency];
	format->Flags = mAUDIO_FORMAT_PCM;
	format->cdata.mp3.Header = header;

	AudioFormatUpdate ( format );


	if ( datasize )
	{
		*datasize = file->size() - (data_start + pos) ;
	}

	file->seek ( data_start+pos, File::START );

	return TRUE;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			AudioFormatSeekToPos ( File *file, const AudioFormat *format, int pos, int data_start )
{
	if ( pos )
	{
		int block_size = 0;

		if ( format->Compression == AUDIO_COMPRESS_MP3 )
		{	
			AudioFormat tformat;
			file->seek ( pos+data_start, File::START );

			AudioFormatInit ( &tformat );
			
			int found = FALSE;
			 
			while ( !found )
			{
				if ( !AudioFormatReadMP3File ( file, &tformat, NULL ) )
				{
					break;
				}

				if ( AudioFormatSame ( &tformat, format ) )
				{
					found = TRUE;
					break;
				}

				file->seek ( 1, File::CURRENT );
			}

			if ( !found )
			{
				pos = 0;
			}
			else
			{
				pos = file->seek ( 0, File::CURRENT ) - data_start ;
			}
		}
		else
		{

			switch ( format->Compression )
			{
				case AUDIO_COMPRESS_NONE:
					block_size = format->Channels*format->SampleWidth;
					break;
				case AUDIO_COMPRESS_IMA_ADPCM:
				case AUDIO_COMPRESS_MS_ADPCM:
					block_size = format->cdata.adpcm.BlockSize;
					break;
			}

			if ( block_size > 1 )
			{
				pos = ( pos / block_size ) *block_size;
			}
		}
	}

	file->seek (  pos  + data_start, File::START );

	return pos;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int AudioFormatSame ( const AudioFormat *f1, const AudioFormat *f2 )
{
	if (			(f1->Rate != f2->Rate )
				||	(f1->Compression != f2->Compression )
				||	(f1->SampleWidth != f2->SampleWidth )
				||	(f1->Channels != f2->Channels )
				||	(f1->Flags != f2->Flags ) )
	{
		return FALSE;
	}

	if ( f1->Compression == AUDIO_COMPRESS_IMA_ADPCM && (f1->cdata.adpcm.BlockSize != f2->cdata.adpcm.BlockSize))
	{
		return FALSE;		
	}

	if ( f1->Compression == AUDIO_COMPRESS_MP3 && (f1->cdata.mp3.Header != f2->cdata.mp3.Header))
	{
		return FALSE;		
	}


	return TRUE;
}