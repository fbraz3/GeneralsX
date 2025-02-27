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
 *                     $Archive:: /G/wwlib/blit.cpp                                           $* 
 *                                                                                             * 
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 4/15/99 10:13a                                              $*
 *                                                                                             * 
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   Bit_Blit -- Blit a block of pixels to the destination surface.                            * 
 *   Bit_Blit -- Blits data to a surface w/ clipping.                                          * 
 *   Buffer_Size -- Determines size of buffer for given dimensions.                            * 
 *   From_Buffer -- Copy graphic data from a buffer to a surface.                              * 
 *   RLE_Blit -- Blits RLE compressed data without extra clipping.                             * 
 *   RLE_Blit -- Blits a rectangle of RLE compressed data to a surface.                        * 
 *   To_Buffer -- Copies a graphic region into a linear RAM buffer.                            * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"blit.h"
#include	"bsurface.h"
//#include	"rle.h"
#include	"xsurface.h"
//#include	<stdlib.h>


/*********************************************************************************************** 
 * Buffer_Size -- Determines size of buffer for given dimensions.                              *   
 *                                                                                             * 
 *    This routine will determine the byte size of a buffer if it were to hold the pixels      * 
 *    of the dimensions specified. It takes into account the bytes per pixel.                  * 
 *                                                                                             * 
 * INPUT:   surface  -- The surface to base the buffer size calculation upon.                  * 
 *                                                                                             * 
 *          width    -- Pixel width of a graphic region.                                       * 
 *                                                                                             * 
 *          height   -- Pixel height of a graphic region.                                      * 
 *                                                                                             * 
 * OUTPUT:  Returns with the number of bytes such a region would consume if it were linearly   * 
 *          packed into a memory buffer.                                                       * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/07/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
int Buffer_Size(Surface const & surface, int width, int height)
{
	return(width * height * surface.Bytes_Per_Pixel());
}


/*********************************************************************************************** 
 * To_Buffer -- Copies a graphic region into a linear RAM buffer.                              * 
 *                                                                                             * 
 *    This routine will copy the graphic rectangle specified, into a RAM buffer. The size of   * 
 *    the RAM buffer must be big enough to hold the pixel data. Use the Buffer_Size() function * 
 *    to determine how big it must be.                                                         * 
 *                                                                                             * 
 * INPUT:   surface  -- The surface to copy the pixel data from.                               * 
 *                                                                                             * 
 *          rect     -- The graphic rectangle to copy from.                                    * 
 *                                                                                             * 
 *          buffer   -- Reference to the buffer that will be filled with the pixel data.       * 
 *                                                                                             * 
 * OUTPUT:  bool; Was the data copy performed without error?                                   * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/07/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool To_Buffer(Surface const & surface, Rect const & rect, Buffer & buffer)
{
	if (!rect.Is_Valid()) return(false);

	BSurface from(rect.Width, rect.Height, surface.Bytes_Per_Pixel(), buffer);
	return(from.Blit_From(Rect(0, 0, rect.Width, rect.Height), surface, rect));
}


/*********************************************************************************************** 
 * From_Buffer -- Copy graphic data from a buffer to a surface.                                * 
 *                                                                                             * 
 *    This routine will take pixel data and move it from the specified buffer and into the     * 
 *    surface rectangle specified. It is the counterpart routine of To_Buffer().               * 
 *                                                                                             * 
 * INPUT:   surface  -- The surface to store the pixel data to.                                * 
 *                                                                                             * 
 *          rect     -- The destination rectangle to store the pixel data to.                  * 
 *                                                                                             * 
 *          buffer   -- Reference to the buffer that contains the pixel data.                  * 
 *                                                                                             * 
 * OUTPUT:  bool; Was the pixel data copy performed without error?                             * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/07/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool From_Buffer(Surface & surface, Rect const & rect, Buffer const & buffer)
{
	if (!rect.Is_Valid()) return(false);

	BSurface from(rect.Width, rect.Height, surface.Bytes_Per_Pixel(), buffer);
	return(surface.Blit_From(rect, from, Rect(0, 0, rect.Width, rect.Height)));
}



/*********************************************************************************************** 
 * Bit_Blit -- Blits data to a surface w/ clipping.                                            * 
 *                                                                                             * 
 *    This routine will take source pixel data and blit it to the surface specified while      * 
 *    also performing clipping on both the source and the destination data. Typical users of   * 
 *    this routine would be to draw shape (sprite) data.                                       * 
 *                                                                                             * 
 * INPUT:   dest     -- Destintaion surface rect. This specifies the destination surface and   * 
 *                      any coordinate clipping rectangle.                                     * 
 *                                                                                             * 
 *          destrect -- The destination rectangle of the blit. The coordinates are relative    * 
 *                      to the destination clipping rectangle.                                 * 
 *                                                                                             * 
 *          source   -- Source surface rect. This specifies the source surface as well as any  * 
 *                      clipping rectangle it may contain.                                     * 
 *                                                                                             * 
 *          srcrect  -- The rectange, relative to the source clipping rectangle, that          * 
 *                      specifies the source blit data. It is presumed that the dimensions of  * 
 *                      the source rectangle are the same as the destination rectangle.        * 
 *                                                                                             * 
 *          blitter  -- The blitter to use for moving the source pixels to the destination     * 
 *                      surface.                                                               * 
 *                                                                                             * 
 * OUTPUT:  bool; Was the blit performed even if it was for only a single pixel. Failure would * 
 *                indicate that the blit was completely clipped away.                          * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/19/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool Bit_Blit(Surface & dest, Rect const & destrect, Surface const & source, Rect const & sourcerect, Blitter const & blitter)
{
	return(Bit_Blit(dest, dest.Get_Rect(), destrect, source, source.Get_Rect(), sourcerect, blitter));
}


/*********************************************************************************************** 
 * Bit_Blit -- Blit a block of pixels to the destination surface.                              * 
 *                                                                                             * 
 *    This routine will blit a block of pixels and perform clipping on the blit as controlled  * 
 *    by the clipping rectangles.                                                              * 
 *                                                                                             * 
 * INPUT:   dest     -- Surface to blit to.                                                    * 
 *                                                                                             * 
 *          dcliprect-- The destination surface clipping rectangle.                            * 
 *                                                                                             * 
 *          ddrect   -- The destination rect of the blit. It is relative to the clipping       * 
 *                      rectangle and will be clipped against same.                            * 
 *                                                                                             * 
 *          source   -- The source surface to blit from.                                       * 
 *                                                                                             * 
 *          scliprect-- The source surface clipping rectangle.                                 * 
 *                                                                                             * 
 *          ssrect   -- The source rectangle of the blit. It is relative to the source         * 
 *                      clipping rectangle and will be clipped against same.                   * 
 *                                                                                             * 
 *          blitter  -- The blitter to use for blitting of this rectangle.                     * 
 *                                                                                             * 
 * OUTPUT:  bool; Was the blit performed? A 'false' return value would indicate that the       * 
 *                blit was clipped into nothing.                                               * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/27/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool Bit_Blit(Surface & dest, Rect const & dcliprect, Rect const & ddrect, Surface const & source, Rect const & scliprect, Rect const & ssrect, Blitter const & blitter)
{
	Rect srect = ssrect;
	Rect drect = ddrect;
	bool overlapped = false;
	void * dbuffer = NULL;
	void * sbuffer = NULL;

	/*
	**	Prepare for the blit by performing any clipping as well as fetching pointers into the
	**	pixel buffers. If there were any errors, then this blit cannot be performed.
	*/
	if (!XSurface::Prep_For_Blit(dest, dcliprect, drect, source, scliprect, srect, overlapped, dbuffer, sbuffer)) {
		return(false);
	}

	/*
	**	If there is no difference between the width and the stride of the source and 
	**	destination surfaces, then the copy can be performed as one huge copy operation.
	**	This is the simplist case and the one that is performed with a full screen
	**	blit.
	*/
	if (drect.Width * dest.Bytes_Per_Pixel() == dest.Stride() && dest.Stride() == source.Stride()) {

		int length = MIN(srect.Height*srect.Width, drect.Height*drect.Width);
		if (overlapped) {
			blitter.BlitBackward(dbuffer, sbuffer, length);
		} else {
			blitter.BlitForward(dbuffer, sbuffer, length);
		}

	} else {

		/*
		**	If the rectangles overlap, then the copy must proceed from the 
		**	last row to the first rather than the normal direction.
		*/
		int sstride = source.Stride();
		int dstride = dest.Stride();
		if (overlapped) {
			sstride = -sstride;
			dstride = -dstride;
			sbuffer = ((char*)sbuffer) + (srect.Height-1) * source.Stride();
			dbuffer = ((char*)dbuffer) + (drect.Height-1) * dest.Stride();
		}

		/*
		**	This perform a line-by-line pixel copy.
		*/
		int height = MIN(srect.Height, drect.Height);
		if (overlapped) {
			for (int y = 0; y < height; y++) {
				blitter.BlitBackward(dbuffer, sbuffer, srect.Width);
				dbuffer = (void*)(((char*)dbuffer) + dstride);
				sbuffer = (void*)(((char*)sbuffer) + sstride);
			}
		} else {
			for (int y = 0; y < height; y++) {
				blitter.BlitForward(dbuffer, sbuffer, srect.Width);
				dbuffer = (void*)(((char*)dbuffer) + dstride);
				sbuffer = (void*)(((char*)sbuffer) + sstride);
			}
		}
	}

	dest.Unlock();
	source.Unlock();

	return(true);
}


