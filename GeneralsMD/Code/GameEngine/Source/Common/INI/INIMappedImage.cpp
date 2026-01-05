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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: INIMappedImage.cpp ///////////////////////////////////////////////////////////////////////
// Author: Colin Day, December 2001
// Desc:   Mapped image INI parsing
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "GameClient/Image.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/** Parse mapped image entry */
//-------------------------------------------------------------------------------------------------
void INI::parseMappedImageDefinition( INI* ini )
{
	// read the name
	const char* c = ini->getNextToken();
	name.set( c );
	
	printf("[INIMappedImage::parseMappedImageDefinition] Parsing mapped image definition:\n");
	printf("  Image name/key: '%s'\n", name.str());

	//
	// find existing item if present, note that we do not support overrides
	// in the images like we do in systems that are more "design" oriented, images
	// are assets as they are
	//
	if( !TheMappedImageCollection )
	{
		printf("[INIMappedImage::parseMappedImageDefinition] WARNING: TheMappedImageCollection is NULL, skipping\n");
		//We don't need it if we're in the builder... which doesn't have this.
		return;
	}

	// Create a temporary image to parse the INI properties
	Image *image = newInstance(Image);
	image->setName( name );
	
	// Parse the INI properties FIRST to get the filename
	printf("[INIMappedImage::parseMappedImageDefinition] Parsing INI properties for '%s'\n", name.str());
	ini->initFromINI( image, image->getFieldParse());

	// Add the image to the collection (now that we have the filename)
	TheMappedImageCollection->addImage(image);
	
	// Log the final result
	printf("[INIMappedImage::parseMappedImageDefinition] Successfully added image:\n");
	printf("  Name: '%s'\n", image->getName().str());
	printf("  Filename: '%s'\n", image->getFilename().str());
}
