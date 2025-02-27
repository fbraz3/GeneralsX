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

/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /VSS_Sync/wwlib/xsurface.cpp                                $* 
 *                                                                                             * 
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             * 
 *                     $Modtime:: 10/16/00 11:42a                                             $*
 *                                                                                             * 
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   Blit_Clip -- Perform rectangle clipping in preparation for a blit.                        * 
 *   XSurface::Blit_From -- Blit entire surface.                                               *
 *   XSurface::Blit_From -- Blit from one surface to this one w/ clipping.                     * 
 *   XSurface::Blit_From -- Blit one region to another.                                        *
 *   XSurface::Blit_Plain -- Blit a plain rectangle from one surface to another.               * 
 *   XSurface::Blit_Trans -- Blit a rectangle with transparency checking.                      * 
 *   XSurface::Draw_Line -- Draw a line and perform clipping.                                  * 
 *   XSurface::Draw_Line -- Draws a line upon the surface.                                     *
 *   XSurface::Draw_Rect -- Draw a rectangle with an arbitrary clipping rectangle.             * 
 *   XSurface::Draw_Rect -- Draws a rectangle on the surface.                                  *
 *   XSurface::Fill -- Fill the entire surface with the color specified.                       *
 *   XSurface::Fill_Rect -- Fill a rectangle but perform clipping on the fill.                 * 
 *   XSurface::Fill_Rect -- Fills a rectangle with the color specified.                        *
 *   XSurface::Get_Pixel -- Fetches a pixel from the surface.                                  *
 *   XSurface::Prep_For_Blit -- Clips and prepares pointers for a blit operation.              * 
 *   XSurface::Put_Pixel -- Stores a pixel at the location specified.                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"blit.h"
#include	"blitblit.h"
#include	"bsurface.h"
#include	"swap.h"
#include	"xsurface.h"
//#include	<stdlib.h>
#include	<string.h>