/*********************************************************************************************** 
 * RLE_Blit -- Blits RLE compressed data without extra clipping.                               * 
 *                                                                                             * 
 *    This routine will blit a rectangle of RLE compressed data to the specified surface. It   * 
 *    is functionally similar to the other RLE blit routine, but does not use any sub          * 
 *    clipping rectangles. The blit is naturally clipped to the edge of the destination        * 
 *    surface.                                                                                 * 
 *                                                                                             * 
 * INPUT:   dest     -- Reference to the destination surface.                                  * 
 *                                                                                             * 
 *          destrect -- The destination rectangle to draw the pixels to.                       * 
 *                                                                                             * 
 *          source   -- Reference to the source RLE surface data.                              * 
 *                                                                                             * 
 *          sourcerect  -- The source rectangle of from the RLE surface to blit from.          * 
 *                                                                                             * 
 *          blitter  -- Reference to the blitter to perform the blit operation with.           * 
 *                                                                                             * 
 * OUTPUT:  bool; Was a blit performed? A 'false' value would mean that the blit has been      * 
 *                clipped into nothing.                                                        * 
 *                                                                                             * 
 * WARNINGS:   The dimensions of the source and destination rectangles should be the same      * 
 *             until such time that the blitter can support scaling (as of this writing, it    * 
 *             cannot).                                                                        * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/27/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool RLE_Blit(Surface & dest, Rect const & destrect, Surface const & source, Rect const & sourcerect, RLEBlitter const & blitter)
{
	return(RLE_Blit(dest, dest.Get_Rect(), destrect, source, source.Get_Rect(), sourcerect, blitter));
}


/*********************************************************************************************** 
 * RLE_Blit -- Blits a rectangle of RLE compressed data to a surface.                          * 
 *                                                                                             * 
 *    This routine will blit a rectangle of REL compressed pixel data from a sprite to the     * 
 *    surface specified. Appropriate clipping and coordinate adjustments will occur as         * 
 *    controlled by the parameters. This is the workhorse RLE blit dispatcher routine.         * 
 *                                                                                             * 
 * INPUT:   dest     -- The destination surface to blit to.                                    * 
 *                                                                                             * 
 *          dcliprect-- The clipping rectangle to use on the destination. Pixels won't be      * 
 *                      drawn outside of this rectangle and the destination rect coordinates   * 
 *                      are biased to this clipping rectange.                                  * 
 *                                                                                             * 
 *          ddrect   -- The destination rectangle of the blit. The upper left coordinates are  * 
 *                      biased to the destination clipping rectangle when blitted. Thus,       * 
 *                      a dest X,Y position of 0,0 really means the upper left corner of the   * 
 *                      destination clipping rectangle.                                        * 
 *                                                                                             * 
 *          source   -- The source surface of the RLE compressed data.                         * 
 *                                                                                             * 
 *          scliprect-- It is quite likely that this will be the full size of the source       * 
 *                      surface.                                                               * 
 *                                                                                             * 
 *          ssrect   -- The source rectangle to blit from within the source surface. It        * 
 *                      behaves similarly to the ddrect parameter, but for the source. The     * 
 *                      width and height of this rectangle should match the width and height   * 
 *                      of the ddrect parameter (scaling is not yet supported).                * 
 *                                                                                             * 
 *          blitter  -- The blitter to use for this pixel copy. It must be an RLE blitter.     * 
 *                                                                                             * 
 * OUTPUT:  bool; Did the blit draw at least one pixel?                                        * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/24/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool RLE_Blit(Surface & dest, Rect const & dcliprect, Rect const & ddrect, Surface const & source, Rect const & scliprect, Rect const & ssrect, RLEBlitter const & blitter)
{
	Rect srect = ssrect;			// Desired source rect.
	Rect drect = ddrect;			// Desired destination rect.

	/*
	**	Adjust the desired draw rectangles to account for clipping. This is where the desired rectangles
	**	get clipped to the bounding rectangles of the surfaces.
	*/
	if (!Blit_Clip(drect, dcliprect, srect, scliprect)) {
		return(false);
	}

	/*
	**	Determine the top and left skip margins. These require special handling
	**	since the shape is compressed.
	*/
	int leftmargin = srect.X - scliprect.X;
	int topmargin = srect.Y - scliprect.Y;

	void * dbuffer = dest.Lock(dcliprect.Top_Left() + drect.Top_Left());
