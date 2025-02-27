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
** File name:  audattr.cpp                                                  **
**                                                                          **
** Created by:  11/16/95 TR                                                 **
**                                                                          **
** Description: <description>                                               **
**                                                                          **
*****************************************************************************/

/*****************************************************************************
**            Includes                                                      **
*****************************************************************************/

#include <wpaudio/altypes.h>						
#include <wpaudio/level.h>
#include <wpaudio/attributes.h>


DBG_DECLARE_TYPE ( AudioAttribs )

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

void 			AudioAttribsInit ( AudioAttribs *attr )
{

	DBG_ASSERT ( attr != NULL);
	DBG_SET_TYPE ( attr, AudioAttribs );

	AudioLevelInit ( &attr->VolumeLevel, AUDIO_VOLUME_MAX );
	AudioLevelInit ( &attr->PitchLevel, 100);
	AudioLevelInit ( &attr->PanPosition, AUDIO_PAN_CENTER);
	AudioAttribsSetPitchDuration (attr, SECONDS(1), 10 );
	AudioAttribsSetVolumeDuration (attr, SECONDS(1), AUDIO_LEVEL_MAX );
	AudioAttribsSetPanDuration (attr, SECONDS(1), AUDIO_LEVEL_MAX );

	AudioLevelSet ( &attr->VolumeLevel, AUDIO_VOLUME_MAX );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioAttribsUpdate ( AudioAttribs *attr )
{

	DBG_ASSERT_TYPE ( attr, AudioAttribs );

	AudioLevelUpdate ( &attr->VolumeLevel );
	AudioLevelUpdate ( &attr->PitchLevel );
	AudioLevelUpdate ( &attr->PanPosition );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int			AudioAttribsChanged ( AudioAttribs *attr )
{

	DBG_ASSERT_TYPE ( attr, AudioAttribs );

	return AudioLevelChanged ( &attr->VolumeLevel ) || AudioLevelChanged ( &attr->PitchLevel ) || AudioLevelChanged ( &attr->PanPosition );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioAttribsApply ( AudioAttribs *attr, AudioAttribs *mod )
{


	DBG_ASSERT_TYPE ( attr, AudioAttribs );
	DBG_ASSERT_TYPE ( mod, AudioAttribs );


	AudioLevelSet ( &attr->VolumeLevel, AudioLevelApply ( &mod->VolumeLevel, AudioAttribsGetVolume ( attr ) ));
	AudioLevelUpdate ( &attr->VolumeLevel );

	{
		//  apply pitch 
		int	level;
		int	change;


		level = AudioAttribsGetPitch ( attr );
		change = AudioAttribsGetPitch ( mod );


		level = (level * change) / 100;

		AudioAttribsSetPitch ( attr, level );
		AudioLevelUpdate ( &attr->PitchLevel );
	}

	{
		//  apply pan 
		int	pos;

		if ( (pos = AudioAttribsGetPan ( mod ) - AUDIO_PAN_CENTER) != 0 )
		{
			if ( ( pos = pos + AudioAttribsGetPan ( attr )) < AUDIO_PAN_LEFT )
			{
				pos = AUDIO_PAN_LEFT ;
			}
			else if ( pos > AUDIO_PAN_RIGHT )
			{
				pos = AUDIO_PAN_RIGHT ;
			}

			AudioAttribsSetPan ( attr, pos );
		}
		AudioLevelUpdate ( &attr->PanPosition );
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			AudioAttribsUsed ( AudioAttribs *attr )
{
	AudioLevelUsed ( &attr->VolumeLevel );
	AudioLevelUsed ( &attr->PanPosition );
	AudioLevelUsed ( &attr->PitchLevel );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int		AudioAttribsCalcPitch ( AudioAttribs *attr, int pitch )
{
	int	level;


	DBG_ASSERT_TYPE ( attr, AudioAttribs );

	level = AudioAttribsGetPitch ( attr );

	return ( pitch * level ) / 100;

}