/***********************************************************************************************
 * XSurface::Draw_Line -- Draws a line upon the surface.                                       *
 *                                                                                             *
 *    This routine will draw a line on the surface between the points specified.               *
 *                                                                                             *
 * INPUT:   startpoint  -- Pixel coordinate of one end point to the line.                      *
 *                                                                                             *
 *          endpoint    -- Pixel coordinate of the other end point to the line.                *
 *                                                                                             *
 *          color       -- The color to draw the line with.                                    *
 *                                                                                             *
 * OUTPUT:  bool; Was the line drawn without error?                                            *
 *                                                                                             *
 * WARNINGS:   This routine is currently very brain-dead. It only draws vertical or            *
 *             horizontal lines. It needs to be updated to handle any angle lines and should   *
 *             perform line-clipping rather than point-pushing if the line intersects the      *
 *             clipping rectangle.                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Draw_Line(Point2D const & startpoint, Point2D const & endpoint, int color)
{
	return(XSurface::Draw_Line(Get_Rect(), startpoint, endpoint, color));
}


/*********************************************************************************************** 
 * XSurface::Draw_Line -- Draw a line and perform clipping.                                    * 
 *                                                                                             * 
 *    Use this routine to draw a line on the surface but also clip the line draw against       * 
 *    an arbitrary sub-rectangle within the surface.                                           * 
 *                                                                                             * 
 * INPUT:   cliprect -- The clipping rectangle for this line draw.                             * 
 *                                                                                             * 
 *          startpoint  -- The starting point of the line draw. This point is relative to the  * 
 *                         clipping rectangle.                                                 * 
 *                                                                                             * 
 *          endpoint    -- The ending point fo the line draw. This point is also relative to   * 
 *                         the clipping rectangle.                                             * 
 *                                                                                             * 
 *          color    -- The screen format color value to store for each pixel of the line.     * 
 *                                                                                             * 
 * OUTPUT:  bool; Was the line drawn? A 'false' return value would indicate that the line      * 
 *                was clipped away.                                                            * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/27/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool XSurface::Draw_Line(Rect const & xcliprect, Point2D const & startpoint, Point2D const & endpoint, int color)
{
	/*
	**	Ensure that the clipping rectangle is legal.
	*/
	Rect cliprect = xcliprect.Intersect(Get_Rect());

	/*
	**	High-speed working variables for the clipping rectangle and clipping operation.
	*/
	Point2D start = startpoint.Bias_To(cliprect);
	Point2D end = endpoint.Bias_To(cliprect);

	/*
	**	Build bits that indicate which end points lie outside the clipping rectangle.
	**	Quick checks against these flag bits will speed the clipping process.
	*/
	enum {
		OFF_TOP=0x01,
		OFF_BOTTOM=0x02,
		OFF_RIGHT=0x04,
		OFF_LEFT=0x08
	};
	int sflag = 0;
	int dflag = 0;

	if (start.X < cliprect.X) sflag |= OFF_LEFT;
	if (start.Y < cliprect.Y) sflag |= OFF_TOP;
	if (start.X >= cliprect.X+cliprect.Width) sflag |= OFF_RIGHT;
	if (start.Y >= cliprect.Y+cliprect.Height) sflag |= OFF_BOTTOM;
	if (end.X < cliprect.X) dflag |= OFF_LEFT;
	if (end.Y < cliprect.Y) dflag |= OFF_TOP;
	if (end.X >= cliprect.X+cliprect.Width) dflag |= OFF_RIGHT;
	if (end.Y >= cliprect.Y+cliprect.Height) dflag |= OFF_BOTTOM;

	/*
	**	Check to see if the line segment falls outside of the viewing rectangle.
	*/
	if ((sflag & dflag) != 0) return(false);

	/*
	**	Clip the start point if necessary. This clipping should really check for
	**	line intersect rather than pushing the end-point within bounds.
	*/
	if (sflag != 0) {
		if (sflag & OFF_LEFT) start.X = cliprect.X;
		if (sflag & OFF_RIGHT) start.X = cliprect.X+cliprect.Width-1;
		if (sflag & OFF_TOP) start.Y = cliprect.Y;
		if (sflag & OFF_BOTTOM) start.Y = cliprect.Y+cliprect.Height-1;
	}
	if (dflag != 0) {
		if (dflag & OFF_LEFT) end.X = cliprect.X;
		if (dflag & OFF_RIGHT) end.X = cliprect.X+cliprect.Width-1;
		if (dflag & OFF_TOP) end.Y = cliprect.Y;
		if (dflag & OFF_BOTTOM) end.Y = cliprect.Y+cliprect.Height-1;
	}

	/*
	**	Force the line segment to be either vertical or horizontal. This is because
	**	we are not using brezenham's alogorithm (which we should be using instead here).
	*/
	if (start.X != end.X) start.Y = end.Y;

	/*
	**	Ensure that the source point is always to the left and up
	**	from the dest point.
	*/
	if (start.X > end.X || start.Y > end.Y) {
		swap(start, end);
		swap(sflag, dflag);
	}

	int bbp = Bytes_Per_Pixel();
	void * buffer = Lock(start);
	if (buffer != NULL) {

		if (start.Y == end.Y) {
			if (bbp == 1) {
				memset(buffer, color, (end.X-start.X)+1);
			} else {
				for (int x = 0; x <= end.X-start.X; x++) {
					((short *)buffer)[x] = (short)color;
				}
			}
		} else {
			for (int y = 0; y <= end.Y-start.Y; y++) {
				if (bbp == 1) {
					*(char*)buffer = (char)color;
				} else {
					*(short*)buffer = (short)color;
				}
				buffer = (void*)(((char*)buffer) + Stride());
			}
		}

		Unlock();
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * XSurface::Draw_Rect -- Draws a rectangle on the surface.                                    *
 *                                                                                             *
 *    This routine will draw a line around the rectangle specified. The line will lie just     *
 *    within the rectangle.                                                                    *
 *                                                                                             *
 * INPUT:   crect -- The rectangle dimensions to use.                                          *
 *                                                                                             *
 *          color -- The color to draw the rectangle.                                          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Draw_Rect(Rect const & crect, int color)
{
	return(XSurface::Draw_Rect(Get_Rect(), crect, color));
}


/*********************************************************************************************** 
 * XSurface::Draw_Rect -- Draw a rectangle with an arbitrary clipping rectangle.               * 
 *                                                                                             * 
 *    This routine will draw the rectangle but will also clip the draw against the clipping    * 
 *    rectangle provided.                                                                      * 
 *                                                                                             * 
 * INPUT:   cliprect -- The clipping rectangle to clip the draw against.                       * 
 *                                                                                             * 
 *          rect     -- The rectangle to draw. The coordinates are relative to the clipping    * 
 *                      rectangle.                                                             * 
 *                                                                                             * 
 *          color    -- The color ot use for this rectangle draw.                              * 
 *                                                                                             * 
 * OUTPUT:  none                                                                               * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/27/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool XSurface::Draw_Rect(Rect const & cliprect, Rect const & crect, int color)
{
	Point2D ul(crect.X, crect.Y);
	Point2D ur(crect.X+crect.Width-1, crect.Y);
	Point2D ll(crect.X, crect.Y+crect.Height-1);
	Point2D lr(crect.X+crect.Width-1, crect.Y+crect.Height-1);

	Draw_Line(cliprect, ul, ur, color);
	Draw_Line(cliprect, ul, ll, color);
	Draw_Line(cliprect, ur, lr, color);
	Draw_Line(cliprect, ll, lr, color);
	return(true);
}


/*********************************************************************************************** 
 * XSurface::Get_Pixel -- Fetches a pixel from the surface.                                    * 
 *                                                                                             * 
 *    This routine will fetch a pixel element from the surface at the location specified.      * 
 *                                                                                             * 
 * INPUT:   point -- Coordinate to fetch the pixel from.                                       * 
 *                                                                                             * 
 * OUTPUT:  Returns with the pixel value at that coordinate. The interpretation of the return  * 
 *          value depends on the pixel format of the surface.                                  * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/07/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
int XSurface::Get_Pixel(Point2D const & point) const
{
	int color = 0;
	void * pointer = ((Surface*)this)->Lock(point);
	if (pointer != NULL) {
		if (Bytes_Per_Pixel() == 2) {
			color = *((unsigned short*)pointer);
		} else {
			color = *((unsigned char*)pointer);
		}
		((Surface*)this)->Unlock();
	}
	return(color);
}


/*********************************************************************************************** 
 * XSurface::Put_Pixel -- Stores a pixel at the location specified.                            * 
 *                                                                                             * 
 *    This routine will store a pixel at the coordinate specified on the surface.              * 
 *                                                                                             * 
 * INPUT:   point    -- The coordinate to place the pixel at.                                  * 
 *                                                                                             * 
 *          color    -- The pixel data to store.                                               * 
 *                                                                                             * 
 * OUTPUT:  bool; Was the pixel stored?                                                        * 
 *                                                                                             * 
 * WARNINGS:   The color value specified is the raw pixel value that will be stored. The       * 
 *             format of this value is dependant upon the pixel format of the surface.         * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/07/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool XSurface::Put_Pixel(Point2D const & point, int color)
{
	void * pointer = Lock(point);
	if (pointer != NULL) {
		if (Bytes_Per_Pixel() == 2) {
			*((unsigned short*)pointer) = (unsigned short)color;
		} else {
			*((unsigned char*)pointer) = (unsigned char)color;
		}
		Unlock();
		return(true);
	}
	return(false);
}


/*********************************************************************************************** 
 * XSurface::Fill_Rect -- Fills a rectangle with the color specified.                          * 
 *                                                                                             * 
 *    This routine will fill the rectangle with a specified color. The rectangle filled will   * 
 *    be clipped appropriately.                                                                * 
 *                                                                                             * 
 * INPUT:   fillrect -- The rectangle to fill.                                                 * 
 *                                                                                             * 
 *          color    -- The color to use when filling the rectangle.                           * 
 *                                                                                             * 
 * OUTPUT:  bool; Was the rectangle filled without error?                                      * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/07/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool XSurface::Fill_Rect(Rect const & fillrect, int color)
{
	return(XSurface::Fill_Rect(fillrect, Get_Rect(), color));
}


/*********************************************************************************************** 
 * XSurface::Fill_Rect -- Fill a rectangle but perform clipping on the fill.                   * 
 *                                                                                             * 
 *    This will fill a rectangle of the specified dimensions. The fill request will be         * 
 *    clipped against the clipping rectangle provided.                                         * 
 *                                                                                             * 
 * INPUT:   cliprect -- The clipping rectangle to use for this fill.                           * 
 *                                                                                             * 
 *          fillrect -- The rectangle to fill with the specified color. The rectangle is       * 
 *                      relative to the clipping rectangle coordinates.                        * 
 *                                                                                             * 
 *          color    -- The screen format pixel value to draw with.                            * 
 *                                                                                             * 
 * OUTPUT:  bool; Was the fill request carried out? A 'false' return value would indicate      * 
 *                that the fill was clipped away.                                              * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/27/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool XSurface::Fill_Rect(Rect const & cliprect, Rect const & fillrect, int color)
{
	if (!fillrect.Is_Valid()) return(false);

	/*
	**	Ensure that the requested clipping rectangle is actually legal.
	*/
	Rect wrect = cliprect.Intersect(Get_Rect());

	/*
	**	Clip the rectangle to be filled against the clipping rectangle supplied.
	*/
	Rect crect = wrect.Intersect(fillrect.Bias_To(cliprect));
	if (!crect.Is_Valid()) return(false);

	void * buffer = Lock(crect.Top_Left());
	if (buffer != NULL) {
		if (Bytes_Per_Pixel() == 1) {
			for (int y = 0; y < crect.Height; y++) {
				memset(buffer, color, crect.Width);
				buffer = ((char *)buffer) + Stride();
			}
		} else {
			for (int y = 0; y < crect.Height; y++) {
				for (int x = 0; x < crect.Width; x++) {
					((unsigned short*)buffer)[x] = (unsigned short)color;
				}
				buffer = ((char *)buffer) + Stride();
			}
		}
		Unlock();
		return(true);
	}
	return(false);
}


