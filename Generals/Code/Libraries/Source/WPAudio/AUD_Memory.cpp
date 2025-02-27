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
** File name:  audmpool.cpp                                                 **
**                                                                          **
** Created by:  10/23/95 TR                                                 **
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

// 'assignment within condition expression'.
#pragma warning(disable : 4706)


DBG_DECLARE_TYPE ( AudioMemoryPool )
DBG_DECLARE_TYPE ( MemoryItem )

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

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioMemoryPool*		MemoryPoolCreate( uint items, uint size )
{
	uint		poolsize;
	AudioMemoryPool	*pool;
	MemoryItem 	*item;
	uint		i;

	
	DBG_ASSERT ( items > 0 );
	DBG_ASSERT ( size > 0 );

	poolsize = items*(size + sizeof(MemoryItem)) + sizeof (AudioMemoryPool);

	if ((pool = (AudioMemoryPool *) AudioMemAlloc ( poolsize )))
	{

		item = (MemoryItem *)( (uint)pool + (uint) sizeof(AudioMemoryPool));
		
		pool->next = item;
		DBG_CODE ( pool->itemsOut = 0;)
		DBG_CODE ( pool->numItems = items;)
		DBG_SET_TYPE ( pool, AudioMemoryPool );

		for ( i=0; i < items-1; i++)
		{
			DBG_SET_TYPE ( item, MemoryItem );
			item->next = (MemoryItem *) ( (uint) item  + (uint) (sizeof(MemoryItem) + size) );
			item = item->next;
		}

		item->next = NULL;
		DBG_SET_TYPE ( item, MemoryItem );

	}

	return pool;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			MemoryPoolDestroy ( AudioMemoryPool *pool )
{

	DBG_ASSERT_TYPE ( pool, AudioMemoryPool );
	
	DBG_CODE
	(
	   	if ( pool->itemsOut > 0 )
		{
		   	DBGPRINTF ( ( "Destroying memory pool with %d items still out\n", pool->itemsOut) );
		}
	)
	
	AudioMemFree ( pool );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void 		  *MemoryPoolGetItem ( AudioMemoryPool *pool )
{
	MemoryItem *item = NULL;


	DBG_ASSERT_TYPE ( pool, AudioMemoryPool );

	if (! (item = pool->next) )
	{
		return NULL;
	}
	
	DBG_CODE ( pool->itemsOut++;)
	
	DBG_MSGASSERT ( pool->itemsOut <= pool->numItems,( "pool overflow" ));

	DBG_ASSERT_TYPE ( item, MemoryItem ); //  !!! Memory corruption !!!
	
	pool->next = item->next;
	
	return (void *) ( (uint) item + (uint) sizeof(MemoryItem));

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			MemoryPoolReturnItem ( AudioMemoryPool *pool, void *data )
{
	MemoryItem	*item;


	DBG_ASSERT_TYPE ( pool, AudioMemoryPool );
	DBG_ASSERT_PTR ( data );

	item = (MemoryItem *) ( (uint) data - (uint) sizeof(MemoryItem));
	
	DBG_ASSERT_TYPE ( item, MemoryItem ); //  returning invalid item to pool 
	
	item->next = pool->next;
	pool->next = item;
	
	DBG_MSGASSERT ( pool->itemsOut > 0,( "Pool underflow" )); //  returning more items than were taken 
	
	DBG_CODE ( pool->itemsOut--; )
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int 		  MemoryPoolCount ( AudioMemoryPool *pool )
{
	MemoryItem *item = NULL;
	int	count = 0;


	DBG_ASSERT_TYPE ( pool, AudioMemoryPool );

	if ( (item = pool->next) )
	{
		while ( item )
		{
		    count++;
		    item = item->next;
		}
	}
	
	return count;
}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void AudioAddSlash ( char *string )
{
	int len = strlen ( string );

	if ( len )
	{
		if ( string[len-1] != '\\' )
		{
			string[len] = '\\';
			string[len+1] = 0;
		}
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int AudioHasPath ( const char *string )
{
	return ( strchr ( string, ':' ) || strchr ( string, '\\' ) || strchr ( string, '/' ) || strchr ( string, '.'));
}

