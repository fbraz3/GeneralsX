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
** File name:  auddev.cpp                                                   **
**                                                                          **
** Created by:  11/16/95   - TR																							**
**                                                                          **
** Description: <description>                                               **
**                                                                          **
*****************************************************************************/

/*****************************************************************************
**            Includes                                                      **
*****************************************************************************/

#include <wpaudio/altypes.h>
#include <stdio.h>
#include <string.h>

#include <wpaudio/memory.h>
#include <wpaudio/list.h>
#include <wpaudio/lock.h>
#include <wpaudio/time.h>
#include <wpaudio/attributes.h>
#include <wpaudio/device.h>
#include <wpaudio/channel.h>

// 'assignment within condition expression'.
#pragma warning(disable : 4706)


DBG_DECLARE_TYPE ( AudioDevice )
DBG_DECLARE_TYPE ( AudioSystem )
DBG_DECLARE_TYPE ( AudioSearchStruct )
DBG_DECLARE_TYPE ( AudioServiceInfoStruct )

/*****************************************************************************
**          Externals                                                       **
*****************************************************************************/



/*****************************************************************************
**           Defines                                                        **
*****************************************************************************/

#define AUDIO_DEFAULT_SERVICE_INTERVAL		(SECONDS(1)/30)
#define VOLUME_SLACK											(AUDIO_LEVEL_MAX/10)

/*****************************************************************************
**        Private Types                                                     **
*****************************************************************************/

// AudioDataStruct:
//	data that interrupt driven code needs to access is place in the struct

typedef struct
{
	ListHead    	systemList;
	ListHead    	devList;
	Lock					devListAccess;
	AudioAttribs	stdChannelAttribs;

} AudioData;

typedef struct
{
	ListNode			nd;
	AudioAttribs	*attribs;

	DBG_TYPE()

} AudioAttribsNode;

DBG_DECLARE_TYPE ( AudioAttribsNode )


/*****************************************************************************
**         Private Data                                                     **
*****************************************************************************/

static		AudioData			audioDataStruct;
static		AudioData			*audioData = &audioDataStruct;
static		int						audioExclusiveSystemLoaded = FALSE;
					int						audioInitialized = FALSE;

/*****************************************************************************
**         Public Data                                                      **
*****************************************************************************/

AudioAttribs		*audioStdChannelAttribs = NULL;

/*****************************************************************************
**         Private Prototypes                                               **
*****************************************************************************/

static 		AudioDevice*		audioCreateDevice ( AudioSystem *system, int unit, AudioFormat *format );
static		void            audioDestroyDevice ( AudioDevice *idev );
static		void            audioDeviceAttribsListUpdate ( AudioDevice *idev );
static		void            audioDeviceAttribsListUsed ( AudioDevice *idev );
static		AudioAttribsNode*       audioDeviceAttribsListFind ( AudioDevice *idev, AudioAttribs * attribs);
static		void            audioAddDevice ( AudioDevice *dev );
static		void            audioRemoveDevice ( AudioDevice  *dev );
static		AudioSystem*     audioCreateSystem ( AudioSystemMaster *master );
static		void     				audioDestroySystem ( AudioSystem *system );
static		void            audioAddSystem ( AudioSystem *system );
static		void            audioRemoveSystem ( AudioSystem *system );
static		void						audioMapDeviceUnit ( int unit, AudioSystem **system, int *systemUnit );
static		AudioDeviceCB 	audioDeviceHandler;