/*********************************************************************************************** 
 * XSurface::Fill -- Fill the entire surface with the color specified.                         * 
 *                                                                                             * 
 *    The color specified will be filled into the entire surface area (but limited by the      * 
 *    surface's current window).                                                               * 
 *                                                                                             * 
 * INPUT:   color -- The pixel value to use when filling the surface.                          * 
 *                                                                                             * 
 * OUTPUT:  bool; Was the surface fill performed without error?                                * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/07/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool XSurface::Fill(int color)
{
	return(Fill_Rect(Get_Rect(), Get_Rect(), color));
}


/*********************************************************************************************** 
 * XSurface::Blit_From -- Blit entire surface.                                                 * 
 *                                                                                             * 
 *    This routine will blit the entire surface.                                               * 
 *                                                                                             * 
 * INPUT:   source   -- Pointer to the source surface to blit from.                            * 
 *                                                                                             * 
 *          trans    -- Perform a transparency aware blit?                                     * 
 *                                                                                             * 
 * OUTPUT:  bool; Was the surface blit performed without error?                                * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/07/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool XSurface::Blit_From(Surface const & source, bool trans)
{
	Rect drect = Get_Rect();
	drect.X = 0;
	drect.Y = 0;

	Rect srect = source.Get_Rect();
	srect.X = 0;
	srect.Y = 0;
	bool result = Blit_From(drect, source, srect, trans);

	return(result);
}


/*********************************************************************************************** 
 * XSurface::Blit_From -- Blit one region to another.                                          * 
 *                                                                                             * 
 *    Use this routine to copy one rectangle of pixel data to another. The pixels being        * 
 *    copied may be processed according to the parameters specified.                           * 
 *                                                                                             * 
 * INPUT:   destrect    -- The destination rectangle to bit to.                                * 
 *                                                                                             * 
 *          source      -- Pointer to the source surface to blit from.                         * 
 *                                                                                             * 
 *          sourcerect  -- The source rectangle to blit from.                                  * 
 *                                                                                             * 
 *          trans       -- Should transparent pixels be scanned for an skipped?                * 
 *                                                                                             * 
 * OUTPUT:  bool; Was the blit operation performed without error?                              * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/07/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool XSurface::Blit_From(Rect const & destrect, Surface const & source, Rect const & sourcerect, bool trans)
{
	return(XSurface::Blit_From(Get_Rect(), destrect, source, source.Get_Rect(), sourcerect, trans));
}


/*********************************************************************************************** 
 * XSurface::Blit_From -- Blit from one surface to this one w/ clipping.                       * 
 *                                                                                             * 
 *    This routine will blit a clipped rectangle from the specified surface to this one.       * 
 *                                                                                             * 
 * INPUT:   dcliprect   -- The clipping rectangle to use for this (destination) surface.       * 
 *                                                                                             * 
 *          destrect    -- The destanation rect of the blit. Coordinates are relative to the   * 
 *                         destination clipping rectangle.                                     * 
 *                                                                                             * 
 *          source      -- The source surface to blit from.                                    * 
 *                                                                                             * 
 *          scliprect   -- The source clipping rectangle for the blit.                         * 
 *                                                                                             * 
 *          sourcrect   -- The source rectangle of the blit. The coordinates are relative to   * 
 *                         the source clipping rectangle.                                      * 
 *                                                                                             * 
 *          trans       -- Is this a transparent pixel aware blit request?                     * 
 *                                                                                             * 
 * OUTPUT:  bool; Was a blit performed? A 'false' value would mean that the blit was clipped   * 
 *                away.                                                                        * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/27/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool XSurface::Blit_From(Rect const & dcliprect, Rect const & destrect, Surface const & source, Rect const & scliprect, Rect const & sourcerect, bool trans)
{
	Rect drect = destrect;
	Rect srect = sourcerect;

	/*
	**	Perform any clipping against the clipping rectangles. Proceed with the blit only
	**	if there are pixels left unclipped.
	*/
	if (Blit_Clip(drect, dcliprect, srect, scliprect)) {

		if (trans) {
			return(Blit_Trans(*this, drect, source, srect));
		}
		return(Blit_Plain(*this, drect, source, srect));
	}
	return(false);
}	


