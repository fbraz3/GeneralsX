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
** File name:  audlock.cpp                                                  **
**                                                                          **
** Created by:  04/01/95 TR                                                 **
**                                                                          **
** Description: <description>                                               **
**                                                                          **
*****************************************************************************/

/*****************************************************************************
**            Includes                                                      **
*****************************************************************************/


#include <wpaudio/altypes.h>
#include <wpaudio/lock.h>

DBG_DECLARE_TYPE ( Lock )

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

#ifdef _DEBUG

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		LockInit ( volatile Lock *lock )
{

	DBG_ASSERT ( lock != NULL);

	LOCK_INIT(lock);

	DBG_SET_TYPE ( lock, Lock );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		LockAcquire ( volatile Lock *lock)
{

	DBG_ASSERT_TYPE ( lock, Lock);
	DBG_ASSERT ( lock->count >= 0 );

	LOCK_ACQUIRE (lock);
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void		LockRelease ( volatile Lock *lock)
{

	DBG_ASSERT_TYPE ( lock, Lock);
	DBG_ASSERT ( lock->count > 0 );

	LOCK_RELEASE(lock);
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		Locked ( volatile Lock *lock)
{

	DBG_ASSERT_TYPE ( lock, Lock);
	DBG_ASSERT ( lock->count >= 0 );

	return LOCKED(lock);
}

#endif

