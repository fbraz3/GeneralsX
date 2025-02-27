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
** File name:  audplay.cpp                                                  **
**                                                                          **
** Created by:  04/??/00 TR                                                 **
**                                                                          **
** Description: <description>                                               **
**                                                                          **
*****************************************************************************/

/*****************************************************************************
**            Includes                                                      **
*****************************************************************************/

#include <stdio.h>
#include <string.h>

#include <wpaudio/altypes.h>
#include <wpaudio/Streamer.h>
#include <wpaudio/StreamBuffering.h>
#include <wpaudio/source.h>
#include <wsys/file.h>
#include <wpaudio/device.h>
#include <wpaudio/channel.h>
#include <wpaudio/thread.h>

// 'assignment within condition expression'.
#pragma warning(disable : 4706)

DBG_DECLARE_TYPE ( AudioStreamer );

/*****************************************************************************
**          Externals                                                       **
*****************************************************************************/



/*****************************************************************************
**           Defines                                                        **
*****************************************************************************/

#define AS_PLAYING				0x00000001
#define AS_PAUSED					0x00000002
#define AS_FILL						0x00000004		// service thread should fill this stream
#define AS_NO_FILE_CLOSE	0x00000080		// file was not opened by us
#define AS_OPEN						0x00000010		// we have a file to play
#define AS_LOOPING				0x00000020		// seamlessly loop stream

#define MIN_FRAME_SIZE (4*1024)

/*****************************************************************************
**        Private Types                                                     **
*****************************************************************************/

typedef struct AudioStreamerTag
{
						ListNode					nd;
	volatile	int								flags;
						AudioDevice				*device;
						AudioChannel			*channel;
						TimeStamp					buffering;
						TimeStamp					end_time_stamp;
						TimeStamp					start_time_stamp;
						STM_STREAM				*stream;
						STM_ACCESS				*in;
						STM_ACCESS				*out;
						AudioSample				sample;
						AudioFormat				format;
						File							*file;
						Lock							pause;
						Lock							lock;
						int								pending_bytes;		// number of bytes submitted to audio playback
						int								total_bytes;
						int								data_start;
						int								bytes_left;
						int								frame_size;
						
						int								stream_pos;
						int								max_volume;
						
						char							stream_name[80];
						
						#ifndef IG_FINAL_RELEASE
						char							name[200];
						#endif

						DBG_TYPE()

} AudioStreamer;



/*****************************************************************************
**         Private Data                                                     **
*****************************************************************************/

static AUD_Thread	*thread = NULL;
static ListHead	streams;
static int initialized = FALSE;

/*****************************************************************************
**         Public Data                                                      **
*****************************************************************************/



/*****************************************************************************
**         Private Prototypes                                               **
*****************************************************************************/

static int	streamNextFrame ( AudioChannel *channel );
static int	streamSampleDone ( AudioChannel *channel );
static int	streamStop ( AudioChannel *channel );
static void	service_stream ( AudioStreamer *as );
static AUD_ThreadCB		service_streams;
static int recalcBuffering ( AudioStreamer *as, TimeStamp buffering, AudioFormat *format );

