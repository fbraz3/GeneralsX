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
 *                     $Archive:: /Commando/Code/wwlib/shapeset.h                             $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 11/28/00 2:44p                                              $*
 *                                                                                             * 
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   ShapeSet::Fetch_Data -- Fetches pointer to raw shape data.                                * 
 *   ShapeSet::Fetch_Rect -- Fetch the sub-rectangle of a shape.                               * 
 *   ShapeSet::Is_Transparent -- Is the specified shape containing transparent pixels?         * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SHAPESET_H
#define SHAPESET_H

#include	"bool.h"
#include	"trect.h"

/*
**	This is the header that appears at the beginning of the ShapeSet file. The header
**	is a multiple of 8 bytes long to facilitate data alignment. The shape header is
**	immediately followed by an N length array of ShapeRecord objects. The actual
**	shape data follows this array.
*/
class ShapeSet
{
	public:
		/*
		**	Fetch pointer to raw shape data (NULL if not present or empty).
		*/
		void * Get_Data(int shape) const;

		/*
		**	Fetch sub-rectangle that the shape data represents.
		*/
		Rect Get_Rect(int shape) const;

		/*
		**	Are there any transparent pixels within the shape?
		*/
		bool Is_Transparent(int shape) const;

		/*
		**	Determines if the specified shape is RLE compressed.
		*/
		bool Is_RLE_Compressed(int shape) const;

		/*
		**	Logical width of the shape set.
		*/
		int Get_Width(void) const {return(Width);}

		/*
		**	Logical height of the shape set.
		*/
		int Get_Height(void) const {return(Height);}

		/*
		**	Number of shapes in this shape set.
		*/
		int Get_Count(void) const{return(Count);}

		/*
		**	Gets the raw data size for a particular shape.
		*/
		int Get_Size(int shape) const;

	protected:
		/*
		**	QShape information flags.
		*/
		short Flags;

		/*
		**	The nominal width and height of the shape (in pixels).
		*/
		short Width;
		short Height;

		/*
		**	The total number of shapes in the file.
		*/
		short Count;

		/*
		**	Each shape is represented by this structure. It appears in a linear array near the
		**	beginning of the file.
		*/
		class ShapeRecord 
		{
			public:
				/*
				**	The sub-offset (relative to logical 0,0 at upper left corner) for this
				**	shape's data.
				*/
				short X;
				short Y;

				/*
				**	The dimensions of this shape's data.
				*/
				short Width;
				short Height;

				/*
				**	Flags that indicate some aspect of this particular shape image.
				*/
				short Flags;
				enum {
					SFLAG_TRANSPARENT=0x01,		// Are there are any transparent pixels present?
					SFLAG_RLE=0x02					// Is it RLE compressed?
				};


				/*
				**	Size of the data for this frame.
				*/
				short Size;

				/*
				**	Offset from the start of the shape data file to where the first pixel
				**	of this shape image data is located.
				*/
				long Data;

				bool Is_Transparent(void) const {return((Flags & SFLAG_TRANSPARENT) != 0);}
				bool Is_RLE_Compressed(void) const {return((Flags & SFLAG_RLE) != 0);}
				short Get_Size(void) const {return(Size);}

				void Flag_Transparent(void) {Flags |= SFLAG_TRANSPARENT;}
				void Flag_RLE_Compressed(void) {Flags |= SFLAG_RLE;}
				void Set_Size(short size) {Size = size;}
		};

		bool Is_Shape_Index_Valid(int index) const {return(unsigned(index) < unsigned(Count));}
		
		ShapeRecord const * Fetch_Record_Pointer(int shape) const 
		{
			if (Is_Shape_Index_Valid(shape)) {
				return((ShapeRecord const *)(((char *)this) + sizeof(ShapeSet) + sizeof(ShapeRecord) * shape));
			}
			return(NULL);
		}

		/*
		**	Prevents a shape object from being constructed illegally.
		*/
		ShapeSet(void) {};
		ShapeSet(ShapeSet const & rvalue);
		ShapeSet const & operator = (ShapeSet const & rvalue);
};


/*********************************************************************************************** 
 * ShapeSet::Get_Data -- Fetches pointer to raw shape data.                                    * 
 *                                                                                             * 
 *    This routine will return a pointer to the shape data. The data is actually a sub-        * 
 *    rectangle within the logical shape rectangle. The sub-rectangle holds the non-           * 
 *    transparent pixels. This fact, as retrieved by the Fetch_Rect function must be used      * 
 *    for proper rendering of the shape.                                                       * 
 *                                                                                             * 
 * INPUT:   shape -- The shape number to extract the pointer for.                              * 
 *                                                                                             * 
 * OUTPUT:  Returns with a pointer to the raw shape data. If the shape does not exist in the   * 
 *          data file, the returned value will be NULL. If the shape contains no pixels, then  * 
 *          NULL is also returned.                                                             * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/26/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
inline void * ShapeSet::Get_Data(int shape) const
{
	ShapeRecord const * record = Fetch_Record_Pointer(shape);
	if (record != NULL && record->Data != 0) return(((char *)this) + record->Data);
	return(NULL);
}


/*********************************************************************************************** 
 * ShapeSet::Get_Rect -- Fetch the sub-rectangle of a shape.                                   * 
 *                                                                                             * 
 *    This routine will fetch the sub-rectangle for a particular shape. This rectangle is      * 
 *    used to properly position the shape when rendering.                                      * 
 *                                                                                             * 
 * INPUT:   shape -- The shape number to fetch the rectangle for.                              * 
 *                                                                                             * 
 * OUTPUT:  Returns with the rectangle of the shape number specified.                          * 
 *                                                                                             * 
 * WARNINGS:   If the shape number is invalid or the shape has no pixels, the returned         * 
 *             rectangle will not pass the Is_Valid() test.                                    * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/26/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
inline Rect ShapeSet::Get_Rect(int shape) const
{
	ShapeRecord const * record = Fetch_Record_Pointer(shape);
	if (record != NULL) {
		return(Rect(record->X, record->Y, record->Width, record->Height));
	}
	return(Rect(0,0,0,0));
}


/*********************************************************************************************** 
 * ShapeSet::Is_Transparent -- Is the specified shape containing transparent pixels?           * 
 *                                                                                             * 
 *    This routine will check to see if the specified shape has any transparent pixels. If it  * 
 *    doesn't, then faster blitting code can be used.                                          * 
 *                                                                                             * 
 * INPUT:   shape -- The shape to examine.                                                     * 
 *                                                                                             * 
 * OUTPUT:  bool; Does the shape contain any transparent pixels? The answer is also false if   * 
 *                the shape number is invalid.                                                 * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/26/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
inline bool ShapeSet::Is_Transparent(int shape) const
{
	ShapeRecord const * record = Fetch_Record_Pointer(shape);
	if (record != NULL) {
		return(record->Is_Transparent());
	}
	return(false);
}


inline bool ShapeSet::Is_RLE_Compressed(int shape) const
{
	ShapeRecord const * record = Fetch_Record_Pointer(shape);
	if (record != NULL) {
		return(record->Is_RLE_Compressed());
	}
	return(false);
}



#endif
