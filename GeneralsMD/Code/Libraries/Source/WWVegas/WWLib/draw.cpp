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

/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Library/Draw.cpp                                  $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Blit_Block -- Blit a block of data to the surface.                                        * 
 *   Draw_Shape -- Draw a shape to the surface.                                                * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"blit.h"
#include	"blitblit.h"
#include	"blitter.h"
#include	"bsurface.h"
#include	"draw.h"
#include	"dsurface.h"
#include	"hsv.h"
#include	"rlerle.h"
#include	"shapeset.h"


/*********************************************************************************************** 
 * Draw_Shape -- Draw a shape to the surface.                                                  * 
 *                                                                                             * 
 *    This is the largely general purpose shape drawing routine for the game. All non-voxel    * 
 *    shape drawing will pass through this routine.                                            * 
 *                                                                                             * 
 * INPUT:   surface  -- The destination surface for the draw.                                  * 
 *                                                                                             * 
 *          convert  -- The converter to use for the pixel transformation.                     * 
 *                                                                                             * 
 *          shapefile   -- Pointer to the shapefile data block.                                * 
 *                                                                                             * 
 *          shapenum -- The shape number within the shape file data block specified.           * 
 *                                                                                             * 
 *          point    -- The draw point for this shape on the destination surface. This point   * 
 *                      is relative to the clipping window.                                    * 
 *                                                                                             * 
 *          window   -- The clipping window to use for this draw.                              * 
 *                                                                                             * 
 *          flags    -- Shape draw control flags. These flags are used to extract the          * 
 *                      proper blitter from the converter object.                              * 
 *                                                                                             * 
 *          remap    -- Auxiliary remap table used for the draw process. House ownership can   * 
 *                      be controlled with this parameter.                                     * 
 *                                                                                             * 
 * OUTPUT:  none                                                                               * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/27/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
void Draw_Shape(Surface & surface, ConvertClass & convert, ShapeSet const * shapefile, int shapenum, Point2D const & point, Rect const & window, ShapeFlags_Type flags, unsigned char const * remap)
{
	assert((flags & SHAPE_PREDATOR) == 0);	// Not yet supported.
	assert(shapefile != NULL);
	assert(shapenum != -1);

	convert.Set_Remap(remap);
	int x = point.X;
	int y = point.Y;
	Rect rect = shapefile->Get_Rect(shapenum);
	void const * buffer = shapefile->Get_Data(shapenum);
	int width = shapefile->Get_Width();
	int height = shapefile->Get_Height();
	BSurface const shape(rect.Width, rect.Height, 1, (void *)buffer);

	/*
	**	Centered shapes must have their start position adjusted
	**	according to their width and height.
	*/
	if (flags & SHAPE_CENTER) {
		x -= width/2;
		y -= height/2;
	}

	/*
	**	Factor the offset of the shape data (logical) into the draw
	**	position. Since the actual embedded shape pixels have no
	**	(transparent) offset, remove the offset coefficient.
	*/
	x += rect.X;
	y += rect.Y;
	rect.X = 0;
	rect.Y = 0;

	/*
	**	If the remap table supplied is not NULL, then set the remap flag so that
	**	the appropriate blitter will be selected.
	*/
	if (remap != NULL) {
		flags = ShapeFlags_Type(flags | SHAPE_REMAP);
	}

	/*
	**	Blit the pixels according to the flags specified and whether this is
	**	an RLE compressed shape. RLE compression uses different blitter routines
	**	than the normal method.
	*/
	if (shapefile->Is_RLE_Compressed(shapenum)) {
		RLEBlitter const * blitter = convert.RLEBlitter_From_Flags(flags);
		if (blitter != NULL) {
			RLE_Blit(surface, window, Rect(x, y, rect.Width, rect.Height), shape, rect, rect, *blitter);

		}
	} else {
		Blitter const * blitter = convert.Blitter_From_Flags(flags);
		if (blitter != NULL) {
			Bit_Blit(surface, window, Rect(x, y, rect.Width, rect.Height), shape, shape.Get_Rect(), rect, *blitter);
		}
	}
}


/*********************************************************************************************** 
 * Blit_Block -- Blit a block of data to the surface.                                          * 
 *                                                                                             * 
 *    This function serves as a wrapper to the normal Bit_Blit function. It is used to control * 
 *    the blitter according to remap parameter specified.                                      * 
 *                                                                                             * 
 * INPUT:   surface  -- The destination surface of this blit.                                  * 
 *                                                                                             * 
 *          convert  -- The convert class for pixel transformation.                            * 
 *                                                                                             * 
 *          source   -- The source surface to blit from.                                       * 
 *                                                                                             * 
 *          sourcerect  -- The source rectangle within the source surface to blit from.        * 
 *                                                                                             * 
 *          point    -- The draw point of this blit (upper left corner).                       * 
 *                                                                                             * 
 *          window   -- The clipping window for this blit. The destination rectangle is        * 
 *                      relative  to this clipping window.                                     * 
 *                                                                                             * 
 *          remap    -- The remapping table pointer (optional) to be used for ownership        * 
 *                      remapping control.                                                     * 
 *                                                                                             * 
 *          blitter  -- Preexisting blitter to use. If this parameter is NULL, then a blitter  * 
 *                      object is created from the convert object supplied.                    * 
 *                                                                                             * 
 * OUTPUT:  none                                                                               * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/27/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
void Blit_Block(Surface & surface, ConvertClass & convert, Surface const & source, Rect const & sourcerect, Point2D const & point, Rect const & window, unsigned char const * remap, Blitter const * blitter)
{
	convert.Set_Remap(remap);

	if (blitter == NULL) {
		blitter = convert.Blitter_From_Flags((remap != NULL) ? SHAPE_REMAP : SHAPE_NORMAL);
	}

	/*
	**	Submit the image to the destination surface.
	*/
	Bit_Blit(surface, window, Rect(point.X, point.Y, sourcerect.Width, sourcerect.Height), source, source.Get_Rect(), sourcerect, *blitter);
}