//	void * dbuffer = dest.Lock(Point2D(dcliprect.X + drect.X, dcliprect.Y + drect.Y));
	if (dbuffer == NULL) return(false);

	/*
	**	Lock the source pointer. This must always lock at location 0,0 since
	**	normal pixel offset logic does not work for RLE compressed buffers. If there
	**	is a pixel offset required, it is handled below.
	*/
	void * sbuffer = source.Lock();
	if (sbuffer == NULL) {
		dest.Unlock();
		return(false);
	}

	/*
	**	Skip any top margin lines. This must be manually performed on a line
	**	by line basis because the length of each line is Variable.
	*/
	while (topmargin > 0) {
		sbuffer = ((unsigned char *)sbuffer) + (*(unsigned short *)sbuffer);
		topmargin--;
	}

	/*
	**	This perform a line-by-line pixel copy.
	*/
	int dstride = dest.Stride();
	int height = MIN(srect.Height, drect.Height);
	for (int y = 0; y < height; y++) {

		/*
		**	Blit the correct sub-portion to the destination surface.
		*/
		blitter.Blit(dbuffer, ((unsigned short *)sbuffer)+1, srect.Width, leftmargin);

		/*
		**	Advance the source and dest pointers for the next line processing.
		*/
		sbuffer = ((unsigned char *)sbuffer) + (*(unsigned short *)sbuffer);
		dbuffer = (void*)(((char*)dbuffer) + dstride);
	}

	dest.Unlock();
	source.Unlock();

	return(true);
}