/*********************************************************************************************** 
 * Blit_Clip -- Perform rectangle clipping in preparation for a blit.                          * 
 *                                                                                             * 
 *    This routine will take a source rectangle and a source clipping window and intersect     * 
 *    these with a dest rectangle and a dest clipping window. The effect will be to alter      * 
 *    the source and dest rectangles so that they will stay within the clipping rectangles     * 
 *    imposed upon the source and destination surfaces. The process clips the rectangles       * 
 *    rather than displacing them when performing its adjustment. It is possible that one      * 
 *    or both rectangles are clipped into oblivion by this routine. This condition will be     * 
 *    flagged with the return value.                                                           * 
 *                                                                                             * 
 * INPUT:   drect    -- Reference to the destination rectangle (relative coordinates to the    * 
 *                      destination window). This is both an input and output parameter.       * 
 *                                                                                             * 
 *          dwindow  -- The destination window to clip the dest rect against.                  * 
 *                                                                                             * 
 *          srect    -- Reference to the source rectangle (relative to the source window).     * 
 *                      This is both an input and output parameter.                            * 
 *                                                                                             * 
 *          swindow  -- The source window to clip the srect against.                           * 
 *                                                                                             * 
 * OUTPUT:  bool; Was the clip performed and the srect and drect parameters still valid.       * 
 *                i.e., they represent at least one pixel that has not been clipped away.      * 
 *                                                                                             * 
 * WARNINGS:   The rectangles may be clipped into nothingness by this routine. Be sure to      * 
 *             check the return value for this condition and perform no blit operation in      * 
 *             that case.                                                                      * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/19/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool Blit_Clip(Rect & drect, Rect const & dwindow, Rect & srect, Rect const & swindow)
{
	/*
	**	If the rectangles are of the same dimensions, then a coordinated clipping process is
	**	possible. This results in an intelligent blit even if the source or destination 
	**	rectangles is partially outside the clipping rectangle.
	*/
	if (drect.Width == srect.Width && drect.Height == srect.Height) {
		/*
		**	Clip the left and top edges against the clipping window.
		*/
		if (drect.X < 0) {
			srect.X += -drect.X;
			srect.Width -= -drect.X;
			drect.Width -= -drect.X;
			drect.X = 0;
		}
		if (drect.Y < 0) {
			srect.Y += -drect.Y;
			srect.Height -= -drect.Y;
			drect.Height -= -drect.Y;
			drect.Y = 0;
		}
		
		/*
		**	Clip the right and bottom edges if they spill past the
		**	clipping window boundaries.
		*/
		int rightspill = (drect.X+drect.Width) - dwindow.Width;
		if (rightspill > 0) {
			srect.Width -= rightspill;
			drect.Width -= rightspill;
		}
		int bottomspill = (drect.Y+drect.Height) - dwindow.Height;
		if (bottomspill > 0) {
			srect.Height -= bottomspill;
			drect.Height -= bottomspill;
		}

		/*
		**	Clip the left and top edges against the clipping
		**	window.
		*/
		if (srect.X < 0) {
			drect.X += -srect.X;
			srect.Width -= -srect.X;
			drect.Width -= -srect.X;
			srect.X = 0;
		}
		if (srect.Y < 0) {
			drect.Y += -srect.Y;
			srect.Height -= -srect.Y;
			drect.Height -= -srect.Y;
			srect.Y = 0;
		}

		/*
		**	Clip the right and bottom edges agaist the clipping window.
		*/
		rightspill = (srect.X+srect.Width) - swindow.Width;
		if (rightspill > 0) {
			srect.Width -= rightspill;
			drect.Width -= rightspill;
		}
		bottomspill = (srect.Y+srect.Height) - swindow.Height;
		if (bottomspill > 0) {
			srect.Height -= bottomspill;
			drect.Height -= bottomspill;
		}

	} else {

		/*
		**	Since the rectangles are not of the same dimensions, scaling is presumed. Clipping
		**	in such a case is merely a legality clip against the bounding rectangle. No coordinated
		**	adjustments can occur. For best results, boundary clipping should be performed prior to
		**	calling this routine.
		*/
		drect = drect.Intersect(dwindow);
		srect = srect.Intersect(swindow);
	}

	return(drect.Is_Valid() && srect.Is_Valid());
}


