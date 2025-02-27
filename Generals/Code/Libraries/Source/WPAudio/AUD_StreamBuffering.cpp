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
** File name:  audstrm.cpp                                                  **
**                                                                          **
** Created by:  11/10/95	TR                                                **
**                                                                          **
** Description: <description>                                               **
**                                                                          **
*****************************************************************************/

/*****************************************************************************
**            Includes                                                      **
*****************************************************************************/

#include <memory.h>

#include <wpaudio/altypes.h>
#include <wpaudio/list.h>
#include <wpaudio/StreamBuffering.h>
#include <wpaudio/memory.h>
#include <wsys/file.h>

// 'assignment within condition expression'.
#pragma warning(disable : 4706)

DBG_DECLARE_TYPE ( STM_SBUFFER )
DBG_DECLARE_TYPE ( STM_ACCESS )
DBG_DECLARE_TYPE ( STM_STREAM )

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

static		STM_SBUFFER* 	STM_next_sbuffer ( STM_SBUFFER * sbuf );
static		void					STM_stream_init ( STM_STREAM * stm );
static		void					STM_stream_reset ( STM_STREAM * stm );
static		void					STM_access_init ( STM_STREAM * stm, STM_ACCESS *access, int id );
static		void					STM_access_reset ( STM_ACCESS *access, STM_SBUFFER *first );
static		void					STM_access_return_to_start ( STM_ACCESS *access );
static		void					STM_sbuffer_init ( STM_SBUFFER * buf );
static		void					STM_sbuffer_reset( STM_SBUFFER *sbuf );

