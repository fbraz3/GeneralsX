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
** File name:  audevent.cpp                                                 **
**                                                                          **
** Created by:  04/28/99 TR                                                 **
**                                                                          **
** Description: <description>                                               **
**                                                                          **
*****************************************************************************/

/*****************************************************************************
**            Includes                                                      **
*****************************************************************************/

#define _DEFINE_EVENT_TOKENS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include <wpaudio/altypes.h>
#include <wpaudio/cache.h>
#include <wpaudio/events.h>
#include <wpaudio/level.h>
#include <wpaudio/profiler.h>
#include <wpaudio/channel.h>
#include <wpaudio/device.h>
#include <wpaudio/attributes.h>
#include <wpaudio/handle.h>

// 'assignment within condition expression'.
#pragma warning(disable : 4706)

DBG_DECLARE_TYPE ( AudioEventClass)
DBG_DECLARE_TYPE ( AudioEvent )
DBG_DECLARE_TYPE ( AudioEventHandle )

/*****************************************************************************
**          Externals                                                       **
*****************************************************************************/



/*****************************************************************************
**           Defines                                                        **
*****************************************************************************/

#define DEBUG_EVENT_CONTENTION	0

#define AUDIO_INVALID_STAMP		0

#define MAX_EVENTS						100

#define AVERAGE_LOUDNESS			10
#define FULL_LOUDNESS					100

#define VOLUME_QUANTIZE_LEVELS	10

#define EVENT_MIN_DELAY				33	// milliseconds

#define NUM_BUCKETS_PRI				( AUDIO_NUM_EVENT_PRIORITIES + 2 )	// Player sounds are +2!
#define NUM_BUCKETS_VOL				10
#define VOLUME_QUANTIZE				( AUDIO_LEVEL_MAX / NUM_BUCKETS_VOL )

#define AUDIO_EVENT_DEFAULT_LIMIT		3

#define ID_HANDLE					((int) &audioEventClasses)

/*****************************************************************************
**        Private Types                                                     **
*****************************************************************************/

typedef enum
{
	AUDIO_EVENT_NEW,						//  event has not been serviced
	AUDIO_EVENT_START_PLAYING,	//	event is ready for playing
	AUDIO_EVENT_WAITING,				//	event is waiting
	AUDIO_EVENT_PLAYING,				//  event is playing
	AUDIO_EVENT_DONE						//  event is over

} AudioEventState;


struct AudioEventClassTag
{
	ListNode								nd;
	int											valid;						//	TRUE if event class has been defined
	AudioEventControl				control;					//  audio event control flags
	AudioLevel							baseLevel;				//  base volume
	AudioPriority						priority;					//  sound priority
	int											count;						//  number of currently active events for this class
	int											limit;						//	maximum number of active events allowed
	int											limitLoop;				//	maximum iterations before looping ends (0 means never)
	int											range;						//  energy level of sound
	int											min_volume;				//  min volume for global event type
	uint										minDelay;					//	minimum delay time (ms)
	uint										maxDelay;					//	maximum delay time (ms)
	int											minFShift;				//  max negative frequency shift percentage
	int											maxFShift;				//  max positive frequency shift percentage
	int											vShift;						//  volume shift percentage
	int											volumeCompression;//	should this event class be volume compressed

	AudioAttribs						*fadeAttribs;			//	Fader to use for this event
	AudioAttribs						*masterAttribs;		//	Master control to use for this event



	ListHead								local;						//	local events list
	int											lastFrame;				//	id of the last frame the local list was sorted

	uint										lastBucketStamp;	//	id of the last frame this class was bucketed
	AudioPriority						maxPri;						//  highest priority of any event of this class
	ListNode								nodeVol;					//	for volume list
	uint										maxVol;						//  current level of loudest event of this class

	// sample list
	char										*sampleName[MAX_AUDIO_EVENT_SAMPLES];
	int											numSamples;
	int											attackCount;
	int											decayCount;
	int											morningCount;
	int											eveningCount;
	int											nightCount;

	char										*name;

	void										*data;

	#ifdef _DEBUG
	AudioFormat							format;						// format of last sample added;
	int											format_same;
	#endif

	DBG_TYPE ()

};

//
//  AudioEvent
//

#define mAUDIO_EVENT_DEAD					0x00000001	//  this event can be returned to the pool
#define mAUDIO_EVENT_PLAYING			0x00000002	//  playing a sample
#define mAUDIO_EVENT_CHANNEL			0x00000004	//  have a channel
#define mAUDIO_EVENT_NO_ATTACK		0x00000008	//  dont play attack sample if there is one
#define mAUDIO_EVENT_NO_DECAY			0x00000010	//  dont play decay sample if there is one
#define mAUDIO_EVENT_END					0x00000020	//  event to finish up
#define mAUDIO_EVENT_DO_END				0x00000040	//  end latch
#define mAUDIO_EVENT_ALLOCATED		0x00000080	//  struct was malloced

struct AudioEventTag
{
						ListNode						nd;										//	global event list
						ListNode						local;								//	local event list
	volatile	int									flags;								//  event control flags
	volatile	AudioEventState			state;								//  event current state
	volatile	AudioEventState			nextState;						//  next event state if waiting
						AudioEventClass			*eclass;							//  event class info
						AudioCacheItem			*item[MAX_AUDIO_EVENT_SAMPLES];		//  audio cache item
						int									numItems;
						int									currentItem;					//  which sample of the event item list we are currently processing
						AudioChannel				*channel;							//  audio channel we are playing on
						AudioAttribs				attribs;							//	event playback attribs
						int									stamp;								//	unique instance id
	volatile	uint								delay;
	volatile	TimeStamp						timeOut;
						AudioSample volatile*startSample;
						int									fshift;
						int									vshift;
						int									adjustPriority;				// priority modifier
						Lock								paused;

						int									sequence[MAX_AUDIO_EVENT_SAMPLES]; // play sequence
						int									numSequence;
						int									loopCount;

						int									loadSequence[MAX_AUDIO_EVENT_SAMPLES];
						int									numLoadSequence;
						int									loadLoopCount;
						int									loadNdx;
						int									loadSaveNdx;
						int									timeOfDay;
						AudioEventHandle		*handle;				// keep tack of handle

	DBG_TYPE ()

};


typedef struct EClassBucketTag
{
	ListHead		bucketVol[ NUM_BUCKETS_VOL ];
} EClassBucket;

/*****************************************************************************
**         Private Data                                                     **
*****************************************************************************/

static ListHead								audioEventClasses;
static AudioCache							*audioCache = NULL;
static AudioDevice						*audioDevice = NULL;
static int										initialized = FALSE;
static int										eventsOn = TRUE;
static int										eventsOK = FALSE;
static ListHead								audioEvents;
static AudioMemoryPool				*audioEventPool = NULL;
static int										audioStamp = AUDIO_INVALID_STAMP+1;
static int										frameStamp = 0;

static uint										bucketStamp = 0;	// Never clear this!
static EClassBucket						eClassBucket[ NUM_BUCKETS_PRI ];

static AudioAttribs						audioCompressionAttribs;

/*****************************************************************************
**         Public Data                                                      **
*****************************************************************************/

int		AudioEventsCount = 0;
int		AudioEventsPeak = 0;
int		AudioGameFrame = 0;		// increment this every game frame

/*****************************************************************************
**         Private Prototypes                                               **
*****************************************************************************/

static AudioChannel*	audioPrepareChannel ( int pri, int flags );
static int						audioEventPrep ( AudioEvent *event );
static int						audioEventStart ( AudioEvent *event );
static AudioSample*		eventFirstSample ( AudioEvent *event );
static AudioSample*		eventNextSample ( AudioEvent *event );
static void						audioEventDestroy ( AudioEvent *event );
static void						eventUnlockSamples ( AudioEvent *event );