/*********************************************************************************************** 
 * XSurface::Prep_For_Blit -- Clips and prepares pointers for a blit operation.                * 
 *                                                                                             * 
 *    This performs the clipping operation required before a blit occurs. It examines the      * 
 *    source and destination coordinate constraints and performs clipping such that only       * 
 *    legal pixels will be blitted. As a consequence it can determine if the blit has been     * 
 *    completely clipped and thus can be skipped.                                              * 
 *                                                                                             * 
 * INPUT:   dest     -- The destination surface rect for the blit.                             * 
 *                                                                                             * 
 *          drect    -- The rectangle within the destination surface rect for the blit. This   * 
 *                      rectangle reference will be adjusted as necessary.                     * 
 *                                                                                             * 
 *          source   -- The source surface rect of the blit.                                   * 
 *                                                                                             * 
 *          srect    -- The source rectangle within the source surface rect for the blit.      * 
 *                      This rectangle reference will be adjusted as necessary.                * 
 *                                                                                             * 
 *          overlapped  -- Output reference that stores the boolean answer to the question --  * 
 *                         are the blit rectangles overlapping on the same surface?            * 
 *                                                                                             * 
 *          dbuffer  -- Output reference that stores a pointer to the locked destination       * 
 *                      surface. It points to the upper left destination corner pixel.         * 
 *                                                                                             * 
 *          sbuffer  -- Output reference that stores a pointer to the locked source surface.   * 
 *                      It points to the upper left corner source pixel.                       * 
 *                                                                                             * 
 * OUTPUT:  bool; Can the blit proceed since there is at least one pixel that has not been     * 
 *                clipped away? It can also fail if a lock could not be performed on the       * 
 *                source or destination surfaces.                                              * 
 *                                                                                             * 
 * WARNINGS:   The surfaces locked by this routine must be unlocked. If the return value is    * 
 *             'true', then the surfaces must be unlocked.                                     * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/19/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool XSurface::Prep_For_Blit(Surface & dest, Rect & drect, Surface const & source, Rect & srect, bool & overlapped, void * & dbuffer, void * & sbuffer)
{
	return(XSurface::Prep_For_Blit(dest, dest.Get_Rect(), drect, source, source.Get_Rect(), srect, overlapped, dbuffer, sbuffer));
#ifdef NEVER
	overlapped = false;
	dbuffer = NULL;
	sbuffer = NULL;

	if (!drect.Is_Valid() || !srect.Is_Valid()) return(false);

	/*
	**	Perform the clipping of the desired blit rectangles against the surface clipping
	**	rectangles. If it happens that the blit is clipped into oblivion, then bail
	**	immediately -- there is nothing left to do.
	*/
	Rect swindow = source.Get_Rect();
	Rect dwindow = dest.Get_Rect();
	if (!Blit_Clip(drect, dwindow, srect, swindow)) {
		return(false);
	}

	/*
	**	Determine if the rectangles overlap such that a forward blit would
	**	be prohibited. This only occurs if the source and destination refer to the
	**	same surface and the rectangles overlap.
	*/
	overlapped = false;
	if (&source == &dest && srect.Is_Overlapping(drect)) {
		if (srect.Y < drect.Y || (srect.Y == drect.Y && srect.X < drect.X)) {
			overlapped = true;
		}
	}

	/*
	**	Fetch pointers to the source and dest upper left pixel. That is, the upper
	**	left pixel of the source and dest sub-rectangles within each surface
	**	respectively.
	*/
	dbuffer = dest.Lock(drect.Point());
	if (dbuffer == NULL) return(false);
	sbuffer = ((Surface &)source).Lock(srect.Point());
	if (sbuffer == NULL) {
		dest.Unlock();
		return(false);
	}

	return(true);