/*****************************************************************************
**          Private Functions                                               **
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		STM_SBUFFER*	STM_next_sbuffer ( STM_SBUFFER * sbuf )
{
	STM_STREAM 		*stm;
	STM_SBUFFER 	*head;

	DBG_Function ("STM_next_sbuffer");

	DBG_ASSERT_TYPE ( sbuf, STM_SBUFFER );

	stm = sbuf->stream;
	head = (STM_SBUFFER *) &stm->buffer_list;
	sbuf = (STM_SBUFFER *)sbuf->nd.next;

	if ( sbuf == head )
	{
		sbuf = (STM_SBUFFER *)sbuf->nd.next;
		if (sbuf == head )
		{
			return NULL;
		}
	}

	return sbuf;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void			STM_stream_init ( STM_STREAM * stm )
{
	int	i;

	/*DBG_Function ("STM_stream_init"); */

	DBG_SET_TYPE (  stm, STM_STREAM );
	ListInit ( &stm->buffer_list );
	LockInit ( &stm->ref );

	/* init access structs */
	for ( i = 0; i < vSTM_MAX_ACCESSORS; i++)
	{
		STM_access_init ( stm, &stm->access[i], i );
	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			STM_stream_reset( STM_STREAM* stm )
{
	STM_SBUFFER		*head,
								*first,
								*buf;

	DBG_Function ("STM_StreamReset");

	DBG_ASSERT_TYPE ( stm, STM_STREAM );

	head  = (STM_SBUFFER *) &stm->buffer_list;

	if ( (first = (STM_SBUFFER *) head->nd.next) == head )
	{
		first = NULL;
	}


	if ( first )	/* there is at least one buffer */
	{
		int id = 0;

		buf = first;

		while ( buf != head )
		{
			STM_sbuffer_reset ( buf );
			buf->id = id++;
			buf = (STM_SBUFFER *) buf->nd.next;
		}

	}

	{	/* reset the access interfaces */
		int i;

		for ( i = 0; i < vSTM_MAX_ACCESSORS; i++)
		{
			STM_access_reset ( &stm->access[i], first );
		}

		/* kick start the IN accessor */
		stm->access[vSTM_ACCESS_ID_IN].bytes_in += stm->bytes;

	}

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void			STM_access_init ( STM_STREAM * stm, STM_ACCESS *access, int id )
{
	DBG_Function ("STM_access_init");

	DBG_ASSERT_TYPE ( stm, STM_STREAM );

	DBG_SET_TYPE ( access, STM_ACCESS );

	access->stream = stm;
	DBG_CODE
	(
		LockInit ( &access->in_service );		/* we use the ref to stop nesting */
	)
	LockInit ( &access->ref );
	access->last_error = vNO_ERROR;
	access->id = id;
	access->mode = vSTM_ACCESS_MODE_UPDATE;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void			STM_access_reset( STM_ACCESS  *access, STM_SBUFFER *first )
{

	DBG_Function ("STM_access_reset");

	DBG_ASSERT_TYPE ( access, STM_ACCESS );

	access->start_buffer = first;
	access->bytes_in = W_UINT_MAX -1000;
	access->bytes_out = access->bytes_in;
	access->position = 0;

	STM_access_return_to_start ( access );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static	void		STM_access_return_to_start ( STM_ACCESS *access )
{
	access->access_pos = 0;
	access->SBuffer = access->start_buffer;
	if ( access->start_buffer )
	{
		access->Block.Data = access->start_buffer->region[access->id].Data;
	}
	else
	{
		access->Block.Data = NULL;
	}
	access->Block.Bytes = 0;
	W_FlagsClear ( access->flags, mSTM_ACCESS_TOP_OF_START);
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void			STM_sbuffer_init ( STM_SBUFFER *buf )
{

	DBG_SET_TYPE (  buf, STM_SBUFFER );
	ListNodeInit ( &buf->nd );
}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

static		void			STM_sbuffer_reset( STM_SBUFFER *sbuf )
{
	int	i;

	DBG_Function ("STM_sbuffer_reset");

	DBG_ASSERT_TYPE ( sbuf, STM_SBUFFER );

	/* init access structs */
	for ( i = 0; i < vSTM_MAX_ACCESSORS; i++)
	{
		sbuf->region[i].Data = sbuf->data_region.Data;
		sbuf->region[i].Bytes = sbuf->data_region.Bytes;
	}


}


/*****************************************************************************
**          Public Functions                                                **
*****************************************************************************/


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

STM_STREAM*		STM_StreamCreate( void )
{
	STM_STREAM	*stm;

	DBG_Function ("STM_StreamCreate");
	MEM_ALLOC_STRUCT ( stm, STM_STREAM, mMEM_INTERNAL| vMEM_ANY );

	STM_stream_init ( stm );
	/* that was simple ;-) */
	return stm;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			STM_StreamDestroy( STM_STREAM* stm )
{
	DBG_Function ("STM_StreamDestroy");

	DBG_ASSERT_TYPE ( stm, STM_STREAM );
	DBG_ASSERT ( ! (stm->flags & mSTM_STREAM_FAKE )); /* you can not destroy a fake stream */
	DBG_ASSERT ( !Locked ( &stm->ref) );			/* you are trying to destroy a stream that is still being accessed */

	STM_StreamDestroyBuffers ( stm );

	DBG_CODE
	(
		int i;

		for ( i = 0; i < vSTM_MAX_ACCESSORS; i++)
		{
			DBG_INVALIDATE_TYPE ( &stm->access[i] ); /* ensure that nobody tries to use an accessor after it is dead */
		}

	)

	DBG_INVALIDATE_TYPE ( stm );
	MEM_Free ( stm );

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			STM_StreamDestroyBuffers( STM_STREAM* stm )
{
	STM_SBUFFER 	*buf,
								*head;

	DBG_Function ("STM_StreamDestroy");

	DBG_ASSERT_TYPE ( stm, STM_STREAM );
	DBG_ASSERT ( !Locked ( &stm->ref) );			/* you are trying to destroy a stream that is still being accessed */

	head = (STM_SBUFFER *) &stm->buffer_list;

	while ( ( buf = (STM_SBUFFER *) head->nd.next) != head )
	{
		STM_SBufferRemove ( buf );
		STM_SBufferDestroy ( buf );
	}

	STM_StreamReset ( stm );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			STM_StreamReset( STM_STREAM* stm )
{
	DBG_Function ("STM_StreamReset");

	DBG_ASSERT_TYPE ( stm, STM_STREAM );
	DBG_ASSERT ( !Locked ( &stm->ref ));	/* you cannot reset the stream will it's in use */

	STM_stream_reset ( stm );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			STM_StreamAddSBuffer( STM_STREAM* stm, STM_SBUFFER *sbuf )
{

	DBG_Function ("STM_StreamAddSBuffer");

	DBG_ASSERT_TYPE ( stm, STM_STREAM );
	DBG_ASSERT_TYPE ( sbuf, STM_SBUFFER );
	DBG_ASSERT ( !(ListNodeInList ( &sbuf->nd ))); /* buffer is already in a list */

	ListNodeAppend ( (ListNode *) &stm->buffer_list, &sbuf->nd );
	stm->buffers++;
	stm->bytes += sbuf->data_region.Bytes;
	sbuf->stream = stm;
	W_FlagsClear ( stm->flags, mSTM_STREAM_RESET_DONE );	/* make sure that a reset is performed again */

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			STM_StreamCreateSBuffers( STM_STREAM* stm, int num_buffers, int buf_size, int align )
{
	int bcount = 0;
	STM_SBUFFER *buf;

	DBG_Function ("STM_StreamAddBuffers");

	DBG_ASSERT_TYPE ( stm, STM_STREAM );
	DBG_ASSERT ( num_buffers > 0 );
	DBG_ASSERT ( buf_size > 0 );
	DBG_ASSERT ( align >= 0 );

	while (num_buffers--)
	{
		if (! (buf = STM_SBufferCreate ( buf_size, align )))
		{
			break;
		}

		STM_StreamAddSBuffer ( stm, buf );
		bcount++;
	}
	return bcount;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

STM_ACCESS*		STM_StreamAcquireAccess ( STM_STREAM *stm, int access_id )
{
	STM_ACCESS	*access;
	DBG_Function ("STM_StreamAcquireAccess");

	DBG_ASSERT_TYPE ( stm, STM_STREAM );
	DBG_ASSERT ( access_id >= 0);						/* ilegal value */
	DBG_ASSERT ( access_id < vSTM_MAX_ACCESSORS );		/* legal values are vSTM_ACCESS_ID_OUT or vSTM_ACCESS_ID_IN */

	access = &stm->access[access_id];

	if ( Locked ( &access->ref ) ) /* someone has already acquired access */
	{
		DBGPRINTF ((  "%s access to stream failed. Access already acquired.\n",access->id == vSTM_ACCESS_ID_IN ? "Write" : "Read" ));
		return NULL;
	}
	
	DBG_CODE
	(
		if ( !stm->buffers)
		{
			msg_assert  ( FALSE,  ( "Trying to acquire a stream with no buffers") );
		}
	)	/* you must add at least one buffer to a stream before you can use it */

	/* if this is the first time the stream has been acquired then perform a reset on the buffers */

	if ( !Locked ( &stm->ref ) )
	{
		STM_stream_reset ( stm );
	}

	LockAcquire ( &access->ref );	/* mark access interface as in use */
	LockAcquire ( &stm->ref );		/* mark stream as in use */

	return access;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

uint			STM_StreamTotalBytesIn ( STM_STREAM *stm )
{

	DBG_Function ("STM_StreamTotalBytesIn");

	DBG_ASSERT_TYPE ( stm, STM_STREAM );

	return STM_AccessTotalBytes (&stm->access[vSTM_ACCESS_ID_OUT]);
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

uint			STM_StreamTotalBytesTillFull ( STM_STREAM *stm )
{

	DBG_Function ("STM_StreamTotalBytesTillFull");

	DBG_ASSERT_TYPE ( stm, STM_STREAM );

	return STM_AccessTotalBytes (&stm->access[vSTM_ACCESS_ID_IN]);
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

uint			STM_StreamTotalBytes ( STM_STREAM *stm )
{

	DBG_Function ("STM_StreamTotalBytes");

	DBG_ASSERT_TYPE ( stm, STM_STREAM );

	return stm->bytes;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			STM_StreamFull ( STM_STREAM *stm )
{

	DBG_Function ("STM_StreamFull");

	DBG_ASSERT_TYPE ( stm, STM_STREAM );
	return STM_StreamTotalBytesTillFull( stm ) == 0;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

STM_SBUFFER*	STM_SBufferCreate( int bytes, int align )
{
	STM_SBUFFER		*buf;

	DBG_Function ("STM_SBufferCreate");

	DBG_ASSERT ( bytes > 0 );
	DBG_ASSERT ( align >= 0 );

	MEM_ALLOC_STRUCT ( buf, STM_SBUFFER, mMEM_INTERNAL | vMEM_ANY );

	DBG_SET_TYPE ( buf, STM_SBUFFER );

	STM_sbuffer_init ( buf );

	/* allocate the buffer, add space for alignment */

	buf->buffer_memory = (uint8 *) AudioMemAlloc ( bytes + ALIGN_UPBY(1,align));

	if ( !buf->buffer_memory )
	{
		goto error;
	}
	buf->align = align;
	buf->data_region.Data = ( uint8 *)ALIGN_UPBY ( buf->buffer_memory, align);
	buf->data_region.Bytes = bytes;

	return buf;

error:

	if ( buf )
	{
		STM_SBufferDestroy ( buf );
	}

	return NULL;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			STM_SBufferDestroy( STM_SBUFFER *sbuf )
{

	DBG_Function ("STM_SBufferDestroy");

	DBG_ASSERT_TYPE ( sbuf, STM_SBUFFER );

	DBG_ASSERT ( !ListNodeInList( &sbuf->nd ));	/* you must remove the buffer from its stream */

	if ( sbuf->buffer_memory )
	{
		MEM_Free ( sbuf->buffer_memory );
	}
	DBG_INVALIDATE_TYPE ( sbuf );
	MEM_Free ( sbuf );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			STM_SBufferRemove( STM_SBUFFER *sbuf )
{
	STM_STREAM	*stm;

	DBG_Function ("STM_SBufferRemove");

	DBG_ASSERT_TYPE ( sbuf, STM_SBUFFER );

	DBG_ASSERT ( ListNodeInList ( &sbuf->nd ));		/* buffer is not presently in a stream */
	DBG_ASSERT ( sbuf->stream != NULL );			/* bad data */
	DBG_ASSERT ( !Locked ( &sbuf->stream->ref));	/* stream is in use */

	ListNodeRemove ( &sbuf->nd );
	stm = sbuf->stream;
	sbuf->stream = NULL;
	stm->buffers--;
	stm->bytes -= sbuf->data_region.Bytes;
	DBG_ASSERT ( stm->bytes >= 0 );				/* somethings gone very wrong */
	DBG_ASSERT ( stm->buffers >= 0 );				/* somethings gone very wrong */
	W_FlagsClear ( stm->flags, mSTM_STREAM_RESET_DONE );	/* make sure that a reset is performed again */

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

STM_SBUFFER*	STM_SBufferNext( STM_SBUFFER *sbuf )
{

	DBG_Function ("STM_SBufferNext");

	DBG_ASSERT_TYPE ( sbuf, STM_SBUFFER );
	DBG_ASSERT ( sbuf->stream != NULL );		/* buffer is not currently in a stream */

	return STM_next_sbuffer ( sbuf );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			STM_AccessRelease ( STM_ACCESS *access )
{

	DBG_Function ("STM_AccessRelease");

	DBG_ASSERT_TYPE ( access, STM_ACCESS );
	DBG_ASSERT ( Locked ( &access->ref ) ); /* already released, or was never acquired */

	LockRelease ( &access->ref );
	LockRelease ( &access->stream->ref );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		STM_AccessSetMode ( STM_ACCESS *access, int mode )
{

	DBG_Function ("STM_AccessSetMode");

	DBG_ASSERT_TYPE ( access, STM_ACCESS );
	DBG_ASSERT ( mode >= 0 );						/* invalid mode */
	DBG_ASSERT ( mode < vSTM_ACCESS_MAX_MODES ); 	/* see vSTM_ACCESS_MODE_?? in wcore/stream.h */
	DBG_ASSERT ( Locked ( &access->ref ) ); 		/* you did not re-acquired access*/

	access->mode = mode;

	return vNO_ERROR;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			STM_AccessMode ( STM_ACCESS *access )
{

	DBG_Function ("STM_AccessMode");

	DBG_ASSERT_TYPE ( access, STM_ACCESS );
	DBG_ASSERT ( Locked ( &access->ref ) ); 		/* you did not re-acquired access*/

	return access->mode;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			STM_AccessID ( STM_ACCESS *access )
{

	DBG_Function ("STM_AccessID");

	DBG_ASSERT_TYPE ( access, STM_ACCESS );
	DBG_ASSERT ( Locked ( &access->ref ) ); 		/* you did not re-acquired access*/

	return access->id;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			STM_AccessTransfer( STM_ACCESS* access, void *data, int bytes )
{
	int			transfer,
			    transfered = 0;
	uint8		*data8;

	DBG_Function ("STM_AccessTransfer");

	DBG_ASSERT_TYPE ( access, STM_ACCESS );
	DBG_ASSERT ( data != NULL );
	DBG_ASSERT ( bytes > 0);
	DBG_ASSERT ( Locked ( &access->ref ) ); 		/* you did not re-acquired access*/

	data8 = (uint8 *) data;

	STM_AccessGetBlock ( access );

	do
	{
		/* how much can we transfer this block */
		if ( (transfer = access->Block.Bytes) > bytes )
		{
			transfer = bytes;
		}

		if ( !transfer )
		{
			break;		/* stream has been exhuasted */
		}


		/* do the transfer */
		switch ( access->id )
		{
			case vSTM_ACCESS_ID_IN:
			{
				memcpy (  access->Block.Data, data8, transfer );
				break;
			}
			case vSTM_ACCESS_ID_OUT:
			default:		/* all accessor default to out */
			{
				memcpy ( data8, access->Block.Data, transfer );
				break;
			}
		}

		/* advance pointers */
		STM_AccessAdvance ( access, transfer );
		data8 += transfer;
		/* record work done */
		bytes -= transfer;
		transfered += transfer;

	} while ( bytes );

	return transfered;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			STM_AccessFileTransfer( STM_ACCESS* access, File *file, int bytes, int *transfered)
{
	int			transfer;


	DBG_Function ("STM_AccessFileTransfer");

	DBG_ASSERT_TYPE ( access, STM_ACCESS );
	DBG_ASSERT ( file != NULL );
	DBG_ASSERT ( bytes >= 0);
	DBG_ASSERT ( Locked ( &access->ref ) ); 		/* you did not re-acquired access*/

	STM_AccessGetBlock ( access );

	int			dummy;
	if ( !transfered )
	{
		transfered = &dummy;
	}

	*transfered = 0;

	do
	{

		if ( (transfer = access->Block.Bytes) > bytes )
		{
			transfer = bytes;
		}


		if ( !transfer )
		{
			break;		/* stream has been exhuasted */
		}

		switch ( access->id )
		{
			case vSTM_ACCESS_ID_IN:
			{
				transfer = file->read ( access->Block.Data, transfer );

				if ( transfer < 0  )
				{
					access->last_error = ERROR_CODE_FAIL;
				}
				break;
			}
			case vSTM_ACCESS_ID_OUT:
			default:		/* all accessor default to out */
			{
				transfer = file->write ( access->Block.Data, transfer );
				if ( transfer < 0  )
				{
					access->last_error = ERROR_CODE_FAIL;
				}
				break;
			}
		}

		if ( !transfer )
		{
			return STM_EOF;
		}

		if ( transfer == -1 )
		{
			return STM_FAIL;
		}
		/* do the transfer */
		STM_AccessAdvance ( access, transfer );
		bytes -= transfer;
		*transfered += transfer;

	} while ( bytes );

	return STM_OK;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		STM_AccessGetError ( STM_ACCESS *access )
{

	DBG_Function ("STM_AccessGetError");

	DBG_ASSERT_TYPE ( access, STM_ACCESS );
	DBG_ASSERT ( Locked ( &access->ref ) ); 		/* you did not re-acquired access*/

	return access->last_error;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			STM_AccessUpdate ( STM_ACCESS *access )
{
	STM_SBUFFER		*sbuf;
	int						bytes_to_update;
	int						bytes_updated;
	STM_ACCESS		*up_stream_accessor;

	DBG_Function ("STM_AccessUpdate");

	DBG_ASSERT_TYPE ( access, STM_ACCESS );
	DBG_ASSERT ( Locked ( &access->ref ) ); 		/* you did not re-acquired access*/

	if ( (bytes_to_update = access->access_pos ) == 0)
	{
		return 0;	/* no updating to perform */
	}

	/* first thing to do is to tell the up stream accessor that there is more
		buffer area avaiable for him to process */
	up_stream_accessor = STM_AccessUpStreamAccessor ( access );
	access->bytes_out += bytes_to_update;		/* reduce our byte count accordingly */
	up_stream_accessor->bytes_in += bytes_to_update;


	/* now move our start buffer to its new posistion */

	sbuf = access->start_buffer ;

	DBG_ASSERT ( sbuf != NULL );	/* fubar */
	bytes_updated = bytes_to_update;
	do
	{
		STM_REGION	*region;

		region = &sbuf->region[access->id];
		if ( region->Bytes <= bytes_to_update )
		{
			bytes_to_update -= region->Bytes;

			/* reset the current sbuffer region and move on to the next */
			region->Data = sbuf->data_region.Data; /* reset back to top of buffer */
			region->Bytes = sbuf->data_region.Bytes;

			sbuf = STM_next_sbuffer ( sbuf );

		}
		else
		{
			/* adjust start of data pointer and bytes accordingly */
			region->Data += bytes_to_update;
			region->Bytes -= bytes_to_update;
			bytes_to_update = 0;	/* no more bytes to up date */
		}


	} while ( bytes_to_update );

	access->start_buffer = sbuf;	/* update to new start buffer */
	STM_AccessReturnToStart ( access );	/* resycn access */

	return bytes_updated;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			STM_AccessAdvance ( STM_ACCESS *access, int bytes_to_advance )
{
	int			bytes_advanced = 0,
					bytes_available,
					advance;
	int			total_bytes;
	int			ok = TRUE;


	DBG_Function ("STM_AccessAdvance");

	DBG_ASSERT_TYPE ( access, STM_ACCESS );
	DBG_ASSERT ( bytes_to_advance >= 0);

	total_bytes = STM_AccessTotalBytes ( access );
	DBG_ASSERT ( access->access_pos <= total_bytes );
	DBG_ASSERT ( Locked ( &access->ref ) ); 		/* you did not re-acquired access*/

	if ( (bytes_available = total_bytes - access->access_pos ) == 0)
	{
		/* we are at the end of the data */
		access->Block.Bytes = 0;
		return 0;
	}

	DBG_ASSERT ( bytes_available >= 0 );
	if ( bytes_available < bytes_to_advance )
	{
		bytes_to_advance = bytes_available;
	}

	DBG_ASSERT ( access->SBuffer != NULL );	/* fubar */

	do
	{
		STM_REGION	*buf_region;

		buf_region = &access->SBuffer->region[access->id];

		/* update block Bytes to real value */
		if ( access->flags & mSTM_ACCESS_TOP_OF_START )
		{
				access->Block.Bytes = bytes_available; /* this must be a valid range as total_bytes
													cannot be greater then the total stream bytes */
		}
		else
		{
			int	actual_bytes =buf_region->Bytes - (  (uint) access->Block.Data - (uint) buf_region->Data );

			if ( actual_bytes < bytes_available )
			{
				access->Block.Bytes = actual_bytes;
			}
			else
			{
				access->Block.Bytes = bytes_available;
			}
		}
		/* advance access */

		if ( (advance = bytes_to_advance) == 0 )
		{
			break;	/* we are done */
		}

		if ( advance >= access->Block.Bytes )
		{
			advance = access->Block.Bytes;
			/* move on to next buffer */
			access->SBuffer = STM_next_sbuffer ( access->SBuffer );
			if ( access->SBuffer == access->start_buffer )
			{
				/* this is a special case where we have come back round
					to the start buffer. This means that area we are interested in
					the area at the top of the buffer */
				W_FlagsSet ( access->flags, mSTM_ACCESS_TOP_OF_START);
				access->Block.Data = access->SBuffer->data_region.Data; /* this is the top of the buffer */
			}
			else
			{
				access->Block.Data = access->SBuffer->region[access->id].Data;
			}
		}
		else
		{
			access->Block.Data += advance;

		}

		bytes_to_advance -= advance;
		bytes_advanced += advance;
		bytes_available -= advance;

	} while ( ok );

	access->access_pos += bytes_advanced; /* update our position */
	access->position += bytes_advanced;

	if ( access->position >= STM_StreamTotalBytes(access->stream) )
	{
		access->position = access->position % STM_StreamTotalBytes ( access->stream );
	}

	if ( (access->access_pos )&& (STM_AccessMode ( access ) == vSTM_ACCESS_MODE_UPDATE) )
	{
			STM_AccessUpdate ( access );
	}

	return bytes_advanced;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			STM_AccessReturnToStart ( STM_ACCESS *access )
{

	DBG_Function ("STM_AccessReturnToStart");

	DBG_ASSERT_TYPE ( access, STM_ACCESS );
	DBG_ASSERT ( Locked ( &access->ref ) ); 		/* you did not re-acquired access*/

	STM_access_return_to_start ( access );
	STM_AccessAdvance ( access, 0 );	/* make sure data block is valid */

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			STM_AccessNextBlock ( STM_ACCESS *access )
{

	DBG_Function ("STM_AccessNextBlock");

	DBG_ASSERT_TYPE ( access, STM_ACCESS );
	DBG_ASSERT ( Locked ( &access->ref ) ); 		/* you did not re-acquired access*/

	STM_AccessGetBlock ( access );
	if ( access->Block.Bytes )
	{
		STM_AccessAdvance ( access, access->Block.Bytes );
	}

	return ERROR_CODE_FAIL;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			STM_AccessGetBlock ( STM_ACCESS *access )
{

	DBG_Function ("STM_AccessGetBlock");

	DBG_ASSERT_TYPE ( access, STM_ACCESS );
	DBG_ASSERT ( Locked ( &access->ref ) ); 		/* you did not re-acquired access*/

	STM_AccessAdvance ( access, 0 );	/* kick start access */

	return access->Block.Bytes;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

uint			STM_AccessTotalBytes ( STM_ACCESS *access )
{
	uint 	total_bytes,
				bytes_in,
				bytes_out;

	DBG_Function ("STM_AccessTotalBytes");

	DBG_ASSERT_TYPE ( access, STM_ACCESS );

	bytes_in = access->bytes_in;
	bytes_out = access->bytes_out;

	if ( (bytes_in < bytes_out))
	{
		total_bytes = (W_UINT_MAX - bytes_out) +1 + bytes_in;
	}
	else
	{
		total_bytes = bytes_in - bytes_out;
	}

	return total_bytes;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

uint			STM_AccessPosition ( STM_ACCESS *access )
{

	DBG_ASSERT_TYPE ( access, STM_ACCESS );
	return access->position;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

STM_ACCESS*			STM_AccessUpStreamAccessor ( STM_ACCESS *access )
{
	int	id;

	DBG_Function ("STM_AccessUpStreamAccessor");

	DBG_ASSERT_TYPE ( access, STM_ACCESS );
	DBG_ASSERT ( access->id >= 0 );
	DBG_ASSERT ( access->id < vSTM_MAX_ACCESSORS );		/* something has gone wrong. id should not be out of range */
	DBG_ASSERT ( Locked ( &access->ref ) ); 		/* you did not re-acquired access*/

	if ( (id = access->id + 1 ) == vSTM_MAX_ACCESSORS )
	{
		id =  0;
	}
	return &access->stream->access[id];
}


#ifdef _DEBUG

#include <wpaudio/time.h>

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			STM_ProfileStart ( STM_PROFILE *pf )
{
	DBG_Function ("STM_ProfileStart");
	DBG_ASSERT ( pf != NULL );

	pf->bytes = 0;
	pf->last_update = AudioGetTime ();
	pf->update_interval = SECONDS(3);
	W_FlagsSet ( pf->flags, mSTM_PROFILE_ACTIVE );
	pf->rate = 0;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			STM_ProfileStop ( STM_PROFILE *pf )
{
	DBG_Function ("STM_ProfileStop");
	DBG_ASSERT ( pf != NULL );
	W_FlagsClear ( pf->flags, mSTM_PROFILE_ACTIVE );
	pf->rate = 0;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			STM_ProfileUpdate ( STM_PROFILE *pf, int bytes )
{
	TimeStamp		duration;

	DBG_Function ("STM_ProfileUpdate");
	DBG_ASSERT ( pf != NULL );


	if ( pf->flags & mSTM_PROFILE_ACTIVE )
	{
		duration = AudioGetTime() - pf->last_update;
		pf->bytes += bytes;
		if ( duration > pf->update_interval )
		{
			pf->rate = (int) ((pf->bytes * SECONDS(1)) / duration);
			pf->last_update = AudioGetTime ();
			pf->bytes = 0;
		}
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			STM_ProfileResult ( STM_PROFILE *pf )
{
	DBG_Function ("STM_ProfileResult");
	DBG_ASSERT ( pf != NULL );
	if ( pf->flags & mSTM_PROFILE_ACTIVE )
	{
		return pf->rate;
	}
	return 0;
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			STM_ProfileActive ( STM_PROFILE *pf )
{
		return pf->flags & mSTM_PROFILE_ACTIVE ;
}

#endif // _DEBUG

