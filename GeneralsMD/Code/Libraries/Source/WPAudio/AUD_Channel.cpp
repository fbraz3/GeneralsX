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
** File name:  audchanl.cpp                                                 **
**                                                                          **
** Created by:  11/16/96 TR                                                 **
**                                                                          **
** Description: <description>                                               **
**                                                                          **
*****************************************************************************/

/*****************************************************************************
**            Includes                                                      **
*****************************************************************************/

#include <string.h>

#include <wpaudio/altypes.h>
#include <wpaudio/memory.h>
#include <wpaudio/list.h>
#include <wpaudio/channel.h>
#include <wpaudio/device.h>

// 'assignment within condition expression'.
#pragma warning(disable : 4706)

DBG_DECLARE_TYPE ( AudioChannel)

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

static		int		audioChannelSampleDone ( AudioChannel *chan );
static		int		audioChannelNextSample ( AudioChannel *chan );
static		int		audioChannelNextFrame ( AudioChannel *chan );

/*****************************************************************************
**          Private Functions                                               **
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioChannel*		audioChannelCreate ( AudioDevice *dev )
{
	AudioChannel	*chan = NULL;


	DBG_ASSERT_TYPE ( dev , AudioDevice );


	ALLOC_STRUCT ( chan, AudioChannel );

	DBG_SET_TYPE ( chan, AudioChannel );

	ListNodeInit ( &chan->nd );
	AudioControlInit ( &chan->Control );
	AudioAttribsInit ( &chan->attribs );
	audioChannelMakeStandard ( chan );		//  set up for standard processing 

	chan->Device = dev;
	chan->driver = dev->driver;

	chan->current_format = dev->DefaultFormat;
	chan->format_changed = FALSE;
	chan->drv_format_changed = FALSE;
	#ifndef IG_FINAL_RELEASE
		chan->sample_name[0] = 0;
	#endif

	if ( dev->driver->openChannel ( chan ) != vNO_ERROR )
	{
		DBGPRINTF (("Audio driver failed to open a new channel\n"));
		goto error;
	}

  return chan;

error:

	if ( chan )
	{
		audioChannelDestroy ( chan );
	}

	return NULL;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		audioChannelDestroy ( AudioChannel *chan )
{

	DBG_ASSERT_TYPE ( chan, AudioChannel );

	chan->driver->closeChannel ( chan );

	AudioMemFree ( chan );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			audioChannelMakeStandard ( AudioChannel *chan )
{

	DBG_ASSERT_TYPE ( chan, AudioChannel );

	//  reset callbacks 
	chan->drvCBNextFrame = (AudioChannelCB *) audioChannelNextFrame;
	chan->drvCBNextSample = (AudioChannelCB *) audioChannelNextSample;
	chan->drvCBSampleDone = (AudioChannelCB *) audioChannelSampleDone;
	chan->CB_NextFrame = NULL;
	chan->CB_NextSample = NULL;
	chan->CB_SampleDone = NULL;
	chan->CB_Stop = NULL;
	chan->Data = NULL;
	chan->Type = AUDIO_CHANNEL_TYPE_STD;

	//  reset control 
	AudioControlInit ( &chan->Control );
	chan->Control.Priority = AUD_NORMAL_PRIORITY;

	//  reset attribs 
	chan->GroupAttribs = audioStdChannelAttribs;
	chan->SfxAttribs = NULL;
	chan->CompAttribs = NULL;
	chan->FadeAttribs = NULL;
	AudioAttribsInit ( &chan->ChannelAttribs );

	//  reset sequencer 

	//  clear audio 
	chan->sample = NULL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		audioChannelRecalcAttribs ( AudioChannel *chan )
{
	AudioDevice *dev;



	DBG_ASSERT_TYPE ( chan, AudioChannel );

	dev = (AudioDevice *) chan->Device;

	DBG_ASSERT_TYPE ( dev, AudioDevice );


 	AudioAttribsInit ( &chan->attribs );
 	AudioAttribsApply ( &chan->attribs, &chan->ChannelAttribs );
	AudioAttribsUsed ( &chan->ChannelAttribs );

 	if ( chan->SfxAttribs )
 	{
 		AudioAttribsApply ( &chan->attribs, chan->SfxAttribs );
		AudioAttribsUsed ( chan->SfxAttribs );
 	}

 	if ( chan->GroupAttribs )
 	{
 		AudioAttribsApply ( &chan->attribs, chan->GroupAttribs );
 	}

 	if ( chan->CompAttribs )
 	{
 		AudioAttribsApply ( &chan->attribs, chan->CompAttribs );
 	}

 	if ( chan->FadeAttribs )
 	{
 		AudioAttribsApply ( &chan->attribs, chan->FadeAttribs );
 	}

 	AudioAttribsApply ( &chan->attribs, &dev->Attribs );

	if ( dev->GroupAttribs )
	{
 		AudioAttribsApply ( &chan->attribs, dev->GroupAttribs );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		int		audioChannelNextFrame ( AudioChannel *chan )
{
	int  err;


	DBG_ASSERT ( chan != NULL );

	if ( chan->CB_NextFrame )
	{
		//  user is controlling frame access 
		if ( (err = chan->CB_NextFrame ( chan )) != vNO_ERROR )
		{
			//  there was an error 
			DBGPRINTF(("Frame error:\n"));
			chan->bytesInFrame = 0;
			chan->bytesRemaining = 0;
			chan->frameData = NULL;
			return err;
		};

		DBG_ASSERT ( chan->bytesInFrame >= 0 );
		DBG_ASSERT ( chan->bytesRemaining >= 0 );

		return vNO_ERROR;

	}

	//  user is not controlling frame access so we handle the default behaviour 

	DBG_ASSERT ( chan->bytesRemaining >= 0 );	//  something corrupted this field 

	if ( chan->frame )
	{
		if (  ( chan->frame = (AudioFrame*) ListNodeNext ( &chan->frame->nd )) )
		{
			//  there is more data to be got 
			chan->frameData = (char *) chan->frame->Data;
			chan->bytesInFrame = chan->frame->Bytes;
			chan->bytesRemaining = chan->bytesInFrame;
			DBG_MSGASSERT ( chan->frame->sample == chan->sample, ("frame does not belong to sample") );
			return vNO_ERROR;
		}

	}

	//  data has been exhausted 

	chan->bytesInFrame = 0;		//  no more frames, drvCBNextSample() will be called 

	return vNO_ERROR;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		int		audioChannelNextSample ( AudioChannel *chan )
{


	DBG_ASSERT ( chan != NULL );

	if (chan->Control.LoopCount)
	{
		if ( chan->Control.LoopCount != AUDIO_CTRL_LOOP_FOREVER )
		{
			chan->Control.LoopCount--;
		}
		AudioChannelSetSample ( chan, chan->sample );
	}
	else
	{
		AudioChannelSetSample ( chan, NULL );
	}

	if ( chan->CB_NextSample)
	{
		chan->CB_NextSample ( chan );
	}


	if ( chan->sample )
	{
		chan->driver->queueIt ( chan );
	}

	return vNO_ERROR;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		int		audioChannelSampleDone ( AudioChannel *chan )
{

	//  reset channel back to default 
	chan->sample = NULL;
	FLAGS_CLEAR ( chan->Control.Status, mAUDIO_CTRL_PLAYING|mAUDIO_CTRL_PAUSED );
	chan->Control.LoopCount = 0;
 	AudioAttribsInit ( &chan->ChannelAttribs );

	#ifndef IG_FINAL_RELEASE
		chan->sample_name[0] = 0;
	#endif


	if ( chan->CB_SampleDone )
	{
		return chan->CB_SampleDone ( chan );
	}

	return vNO_ERROR;
}

/*****************************************************************************
**          Public Functions                                                **
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			AudioChannelTaken ( AudioChannel *chan )
{

	DBG_ASSERT ( audioInitialized ); //  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( chan, AudioChannel );

	return chan->Control.Status & mAUDIO_CTRL_ALLOCATED;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		AudioChannelReserve ( AudioChannel *chan, AudioChannelType type )
{

	DBG_ASSERT ( audioInitialized ); //  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( chan, AudioChannel );

	DBG_ASSERT ( chan->Type == AUDIO_CHANNEL_TYPE_STD ); 	//  you can only reserve standard channels 

	chan->Type = type;

	AudioChannelStop ( chan );
	FLAGS_SET ( chan->Control.Status, mAUDIO_CTRL_ALLOCATED );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioChannelRelease ( AudioChannel *chan )
{

	DBG_ASSERT ( audioInitialized ); //  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( chan, AudioChannel );

	if ( chan->Control.Status & mAUDIO_CTRL_ALLOCATED)
	{
		FLAGS_CLEAR ( chan->Control.Status, mAUDIO_CTRL_ALLOCATED );
		chan->Type = AUDIO_CHANNEL_TYPE_STD;
		audioChannelMakeStandard ( chan );
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioChannelDestroy ( AudioChannel *chan)
{

	DBG_ASSERT ( audioInitialized ); //  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( chan, AudioChannel );

	AudioChannelStop ( chan ); //  stop anything that is playing 

	audioRemoveChannel ( chan );

	audioChannelDestroy ( chan );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		AudioChannelStop ( AudioChannel *chan )
{



	DBG_ASSERT ( audioInitialized ); //  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( chan, AudioChannel );

	chan->driver->lock ( chan);
	if ( chan->Control.Status & (mAUDIO_CTRL_PLAYING|mAUDIO_CTRL_PAUSED) )
	{
		chan->driver->stop ( chan );
		DBG_ASSERT ( !(chan->Control.Status & (mAUDIO_CTRL_PLAYING|mAUDIO_CTRL_PAUSED)) );
	}
	chan->driver->unlock ( chan );

	if ( chan->CB_Stop )
	{
		chan->CB_Stop ( chan );
	}

	chan->sample = NULL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioChannelPause ( AudioChannel *chan )
{

	DBG_ASSERT ( audioInitialized ); //  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( chan, AudioChannel );

	chan->driver->lock ( chan );
	if ( chan->Control.Status & mAUDIO_CTRL_PLAYING )
	{
		FLAGS_CLEAR (chan->Control.Status, mAUDIO_CTRL_PLAYING);
		chan->driver->pause ( chan );
		FLAGS_SET (chan->Control.Status, mAUDIO_CTRL_PAUSED);
	}
	chan->driver->unlock ( chan );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioChannelResume ( AudioChannel *chan )
{

	DBG_ASSERT ( audioInitialized ); //  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( chan, AudioChannel );

	chan->driver->lock ( chan );
	if ( chan->Control.Status & mAUDIO_CTRL_PAUSED )
	{
		chan->driver->resume ( chan );
		FLAGS_CLEAR (chan->Control.Status, mAUDIO_CTRL_PAUSED);
		FLAGS_SET (chan->Control.Status, mAUDIO_CTRL_PLAYING);
	}
	chan->driver->unlock ( chan );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioChannelLock ( AudioChannel *chan )
{

	DBG_ASSERT ( audioInitialized ); //  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( chan, AudioChannel );

	chan->driver->lock ( chan );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioChannelUnlock ( AudioChannel *chan )
{

	DBG_ASSERT ( audioInitialized ); //  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( chan, AudioChannel );

	chan->driver->unlock ( chan );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioChannelUse ( AudioChannel *chan )
{

	DBG_ASSERT ( audioInitialized ); //  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( chan, AudioChannel );

	chan->driver->lock ( chan );
 	FLAGS_SET ( chan->Control.Status, mAUDIO_CTRL_INUSE);
	chan->driver->unlock ( chan );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioChannelNoUse ( AudioChannel *chan )
{

	DBG_ASSERT ( audioInitialized ); //  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( chan, AudioChannel );

	chan->driver->lock ( chan );
 	FLAGS_CLEAR ( chan->Control.Status, mAUDIO_CTRL_INUSE);
	chan->driver->unlock ( chan );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		AudioChannelStart ( AudioChannel *chan )
{
	int	err = ERROR_CODE_FAIL;




	DBG_ASSERT ( audioInitialized ); //  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( chan, AudioChannel );

	if ( (chan->Control.Status & (mAUDIO_CTRL_PAUSED | mAUDIO_CTRL_PLAYING)))
	{
		DBGPRINTF (("Failed to start channel as channel is currently active\n"));
		return ERROR_CODE_NO_CHANNEL;
	}
	if ( chan->sample == NULL )
	{
		DBGPRINTF (("Failed to start channel as no sample data was given\n"));
		return ERROR_CODE_MISSING_DATA;
	}

	AudioAttribsUpdate ( &chan->ChannelAttribs );
	audioChannelRecalcAttribs ( chan );

	chan->driver->lock ( chan );
	if ( (err = chan->driver->start ( chan )) == vNO_ERROR )
	{
		FLAGS_SET ( chan->Control.Status, mAUDIO_CTRL_PLAYING);
	}
	chan->driver->unlock ( chan );

	return err;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioChannelSetSample ( AudioChannel *chan, AudioSample *sample )
{

	DBG_ASSERT ( audioInitialized ); //  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( chan, AudioChannel );

	if ( (chan->sample = sample) )
	{
	 	DBG_ASSERT_TYPE ( sample, AudioSample);

		#ifndef IG_FINAL_RELEASE
			strncpy ( chan->sample_name, sample->name, sizeof(chan->sample_name));
			chan->sample_name[ sizeof(chan->sample_name) -1 ] = 0;
		#endif											  

		if ( chan->frame = AudioSampleFirstFrame ( sample ) )
		{
			chan->frameData = (char *) chan->frame->Data;
			chan->bytesInFrame = chan->frame->Bytes;
			chan->bytesRemaining = chan->bytesInFrame;
			sample->Data = NULL;
			DBG_MSGASSERT ( chan->frame->sample == sample, ("frame does not belong to sample") );
		}
		else
		{
			chan->frameData = chan->sample->Data;
			chan->bytesInFrame = chan->sample->Bytes;
			chan->bytesRemaining = chan->bytesInFrame;
		}
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			AudioChannelSetFormat ( AudioChannel *chan, AudioFormat *new_format )
{


	if ( !memcmp ( new_format, &chan->current_format, sizeof ( AudioFormat )))
	{
		chan->format_changed = FALSE;
	}
	else
	{
		chan->format_changed = TRUE;
		chan->current_format = *new_format;
	}

	return chan->format_changed;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int				AudioChannelIsAudible ( AudioChannel *chan )
{
		return (chan->Control.Status & mAUDIO_CTRL_PLAYING);
}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioControlInit ( AudioControl *ctrl )
{
 ctrl->LoopCount = 0;
 ctrl->Status = 0;
}