#endif
}


/*********************************************************************************************** 
 * XSurface::Prep_For_Blit -- Clips and prepares pointers for a blit operation.                * 
 *                                                                                             * 
 *    This performs the clipping operation required before a blit occurs. It examines the      * 
 *    source and destination coordinate constraints and performs clipping such that only       * 
 *    legal pixels will be blitted. As a consequence it can determine if the blit has been     * 
 *    completely clipped and thus can be skipped.                                              * 
 *                                                                                             * 
 * INPUT:   dest     -- The destination surface rect for the blit.                             * 
 *                                                                                             * 
 *          dcliprect-- The destination clipping rectangle.                                    * 
 *                                                                                             * 
 *          drect    -- The rectangle within the destination surface rect for the blit. This   * 
 *                      rectangle reference will be adjusted as necessary.                     * 
 *                                                                                             * 
 *          source   -- The source surface rect of the blit.                                   * 
 *                                                                                             * 
 *          scliprect-- The source clipping rectangle.                                         * 
 *                                                                                             * 
 *          srect    -- The source rectangle within the source surface rect for the blit.      * 
 *                      This rectangle reference will be adjusted as necessary.                * 
 *                                                                                             * 
 *          overlapped  -- Output reference that stores the boolean answer to the question --  * 
 *                         are the blit rectangles overlapping on the same surface?            * 
 *                                                                                             * 
 *          dbuffer  -- Output reference that stores a pointer to the locked destination       * 
 *                      surface. It points to the upper left destination corner pixel.         * 
 *                                                                                             * 
 *          sbuffer  -- Output reference that stores a pointer to the locked source surface.   * 
 *                      It points to the upper left corner source pixel.                       * 
 *                                                                                             * 
 * OUTPUT:  bool; Can the blit proceed since there is at least one pixel that has not been     * 
 *                clipped away? It can also fail if a lock could not be performed on the       * 
 *                source or destination surfaces.                                              * 
 *                                                                                             * 
 * WARNINGS:   The surfaces locked by this routine must be unlocked. If the return value is    * 
 *             'true', then the surfaces must be unlocked.                                     * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/19/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool XSurface::Prep_For_Blit(Surface & dest, Rect const & dcliprect, Rect & drect, Surface const & source, Rect const & scliprect, Rect & srect, bool & overlapped, void * & dbuffer, void * & sbuffer)
{
	overlapped = false;
	dbuffer = NULL;
	sbuffer = NULL;

	if (!drect.Is_Valid() || !dcliprect.Is_Valid() || !srect.Is_Valid() || !scliprect.Is_Valid()) return(false);

	/*
	**	Perform the clipping of the desired blit rectangles against the surface clipping
	**	rectangles. If it happens that the blit is clipped into oblivion, then bail
	**	immediately -- there is nothing left to do.
	*/
	if (!Blit_Clip(drect, dcliprect, srect, scliprect)) {
		return(false);
	}

	/*
	**	Determine if the rectangles overlap such that a forward blit would
	**	be prohibited. This only occurs if the source and destination refer to the
	**	same surface and the rectangles overlap.
	*/
	overlapped = false;
	if (&source == &dest && srect.Is_Overlapping(drect)) {
		if (srect.Y < drect.Y || (srect.Y == drect.Y && srect.X < drect.X)) {
			overlapped = true;
		}
	}

	/*
	**	Fetch pointers to the source and dest upper left pixel. That is, the upper
	**	left pixel of the source and dest sub-rectangles within each surface
	**	respectively.
	*/
	dbuffer = dest.Lock(dcliprect.Top_Left() + drect.Top_Left());
	if (dbuffer == NULL) return(false);
	sbuffer = source.Lock(scliprect.Top_Left() + srect.Top_Left());
	if (sbuffer == NULL) {
		dest.Unlock();
		return(false);
	}

	return(true);
}	