/*****************************************************************************
**          Private Functions                                               **
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static void audioInitEventClass ( AudioEventClass *eclass )
{
	memset ( eclass, 0, sizeof(AudioEventClass ));
	ListNodeInit ( &eclass->nd );
	DBG_SET_TYPE ( eclass, AudioEventClass );

	AudioEventClassReset ( eclass );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static void eventAddLocalSort ( AudioEvent *new_event )
{
	AudioEventClass *eclass;
	ListNode				*node, *head;
	AudioEvent			*event;
	uint			volume, new_volume;

	DBG_ASSERT_TYPE ( new_event, AudioEvent );

	eclass = new_event->eclass;

	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	new_volume = AudioAttribsGetVolume ( &new_event->attribs );

	head = (ListNode*) &eclass->local;

	node = head->next;

	while ( node != head )
	{
		int	dif, slack;
		uint baseLev;

		event = (AudioEvent *) ( (uint) node - ((uint) &new_event->local - (uint) new_event ));
		volume = AudioAttribsGetVolume ( &event->attribs );
		baseLev = (uint) AudioLevelGet( &eclass->baseLevel );
		DBG_ASSERT( volume <= baseLev );
		slack = baseLev / VOLUME_QUANTIZE_LEVELS;

		dif = (volume - new_volume);

		DBG_ASSERT_TYPE ( event, AudioEvent );

		if ( ((dif)>=0 ? dif : -dif ) < slack )
		{
			// we consider the events to be of the same volume

			if ( (eclass->control & AUDIO_EVENT_CTRL_INTERRUPT) )
			{
				// give new events priority over old ones
				if (  (event->state != AUDIO_EVENT_NEW) )
				{
						break;
				}
			}
			else
			{
				// give old events priority over new ones
				if (  (event->state == AUDIO_EVENT_NEW) )
				{
						break;
				}
			}
		}
		else if ( volume < new_volume )
		{
				break;
		}

		node = node->next;
	}

	ListNodeInit ( &new_event->local );
	ListNodeInsert ( node, &new_event->local );
	eclass->count++;

	if ( eclass->limit && (eclass->count > eclass->limit) )
	{
		event = (AudioEvent *) ( (uint) head->prev - ((uint) &new_event->local - (uint) new_event ));
		ListNodeRemove ( &event->local );
		AudioEventKill ( event );
		eclass->count--;
		DBG_ASSERT ( eclass->count == eclass->limit );
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static void eventWait ( AudioEvent *event, TimeStamp delay, AudioEventState nextState )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );

	event->nextState = nextState;
	if ( AudioEventIsPaused ( event ))
	{
		event->timeOut = delay;
	}
	else
	{
		event->timeOut = AudioGetTime() + delay;
	}
	event->state = AUDIO_EVENT_WAITING;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static AudioSample*	eventFirstSample ( AudioEvent *event )
{
	int j=0;
	int i;

	DBG_ASSERT_TYPE ( event, AudioEvent );

	if ( (event->numSequence = event->numItems) == 0 || event->flags & mAUDIO_EVENT_END )
	{
		return NULL;
	}

	if ( event->eclass->minDelay < EVENT_MIN_DELAY )
	{
		
		if ( event->eclass->control & AUDIO_EVENT_CTRL_ATTACK )
		{
			j++;
			event->numSequence--;
			event->currentItem = 0;
		}
		else
		{
			event->currentItem = -1;
		}
		
		if ( event->eclass->control & AUDIO_EVENT_CTRL_DECAY )
		{
			event->numSequence--;
		}
		
		if ( event->numSequence <= 0 )
		{
			event->eclass->valid = FALSE;
			return NULL;
		}
		
		for ( i=0 ; i < event->numSequence; i++, j++ )
		{
			event->sequence[ i ] = j;
		}
		
		if ( event->flags & mAUDIO_EVENT_NO_ATTACK || !(event->eclass->control & AUDIO_EVENT_CTRL_ATTACK) )
		{
			return eventNextSample ( event );
		}
	}

	return AudioCacheItemSample ( event->item[0] );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static AudioSample*	eventNextSample ( AudioEvent *event )
{
	int i;
	AudioEventControl control;

	DBG_ASSERT_TYPE ( event, AudioEvent );

	control = event->eclass->control;

 	if ( event->eclass->minDelay >= EVENT_MIN_DELAY && !(event->flags & mAUDIO_EVENT_END) )
 	{	
 			return NULL;
 	}

	if ( event->numSequence == 0 || event->flags & mAUDIO_EVENT_END )
	{
		// end of sequence
		if ( control & AUDIO_EVENT_CTRL_LOOP && !(event->flags & mAUDIO_EVENT_END))
		{
			if ( !event->eclass->limitLoop || (event->loopCount < event->eclass->limitLoop - 1) )
			{
				event->loopCount++;
				return eventFirstSample ( event );	// begin again
			}
		}

		// the end
		if ( (!(event->eclass->control & AUDIO_EVENT_CTRL_DECAY) || event->flags & mAUDIO_EVENT_NO_DECAY)
				|| event->numItems < 1 )
		{
			return NULL;
		}

		FLAGS_SET ( event->flags , mAUDIO_EVENT_NO_DECAY ); // only want the decay to play once
		return  AudioCacheItemSample ( event->item[event->numItems-1] );
	}

	// choose the nex sample to play

	if ( control & AUDIO_EVENT_CTRL_RANDOM )
	{
		int ndx;
		ndx = AudioRandomPick( 0, event->numSequence -1 );
		event->currentItem =event->sequence[ndx];
		// remove from sequence

		for ( i=ndx; i < event->numSequence -1 ; i++)
		{
			event->sequence[i] = event->sequence[i+1];
		}
	}
	else
	{
		event->currentItem++;
	}

	event->numSequence--;

	return AudioCacheItemSample ( event->item[event->currentItem] );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int todAdjustStart( AudioEvent *event )
{
	int afternoonCount = AudioEventClassNumSound(event->eclass) - (event->eclass->morningCount + event->eclass->eveningCount + event->eclass->nightCount + event->eclass->attackCount + event->eclass->decayCount);
	if (event->timeOfDay == 1) {
		// morning
		return 0;
	} 

	if (event->timeOfDay == 2) {
		// afternoon
		return event->eclass->morningCount;
	}

	if (event->timeOfDay == 3) {
		// evening
		if (event->eclass->eveningCount != 0) {
			return event->eclass->morningCount + afternoonCount;
		}
	}

	if (event->timeOfDay == 4) {
		// night
		if (event->eclass->nightCount != 0) {
			return event->eclass->morningCount + afternoonCount + event->eclass->eveningCount;
		}
	}

	// default to returning the afternoon sounds
	return event->eclass->morningCount;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int todAdjustEnd( AudioEvent *event )
{
	int afternoonCount = AudioEventClassNumSound(event->eclass) - (event->eclass->morningCount + event->eclass->eveningCount + event->eclass->nightCount + event->eclass->attackCount + event->eclass->decayCount);
	if (event->timeOfDay == 1) {
		// morning
		if (event->eclass->morningCount != 0) {
			return afternoonCount + event->eclass->eveningCount + event->eclass->nightCount;
		} 
	} 

	if (event->timeOfDay == 2) {
		// afternoon
		return event->eclass->eveningCount + event->eclass->nightCount;
	}

	if (event->timeOfDay == 3) {
		// evening
		if (event->eclass->eveningCount != 0) {
			return event->eclass->nightCount;
		}
	}

	if (event->timeOfDay == 4) {
		// night
		if (event->eclass->nightCount != 0) {
			return 0;
		}
	}

	// default to returning the afternoon sounds
	return event->eclass->eveningCount + event->eclass->nightCount;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/


static int eventLoadSchedule( AudioEvent *event )
{
	AudioEventClass		*eclass;
	AudioEventControl control;
	int								i;
	int								item_ndx;

	eclass = event->eclass;
	control = eclass->control;

	if ( event->numLoadSequence == event->loadNdx )
	{
		int ndx = 0;

		if ( (!(eclass->control & AUDIO_EVENT_CTRL_LOOP) && event->loadLoopCount == 1 )
			  || ((eclass->control & AUDIO_EVENT_CTRL_LOOP) && eclass->limitLoop && (event->loadLoopCount) >= eclass->limitLoop ))
		{
			return -1;	// sequence is finished
		}

		// Time to create a new schedule.

		if ( eclass->attackCount && !(event->flags & mAUDIO_EVENT_NO_ATTACK))
		{
			event->loadSequence[ndx++] = AudioRandomPick ( 0, eclass->attackCount-1);
		}

		if ( eclass->control & AUDIO_EVENT_CTRL_ALL )
		{
			for ( i = eclass->attackCount + todAdjustStart(event); i < eclass->numSamples - (eclass->decayCount + todAdjustEnd(event)); i++ )
			{
				event->loadSequence[ ndx++ ] = i;
			}
		}
		else if ( eclass->control & AUDIO_EVENT_CTRL_RANDOM )
		{
			if ( event->loadLoopCount == 0 )
			{
				i = AudioRandomPick ( eclass->attackCount + todAdjustStart(event), eclass->numSamples - (eclass->decayCount + todAdjustEnd(event) + 1));
				event->loadSaveNdx = i;
			}
			else
			{
				i = event->loadSaveNdx;
			}
			event->loadSequence[ ndx++ ] = i;
		}
		else
		{
			event->loadSequence[ ndx++ ] = eclass->attackCount;
		}

		event->numLoadSequence = ndx;
		event->loadNdx = 0;
		event->loadLoopCount++;
	}

	// carry out the existing schedule

	if ( control & AUDIO_EVENT_CTRL_RANDOM && 
				( (!eclass->attackCount || (event->flags & mAUDIO_EVENT_NO_ATTACK)) || event->loadNdx > 0 ))
	{
		int ndx, currentSampleIndex;

		ndx = AudioRandomPick( (!eclass->attackCount || (event->flags & mAUDIO_EVENT_NO_ATTACK)) ? 0 : 1, event->numLoadSequence - 1 );
		item_ndx = currentSampleIndex = event->loadSequence[ ndx ];

		// remove from sequence
		for ( i = ndx; i < event->numLoadSequence - 1; i++ )
		{
			event->loadSequence[ i ] = event->loadSequence[ i + 1 ];
		}

		event->numLoadSequence--;
	}
	else
	{
		item_ndx = event->loadSequence[ event->loadNdx ];
		event->loadNdx++;
	}


	return item_ndx;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static void eventLoadItem( AudioEvent *event, int sampleIndex )
{
	if ( event->item[ event->numItems ]
				= AudioCacheLoadItem( audioCache, event->eclass->sampleName[ sampleIndex ] ) )
	{
		AudioCacheItemLock( event->item[ event->numItems ] );
		event->numItems++;
	}
#if DEBUG_EVENT_CONTENTION
	else
	{
		if ( event->channel->Control.Priority >= AUDIO_EVENT_CRITICAL_PRIORITY )
		{
			DBGPRINTF(( "eventLoadItem: '%s' could not load critical sample.\n", AudioEventName ( event ) ));
		}
	}
#endif
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static void	eventLoadAndLockSamples ( AudioEvent *event )
{
	int								i, ndx;
	AudioEventControl	control;
	AudioEventClass		*eclass;

	DBG_ASSERT_TYPE ( event, AudioEvent );

	control = event->eclass->control;
	eclass = event->eclass;

	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	// This function will get called repeatedly for events with delay,
	// because these events re-enter state EVENT_START_PLAYING.
	// We need to load only the sample that's about to be used.

	if ( event->numItems )
	{
		if ( eclass->minDelay >= EVENT_MIN_DELAY )
		{
			eventUnlockSamples( event );
		}
		else
		{
			/// @todo, workout what this comment means
			// Shouldn't happen, but if it does, we're protected from
			// gobbling cache permanently by forgetting our old load...
			DBGPRINTF(( "eventLoadAndLockSamples: You are not reading this.\n" ));
			return;
		}
	}

	if ( eclass->minDelay < EVENT_MIN_DELAY )
	{
		// we are going to be using all samples

		if ( eclass->attackCount )
		{
			eventLoadItem ( event, AudioRandomPick ( 0, eclass->attackCount -1 ));
		}

		if ( eclass->control & AUDIO_EVENT_CTRL_ALL )
		{
			for ( i=eclass->attackCount + todAdjustStart(event); i < eclass->numSamples - (eclass->decayCount + todAdjustEnd(event)); i++ )
			{
				eventLoadItem( event, i );
			}
		}
		else if ( eclass->control & AUDIO_EVENT_CTRL_RANDOM )
		{
			i = AudioRandomPick ( eclass->attackCount + todAdjustStart(event), eclass->numSamples - (eclass->decayCount + todAdjustEnd(event) + 1) );
			eventLoadItem( event, i );
		}
		else
		{
			eventLoadItem( event, eclass->attackCount );
		}

		if ( eclass->decayCount )
		{
			eventLoadItem ( event, AudioRandomPick ( eclass->numSamples - eclass->decayCount, eclass->numSamples -1 ));
		}
	}
	else
	{
		int decay_ndx = -1;
		if ( eclass->decayCount && eclass->control & AUDIO_EVENT_CTRL_DECAY )
		{
			decay_ndx = AudioRandomPick ( eclass->numSamples - eclass->decayCount, eclass->numSamples-1);
		}
		// This event has delay.  Load only the sample about to be used.
		// Note: we will leave the existing scheduling mechanism in place.
		// With only one sample loaded, however, it will have no decisions
		// to make.  We will decide the true schedule in the function below.
		if ( (ndx = eventLoadSchedule( event )) >= 0 )
		{
			eventLoadItem( event, ndx );
			if ( decay_ndx != -1 )
			{
				eventLoadItem( event, decay_ndx ); // add it incase it is needed
			}
		}
		else
		{
			// end of sequence, must play decay if needed
			if ( decay_ndx != -1 && !(event->flags & mAUDIO_EVENT_NO_DECAY))
			{
				eventLoadItem( event, decay_ndx );
				FLAGS_SET ( event->flags, mAUDIO_EVENT_NO_DECAY );
			}
		}
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static void eventUnlockSamples ( AudioEvent *event )
{
	int i;

	DBG_ASSERT_TYPE ( event, AudioEvent );

	for ( i=0; i < event->numItems; i++ )
	{
		AudioCacheItemUnlock ( event->item[i] );
		event->item[i] = NULL;
	}

	event->numItems = 0;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static void				eventReleaseChannel ( AudioEvent *event )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );

	if ( event->channel && (event == (AudioEvent *) event->channel->Data) )
	{
 		if ( event->flags & mAUDIO_EVENT_PLAYING )
 		{
 			AudioChannelStop ( event->channel );
 			DBG_ASSERT ( !(event->flags & mAUDIO_EVENT_PLAYING) );
 		}
 		AudioChannelNoUse ( event->channel );
 		event->channel->Data = NULL;
 		event->channel->CB_Stop = NULL;
 		event->channel->CB_NextSample = NULL;
 		event->channel->CB_SampleDone = NULL;
 		event->channel->SfxAttribs = NULL;
 		event->channel->CompAttribs = NULL;
 		event->channel->FadeAttribs = NULL;
 		event->channel = NULL;
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static	int	audioEventStop ( AudioChannel *chan )
{
	AudioEvent *event = (AudioEvent *) chan->Data;

	DBG_ASSERT_TYPE ( event, AudioEvent );

	event->state = AUDIO_EVENT_DONE;

	return vNO_ERROR;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static void				audioEventDestroy ( AudioEvent *event )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );

	DBG_MSGASSERT ( event->flags & mAUDIO_EVENT_DEAD, ("trying to destroy an event that is not dead"));

	ListNodeRemove ( &event->nd );

	event->stamp = AUDIO_INVALID_STAMP;

	DBG_INVALIDATE_TYPE ( event );

	if ( event->flags & mAUDIO_EVENT_ALLOCATED )
	{
		MEM_Free (event);
	}
	else
	{
		MemoryPoolReturnItem ( audioEventPool, event );
	}
	AudioEventsCount--;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static	int	audioEventSampleDone ( AudioChannel *chan )
{
	AudioEvent *event = (AudioEvent *) chan->Data;
	AudioEventState new_state;


	DBG_ASSERT_TYPE ( event, AudioEvent );

	FLAGS_CLEAR ( event->flags, mAUDIO_EVENT_PLAYING );

	if ( event->state == AUDIO_EVENT_DONE )
	{
		return vNO_ERROR;
	}

	if ( event->startSample	// If EVENT_CTRL_LOOP, preceding will be true even if load-sequenced
			 || event->numLoadSequence || event->delay )
	{
		new_state = AUDIO_EVENT_START_PLAYING;
	}
	else
	{
		new_state = AUDIO_EVENT_DONE ;
		event->delay = 0;
	}

	if ( event->delay )
	{
		eventWait ( event, MSECONDS (event->delay), new_state );
		event->delay = 0;
	}
	else
	{
		event->state = new_state;
	}

	return vNO_ERROR;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static	int	audioEventNextSample ( AudioChannel *chan )
{
	AudioEvent	*event = (AudioEvent *) chan->Data;
	AudioSample		*sample;
	uint		delay;

	sample = eventNextSample ( event );

	if ( event->eclass->maxDelay < EVENT_MIN_DELAY )
	{
			// There is no delay
			AudioChannelSetSample ( chan,  sample );
			return vNO_ERROR;
	}

	// There is a delay value. However see if it applies to this sample

	if ( sample )
	{
		// because there is a next sample we are in the middle of a event sequence.
		// We only delay samples within the sequence if it is ambient

		if ( !(event->eclass->control & (AUDIO_EVENT_CTRL_AMBIENT)) )
		{
			// this is not an ambient seqeuence so no delay
			AudioChannelSetSample ( chan,  sample );
			return vNO_ERROR;
		}
	}
	else
	{
		// There is no next sample so we are at the end of the sequence
		// If this is NOT a postdelay event the don't delay
		if ( !(event->eclass->control & (AUDIO_EVENT_CTRL_POSTDELAY)) )
		{
			AudioChannelSetSample ( chan,  NULL );
			return vNO_ERROR;
		}
	}

	// we are required to wait before going on to the next sample

	AudioChannelSetSample ( chan,  NULL );

	delay = (uint) AudioRandomPick (  event->eclass->minDelay, event->eclass->maxDelay );

	if ( delay < EVENT_MIN_DELAY )
	{
		AudioChannelSetSample ( chan,  sample );
		return vNO_ERROR;
	}

	event->delay = delay;
	event->startSample = sample;

	return vNO_ERROR;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

//
//  We use buckets to sort event classes by priority and volume.
//  This is a new frame.  Forget all previous sorting information.
//

static void	bucketReset( void )
{
	int						i, k;
	EClassBucket	*ecb;

	for ( i = 0, ecb = eClassBucket; i < NUM_BUCKETS_PRI; i++, ecb++ )
	{
		for ( k = 0; k < NUM_BUCKETS_VOL; k++ )
		{
			ListInit( &ecb->bucketVol[ k ] );
		}
	}

	bucketStamp++;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

//
//  Place this event into buckets according to priority and volume.
//  This will be used to choose event classes for sacrifice.
//

static void	bucketAdd( AudioEvent *event )
{
	EClassBucket		*ecb;
	AudioEventClass	*eclass;
	AudioPriority		pri;
	uint			vol;

	eclass = event->eclass;
	pri = (AudioPriority) ((uint) eclass->priority + (uint) event->adjustPriority);
	DBG_ASSERT( pri < NUM_BUCKETS_PRI );
	vol = AudioAttribsGetVolume( &event->attribs ) / VOLUME_QUANTIZE;
	ecb = &eClassBucket[ pri ];

	if ( eclass->lastBucketStamp == bucketStamp )
	{
		// Already in buckets.  Check if re-sorting necessary.

		if ( pri > eclass->maxPri )
		{
			eclass->maxPri = pri;
			eclass->maxVol = vol;

			// Remove and re-insert.
			ListNodeRemove( &eclass->nodeVol );
			ListNodeInsert( &ecb->bucketVol[ vol ], &eclass->nodeVol );
		}
		else if ( pri == eclass->maxPri )
		{
			if ( vol > eclass->maxVol )
			{
				eclass->maxVol = vol;

				// Remove and re-insert.
				ListNodeRemove( &eclass->nodeVol );
				ListNodeInsert( &ecb->bucketVol[ vol ], &eclass->nodeVol );
			}
		}
	}
	else
	{
		// First use this frame.  Initialize and put in buckets.
		eclass->lastBucketStamp = bucketStamp;

		eclass->maxPri = pri;
		eclass->maxVol = vol;

		ListNodeInit( &eclass->nodeVol );
		ListNodeInsert( &ecb->bucketVol[ vol ], &eclass->nodeVol );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static void bucketRemove( AudioEventClass	*eclass )
{
	ListNodeRemove( &eclass->nodeVol );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

//
//  Get a "weaker" event class from the buckets.  It must have
//  lower priority than 'pri'.  Find the quietest such class.
//

static AudioEventClass *bucketGetWeaker( AudioPriority pri )
{
	EClassBucket		*ecb;
	ListNode				*node;
	int							i, v;

	for ( i = 0, ecb = eClassBucket; i < pri; i++, ecb++ )
	{
		for ( v = 0; v < NUM_BUCKETS_VOL; v++ )
		{
			node = ListFirstItem( &ecb->bucketVol[ v ] );

			if ( node )
			{
				AudioEventClass *event = (AudioEventClass*) node;

				event = (AudioEventClass*) ( (uint) event - ( (uint) &event->nodeVol - (uint) event ));
				return event;
			}
		}
	}

	return NULL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

//
// Destroy an entire class of events to make room for newEvent.
// Returns TRUE if any events were killed, FALSE otherwise.
//

static int	eventClassSacrifice( AudioEvent *newEvent )
{
	AudioEvent *event, *next, *head;
	AudioEventClass	*eclassSacrifice = NULL;
	int				killed = FALSE;

	eclassSacrifice = bucketGetWeaker( (AudioPriority) ( (uint) newEvent->eclass->priority
			+ (uint) newEvent->adjustPriority) );

	if ( eclassSacrifice == NULL )
	{
		return FALSE;
	}

	#if DEBUG_EVENT_CONTENTION
		DBGPRINTF(( "eventClassSacrifice: class '%s' killed by class '%s'.\n",
			AudioEventClassName (eclassSacrifice), AudioEventName ( newEvent)));
	#endif

	bucketRemove( eclassSacrifice );

	head = (AudioEvent *) &audioEvents;
	event = (AudioEvent *) head->nd.next;

	while ( event != head )
	{
		next = (AudioEvent *) event->nd.next;

		if ( event->eclass == eclassSacrifice )
		{
			AudioEventKill( event );
			audioEventDestroy( event );
			killed = TRUE;
		}

		event = next;
	}

	return killed;	// Technically, if we get here, should always be TRUE.
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static AudioChannel*	audioPrepareChannel ( int pri, int flags )
{
	AudioChannel		*chan = NULL;


	if ( audioDevice && ( chan = AudioDeviceGetChannel( audioDevice, AUDIO_CHANNEL_TYPE_STD )))
	{
		if ( !(chan->Control.Status & (mAUDIO_CTRL_PLAYING|mAUDIO_CTRL_PAUSED|mAUDIO_CTRL_INUSE) ) || chan->Control.Priority <= pri )
		{
			#ifdef _DEBUGx
			if (chan->Control.Status & (mAUDIO_CTRL_PLAYING|mAUDIO_CTRL_PAUSED|mAUDIO_CTRL_INUSE) )
			{
				DBGPRINTF (("Stealing channel 0x%08x\n", chan));
			}
			#endif

			AudioChannelStop ( chan );
			chan->Control.Priority = pri;

			if (flags & mAUDIO_LOOP )
			{
				chan->Control.LoopCount = AUDIO_CTRL_LOOP_FOREVER;
			}
			else
			{
				chan->Control.LoopCount = 0;
			}

		}
		else
		{
			goto error;
		}
	}
#if 0
	else
	{
		DBGPRINTF(( "Could not get a channel\n" ));
	}
#endif

	return chan;

error:

	return NULL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int		audioEventPrep ( AudioEvent *event )
{
	AudioEventClass	*eclass;


	DBG_ASSERT_TYPE ( event, AudioEvent );

	eclass = event->eclass;

	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	if ( eclass->numSamples == 0 || !eclass->valid)
	{
		return FALSE;
	}

	event->loopCount = 0;

	if ( !(event->channel = audioPrepareChannel ( eclass->priority + event->adjustPriority, 0 ) ))
	{
		return FALSE;
	}

	event->fshift = 100 + AudioRandomPick( eclass->minFShift, eclass->maxFShift );
	event->vshift = AudioRandomPick( 0, eclass->vShift );

	AudioChannelUse ( event->channel );
	event->channel->Data = event;
	event->channel->CB_NextSample = audioEventNextSample;
	event->channel->CB_SampleDone = audioEventSampleDone;
	event->channel->CB_Stop = audioEventStop;
	event->state = AUDIO_EVENT_START_PLAYING;
	event->channel->SfxAttribs = &event->attribs;
	event->channel->CompAttribs = eclass->volumeCompression ? &audioCompressionAttribs : NULL ;

	event->channel->FadeAttribs = eclass->fadeAttribs;
	event->channel->GroupAttribs = eclass->masterAttribs;


	if ( eclass->maxDelay > EVENT_MIN_DELAY )
	{
		if ( !(eclass->control & (AUDIO_EVENT_CTRL_POSTDELAY)))
		{
			int	delay;
		
			delay = AudioRandomPick(  (eclass->control&AUDIO_EVENT_CTRL_AMBIENT) ? EVENT_MIN_DELAY : eclass->minDelay, eclass->maxDelay );
		
			if ( delay >= EVENT_MIN_DELAY )
			{
				eventWait ( event, MSECONDS(delay), AUDIO_EVENT_START_PLAYING );
			}
		}
	}

	// NOTE: we do not actually start the event playing

	return TRUE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static int audioEventStart ( AudioEvent *event )
{
	AudioSample	*sample;

	DBG_ASSERT_TYPE ( event, AudioEvent );

	DBG_MSGASSERT ( event->state == AUDIO_EVENT_START_PLAYING, ("event in a bad state"));
	DBG_MSGASSERT ( !AudioEventIsPaused(event), ("trying to start a paused event"));


	if ( event != (AudioEvent *)event->channel->Data )
	{
		// channel was stolen;
		return FALSE;
	}

	// IMPORTANT: set state to playing before starting the channel
	// It is possible that the channel could finish before we get to set the
	// new state.

	AudioChannelLock ( event->channel );

	msg_assert ( !(event->channel->Control.Status & mAUDIO_CTRL_ACTIVE ), ("bad assert. Show this to Tommy immediately!!"));

	sample = (AudioSample *) event->startSample;
	FLAGS_SET ( event->flags, mAUDIO_EVENT_NO_ATTACK );

	AudioChannelSetSample ( event->channel, sample );
	AudioChannelSetPitch ( event->channel, event->fshift );
	int volume = AUDIO_LEVEL_MAX;
	if ( event->vshift > 0 )
	{
		volume -= (AUDIO_LEVEL_MAX * event->vshift)/100;
	}
	AudioChannelSetVolume ( event->channel, volume );
	event->state = AUDIO_EVENT_PLAYING;
	FLAGS_SET ( event->flags, mAUDIO_EVENT_PLAYING );
 	event->startSample = NULL;

	if ( AudioChannelStart ( event->channel ) != vNO_ERROR )
	{

		event->state = AUDIO_EVENT_START_PLAYING;
		event->startSample = sample;
		FLAGS_CLEAR ( event->flags, mAUDIO_EVENT_PLAYING );
		AudioChannelUnlock ( event->channel );
		return FALSE;
	}

	AudioChannelUnlock ( event->channel );
	return TRUE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static uint	eventCalcVolume ( AudioEvent *event, int new_volume )
{
	uint volume;

	DBG_ASSERT_TYPE ( event, AudioEvent );
	DBG_ASSERT_TYPE ( event->eclass, AudioEventClass );

	if ( new_volume > AUDIO_VOLUME_MAX )
	{
		new_volume = AUDIO_VOLUME_MAX;
	}
	else	if ( new_volume < AUDIO_VOLUME_MIN )
	{
		new_volume = AUDIO_VOLUME_MIN;
	}

	volume = AudioLevelApply( &event->eclass->baseLevel, new_volume );

	return volume;
}

/*****************************************************************************
**          Public Functions                                                **
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int AudioEventSetUp( AudioDevice *device, AudioCache *cache )
{

	ListInit ( &audioEvents );
	ListInit ( &audioEventClasses );

	audioDevice = device;

	AudioAttribsInit ( &audioCompressionAttribs );

	AudioEventsCount = AudioEventsPeak = 0;


	initialized = TRUE;
	eventsOn = TRUE;
	eventsOK = FALSE;

	audioCache = cache;
	if ( !audioCache || !audioDevice )
	{
		return FALSE;
	}


	audioEventPool = MemoryPoolCreate ( MAX_EVENTS, sizeof ( AudioEvent ) );
	DBGPRINTF (( "event mempool size = %d\n", MAX_EVENTS * sizeof (AudioEvent )));

	eventsOK = audioEventPool != NULL;

	return TRUE;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void AudioEventCloseDown( void )
{
	AudioEventClass *eclass;
	AudioKillAllEvents ( );
	AudioFlushAllDeadEvents ();

	while ( eclass = (AudioEventClass *) ListFirstItem ( &audioEventClasses ))
	{
		ListNodeRemove ( &eclass->nd );
		for( Int i = 0 ; i < MAX_AUDIO_EVENT_SAMPLES; i++ )
		{
			if ( eclass->sampleName[i] )
			{
				// allocated with strdup
				free( eclass->sampleName[i] );
			}	
		}
		MEM_Free ( eclass );
	}

	if ( audioCache )
	{
		audioCache = NULL;
	}

	if ( audioEventPool )
	{
		MemoryPoolDestroy ( audioEventPool );
		audioEventPool = NULL;
	}

	eventsOK = FALSE;
	initialized = FALSE;
}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioServiceAllEvents ( void )
{
	static int in = FALSE;
	AudioEvent	*event, *next, *head;
	int					loudness;
	int					activeEvents = 0;
	int					prepedEvents = 0;
	int					processedEvents = 0;

	if ( !initialized || in )
	{
		return;
	}

	in = TRUE;


	#ifndef IG_FINAL_RELEASE
	{
		static int lastFrame = 0;
	
		if ( lastFrame != AudioGameFrame )
		{
			if ( audioCache )
			{
				ProfCacheNewFrame ( &audioCache->profile );
			}
			lastFrame = AudioGameFrame;
		}
	
	}
	#endif

	frameStamp++;		// new frame

	// because of volume compression we require two passes
	// because of channel stealing we require three passes
	// because of event culling we require four passes

	head = (AudioEvent *) &audioEvents;

	// FIRST PASS: sort events locally by volume

	event = (AudioEvent *) head->nd.next;

	while ( event != head )
	{

		DBG_ASSERT_TYPE ( event, AudioEvent );
		DBG_ASSERT_TYPE ( event->eclass, AudioEventClass );

		if ( event->eclass->lastFrame != frameStamp )
		{
			// reset local list
			ListInit ( &event->eclass->local);
			event->eclass->count = 0;
			event->eclass->lastFrame = frameStamp;
		}

		AudioAttribsUpdate ( &event->attribs );

		if ( event->state != AUDIO_EVENT_DONE )
		{
			eventAddLocalSort ( event );
		}
		event = (AudioEvent *) event->nd.next;
	}

	// SECOND PASS: service all events so that we know what
	// we are required to do

	event = (AudioEvent *) head->nd.next;

	while ( event != head )
	{

		if ( event->state != AUDIO_EVENT_DONE )
		{
			processedEvents ++;
		}

		AudioEventService ( event );
		event = (AudioEvent *) event->nd.next;
	}

	// THIRD PASS: workout the volume compression level
	// and remove dead events

	event = (AudioEvent *) head->nd.next;
	loudness = 0;
	bucketReset();		// clear our memory of what classes are in use

	while ( event != head )
	{
		next = (AudioEvent *) event->nd.next;

		if ( !event->channel
				 || event != (AudioEvent *) event->channel->Data )
		{
			#if DEBUG_EVENT_CONTENTION
			{
				AudioEvent	thief;

				if ( event->channel )
				{
					// Channel must have been stolen, then.
					thief = (AudioEvent *) event->channel->Data;
					DBGPRINTF(( "AudioServiceAllEvents: class '%s' stole channel from '%s'.\n",
						AudioEventName ( thief ), AudioEventName ( event ) ));
				}
			}
			#endif

			// We have no channel.  No point in going on.
			AudioEventKill( event );
		}

		if ( event->flags & mAUDIO_EVENT_DEAD )
		{
			audioEventDestroy( event );
		}
		else
		{
			if ( !AudioEventIsPaused ( event ))
			{
				loudness += AudioLevelApply ( &event->eclass->baseLevel, AVERAGE_LOUDNESS );
			}
			bucketAdd( event );
		}

		event = next;
	}

	if ( loudness > FULL_LOUDNESS )
	{
		AudioAttribsAdjustVolume ( &audioCompressionAttribs, (AUDIO_LEVEL_MAX * FULL_LOUDNESS)/loudness );
	}
	else
	{
		AudioAttribsAdjustVolume ( &audioCompressionAttribs, AUDIO_LEVEL_MAX);
	}

	AudioAttribsUpdate ( &audioCompressionAttribs );

	// FOURTH PASS: now that the compression level has been set
	// start any events that have been preped

	event = (AudioEvent *) head->nd.next;

	while ( event != head )
	{
		next = (AudioEvent *) event->nd.next;

		if ( event->state == AUDIO_EVENT_START_PLAYING && !AudioEventIsPaused( event ) )
		{
			eventLoadAndLockSamples ( event );

			while ( !( event->startSample = eventFirstSample( event ) ) )
			{
				// The time for subtlety has passed.  Attempt to clear
				// enough cache space for this event by destroying an
				// entire event class of lower priority (if any).  Do
				// this until our event gets into the cache.

				if ( eventClassSacrifice( event ) )
				{
					next = (AudioEvent *) event->nd.next;	// May have been killed.
					eventLoadAndLockSamples ( event );
				}
				else
				{
					#if DEBUG_EVENT_CONTENTION
						// We could not find an event class of lower priority.
						DBGPRINTF(( "AudioServiceAllEvents: class '%s' could not get cache.\n",
							AudioEventName ( event->eclass) ));

						if ( event->channel->Control.Priority >= AUDIO_EVENT_CRITICAL_PRIORITY )
						{
							AudioEvent	event2, next2, head2;

							DBGPRINTF(( "Event dump -- these events are alive and own cache:\n" ));

							head2 = (AudioEvent *) &audioEvents;
							event2 = (AudioEvent *) head2->nd.next;

							while ( event2 != head2 )
							{
								next2 = (AudioEvent *) event2->nd.next;
								if ( event2 != event )
								{
									DBGPRINTF(( "- '%s'\n", AudioEventName ( event2 ) ));
								}
								event2 = next2;
							}
						}
					#endif

					break;
				}
			}

			if ( !( event->startSample = eventFirstSample( event ) )
					 || !audioEventStart( event ) )
			{
				AudioEventKill ( event );
				audioEventDestroy (  event );
			}
			else
			{
				activeEvents++;
			}
			prepedEvents++;
		}

		event = next;
	}


//	DBGPRINTF (("compression = %3d (%5d) - active events = %3d of %3d; preped = %3d\n",
//					(AudioAttribsGetVolume ( &audioCompressionAttribs ) *100) /AUDIO_LEVEL_MAX,
//					loudness,	activeEvents, processedEvents, prepedEvents));


	in = FALSE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioKillAllEvents ( void )
{
	AudioEvent	*event, *next, *head;


	if ( !initialized )
	{
		return;
	}

	head = (AudioEvent *) &audioEvents;

	event = (AudioEvent *) head->nd.next;

	while ( event != head )
	{
		next = (AudioEvent *) event->nd.next;

		AudioEventKill ( event );
		audioEventDestroy ( event );

		event = next;
	}


}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioPauseAllEvents ( void )
{
	AudioEvent	*event, *head;


	if ( !initialized )
	{
		return;
	}

	head = (AudioEvent *) &audioEvents;

	event = (AudioEvent *) head->nd.next;

	while ( event != head )
	{
		AudioEventPause ( event );
		event = (AudioEvent *) event->nd.next;
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioResumeAllEvents ( void )
{
	AudioEvent	*event, *head;


	if ( !initialized )
	{
		return;
	}

	head = (AudioEvent *) &audioEvents;

	event = (AudioEvent *) head->nd.next;

	while ( event != head )
	{
		AudioEventResume ( event );
		event = (AudioEvent *) event->nd.next;
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioFlushAllDeadEvents ( void )
{
	AudioEvent	*event, *next, *head;


	if ( !initialized )
	{
		return;
	}

	head = (AudioEvent *) &audioEvents;

	event = (AudioEvent *) head->nd.next;

	while ( event != head )
	{
		next = (AudioEvent *) event->nd.next;

		if ( event->flags & mAUDIO_EVENT_DEAD )
		{
			audioEventDestroy ( event );
		}

		event = next;
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void AudioValidateAllEventClasses ( void )
{
	AudioEventClass *eclass;
	eclass = (AudioEventClass *) ListFirstItem ( &audioEventClasses );

	while ( eclass )
	{
		DBG_ASSERT_TYPE ( eclass, AudioEventClass );

		eclass->valid = TRUE;
		eclass = (AudioEventClass *) ListNextItem ( &eclass->nd);
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

Int							AudioEventCount( void )
{
	return ListCountItems( &audioEventClasses );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioEventClass*AudioGetFirstEventClass( void )
{
	return (AudioEventClass *) ListFirstItem ( &audioEventClasses );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioEventClass*AudioEventClassNext ( AudioEventClass *eclass	)
{
	if ( eclass )
	{
		return (AudioEventClass *) ListNextItem ( &eclass->nd);
	}

	return NULL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int							AudioEventClassRange ( AudioEventClass *eclass	)
{

	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	if ( !initialized )
	{
		return 0;
	}

	return eclass->range;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int							AudioEventClassMinVolume ( AudioEventClass *eclass	)
{

	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	if ( !initialized )
	{
		return 0;
	}

	return eclass->min_volume;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int							AudioEventClassValid ( AudioEventClass *eclass	)
{

	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	if ( !initialized )
	{
		return FALSE;
	}

	return eclass->valid;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioEvent	*AudioEventCreate ( AudioEventClass	*eclass )
{
	AudioEvent		*event;
	int allocated = FALSE;

	if ( !eventsOn || !eventsOK )
	{
		return NULL;
	}

	DBG_ASSERT_TYPE ( eclass, AudioEventClass );


	if ( !eclass->valid )
	{
		return NULL;
	}

	if ( !(event = (AudioEvent *) MemoryPoolGetItem ( audioEventPool )))
	{
		AudioFlushAllDeadEvents ( );

		if ( !(event = (AudioEvent *) MemoryPoolGetItem ( audioEventPool )))
		{
			ALLOC_STRUCT ( event, AudioEvent);
			allocated = TRUE;

			if ( !event )
			{
				DBGPRINTF (("event pool overflow\n"));
				return NULL;
			}
		}
	}

	AudioEventsCount++;
	if ( AudioEventsCount > AudioEventsPeak )
	{
		AudioEventsPeak = AudioEventsCount;
	}

	// initialise the event
	memset ( event, 0, sizeof ( AudioEvent ));

	event->eclass = eclass;
	ListNodeInit ( &event->nd );
	event->state = AUDIO_EVENT_NEW;
	event->adjustPriority = 0;
	event->numItems = event->numSequence = event->numLoadSequence = 0;
	AudioAttribsInit ( &event->attribs );
	LockInit ( &event->paused );

	DBG_SET_TYPE ( event, AudioEvent );

	ListAddToTail ( &audioEvents, &event->nd );
	event->stamp = audioStamp++;

	if ( allocated )
	{
		FLAGS_SET ( event->flags, mAUDIO_EVENT_ALLOCATED );
	}

	return event;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void						AudioEventNoAttack ( AudioEvent *event )
{
	DBG_ASSERT_TYPE ( event, AudioEvent );
	DBG_ASSERT ( initialized );

	FLAGS_SET ( event->flags, mAUDIO_EVENT_NO_ATTACK );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioEventKill ( AudioEvent *event )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );
	DBG_ASSERT ( initialized );

	if ( event->flags & mAUDIO_EVENT_DEAD )
	{
		return;
	}

	eventReleaseChannel ( event );
	eventUnlockSamples ( event );
	event->state = AUDIO_EVENT_DONE;
	event->stamp = AUDIO_INVALID_STAMP;
	event->handle = NULL;

	FLAGS_SET ( event->flags, mAUDIO_EVENT_DEAD );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioEventPause ( AudioEvent *event )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );
	DBG_ASSERT ( initialized );

	if ( !AudioEventIsPaused ( event ))
	{
		if ( event->state == AUDIO_EVENT_WAITING )
		{
			event->timeOut -= AudioGetTime ();
		}
	}

	LockAcquire ( &event->paused );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioEventResume ( AudioEvent *event )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );
	DBG_ASSERT ( initialized );

	if ( AudioEventIsPaused ( event ))
	{
		LockRelease ( &event->paused );
		if ( !AudioEventIsPaused ( event ) )
		{
			if ( event->state == AUDIO_EVENT_WAITING )
			{
				event->timeOut += AudioGetTime();
			}

		}
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int				AudioEventIsPaused ( AudioEvent *event )
{
	DBG_ASSERT_TYPE ( event, AudioEvent );
	return Locked ( &event->paused );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		AudioEventEnd ( AudioEvent *event )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );
	DBG_ASSERT ( initialized );

	if ( event->flags & mAUDIO_EVENT_END )
	{
		return;
	}

	if ( event->channel && event == (AudioEvent *) event->channel->Data )
	{
		AudioChannelLock ( event->channel );
		FLAGS_SET ( event->flags, mAUDIO_EVENT_END | mAUDIO_EVENT_DO_END );
		AudioChannelUnlock ( event->channel );
	}
	else
	{
		FLAGS_SET ( event->flags, mAUDIO_EVENT_END | mAUDIO_EVENT_DO_END );
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioEventService ( AudioEvent *event )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );
	DBG_ASSERT ( initialized );

	if ( !eventsOn )
	{
		event->state = AUDIO_EVENT_DONE;
	}



retry:

	switch ( event->state )
	{

		case AUDIO_EVENT_NEW:
			{
					if ( !audioEventPrep (  event ) )
					{
						AudioEventKill ( event );
					}

			}
			break;

		case AUDIO_EVENT_START_PLAYING:
			break;

		case AUDIO_EVENT_PLAYING:
				// check for rogue looping event
				if ( AudioEventNeverEnds ( event ) )
				{
					if ( !(event->flags & mAUDIO_EVENT_END ) && (!event->handle || AudioEventHandleGet ( event->handle ) != event))
					{
						// we have a looping smaple that has no handle attacked to it.
						// this means that the event can never be stopped (unless it gets bumped
						// by a higher priority event), which probable means that something has got wrong

						msg_assert ( FALSE, ("A looping audio event '%s' is playing but has no handle. \n\nPlease tell Tommy about this assert. It is ok to ignore this assert", AudioEventName ( event )));
						AudioEventKill ( event );
					}
				}

			if ( event->flags & mAUDIO_EVENT_DO_END )
			{

			do_ending:

				AudioChannel *chan = event->channel;


				if ( chan )
				{
					AudioChannelLock ( chan );

					FLAGS_CLEAR ( event->flags , mAUDIO_EVENT_DO_END );

					AudioChannelStop ( event->channel );

					if ( event->eclass->control & AUDIO_EVENT_CTRL_DECAY && !(event->flags & mAUDIO_EVENT_NO_DECAY))
					{
						event->state = AUDIO_EVENT_START_PLAYING;
						event->startSample = eventNextSample ( event );

						//msg_warning ( event->startSample, ("You can safely ignore this assert, but let me know about it.\n\nThanks, Tommy - ext 205"));

						if ( !AudioEventIsPaused ( event ))
						{
							if ( !audioEventStart ( event ) )
							{
								AudioEventKill ( event );
							}
						}
					}

					AudioChannelUnlock ( chan );
				}
				else
				{
					FLAGS_CLEAR ( event->flags , mAUDIO_EVENT_DO_END );
				}
			}
			break;

		case AUDIO_EVENT_WAITING:
			{
				if ( event->flags & mAUDIO_EVENT_DO_END )
				{
					 goto do_ending;
				}

				if ( !AudioEventIsPaused( event ) && AudioGetTime () > event->timeOut )
				{
					event->state = event->nextState;
					goto retry;
				}
			}
			break;

		case AUDIO_EVENT_DONE:
			{
						AudioEventKill ( event );
			}
			break;

		default:
			DBG_MSGASSERT ( FALSE, ("bad event type"));


	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioEventHandleInit ( AudioEventHandle *handle )
{

	DBG_ASSERT_PTR ( handle );

	handle->event = NULL;
	handle->stamp = AUDIO_INVALID_STAMP;
	handle->eclass = NULL;
	handle->id = ID_HANDLE;

	DBG_SET_TYPE ( handle, AudioEventHandle );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioEventHandleDeinit ( AudioEventHandle *handle )
{

	DBG_ASSERT_TYPE ( handle, AudioEventHandle );

	AudioEventHandleStop ( handle );

	handle->id = 0;
	DBG_INVALIDATE_TYPE ( handle );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioEventHandleStop ( AudioEventHandle *handle )
{

	DBG_ASSERT_TYPE ( handle, AudioEventHandle );

	AudioEvent *event = AudioEventHandleGet ( handle );

	if ( event )
	{
		AudioEventKill ( event );
		handle->event = NULL;
	}

	handle->eclass = NULL;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioEventHandleStopLooping ( AudioEventHandle *handle )
{

	DBG_ASSERT_TYPE ( handle, AudioEventHandle );

	AudioEvent *event = AudioEventHandleGet ( handle );

	if ( event && AudioEventNeverEnds ( event ))
	{
		AudioEventKill ( event );
		handle->event = NULL;
	}
	handle->eclass = NULL;

}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioEventHandleEnd ( AudioEventHandle *handle )
{

	DBG_ASSERT_TYPE ( handle, AudioEventHandle );

	AudioEvent *event = AudioEventHandleGet ( handle );

	if ( event )
	{
		AudioEventEnd ( event );
		handle->event = NULL;
	}
	handle->eclass = NULL;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioEventHandleEndLooping ( AudioEventHandle *handle )
{

	DBG_ASSERT_TYPE ( handle, AudioEventHandle );

	AudioEvent *event = AudioEventHandleGet ( handle );

	if ( event && AudioEventNeverEnds ( event ))
	{
		AudioEventEnd ( event );
		handle->event = NULL;
	}
	handle->eclass = NULL;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void				AudioEventHandleSet ( AudioEventHandle *handle, AudioEvent *event, AudioEventClass *eclass )
{

	DBG_ASSERT_TYPE( handle, AudioEventHandle );

	if ( handle->id != ID_HANDLE )
	{
		return;
	}

 	handle->eclass = eclass;
	if ( ( handle->event = event ))
	{
		DBG_ASSERT_TYPE ( event, AudioEvent  );

		handle->stamp = event->stamp;
		if ( !eclass )
		{
			handle->eclass = event->eclass;
		}
		event->handle = handle;
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioEvent*	AudioEventHandleGet ( AudioEventHandle *handle )
{
	AudioEvent *event;


	DBG_ASSERT_TYPE ( handle, AudioEventHandle );

	if (!initialized || handle->id != ID_HANDLE )
	{
		return NULL;
	}

	if ( ( event = handle->event) )
	{
		if ( ( handle->eclass != event->eclass ) || handle->stamp != event->stamp )
		{
			handle->event = event = NULL;
		}

	}

	return event;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioEventClass*	AudioEventHandleGetClass ( AudioEventHandle *handle )
{

	DBG_ASSERT_TYPE ( handle, AudioEventHandle );

	if (!initialized || handle->id != ID_HANDLE )
	{
		return NULL;
	}

	return handle->eclass;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioEventSetVolume ( AudioEvent *event, int new_volume )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );

	AudioLevelSet ( &event->attribs.VolumeLevel, eventCalcVolume ( event, new_volume) );
	AudioLevelUpdate ( &event->attribs.VolumeLevel );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioEventAdjustVolume ( AudioEvent *event, int new_volume )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );

	if ( event->state == AUDIO_EVENT_PLAYING )
	{
		AudioLevelAdjust ( &event->attribs.VolumeLevel, eventCalcVolume ( event, new_volume) );
	}
	else
	{
		AudioEventSetVolume ( event, new_volume );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static uint eventCalcPan ( int new_pan )
{

	// convert to audiolevel pan
	if ( new_pan > AUDIO_PAN_RIGHT )
	{
		new_pan = AUDIO_PAN_RIGHT;
	}
	else if ( new_pan < AUDIO_PAN_LEFT )
	{
		new_pan = AUDIO_PAN_LEFT;
	}

	return new_pan;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioEventSetPan ( AudioEvent *event, int new_pan )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );


	AudioLevelSet ( &event->attribs.PanPosition, eventCalcPan ( new_pan ) );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioEventAdjustPan ( AudioEvent *event, int new_pan )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );

	if ( event->state == AUDIO_EVENT_PLAYING )
	{
		AudioLevelAdjust ( &event->attribs.PanPosition, eventCalcPan ( new_pan ) );
	}
	else
	{
		AudioEventSetPan ( event, new_pan );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioEventSetAdjustDuration ( AudioEvent *event, TimeStamp time )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );
	AudioLevelSetDuration ( &event->attribs.VolumeLevel, time, AUDIO_LEVEL_MAX );
	AudioLevelSetDuration ( &event->attribs.PitchLevel, time, AUDIO_LEVEL_MAX );
	AudioLevelSetDuration ( &event->attribs.PanPosition, time, AUDIO_LEVEL_MAX );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioEventClass*	AudioEventGetClass ( AudioEvent *event )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );
	return event->eclass;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int	AudioEventNeverEnds ( AudioEvent *event )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );
	return AudioEventClassNeverEnds ( event->eclass );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void						AudioEventSetTimeOfDay ( AudioEvent *event, int new_tod )
{
	DBG_ASSERT_TYPE ( event, AudioEvent );
	event->timeOfDay = new_tod;
}



/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

char*  AudioEventName ( AudioEvent *event )
{

	DBG_ASSERT_TYPE ( event, AudioEvent );

	if ( !initialized )
	{
		return "no events";
	}

	if ( !event->eclass )
	{
		return "unknown";
	}

	return AudioEventClassName ( event->eclass);
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

char*  AudioEventClassName ( AudioEventClass *eclass )
{

	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	if ( !initialized )
	{
		return "no events";
	}

	if ( !eclass->name )
	{
		return "(null)";
	}

	return eclass->name;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioEventClassSetName ( AudioEventClass *eclass, char *new_name )
{

	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	eclass->name = new_name;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void AudioEventClassSetFadeAttribs ( AudioEventClass *eclass, AudioAttribs *attribs )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	eclass->fadeAttribs = attribs;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void AudioEventClassSetMasterAttribs ( AudioEventClass *eclass, AudioAttribs *attribs )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	eclass->masterAttribs = attribs;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void						AudioEventClassSetData ( AudioEventClass *eclass, void *data)
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	eclass->data = data;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void*						AudioEventClassGetData ( AudioEventClass *eclass )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	return eclass->data;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

const char* AudioEventClassGetSoundName  ( AudioEventClass *eclass, int index )
{
	static const char *bogus = "";
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	if (index < 0 || index >= eclass->numSamples) {
		return bogus;
	}

	return eclass->sampleName[index];
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioEventClass *AudioEventClassCreate ( void )
{
	AudioEventClass *eclass;

	if ( !initialized )
	{
		return NULL;
	}


	ALLOC_STRUCT ( eclass, AudioEventClass );

	audioInitEventClass ( eclass );

	ListAddToTail ( &audioEventClasses, &eclass->nd );

	return eclass;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int	AudioEventClassAddSound ( AudioEventClass *eclass, const char *name )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	if ( eclass->numSamples == MAX_AUDIO_EVENT_SAMPLES )
	{
		msg_warning ( FALSE, ("Too many samples in audio event '%s' (Max %d)", AudioEventClassName ( eclass), MAX_AUDIO_EVENT_SAMPLES ));
		return FALSE;
	}

	if ( initialized )
	{

		if ( name != NULL )
		{
			eclass->sampleName[eclass->numSamples++] = strdup( name );
		}
		else
		{
			//DBGPRINTF (("Missing sample %s\n", name ));
			eclass->valid = FALSE;
			return FALSE;
		}
	}

	return TRUE;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int							AudioEventClassNumSound ( AudioEventClass *eclass )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );
	return eclass->numSamples;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void						AudioEventClassSetAttackCount ( AudioEventClass *eclass, int count )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );
	msg_assert ( count >= 0,("Attack count must be positive")  );
	eclass->attackCount = count;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void						AudioEventClassSetDecayCount ( AudioEventClass *eclass, int count )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );
	msg_assert ( count >= 0,("Decay count must be positive")  );
	eclass->decayCount = count;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void						AudioEventClassSetMorningSoundCount( AudioEventClass *eclass, int count )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );
	msg_assert ( count >= 0,("Morning Sound count must be positive")  );
	eclass->morningCount = count;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void						AudioEventClassSetEveningSoundCount( AudioEventClass *eclass, int count )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );
	msg_assert ( count >= 0,("Evening Sound count must be positive")  );
	eclass->eveningCount = count;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void						AudioEventClassSetNightSoundCount( AudioEventClass *eclass, int count )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );
	msg_assert ( count >= 0,("Night Sound count must be positive")  );
	eclass->nightCount = count;
}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioEventClassSetPriority ( AudioEventClass *eclass, AudioPriority pri )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );
	eclass->priority = pri;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioEventClassSetVolume ( AudioEventClass *eclass, int vol )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );
	AudioLevelSet ( &eclass->baseLevel, vol );
	AudioLevelUpdate ( &eclass->baseLevel );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioEventClassSetControl ( AudioEventClass *eclass, AudioEventControl control )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );
	eclass->control = control;
#ifdef _DEBUG
	msg_warning ( !( !eclass->format_same
									&& (control & (AUDIO_EVENT_CTRL_LOOP|AUDIO_EVENT_CTRL_ALL )
									== (AUDIO_EVENT_CTRL_LOOP|AUDIO_EVENT_CTRL_ALL ) )),
									("Audio event \"%s\" set to \"loop all\", however not all sounds are of the same format\n"
									 "\nSame format means they must have the same Playback rate, same original sample width, and same number of channels", AudioEventClassName (eclass) ));
#endif	//_DEBUG

	if ( eclass->control & AUDIO_EVENT_CTRL_ATTACK )
	{
		if ( eclass->attackCount == 0)
		{
			eclass->attackCount = 1	;
		}
	}
	else
	{
		eclass->attackCount = 0;
	}

	if ( eclass->control & AUDIO_EVENT_CTRL_DECAY )
	{
		if ( eclass->decayCount == 0)
		{
			eclass->decayCount = 1;
		}
	}
	else
	{
		eclass->decayCount = 0;
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioEventClassSetLimit ( AudioEventClass *eclass, int limit )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );
	eclass->limit = limit;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioEventClassSetRange ( AudioEventClass *eclass, int range )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );
	eclass->range = range;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioEventClassSetMinVolume ( AudioEventClass *eclass, int min_vol )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );
	eclass->min_volume = min_vol;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioEventClassSetDelay ( AudioEventClass *eclass, int minDelay, int maxDelay )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	msg_assert ( maxDelay >= minDelay, ("bad parameters on SetDelay"));
	eclass->minDelay = minDelay;
	eclass->maxDelay = maxDelay;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioEventClassSetPitchShift ( AudioEventClass *eclass, int minShift, int maxShift )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	msg_assert ( maxShift >= minShift, ("bad parameters on SetPitchShift"));
	eclass->minFShift = minShift;
	eclass->maxFShift = maxShift;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioEventClassSetVolumeShift ( AudioEventClass *eclass, int VShift)
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	if ( VShift > 100 )
	{
		VShift = 100;
	}
	else if ( VShift < 0 )
	{
		VShift = 0;
	}
	eclass->vShift = VShift;
}

//============================================================================
// AudioEventClassSetVolumeCompression 
//============================================================================

void	AudioEventClassSetVolumeCompression ( AudioEventClass *eclass, int on)
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	eclass->volumeCompression = on;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioEventClassReset ( AudioEventClass *eclass )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );
	ListNode nd;
	char *name;
	AudioAttribs *fadeAttribs;
	AudioAttribs *masterAttribs;


	// save fields that do not change
	nd = eclass->nd;
	name = eclass->name;
	masterAttribs = eclass->masterAttribs;
	fadeAttribs = eclass->fadeAttribs;

	memset ( eclass, 0, sizeof(AudioEventClass ));

	eclass->nd = nd;
	eclass->masterAttribs = masterAttribs;
	eclass->fadeAttribs = fadeAttribs;
	DBG_SET_TYPE ( eclass, AudioEventClass );

	eclass->valid = TRUE;
	eclass->control = AUDIO_EVENT_CTRL_NONE;
	eclass->limit = AUDIO_EVENT_DEFAULT_LIMIT;
	eclass->priority = AUDIO_EVENT_NORMAL_PRIORITY;
	eclass->range = 10;	// in cells
	eclass->min_volume = 40;
	AudioLevelInit ( &eclass->baseLevel, AUDIO_LEVEL_MAX );
	#ifdef _DEBUG
	eclass->format_same = TRUE;
	AudioFormatInit ( &eclass->format );
	#endif
	
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void	AudioEventClassSetLoopCount ( AudioEventClass *eclass, int count )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	eclass->limitLoop = count;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int	AudioEventClassNeverEnds ( AudioEventClass *eclass )
{
	DBG_ASSERT_TYPE ( eclass, AudioEventClass );

	return ((eclass->control & AUDIO_EVENT_CTRL_LOOP) && !eclass->limitLoop);
}



/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void						AudioEventTurnOff ( void )
{
	eventsOn = FALSE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void						AudioEventTurnOn ( void )
{
	eventsOn = TRUE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int				AudioEventOn ( void )
{
	return eventsOn;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void						AudioEventsDump ( void (*print) ( char *text ))
{
	if ( audioCache )
	{
		char buffer[200];
		ProfCacheText ( &audioCache->profile, print );
		sprintf ( buffer, "AudioEvents : %05d - Max: %05d\n", AudioEventsCount, AudioEventsPeak);
		print ( buffer );

	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void					AudioFlushCache ( void )
{
	if ( audioCache )
	{
		AudioCacheInvalidate ( audioCache );
		while ( AudioCacheFreeOldestItem( audioCache ) );
	}
}