/*****************************************************************************
**          Private Functions                                               **
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void            audioDeviceAttribsListUpdate ( AudioDevice *idev )
{
	AudioAttribsNode *node = ( AudioAttribsNode *) ListFirstItem ( &idev->attribsList );

	while ( node )
	{
		DBG_ASSERT_TYPE ( node, AudioAttribsNode );

		AudioAttribsUpdate ( node->attribs );

		node = (AudioAttribsNode *) ListNodeNext ( &node->nd );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void            audioDeviceAttribsListUsed ( AudioDevice *idev )
{
	AudioAttribsNode *node = ( AudioAttribsNode *) ListFirstItem ( &idev->attribsList );

	while ( node )
	{
		DBG_ASSERT_TYPE ( node, AudioAttribsNode );

		AudioAttribsUsed ( node->attribs );

		node = (AudioAttribsNode *) ListNodeNext ( &node->nd );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		AudioAttribsNode*       audioDeviceAttribsListFind ( AudioDevice *idev, AudioAttribs * attribs)
{
	AudioAttribsNode *node = ( AudioAttribsNode *) ListFirstItem ( &idev->attribsList );

	while ( node )
	{
		DBG_ASSERT_TYPE ( node, AudioAttribsNode );

		if ( node->attribs == attribs )
		{
			return node;
		}

		node = (AudioAttribsNode *) ListNodeNext ( &node->nd );
	}
	return NULL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static 	AudioDevice*      audioCreateDevice ( AudioSystem *system, int unit, AudioFormat *format )
{
	AudioDevice  *dev = NULL;
	AudioSystemMaster *master = system->master;


	//  create and initialise dev struct 

	ALLOC_STRUCT ( dev, AudioDevice );

	//  initialise driver structure 
	DBG_SET_TYPE ( dev, AudioDevice );

  ListNodeInit ( &dev->nd );
  dev->systemUnit = unit;
  dev->system = system;
	LockInit ( &dev->lock );
	LockAcquire ( &system->lock );				//  mark system as being used 
  system->unit[unit] = dev;
	dev->driver = master->driver;
	ListInit ( &dev->channelList);
	ListInit ( &dev->attribsList);
	AudioAttribsInit ( &dev->Attribs );
	LockInit ( &dev->channelAccess );
	dev->deviceHandler = audioDeviceHandler ;
	AudioServiceInfoInit ( & dev->attribsUpdate );
	AudioServiceSetInterval ( &dev->attribsUpdate, AUDIO_DEFAULT_SERVICE_INTERVAL );
	AudioServiceSetMustServiceInterval ( &dev->attribsUpdate, AUDIO_DEFAULT_SERVICE_INTERVAL*5 );
	AudioServiceSetResetInterval ( &dev->attribsUpdate, AUDIO_DEFAULT_SERVICE_INTERVAL*5 );
	AudioServiceInfoInit ( & dev->mixerUpdate );
	AudioServiceSetInterval ( &dev->mixerUpdate, AUDIO_DEFAULT_SERVICE_INTERVAL );
	AudioFormatInit ( &dev->DefaultFormat );	//  default sample sormat 
	AudioFormatInit ( &dev->Format );

	if ( format )
	{
		AudioDeviceSetDefaultFormat ( dev, format );
	}

  //  initialise device driver 
  if ( master->open ( dev ) != vNO_ERROR )
  {
  	goto error;
  }

  return dev;


error:    

	if (dev)
	{
        audioDestroyDevice ( dev );
	}

    return NULL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static 	void            audioDestroyDevice ( AudioDevice *dev )
{

	DBG_ASSERT_TYPE ( dev, AudioDevice );

	AudioDeviceAttribsRemoveAll ( dev );

	AudioDeviceDestroyAllChannels ( dev, AUDIO_CHANNEL_TYPE_ALL );

  dev->system->master->close ( dev );

  dev->system->unit[dev->systemUnit] = NULL;

	LockRelease ( &dev->system->lock );

  AudioMemFree ( dev );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static 	void            audioAddDevice ( AudioDevice *dev )
{
	LockAcquire ( &audioData->devListAccess );
  ListAddToTail ( &audioData->devList, &dev->nd );
	LockRelease ( &audioData->devListAccess );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static 	void            audioRemoveDevice ( AudioDevice  *dev )
{
	LockAcquire ( &audioData->devListAccess );
	ListNodeRemove ( &dev->nd );
	LockRelease ( &audioData->devListAccess );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		AudioSystem*     audioCreateSystem ( AudioSystemMaster *master )
{
	AudioSystem 		*system = NULL;


	ALLOC_STRUCT ( system, AudioSystem );

	DBG_SET_TYPE ( system, AudioSystem );

	ListNodeInit ( &system->nd );
	LockInit ( &system->lock );
	system->master = master;

	if ( master->load ( system ) != vNO_ERROR )
	{
		goto error;
	}

	return system;

error:

	if ( system )
	{
		audioDestroySystem ( system );
	}

	return NULL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void   		audioDestroySystem ( AudioSystem *system )
{

	system->master->unload ( system );

	FLAGS_CLEAR (system->master->flags, mAUDIO_SYSTEM_MASTER_LOADED);

	AudioMemFree ( system );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void		audioAddSystem ( AudioSystem *system )
{
	ListAddToTail ( &audioData->systemList, &system->nd );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void		audioRemoveSystem ( AudioSystem *system )
{
	ListNodeRemove ( &system->nd );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		audioAddChannel ( AudioDevice *dev, AudioChannel *chan )
{

	chan->Device = dev;
	dev->channels++;

	LockAcquire ( &dev->channelAccess );

	ListAddToTail ( &dev->channelList, &chan->nd );

	LockRelease ( &dev->channelAccess );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		audioRemoveChannel ( AudioChannel *chan )
{
	AudioDevice *dev = (AudioDevice *) chan->Device;

	ListNodeRemove ( &chan->nd );
	dev->channels--;


	LockAcquire ( &dev->channelAccess );

	ListNodeRemove ( &chan->nd );

	LockRelease ( &dev->channelAccess );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void		audioMapDeviceUnit ( int unit, AudioSystem **pSystem, int *pUnit )
{
	AudioSystem	*system;
	AudioSearch	sh;

	system = AudioFirstSystem ( &sh );

	while ( system )
	{
		if ( unit < system->numUnits )
		{
			break;
		}

		unit -= system->numUnits;
		system = AudioNextSystem ( &sh );
	}

	if ( (*pSystem = system) )
	{
		*pUnit = unit;	//  we found it 
	}
	else
	{
		*pUnit = -1;	//  no such unit 
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		int	audioDeviceHandler ( AudioDevice *dev )
{
	AudioChannel		*chan,
					*next,
					*head;
	int			change = FALSE;
	TimeStamp			now;

	if (!NotLocked(&dev->channelAccess ))
	{
		//  channel lists are being accessed so we can do nothing at this time 
		return ERROR_CODE_IN_USE;
	}

	now = AudioGetTime ();

	if ( !AudioServiceNeeded ( &dev->attribsUpdate, now ) )
	{
		return vNO_ERROR;
	}

	AudioServicePerform ( &dev->attribsUpdate, now );

	audioDeviceAttribsListUpdate ( dev );
	AudioAttribsUpdate ( &dev->Attribs );

	if ( !dev->GroupAttribs || !(change = AudioAttribsChanged( dev->GroupAttribs )))
	{
		change = AudioAttribsChanged( &dev->Attribs ) ;
	}


	chan = head = (AudioChannel *) &dev->channelList;
	next = (AudioChannel *) chan->nd.next;


	while ( (chan = next) != head )
	{
		next = (AudioChannel *) chan->nd.next;

		if ( ( chan->Control.Status & mAUDIO_CTRL_PLAYING ))
		{
			//  update channel attributes 

			if ( chan->sample )
			{
				AudioAttribsUpdate ( &chan->ChannelAttribs );

				if ( change || AudioAttribsChanged (&chan->ChannelAttribs) ||
						(chan->GroupAttribs && AudioAttribsChanged ( chan->GroupAttribs )) ||
						(chan->CompAttribs && AudioAttribsChanged ( chan->CompAttribs )) ||
						(chan->FadeAttribs && AudioAttribsChanged ( chan->FadeAttribs )) ||
						(chan->SfxAttribs && AudioAttribsChanged ( chan->SfxAttribs )) )
				{
					audioChannelRecalcAttribs ( chan );
					chan->driver->update ( chan );
				}
			}
		}
	}

	AudioAttribsUsed ( &dev->Attribs );
	audioDeviceAttribsListUsed ( dev );


	return vNO_ERROR;
}

/*****************************************************************************
**          Public Functions                                                **
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int       AudioSetUp ( void )
{


	DBG_ASSERT ( !audioInitialized ); 	//  audio system was already initialized 

	DBGPRINTF (("Initializing audio module\n"));

	DBG_ASSERT ( audioData != NULL );

	InitAudioTimer();

	//  Initialize lists 
  ListInit ( &audioData->systemList );
  ListInit ( &audioData->devList );
	LockInit ( &audioData->devListAccess );

	audioStdChannelAttribs = &audioData->stdChannelAttribs;
	AudioAttribsInit ( audioStdChannelAttribs );

    //  Process tags 

  audioInitialized = TRUE;

	return vNO_ERROR;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void            AudioCloseDown ( void)
{


	if (!audioInitialized)
  {
    return;
  }


  AudioDestroyAllDevices ();

  AudioUnloadAllSystems ();

  DBGPRINTF (("WPAudio system has been shut down\n"));
	audioInitialized = FALSE;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioServiceAllDevices ( void  )
{
	AudioDevice *dev,*head;
	AudioData *data = audioData;


	if ( !audioInitialized || !NotLocked ( &data->devListAccess ))
	{
		return;
	}

	dev = head = (AudioDevice *) &data->devList;

	while ( (dev = (AudioDevice *) dev->nd.next) != head )
	{
		DBG_ASSERT ( dev->deviceHandler != NULL );
		if ( !(dev->flags & mAUDIO_DEV_DONT_SERVICE ))
		{
			dev->deviceHandler ( dev );
		}
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioSystem*		AudioLoadSystem ( AudioSystemMaster *master )
{
	AudioSystem		*system;


	DBG_ASSERT ( audioInitialized );		//  AudioSetUp() was not called 
	DBG_ASSERT ( master != NULL );
	DBG_ASSERT ( master->stamp == AUDIO_STAMP_SYSTEM_MASTER );

	DBG_ASSERT ( ! (master->flags & mAUDIO_SYSTEM_MASTER_LOADED ));	//  master was already loaded 

	DBG_ASSERT ( !audioExclusiveSystemLoaded);

	DBGPRINTF (("Loading driver system for %s\n", master->Name));
	if ( ! (system = audioCreateSystem ( master )))
	{
		goto error;
	}

	master->flags |= mAUDIO_SYSTEM_MASTER_LOADED;

	audioAddSystem ( system );

	if ( master->Properties & mAUDIO_SYS_PROP_EXCLUSIVE )
	{
		audioExclusiveSystemLoaded = TRUE;
	}

	return system;

error:
	if ( system)
	{
		audioDestroySystem ( system );
	}

	return NULL;
}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioUnloadSystem ( AudioSystem *system )
{


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 

	DBG_ASSERT_TYPE  ( system, AudioSystem );

	DBG_ASSERT ( !Locked ( &system->lock ));	//  trying to unload a system while it is still being used 

	audioRemoveSystem ( system );
	audioDestroySystem ( system );

	audioExclusiveSystemLoaded = FALSE;
}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioUnloadAllSystems ( void )
{

	AudioSystem		*sys,*next;
	AudioSearch		sh;


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 

	sys = AudioFirstSystem ( &sh );

	while (sys)
	{
		next = AudioNextSystem( &sh );

		AudioUnloadSystem ( sys );

		sys = next;
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioSystem*		AudioFirstSystem ( AudioSearch *sh )
{


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 

	DBG_SET_TYPE ( sh, AudioSearchStruct );
	sh->item = ListFirstItem ( &audioData->systemList );

	DBG_CODE
	(
		if ( sh->item )
		{
			DBG_ASSERT_TYPE ( (AudioSystem *) sh->item, AudioSystem );
		}
	)

	return (AudioSystem *) sh->item;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioSystem*		AudioNextSystem ( AudioSearch *sh )
{


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( sh, AudioSearchStruct );

	if ( sh->item )
	{
		sh->item = ListNextItem ( (ListNode*) sh->item );
	}

	DBG_CODE
	(
		if ( sh->item )
		{
			DBG_ASSERT_TYPE ( (AudioSystem *) sh->item, AudioSystem );
		}
	)

	return (AudioSystem *) sh->item;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			AudioNumberOfDevices ( void )
{
	int		devices = 0;
	AudioSystem	*system;
	AudioSearch	sh;


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 

	system = AudioFirstSystem ( &sh );

	while ( system )
	{
		devices += system->numUnits;
		system = AudioNextSystem ( &sh );
	}


	return devices;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioDevice*		AudioOpenDevice ( int unit, AudioFormat *format )
{

  AudioDevice *dev = NULL;
  AudioSystem	*system;
	int		systemUnit;


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 

	if ( unit == AUDIO_DEVICE_DEFAULT )
	{
		unit = 0;
	}

	//  map  unit number to system and system unit number 
	audioMapDeviceUnit ( unit, &system, &systemUnit );

	if ( !system )		//  there is no such unit 
	{
		DBGPRINTF (("No such unit\n"));
		goto error;
	}

    if ( !( dev = system->unit[systemUnit]))
    {
		//  first time opening this device 

		if ( ! (dev = audioCreateDevice ( system, systemUnit, format )))
		{
			DBGPRINTF (("Unable to create device\n"));
        	goto error;		//  failed to open the device driver 
		}

		dev->Unit = unit;

		audioAddDevice ( dev );		//  add it to the device list 

    }

    LockAcquire (&dev->lock);       //  mark as being used 



    return dev;


error:
	if ( dev )
	{
    	AudioDeviceClose ( dev );
	}

    return NULL;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioDeviceSetDefaultFormat ( AudioDevice *dev, AudioFormat *format )
{

	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );
	DBG_ASSERT ( format != NULL );

	memcpy ( &dev->DefaultFormat, format, sizeof ( AudioFormat ) );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioDevice*		AudioFirstDevice ( AudioSearch *sh )
{


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 

	DBG_SET_TYPE ( sh, AudioSearchStruct );
	sh->item = ListFirstItem ( &audioData->devList  );

	DBG_CODE
	(
		if ( sh->item )
		{
			DBG_ASSERT_TYPE ( (AudioDevice *) sh->item, AudioDevice );
		}
	)

	return (AudioDevice *) sh->item;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioDevice*		AudioNextDevice ( AudioSearch *sh )
{


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( sh, AudioSearchStruct );

	if ( sh->item )
	{
		sh->item = ListNextItem ( (ListNode*) sh->item );
	}

	DBG_CODE
	(
		if ( sh->item )
		{
			DBG_ASSERT_TYPE ( (AudioDevice *) sh->item, AudioDevice );
		}
	)

	return (AudioDevice *) sh->item;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioDestroyAllDevices ( void )
{
	AudioDevice		*dev,*next;
	AudioSearch		sh;


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 


	LockAcquire ( &audioData->devListAccess);

	dev = AudioFirstDevice ( &sh );

	while (dev)
	{
		next = AudioNextDevice( &sh );

		AudioDeviceDestroy ( dev );

		dev = next;
	}

	LockRelease ( &audioData->devListAccess);

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioDeviceClose ( AudioDevice *dev )
{

	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );

	LockRelease ( &dev->lock );

	if ( !Locked ( &dev->lock ))
	{
        AudioDeviceDestroy ( dev );
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioDeviceDestroy ( AudioDevice *dev )
{

	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );

	audioRemoveDevice ( dev );
  audioDestroyDevice ( dev );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		AudioDeviceSetFormat ( AudioDevice *dev, AudioFormat *format)
{

	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );
	DBG_ASSERT ( format );


	return ERROR_CODE_FAIL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioDeviceSetVolume ( AudioDevice *dev, int newVolume )
{

	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );

	AudioAttribsSetVolume ( &dev->Attribs, newVolume );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioDeviceAdjustVolume ( AudioDevice *dev, int newVolume )
{

	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );

	AudioAttribsAdjustVolume ( &dev->Attribs, newVolume );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			AudioDeviceMaxChannels ( AudioDevice *dev )
{

	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );

	return dev->maxChannels;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			AudioDeviceChannels ( AudioDevice *dev )
{

	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );


	return dev->channels;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioChannel*	AudioDeviceCreateChannel ( AudioDevice *dev )
{
	AudioChannel	*channel = NULL;


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );

	if ( dev->channels >= dev->maxChannels )
	{
	 	DBGPRINTF (("Maximum of %d channels already open\n", dev->maxChannels));
	 	goto error;
	}

	if ( !( channel = audioChannelCreate ( dev )) )
	{
		goto error;
	}


	audioAddChannel ( dev, channel );


	return channel;

error:

	if ( channel )
	{
		audioChannelDestroy ( channel );
	}

	return NULL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		AudioDeviceCreateChannels ( AudioDevice *dev, int num )
{
	int	count = 0;


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );

	DBG_ASSERT ( num != 0);

	do
	{

		if ( !AudioDeviceCreateChannel ( dev ))
		{
			break;
		}
		count ++;
	} while (--num);

	return count;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioChannel*	AudioDeviceFirstChannel ( AudioDevice *dev, AudioSearch *sh )
{

	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );
	DBG_ASSERT ( sh != NULL );

	DBG_SET_TYPE ( sh, AudioSearchStruct );

	sh->item = ListFirstItem ( &dev->channelList );


	if ( sh->item )
	{
		DBG_ASSERT_TYPE ( (AudioChannel *) sh->item, AudioChannel );
	}

	return (AudioChannel *) sh->item;
}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioChannel*	AudioDeviceNextChannel ( AudioSearch *sh )
{

	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( sh, AudioSearchStruct );

	if ( sh->item )
	{
		sh->item = ListNextItem ( (ListNode *) sh->item );
	}

	if ( sh->item )
	{
		DBG_ASSERT_TYPE ( (AudioChannel *) sh->item, AudioChannel );
	}


	return (AudioChannel *) sh->item;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioChannel*	AudioDeviceGetChannel ( AudioDevice *dev, AudioChannelType type )
{
	AudioChannel			*channel;
	AudioSearch	sh;
	int								lowestPriorityPlaying,
										lowestPriorityPaused;
	AudioChannel			*lowestPlaying,
										*lowestPaused;
	uint				lowestPlayingVolume;


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );

	if ( type == AUDIO_CHANNEL_TYPE_ANY)
	{
		//	you must know what type of channel you want so AUDIO_CHANNEL_ANY is taken
		//	to mean a standard channel

		type = AUDIO_CHANNEL_TYPE_STD;
	}

	// search through the channel list.
	//	First try to get a non active channel
	//	Then settle on getting the lowest priority paused channel.
	//	Then settle on getting the lowest priority playing channel.
	//  ...and among the channels with lowest priority, get the one
	//  with lowest volume.

	lowestPriorityPlaying = AUD_HIGHEST_PRIORITY;
	lowestPriorityPaused = AUD_HIGHEST_PRIORITY;
	lowestPlaying = NULL;
	lowestPaused = NULL;
	lowestPlayingVolume = 0xFFFFFFFF;

	channel = AudioDeviceFirstChannel ( dev, &sh );

	while ( channel )
	{
		if ( channel->Type == type )
		{
			if ( !(FLAGS_CHECK (channel->Control.Status, mAUDIO_CTRL_ACTIVE | mAUDIO_CTRL_INUSE )))
			{
				break;	//  we found a free channel 
			}
			if ( (FLAGS_CHECK (channel->Control.Status, mAUDIO_CTRL_PLAYING )))
			{
				if ( channel->Control.Priority < lowestPriorityPlaying )
				{
					lowestPriorityPlaying = channel->Control.Priority;
					lowestPlaying = channel;
					lowestPlayingVolume = channel->attribs.VolumeLevel.level;
				}
				else if ( channel->Control.Priority == lowestPriorityPlaying )
				{
					// If priorities are equal, choose the channel whose volume
					// is lowest -- but only award lowest status if this one is
					// quieter than the previous best by a significant amount.

					if ( channel->attribs.VolumeLevel.level < lowestPlayingVolume
							 && ( lowestPlayingVolume - channel->attribs.VolumeLevel.level >= VOLUME_SLACK ) )
					{
						lowestPriorityPlaying = channel->Control.Priority;
						lowestPlaying = channel;
						lowestPlayingVolume = channel->attribs.VolumeLevel.level;
					}
				}
			}
			else
			{
				if ( channel->Control.Priority < lowestPriorityPaused )
				{
					lowestPriorityPaused = channel->Control.Priority;
					lowestPaused = channel;
				}
			}
		}
		channel = AudioDeviceNextChannel ( &sh );
	}

	if ( !channel )	//  we did not find a free channel 
	{
		if ( !( channel = lowestPaused) || (lowestPriorityPaused > lowestPriorityPlaying) )
		{
			//  we did not find a paused channel of lower priority
			channel = lowestPlaying;
		}
	}

	return channel;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioChannel*	AudioDeviceReserveChannel ( AudioDevice *dev, AudioChannelType newType )
{
	AudioChannel	*chan;


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );

	DBG_ASSERT ( newType != AUDIO_CHANNEL_TYPE_ALL );	//  this does not make sense, think about it 
	DBG_ASSERT ( newType != AUDIO_CHANNEL_TYPE_STD );	//  illegal value 

	chan = AudioDeviceGetChannel ( dev, AUDIO_CHANNEL_TYPE_STD );

	if ( chan )
	{
		AudioChannelReserve ( chan, newType );
	}

	return chan;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		AudioDeviceDestroyAllChannels ( AudioDevice *dev, AudioChannelType type )
{
	AudioChannel		*channel,*next;
	AudioSearch		sh;


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );


	channel = AudioDeviceFirstChannel ( dev, &sh );

	while (channel)
	{
		next = AudioDeviceNextChannel( &sh );

		if ( type == AUDIO_CHANNEL_TYPE_ALL || channel->Type == type )
		{
			AudioChannelDestroy ( channel );
		}

		channel = next;
	}

	return vNO_ERROR;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioDeviceStopAllChannels ( AudioDevice *dev, AudioChannelType type)
{
	AudioChannel		*channel,*next;
	AudioSearch		sh;


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );


	channel = AudioDeviceFirstChannel ( dev, &sh );

	while (channel)
	{
		next = AudioDeviceNextChannel( &sh );

		if ( type == AUDIO_CHANNEL_TYPE_ALL || channel->Type == type )
		{
			AudioChannelStop ( channel );
		}

		channel = next;
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioDevicePauseAllChannels ( AudioDevice *dev, AudioChannelType type )
{
	AudioChannel		*channel;
	AudioSearch		sh;


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );


	channel = AudioDeviceFirstChannel ( dev, &sh );

	while (channel)
	{
		if ( FLAGS_CHECK ( channel->Control.Status, mAUDIO_CTRL_PLAYING ) )
		{
			if ( type == AUDIO_CHANNEL_TYPE_ALL || channel->Type == type )
			{
				AudioChannelPause ( channel );
			}
		}
		channel = AudioDeviceNextChannel( &sh );
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioDeviceResumeAllChannels ( AudioDevice *dev, AudioChannelType type )
{
	AudioChannel		*channel;
	AudioSearch		sh;


	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );

	channel = AudioDeviceFirstChannel ( dev, &sh );

	while (channel)
	{

		if ( FLAGS_CHECK ( channel->Control.Status, mAUDIO_CTRL_PAUSED ) )
		{
			if ( type == AUDIO_CHANNEL_TYPE_ALL || channel->Type == type )
			{
				AudioChannelResume ( channel );
			}
		}
		channel = AudioDeviceNextChannel( &sh );
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		AudioDeviceService ( AudioDevice *dev)
{
	int result;

	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );

	result =  dev->deviceHandler ( dev );

	return result;
}


void	AudioDeviceAttribsAdd ( AudioDevice *dev, AudioAttribs *attribs )
{
	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );

	if ( !attribs )
	{
		return;
	}

	if ( audioDeviceAttribsListFind ( dev, attribs ))
	{
		return;
	}

	AudioAttribsNode *node;

	ALLOC_STRUCT ( node, AudioAttribsNode );

	DBG_SET_TYPE ( node, AudioAttribsNode );

	ListNodeInit ( &node->nd );

	node->attribs = attribs;

	LockAcquire ( &dev->channelAccess );

	ListAddToTail ( &dev->attribsList, &node->nd );

	LockRelease ( &dev->channelAccess );
}

void	AudioDeviceAttribsRemove ( AudioDevice *dev, AudioAttribs *attribs )
{
	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );

	AudioAttribsNode *node = audioDeviceAttribsListFind ( dev, attribs );

	if ( node )
	{
		DBG_ASSERT_TYPE ( node, AudioAttribsNode );

		LockAcquire ( &dev->channelAccess );

		ListNodeRemove ( &node->nd );

		LockRelease ( &dev->channelAccess );

		DBG_INVALIDATE_TYPE ( node );

		AudioMemFree ( node );

	}
}

void	AudioDeviceAttribsRemoveAll ( AudioDevice *dev )
{
	DBG_ASSERT ( audioInitialized );	//  AudioSetUp() was not called 
	DBG_ASSERT_TYPE ( dev, AudioDevice );
	AudioAttribsNode *node;

	while ( (node = (AudioAttribsNode *) ListFirstItem ( &dev->attribsList )) )
	{
		DBG_ASSERT_TYPE ( node, AudioAttribsNode );

		ListNodeRemove ( &node->nd );

		DBG_INVALIDATE_TYPE ( node );

		AudioMemFree ( node );

	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void AudioServiceInfoInit ( AudioServiceInfo *asi )
{
	DBG_ASSERT_PTR ( asi );

	DBG_SET_TYPE ( asi, AudioServiceInfoStruct );

	asi->serviceInterval = 0;
	asi->mustServiceInterval = 0;
	asi->lastInterval = 0;
	asi->longestInterval = 0;
	asi->lastServiceTime = 0;
	asi->count = 0;
	asi->lastcount = 0;
	asi->periodInterval = SECONDS(5);
	asi->longestIntervalforPeriod = 0;
	asi->periodStart = 0;
	asi->animPos = 0;
	asi->missCount = 0;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void AudioServiceSetInterval ( AudioServiceInfo *asi, TimeStamp interval )
{

	DBG_ASSERT_TYPE ( asi, AudioServiceInfoStruct );

	asi->serviceInterval = interval;
	asi->mustServiceInterval = interval;
	asi->longestReset = asi->serviceInterval*10;


}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void AudioServiceSetMustServiceInterval ( AudioServiceInfo *asi, TimeStamp interval )
{

	DBG_ASSERT_TYPE ( asi, AudioServiceInfoStruct );

	asi->mustServiceInterval = interval;
}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void AudioServiceSetResetInterval ( AudioServiceInfo *asi, TimeStamp interval )
{

	DBG_ASSERT_TYPE ( asi, AudioServiceInfoStruct );

	asi->longestReset = interval;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int AudioServiceNeeded ( AudioServiceInfo *asi, TimeStamp now )
{
	TimeStamp interval;

	DBG_ASSERT_TYPE ( asi, AudioServiceInfoStruct );

	interval = now - asi->lastServiceTime;

	if ( interval < asi->serviceInterval )
	{
		return FALSE;
	}

	return TRUE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void AudioServicePerform ( AudioServiceInfo *asi, TimeStamp now )
{
	TimeStamp interval;

	DBG_ASSERT_TYPE ( asi, AudioServiceInfoStruct );

	interval = now - asi->lastServiceTime;

	asi->lastInterval = interval;
	asi->count++;

	if ( asi->longestInterval > asi->longestReset )
	{
			asi->longestInterval = 0;	//  reset the longest count if it is extremely long 
	}

	if ( interval > asi->longestInterval )
	{
		asi->longestInterval = interval;
	}

	if ( interval > asi->mustServiceInterval )
	{
		asi->missCount++;
	}

	if ( interval > asi->longestIntervalforPeriod )
	{
		asi->longestIntervalforPeriod = interval;
	}

	asi->lastServiceTime = now;
}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioServiceInfoDump ( AudioServiceInfo *asi, char *string )
{
	static char *anim = "|/-\\";
	TimeStamp now = AudioGetTime();

	if ( asi->count != asi->lastcount )
	{
		asi->animPos++;
		if ( asi->animPos >= strlen (anim ))
		{
			asi->animPos = 0;
		}

		asi->lastcount = asi->count;
	}

	if ( asi->periodStart + asi->periodInterval < now )
	{
		asi->longestIntervalforPeriod = asi->serviceInterval;
		asi->periodStart = now;
	}

	sprintf ( string, "%05dms (%04d,%04d,%04d,~%2d) %c \n",
							(int) asi->serviceInterval,
							(int) asi->lastInterval,
							(int) asi->longestIntervalforPeriod,
							(int) asi->longestInterval,
							(int) asi->missCount,
							anim[asi->animPos] );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		AudioDeviceDump ( AudioDevice *dev, void (*print) ( char *), int names  )
{
	char	string[200];
	char	temp[100];
	AudioChannel	*chan;
	AudioSearch	sh;
	char 		*state;
	char 		*type;
	int		count;

	DBG_ASSERT_TYPE ( dev, AudioDevice );

	print ( "Audio Device Dump ---------------------\n");

	sprintf ( string, "%s (%d)\n",
							dev->system->master->Name,
							dev->Unit);
	print ( string );

	AudioServiceInfoDump ( &dev->mixerUpdate, temp );
	sprintf ( string, "mixer:%s", temp );
	print ( string );

	AudioServiceInfoDump ( &dev->attribsUpdate, temp );
	sprintf ( string, "level:%s", temp );
	print ( string );

	sprintf (string, "Frames: %d Oversamp: %d Lag: %d\n", dev->frames, dev->over_sample, dev->frame_lag );
	print ( string );

	if ( names )
	{
		sprintf ( string, "Chan:TYPE: STATE : FORMAT  :PRI :VOL: NAME\n");
	}
	else
	{
		sprintf ( string, "Chan:TYPE: STATE : FORMAT  :PRI :VOL:\n");
	}
	print ( string );

	chan = AudioDeviceFirstChannel ( dev, &sh );
	count = 0;
	while (chan)
	{
		if ( FLAGS_CHECK ( chan->Control.Status, mAUDIO_CTRL_PLAYING) )
		{
			state = "PLAYING";
		}
		else if ( FLAGS_CHECK ( chan->Control.Status, mAUDIO_CTRL_PAUSED))
		{
			state = "PAUSED ";
		}
		else
		{
			state = "FREE   ";
		}
		if ( chan->Type >= AUDIO_CHANNEL_TYPE_USER )
		{
			type = "USER";
		}
		else if ( chan->Type == AUDIO_CHANNEL_TYPE_STD )
		{
			type = "STD ";
		}
		else
		{
			type = "??? ";
		}

		AudioFormat *format;
		char fstr[200];
		char name[200];

		name[0] = 0;

		if ( names )
		{
			#ifndef IG_FINAL_RELEASE
				strcpy ( name, chan->sample_name );
			#endif
		}

		format = &chan->current_format;

		char temp[3];
		if ( format->Compression == AUDIO_COMPRESS_MP3 )
		{
			strcpy ( temp, "mp" );
		}
		else
		{
			sprintf ( temp, "%2d", format->Compression == AUDIO_COMPRESS_NONE ? format->SampleWidth *8 : 4 );
		}

		sprintf ( fstr, "%02d-%s-%c%c%c", 
							format->Rate / 1000, 
							temp,
							format->Channels == 1 ? 'm' : 's', 
							chan->format_changed ? '*' : ' ',
							chan->drv_format_changed ? '+' : ' ' );

		//sprintf ( string, "Chan:TYPE: STATE : FORMAT :PRI :VOL:\n");
		sprintf ( string, "#%2d :%s:%s:%s:%4d:%3d:%.31s\n",
							count,
							type,
							state,
							fstr,
							chan->Control.Priority,
							AudioLevelApply ( &chan->attribs.VolumeLevel, 100 ),
							name);

		print ( string );
		count++;
		chan = AudioDeviceNextChannel( &sh );
	}

	print ( "--------------------------------------\n");
}