/*****************************************************************************
**          Private Functions                                               **
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int	streamNextFrame ( AudioChannel *channel )
{
	AudioStreamer *as = (AudioStreamer*) channel->Data;

	DBG_ASSERT_TYPE ( as, AudioStreamer );

	STM_AccessAdvance ( as->out, as->pending_bytes );	// mark these bytes as having been used
	as->stream_pos += as->pending_bytes;
	if ( as->stream_pos > as->total_bytes )
	{
		as->stream_pos %= as->total_bytes;
	}

	STM_AccessGetBlock ( as->out );		

	if ( as->frame_size > as->out->Block.Bytes )
	{
		as->pending_bytes = as->out->Block.Bytes;
	}
	else
	{
		as->pending_bytes = as->frame_size;
	}
	
	channel->bytesInFrame = as->pending_bytes;
	channel->bytesRemaining = channel->bytesInFrame;
	channel->frameData = (char *) as->out->Block.Data;

	return vNO_ERROR;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int	streamSampleDone ( AudioChannel *channel )
{
	return streamStop ( channel );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int	streamStop ( AudioChannel *channel )
{
	AudioStreamer *as = (AudioStreamer*) channel->Data;

	DBG_ASSERT_TYPE ( as, AudioStreamer );

	if ( as->flags & (AS_PLAYING|AS_PAUSED))
	{
		as->end_time_stamp = AudioGetTime();
	}
	FLAGS_CLEAR ( as->flags, AS_PLAYING | AS_PAUSED | AS_FILL );
	LockInit ( &as->pause );
	return vNO_ERROR;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int streamFill ( AudioStreamer *as, TimeStamp amount )
{
	int bytes_to_fill;
	int transfered = 0;	// initialize to prevent compiler warning
	int bytes;
	int result;
	int looping;

	DBG_ASSERT_TYPE ( as, AudioStreamer );

	AUD_ThreadBeginCriticalSection ( thread );

	looping = ( as->flags & AS_LOOPING ) ;

	if ( amount )
	{
		bytes_to_fill = AudioFormatBytes ( &as->format, amount );
	}
	else
	{
		bytes_to_fill = (int) STM_AccessTotalBytes ( as->in );
	}

	while ( bytes_to_fill )
	{
		if ( (bytes = bytes_to_fill) > as->bytes_left )
		{
			bytes = as->bytes_left;
		}

		result = STM_AccessFileTransfer ( as->in, as->file, bytes, &transfered );

		as->bytes_left -= transfered;
		bytes_to_fill -= transfered;

		if ( as->bytes_left <= 0 || result == STM_EOF || result == STM_FAIL )
		{
			if ( looping )
			{
				// seek to start of stream and continue filling
				as->file->seek ( as->data_start, File::START );
				as->bytes_left = as->total_bytes;
			}
			else
			{
				FLAGS_CLEAR ( as->flags, AS_FILL );
				as->bytes_left = 0;
				bytes_to_fill = 0;
			}
		}
	}

	AUD_ThreadEndCriticalSection ( thread );
	return transfered;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static void service_stream ( AudioStreamer *as )
{
		DBG_ASSERT_TYPE ( as, AudioStreamer );

		if ( ! (as->flags & AS_FILL ) )
		{
			return;
		}

		streamFill ( as, 0 );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int service_streams ( AUD_Thread *thread, void *data )
{
	AudioStreamer *as;
	ListHead *list = (ListHead *) data;

	if ( list )
	{
		as = (AudioStreamer*) ListFirstItem ( list );
			
		while ( as )
		{
			service_stream ( as );
		
			as = (AudioStreamer*) ListNextItem ( &as->nd );
		}
	}

	return TRUE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int recalcBuffering ( AudioStreamer *as, TimeStamp buffering, AudioFormat *format )
{
	uint size;
	STM_STREAM *stm;

	DBG_ASSERT_TYPE ( as, AudioStreamer );

	stm = as->stream;

	DBG_ASSERT ( stm );
	DBG_ASSERT ( format );


	size = (uint) AudioFormatBytes ( format, buffering );
	as->frame_size = AudioFormatBytes ( format, AudioChannelFrameTime ( as->channel )*2 );
	if ( as->frame_size < MIN_FRAME_SIZE )
	{
		as->frame_size = MIN_FRAME_SIZE;
	}

	if ( STM_StreamTotalBytes ( stm ) != size )
	{
		STM_SBUFFER *buffer;

		STM_StreamDestroyBuffers ( stm );

		buffer = STM_SBufferCreate ( size, 0 );

		if ( buffer )
		{
			STM_StreamAddSBuffer ( stm, buffer );
		}
	}

	return STM_StreamTotalBytes ( stm ) == size;
}

/*****************************************************************************
**          Public Functions                                                **
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		AudioStreamerInit ( void )
{
	ListInit ( &streams );


	thread = AUD_ThreadCreate ( "Streamer Thread", AUD_THREAD_PRI_HIGH, service_streams );

	if ( thread )
	{
		AUD_ThreadSetInterval ( thread, SECONDS(1) );
		AUD_ThreadSetData ( thread, &streams );
	}

	initialized = TRUE;

	return TRUE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioStreamerDeinit ( void )
{
	AudioStreamer *as;

	if ( !initialized )
	{
		return ;
	}

	msg_assert ( ListIsEmpty ( &streams), ("Not all streams were destroyed"));

	while ( as = (AudioStreamer*) ListFirstItem ( &streams ))
	{
		AudioStreamerStop ( as );
		AudioStreamerDestroy ( as );
	}

	if ( thread )
	{
		AUD_ThreadDestroy ( thread );
		thread = NULL;
	}

	initialized = FALSE;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioStreamer* AudioStreamerCreate ( AudioDevice *dev, TimeStamp buffering )
{
	AudioStreamer *as;

	if ( !dev || !initialized )
	{
		return NULL;
	}

	if ( buffering <=0 || buffering < SECONDS(7))
	{
		buffering = SECONDS(7);
	}

	ALLOC_STRUCT ( as, AudioStreamer );

	DBG_SET_TYPE ( as, AudioStreamer );

	ListNodeInit ( &as->nd );
	as->device = dev;
	as->channel = AudioDeviceReserveChannel ( dev, AUDIO_CHANNEL_TYPE_USER );
	as->buffering = buffering;
	AudioSampleInit ( &as->sample );
	AudioFormatInit ( &as->format );
	LockInit ( &as->pause );
	LockInit ( &as->lock );
	strcpy ( as->stream_name, "Audio Stream" );


	if ( !as->channel )
	{
		msg_assert ( FALSE, ("Unable to reserve channel for streamer"));
		goto error;
	}

	as->max_volume = AUDIO_VOLUME_MAX;

	as->channel->GroupAttribs = NULL;
	as->channel->SfxAttribs = NULL;
	as->channel->CompAttribs = NULL;
	as->channel->FadeAttribs = NULL;
	as->channel->Data = (void *) as;
	as->channel->CB_NextFrame = streamNextFrame;
	as->channel->CB_SampleDone = streamSampleDone;
	as->channel->CB_Stop = streamStop;

	as->stream = STM_StreamCreate ( );

	if ( !as->stream )
	{
		goto error;
	}

	AUD_ThreadBeginCriticalSection ( thread );
	ListNodeAppend ( &streams, &as->nd );
	AUD_ThreadEndCriticalSection ( thread );

	return as;
error:

	AudioStreamerDestroy ( as );
	return NULL;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerDestroy ( AudioStreamer *as )
{

	DBG_ASSERT_TYPE ( as, AudioStreamer );

	msg_assert ( !(as->flags & AS_PLAYING), ("Trying to destroy as stream while is is still playing"));

	if ( ListNodeInList ( &as->nd ))
	{
		AUD_ThreadBeginCriticalSection ( thread );
		ListNodeRemove ( &as->nd );
		AUD_ThreadEndCriticalSection ( thread );
	}

	AudioStreamerClose ( as );

	if ( as->in )
	{
		STM_AccessRelease ( as->in );
		as->in = NULL;
	}

	if ( as->out )
	{
		STM_AccessRelease ( as->out );
		as->out = NULL;
	}

	if ( as->stream )
	{
		STM_StreamDestroy ( as->stream );
	}

	if ( as->channel )
	{
		AudioChannelRelease ( as->channel );
	}

	if ( as->file )
	{
		if ( ! (as->flags & AS_NO_FILE_CLOSE ))
		{
			as->file->close() ;
		}
		as->file = NULL;
	}

	DBG_INVALIDATE_TYPE ( as );

	MEM_Free ( as );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerSetAttribs ( AudioStreamer *as, AudioAttribs *attribs )
{

	DBG_ASSERT_TYPE ( as, AudioStreamer );
	
	as->channel->GroupAttribs = attribs;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerSetFadeAttribs ( AudioStreamer *as, AudioAttribs *attribs )
{

	DBG_ASSERT_TYPE ( as, AudioStreamer );
	
	as->channel->FadeAttribs = attribs;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int						AudioStreamerOpen ( AudioStreamer *as, File *file)
{
	
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	if ( Locked ( &as->lock ) || !file )
	{
		return FALSE;
	}

	AUD_ThreadBeginCriticalSection ( thread );

	AudioStreamerClose ( as );

	FLAGS_SET ( as->flags, AS_NO_FILE_CLOSE ); // we did not open this file so do not close it
	as->file = file;

	if ( !AudioFormatReadWaveFile ( as->file, &as->format, &as->total_bytes ))
	{
		goto error;
	}

	strncpy ( as->name, file->getName(), ArrayLen ( as->name ) );
	ArrayEnd ( as->name );
	AudioSampleSetName ( &as->sample, as->name );
	as->data_start = as->file->position();
	as->stream_pos = 0;

	FLAGS_SET ( as->flags, AS_OPEN );

	AUD_ThreadEndCriticalSection ( thread );
	return TRUE;

error:

	if ( as->file )
	{
		as->file = NULL;
	}

	FLAGS_CLEAR ( as->flags, AS_OPEN );

	AUD_ThreadEndCriticalSection ( thread );
	return FALSE;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int						AudioStreamerOpen ( AudioStreamer *as, const char *filename )
{
	File *file;
	int opened;

	file = TheFileSystem->open ( (char *)filename, File::READ|File::BINARY );

	if ( !file )
	{
		return FALSE;
	}

	opened = AudioStreamerOpen ( as, file );

	if ( !opened )
	{
		file->close();
		return FALSE;
	}

	// we did open this file so automatically close when stream is closed
	FLAGS_CLEAR ( as->flags, AS_NO_FILE_CLOSE );
	return TRUE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int						AudioStreamerStart ( AudioStreamer *as )
{
	TimeStamp time;
	
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	if ( Locked ( &as->lock ) || !(as->flags & AS_OPEN ))
	{
		return FALSE;
	}

	AUD_ThreadBeginCriticalSection ( thread );
	as->stream_pos = AudioFormatSeekToPos ( as->file, &as->format, as->stream_pos, as->data_start );
	as->bytes_left = as->total_bytes - as->stream_pos;

	if ( as->in )
	{
		STM_AccessRelease ( as->in );
		as->in = NULL;
	}

	if ( as->out )
	{
		STM_AccessRelease ( as->out );
		as->out = NULL;
	}

	if ( !recalcBuffering ( as, as->buffering, &as->format ) )
	{
		goto error;
	}

	STM_StreamReset ( as->stream);

	as->in = STM_StreamAcquireAccess ( as->stream, vSTM_ACCESS_ID_IN );
	as->out = STM_StreamAcquireAccess ( as->stream, vSTM_ACCESS_ID_OUT );

	if ( !as->in || !as->out )
	{
		goto error;
	}

	time = max ( AudioChannelBufferTime ( as->channel )*2, AUD_ThreadGetInterval(thread)*4);
	time = max ( time, (TimeStamp)SECONDS(4));

	if ( !streamFill ( as, time ))
	{
		goto error;
	}

	STM_AccessGetBlock ( as->out );


	as->sample.Data = (char *) as->out->Block.Data;
	as->sample.Bytes = AudioFormatBytes ( &as->format, AudioChannelFrameTime ( as->channel ));
	if ( as->sample.Bytes > as->out->Block.Bytes )
	{
		as->sample.Bytes = as->out->Block.Bytes ;
	}
	as->sample.Format = &as->format;
	as->pending_bytes = as->sample.Bytes;

	//AudioAttribsSetVolume ( &as->channel->ChannelAttribs, AUDIO_VOLUME_MAX );
	AudioChannelSetSample ( as->channel, &as->sample );

	FLAGS_SET ( as->flags, AS_PLAYING );

	if ( AudioChannelStart ( as->channel ) != vNO_ERROR )
	{
		FLAGS_CLEAR ( as->flags, AS_PLAYING );
		STM_AccessRelease ( as->in );
		as->in = NULL;
		STM_AccessRelease ( as->out );
		as->out = NULL;
		goto error;
	}

	as->start_time_stamp = AudioGetTime();
	as->end_time_stamp = as->start_time_stamp;
	FLAGS_SET ( as->flags, AS_FILL );

	AUD_ThreadEndCriticalSection ( thread );
	return TRUE;

error:

	AUD_ThreadEndCriticalSection ( thread );
	return FALSE;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerStop ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	if ( NotLocked ( &as->lock ) )
	{
		if ( as->channel )
		{
			AUD_ThreadBeginCriticalSection ( thread );
			AudioChannelStop ( as->channel );
			AUD_ThreadEndCriticalSection ( thread );
		}
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerClose ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	if ( NotLocked ( &as->lock ) )
	{
		AUD_ThreadBeginCriticalSection ( thread );
		AudioStreamerStop ( as );

		if ( as->file )
		{
			if ( ! (as->flags & AS_NO_FILE_CLOSE ))
			{
				as->file->close();
			}
			as->file = NULL;
		}

		FLAGS_CLEAR ( as->flags, AS_OPEN | AS_NO_FILE_CLOSE | AS_LOOPING ) ;
		AUD_ThreadEndCriticalSection ( thread );
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerLock ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	LockAcquire ( &as->lock );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerUnlock ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	LockRelease ( &as->lock );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerPause ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	if ( NotLocked ( &as->pause ))
	{
		if ( as->channel )
		{
			AUD_ThreadBeginCriticalSection ( thread );
			AudioChannelPause ( as->channel );
			if ( as->flags & AS_PLAYING )
			{
				FLAGS_SET ( as->flags, AS_PAUSED );
				FLAGS_CLEAR ( as->flags, AS_PLAYING );
			}
			AUD_ThreadEndCriticalSection ( thread );
		}
	}

	LockAcquire ( &as->pause );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerResume ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	if ( Locked ( &as->pause ) )
	{
		LockRelease ( &as->pause );

		if ( NotLocked ( &as->pause ))
		{
			if ( as->channel )
			{
				AUD_ThreadBeginCriticalSection ( thread );
				if ( as->flags & AS_PAUSED )
				{
					FLAGS_CLEAR ( as->flags, AS_PAUSED );
					FLAGS_SET ( as->flags, AS_PLAYING );
				}
				AudioChannelResume ( as->channel );
				AUD_ThreadEndCriticalSection ( thread );
			}
		}
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		AudioStreamerIsPlaying ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as , AudioStreamer );

	return as->flags & AS_PLAYING ;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		AudioStreamerActive ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as , AudioStreamer );

	return as->flags & (AS_PLAYING|AS_PAUSED) ;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		AudioStreamerSetName ( AudioStreamer *as, const char *name )
{
	DBG_ASSERT_TYPE ( as , AudioStreamer );

	strncpy ( as->stream_name, name, sizeof(as->stream_name));
	as->stream_name[ sizeof(as->stream_name) -1 ] = 0;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

const char *AudioStreamerName ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as , AudioStreamer );

		return as->stream_name;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioStreamerFadeIn ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	AudioAttribsAdjustVolume ( &as->channel->ChannelAttribs, as->max_volume );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioStreamerFadeOut ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	AudioAttribsAdjustVolume ( &as->channel->ChannelAttribs, AUDIO_VOLUME_MIN );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int	AudioStreamerIsFading ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	return AudioStreamerIsPlaying ( as ) && !AudioAttribsVolumeAdjusted ( &as->channel->ChannelAttribs );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioStreamerWaitForFade ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );
	TimeStamp time_out = AudioGetTime() + as->channel->ChannelAttribs.VolumeLevel.duration;

	while ( AudioStreamerIsFading ( as ) && (AudioGetTime() < time_out) )
	{
		//ServiceAllAudio (); TODO: trolfs
		break;
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

TimeStamp			AudioStreamerEndTimeStamp ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	return as->end_time_stamp;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void AudioStreamerStarve ( void )
{
	AudioStreamer *as;

	if ( !initialized )
	{
		return;
	}

	as = (AudioStreamer*) ListFirstItem ( &streams );
	
	AUD_ThreadBeginCriticalSection ( thread );

	while ( as )
	{
		FLAGS_CLEAR ( as->flags, AS_FILL );
		as = (AudioStreamer*) ListNextItem ( &as->nd );
	}

	AUD_ThreadEndCriticalSection ( thread );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void AudioStreamerStopAllStreams ( void )
{
	AudioStreamer *as;

	if ( !initialized )
	{
		return;
	}

	AUD_ThreadBeginCriticalSection ( thread );

	as = (AudioStreamer*) ListFirstItem ( &streams );

	while ( as )
	{
		AudioStreamerStop ( as );
		as = (AudioStreamer*) ListNextItem ( &as->nd );
	}
	AUD_ThreadEndCriticalSection ( thread );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void AudioStreamerCloseAllStreams ( void )
{
	AudioStreamer *as;

	if ( !initialized )
	{
		return;
	}

	as = (AudioStreamer*) ListFirstItem ( &streams );

	while ( as )
	{
		AudioStreamerClose ( as );
		as = (AudioStreamer*) ListNextItem ( &as->nd );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerCloseAllStreamsUsingFile ( File *file )
{
	AudioStreamer *as;

	if ( !initialized )
	{
		return;
	}

	as = (AudioStreamer*) ListFirstItem ( &streams );

	while ( as )
	{
		if ( as->file == file )
		{
			AudioStreamerClose ( as );
		}
		as = (AudioStreamer*) ListNextItem ( &as->nd );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerLockAllStreams ( void )
{
	AudioStreamer *as;

	if ( !initialized )
	{
		return;
	}

	as = (AudioStreamer*) ListFirstItem ( &streams );

	while ( as )
	{
		AudioStreamerLock ( as );
		as = (AudioStreamer*) ListNextItem ( &as->nd );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerUnlockAllStreams ( void )
{
	AudioStreamer *as;

	if ( !initialized )
	{
		return;
	}

	as = (AudioStreamer*) ListFirstItem ( &streams );

	while ( as )
	{
		AudioStreamerUnlock ( as );
		as = (AudioStreamer*) ListNextItem ( &as->nd );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerPauseAllStreams ( void )
{
	AudioStreamer *as;

	if ( !initialized )
	{
		return;
	}

	as = (AudioStreamer*) ListFirstItem ( &streams );

	while ( as )
	{
		AudioStreamerPause ( as );
		as = (AudioStreamer*) ListNextItem ( &as->nd );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerResumeAllStreams ( void )
{
	AudioStreamer *as;

	if ( !initialized )
	{
		return;
	}

	as = (AudioStreamer*) ListFirstItem ( &streams );

	while ( as )
	{
		AudioStreamerResume ( as );
		as = (AudioStreamer*) ListNextItem ( &as->nd );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerFadeOutAllStreams ( void )
{
	AudioStreamer *as;

	if ( !initialized )
	{
		return;
	}

	as = (AudioStreamer*) ListFirstItem ( &streams );

	while ( as )
	{
		AudioStreamerFadeOut ( as );
		as = (AudioStreamer*) ListNextItem ( &as->nd );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerFadeInAllStreams ( void )
{
	AudioStreamer *as;

	if ( !initialized )
	{
		return;
	}

	as = (AudioStreamer*) ListFirstItem ( &streams );

	while ( as )
	{
		AudioStreamerFadeIn ( as );
		as = (AudioStreamer*) ListNextItem ( &as->nd );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int					AudioStreamerAllFaded ( void )
{
	AudioStreamer *as;

	if ( !initialized )
	{
		return true;
	}

	as = (AudioStreamer*) ListFirstItem ( &streams );

	while ( as )
	{
		if ( AudioStreamerIsFading ( as ) )
		{
			return FALSE;
		}
		as = (AudioStreamer*) ListNextItem ( &as->nd );
	}

	return TRUE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int						AudioStreamerGetPosition ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	return as->stream_pos;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerSetPosition ( AudioStreamer *as, int pos  )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	int playing = AudioStreamerIsPlaying ( as );

	AudioStreamerStop ( as );

	as->stream_pos = pos;

	if ( playing )
	{
		AudioStreamerStart ( as );
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

TimeStamp			AudioStreamerGetTimePosition ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	if ( as->flags & AS_OPEN )
	{
		return AudioFormatTime ( &as->format, as->stream_pos );
	}

	return 0;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerSetTimePosition ( AudioStreamer *as, TimeStamp time  )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	if ( as->flags & AS_OPEN )
	{
		AudioStreamerSetPosition ( as, AudioFormatBytes ( &as->format, time ));
	}
	else
	{
		AudioStreamerSetPosition ( as, 0 );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerSetVolume ( AudioStreamer *as, int volume )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	AudioChannelSetVolume ( as->channel, volume );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int						AudioStreamerGetVolume ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	return AudioChannelGetVolume ( as->channel );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerSetMaxVolume ( AudioStreamer *as, int volume )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	if ( volume > AUDIO_VOLUME_MAX )
	{
		volume = AUDIO_VOLUME_MAX;
	}
	else if ( volume < AUDIO_VOLUME_MIN )
	{
		volume = AUDIO_VOLUME_MIN;
	}

	as->max_volume = volume;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int						AudioStreamerGetMaxVolume ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	return as->max_volume;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerSetLooping ( AudioStreamer *as, int loop )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );
	
	AUD_ThreadBeginCriticalSection ( thread );

	if ( loop )
	{
		FLAGS_SET ( as->flags, AS_LOOPING );
	}
	else
	{
		FLAGS_CLEAR ( as->flags, AS_LOOPING );
	}

	AUD_ThreadEndCriticalSection ( thread );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int						AudioStreamerIsLooping ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	return as->flags & AS_LOOPING;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int						AudioStreamerIsAudible ( AudioStreamer *as )
{
	DBG_ASSERT_TYPE ( as, AudioStreamer );

	return AudioChannelIsAudible ( as->channel );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioStreamerSetPriority ( AudioStreamer *as, Int priority )
{
}

#define vRANGE 15

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static void streamDump ( AudioStreamer *as, int index, void (*print) ( char *text) )
{
	int range = vRANGE;
	int in;
	int total;
	int percent;
	char buffer[200];
	uint out_pos;
	uint in_pos;

	if ( !as->stream )
	{
		sprintf ( buffer, "%02d: Invalid Stream\n", index);
		print ( buffer );
		return;
	}	

	total = STM_StreamTotalBytes ( as->stream );
	in = 0;
	out_pos = 0;
	in_pos = 0;
	percent = 0;

	if ( total )
	{
		in = (STM_StreamTotalBytesIn ( as->stream )*range)/total;

		if ( as->out )
		{
			out_pos = (STM_AccessPosition ( as->out ) * range)/total;
		}

		if ( as->in )
		{
			in_pos = (STM_AccessPosition ( as->in ) * range)/total;
		}

		percent = (STM_StreamTotalBytesIn ( as->stream )*100)/total;

	}

	if ( out_pos == in_pos )
	{
		if ( in == 0)
		{
			if ( out_pos < vRANGE-1 )
			{
				in_pos = (out_pos + 1);
			}
			else
			{
				in_pos = 0;
			}
		}
		else
		{
			if ( out_pos > 0 )
			{
				in_pos = (out_pos - 1);
			}
			else
			{
				in_pos = vRANGE-1	;
			}
		}
	}

	char gauge[vRANGE+1];
	uint i;

	for ( i =0; i < vRANGE; i++)
	{
		gauge[i] = ' ';
	}

	gauge[vRANGE] = 0;

	if ( out_pos > in_pos )
	{
		for ( i =out_pos+1; i < vRANGE; i++)
			gauge[i] = '*';

		for ( i =0; i < in_pos; i++)
			gauge[i] = '*';
	}
	else
	{
		for ( i =out_pos+1; i < in_pos; i++)
			gauge[i] = '*';
	}

	gauge[in_pos] = '>';
	gauge[out_pos] = '<';

	char name[sizeof(as->name)+1];
	char *ptr ;
	int seconds, minutes;
	strcpy ( name, as->name );

	ptr = strrchr ( name, '.' );
	if ( ptr )
	{
		*ptr = 0;
	}

	ptr = strrchr ( name, '\\' );
	if ( ptr )
	{
		strcpy ( name, ptr );
	}

	TimeStamp time = AudioFormatTime ( &as->format, as->total_bytes - as->stream_pos );

	minutes = (int) IN_MINUTES(time);
	time -= MINUTES(minutes);
	seconds = (int) IN_SECONDS(time);
	time -= SECONDS(seconds);
	time = (int) IN_MSECONDS(time);



	sprintf ( buffer, "%.12s %4d Kb %.31s \n", AudioStreamerName( as ), total/1024, name);
	print ( buffer );

	sprintf ( buffer, " Status: %s %s %s %d:%02d:%03d \n",  
					as->flags & AS_PLAYING ? "Play  " : (as->flags & AS_PAUSED ? "Paused" : "Stop  ") ,
					as->flags & AS_FILL ? "Fill" : "    ", 
					Locked( &as->lock ) ? "Locked" : "      ", 
					minutes, seconds, time );
	print ( buffer );

	sprintf ( buffer, " Buffer: |%s| %3d %%\n", gauge, percent );
	print ( buffer );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioStreamerDump ( void (*print) ( char *text ))
{
	AudioStreamer *as;
	char buffer[200];
	char temp[100];
	int index = 0;
	
	if ( !initialized )
	{
		print ("\nAudio Streamer not initialized\n" );
		return ;
	}

	print ("\nAudio Streamer\n" );

	AudioServiceInfoDump ( AUD_ThreadServiceInfo (thread), temp );
	sprintf ( buffer, "service:%s\n", temp );
	print (buffer );

	as = (AudioStreamer*) ListFirstItem ( &streams );

	if ( as )
	{
		while ( as )
		{
			streamDump ( as , index++, print);
			as = (AudioStreamer*) ListNextItem ( &as->nd );
		}
	}
	else
	{
		print ("No streams created\n" );
	}

}