/*********************************************************************************************** 
 * XSurface::Blit_Plain -- Blit a plain rectangle from one surface to another.                 * 
 *                                                                                             * 
 *    This routine will perform a simple blit of a rectangle from one surface to another.      * 
 *                                                                                             * 
 * INPUT:   dest     -- The destination surface for the blit.                                  * 
 *                                                                                             * 
 *          destrect -- The destination rectangle for the blit.                                * 
 *                                                                                             * 
 *          source   -- The source surface.                                                    * 
 *                                                                                             * 
 *          sourcerect  -- The rectangle in the source surface to blit from.                   * 
 *                                                                                             * 
 * OUTPUT:  bool; Was the blit performed?                                                      * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/19/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool XSurface::Blit_Plain(Surface & dest, Rect const & destrect, Surface const & source, Rect const & sourcerect)
{
	if (dest.Bytes_Per_Pixel() == 1) {
		return(Bit_Blit(dest, destrect, source, sourcerect, BlitPlain<unsigned char>()));
	}
	return(Bit_Blit(dest, destrect, source, sourcerect, BlitPlain<unsigned short>()));
}


/*********************************************************************************************** 
 * XSurface::Blit_Trans -- Blit a rectangle with transparency checking.                        * 
 *                                                                                             * 
 *    This routine will perform a simple blit of one rectangle to another on the surfaces      * 
 *    specified.                                                                               * 
 *                                                                                             * 
 * INPUT:   dest     -- The destination surface.                                               * 
 *                                                                                             * 
 *          destrect -- The destination rectangle within the surface.                          * 
 *                                                                                             * 
 *          source   -- The source surface.                                                    * 
 *                                                                                             * 
 *          sourcerect  -- The source rectangle.                                               * 
 *                                                                                             * 
 * OUTPUT:  bool; Was the blit performed?                                                      * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/19/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool XSurface::Blit_Trans(Surface & dest, Rect const & destrect, Surface const & source, Rect const & sourcerect)
{
	switch (dest.Bytes_Per_Pixel())
	{
		case 1:
			return(Bit_Blit(dest, destrect, source, sourcerect, BlitTrans<unsigned char>()));	
		case 2:
			return(Bit_Blit(dest, destrect, source, sourcerect, BlitTrans<unsigned short>()));	
		case 4:
			return(Bit_Blit(dest, destrect, source, sourcerect, BlitTrans<unsigned int>()));	
		default:
			return(false);
	}
}



