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
 *                     $Archive:: /G/wwlib/dsurface.cpp                                       $*
 *                                                                                             *
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/23/00 2:26p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   DSurface::Blit_From -- Blit from one surface to this one.                                 * 
 *   DSurface::Blit_From -- Blit graphic memory from one rectangle to another.                 *
 *   DSurface::Build_Hicolor_Pixel -- Construct a hicolor pixel according to the surface pixel * 
 *   DSurface::Build_Remap_Table -- Build a highcolor remap table.                             * 
 *   DSurface::Bytes_Per_Pixel -- Fetches the bytes per pixel of the surface.                  *
 *   DSurface::Create_Primary -- Creates a primary (visible) surface.                          *
 *   DSurface::DSurface -- Create a surface attached to specified DDraw Surface Object.        *
 *   DSurface::DSurface -- Default constructor for surface object.                             *
 *   DSurface::DSurface -- Off screen direct draw surface constructor.                         *
 *   DSurface::Fill_Rect -- Fills a rectangle with clipping control.                           * 
 *   DSurface::Fill_Rect -- This routine will fill the specified rectangle.                    *
 *   DSurface::Lock -- Fetches a working pointer into surface memory.                          *
 *   DSurface::Restore_Check -- Checks for and restores surface memory if necessary.           *
 *   DSurface::Stride -- Fetches the bytes between rows.                                       *
 *   DSurface::Unlock -- Unlock a previously locked surface.                                   *
 *   DSurface::~DSurface -- Destructor for a direct draw surface object.                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"dsurface.h"
#include	<assert.h>

extern	LPDIRECTDRAW	DirectDrawObject;	//pointer to direct draw object
extern	LPDIRECTDRAWSURFACE	PaletteSurface;

/*
**	Clipper object (for primary surface).
*/
LPDIRECTDRAWCLIPPER DSurface::Clipper = NULL;

int DSurface::RedRight = 0;
int DSurface::RedLeft = 0;
int DSurface::BlueRight = 0;
int DSurface::BlueLeft = 0;
int DSurface::GreenRight = 0;
int DSurface::GreenLeft = 0;

unsigned short DSurface::HalfbrightMask = 0;
unsigned short DSurface::QuarterbrightMask = 0;
unsigned short DSurface::EighthbrightMask = 0;

DDPIXELFORMAT DSurface::PixelFormat;


/***********************************************************************************************
 * DSurface::DSurface -- Off screen direct draw surface constructor.                           *
 *                                                                                             *
 *    This constructor will create a Direct Draw enabled surface in video memory if possible.  *
 *    Such a surface will be able to use hardware assist if possible. The surface created      *
 *    is NOT visible. It only exists as a work surface and cannot be flipped to the visible    *
 *    surface. It can only be blitted to the visible surface.                                  *
 *                                                                                             *
 * INPUT:   width    -- The width of the surface to create.                                    *
 *                                                                                             *
 *          height   -- The height of the surface to create.                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The surface pixel format is the same as that of the visible display mode. It    *
 *             is important to construct surfaces using this routine, only AFTER the display   *
 *             mode has been set.                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
DSurface::DSurface(int width, int height, bool system_memory, DDPIXELFORMAT *pixform) :
	XSurface(width, height),
	BytesPerPixel(0),
	LockPtr(NULL),
	IsPrimary(false),
	IsVideoRam(false),
	SurfacePtr(NULL),
	Description(NULL),
	DCUnlockCount(0)
{
	Description = W3DNEW DDSURFACEDESC;
	if (Description != NULL) {
		memset(Description, '\0', sizeof(DDSURFACEDESC));
		Description->dwSize = sizeof(DDSURFACEDESC);
		Description->dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		Description->dwWidth = width;
		Description->dwHeight = height;

		Description->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

		if (system_memory == true)
			Description->ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

		/*
		** Was a custom (non-display-depth) pixel format specified?
		*/
		if (pixform)
		{
			Description->ddpfPixelFormat=*pixform;
			Description->dwFlags |= DDSD_PIXELFORMAT;
		}


		DirectDrawObject->CreateSurface(Description, &SurfacePtr, NULL);

		/*
		**	Get a description of the surface that was just allocated.
		*/
		if (SurfacePtr != NULL) {
			memset(Description, '\0', sizeof(DDSURFACEDESC));
			Description->dwSize = sizeof(DDSURFACEDESC);
			SurfacePtr->GetSurfaceDesc(Description);
			BytesPerPixel = (Description->ddpfPixelFormat.dwRGBBitCount+7)/8;
			IsVideoRam = ((Description->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) != 0);


			/*
			**	If this is a hicolor surface, then build the shift values for
			**	building and extracting the colors from the hicolor pixel.
			*/
			if (BytesPerPixel == 2) {
				int index;
				int shift = Description->ddpfPixelFormat.dwRBitMask;
				ThisRedRight = 0;
				ThisRedLeft = 0;
				for (index = 0; index < 16; index++) {
					if (shift & 0x01) break;
					shift >>= 1;
					ThisRedRight++;
				}
				for (index = 0; index < 8; index++) {
					if (shift & 0x80) break;
					shift <<= 1;
					ThisRedLeft++;
				}

				shift = Description->ddpfPixelFormat.dwGBitMask;
				ThisGreenRight = 0;
				ThisGreenLeft = 0;
				for (index = 0; index < 16; index++) {
					if (shift & 0x01) break;
					ThisGreenRight++;
					shift >>= 1;
				}
				for (index = 0; index < 8; index++) {
					if (shift & 0x80) break;
					ThisGreenLeft++;
					shift <<= 1;
				}

				shift = Description->ddpfPixelFormat.dwBBitMask;
				ThisBlueRight = 0;
				ThisBlueLeft = 0;
				for (index = 0; index < 16; index++) {
					if (shift & 0x01) break;
					ThisBlueRight++;
					shift >>= 1;
				}
				for (index = 0; index < 8; index++) {
					if (shift & 0x80) break;
					ThisBlueLeft++;
					shift <<= 1;
				}

			}
		}
	}
}


/***********************************************************************************************
 * DSurface::~DSurface -- Destructor for a direct draw surface object.                         *
 *                                                                                             *
 *    This will destruct (make invalid) the direct draw surface.                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
DSurface::~DSurface(void)
{
	/*
	**	If this is the primary surface, then the clipper must be detached from
	**	this surface and the clipper object deleted.
	*/
	if (IsPrimary && SurfacePtr != NULL && Clipper != NULL) {
		SurfacePtr->SetClipper(NULL);
		Clipper->Release();
		Clipper = NULL;
	}

	/*
	**	Delete the description of the surface.
	*/
	delete Description;
	Description = NULL;

	if (SurfacePtr != NULL)  {
		SurfacePtr->Release();
	}
	SurfacePtr = NULL;
}


/***********************************************************************************************
 * DSurface::DSurface -- Default constructor for surface object.                               *
 *                                                                                             *
 *    This default constructor for a surface object should not be used. Although it properly   *
 *    creates a non-functional surface, there is no use for such a surface. This default       *
 *    constructor is provided for those rare cases where symatics require a default            *
 *    constructor.                                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
DSurface::DSurface(void) :
	BytesPerPixel(0), 
	LockPtr(NULL), 
	SurfacePtr(NULL), 
	Description(NULL),
	DCUnlockCount(0)
{
	Description = W3DNEW DDSURFACEDESC;
	memset(Description, '\0', sizeof(DDSURFACEDESC));
	Description->dwSize = sizeof(DDSURFACEDESC);
}



/***********************************************************************************************
 * DSurface::GetDC -- Get the windows device context from our surface                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS: Any current locks will get unlocked while the DC is held                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/21/2000 NAK : Created.                                                                 *
 *=============================================================================================*/
HDC DSurface::GetDC(void)
{
	HDC hdc = NULL;
	HRESULT hr;


	// We have to remove all current locks to get the device context unfortunately...
	while (LockCount) {
		Unlock();
		DCUnlockCount++;
	}


	hr = SurfacePtr->GetDC(&hdc);
	if (hr != DD_OK)
	{
		while(DCUnlockCount)  // restore the lock state
		{
			Lock();
			DCUnlockCount--;
		}
		return(NULL);
	}

	// GetDC() locks the surface internally, so we need to reflect that here
	if (hr == DD_OK) {
		LockCount++;
	}else{
		hdc = NULL;
	}

	return (hdc);
}


/***********************************************************************************************
 * DSurface::ReleaseDC -- Release the windows device context from our surface                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS: Restores any locks held before the call to GetDC()                                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/21/2000 NAK : Created.                                                                 *
 *=============================================================================================*/
int DSurface::ReleaseDC(HDC hdc)
{
	HRESULT hr;

	hr = SurfacePtr->ReleaseDC(hdc);
	assert(hr == DD_OK);

	// ReleaseDC() unlocks the surface internally, so we need to reflect that here.
	if ((hr == DD_OK) && (LockCount > 0)) {
		LockCount--;
	}

	while(DCUnlockCount)  // restore the lock state
	{
		Lock();
		DCUnlockCount--;
	}

	return (1);
}




/***********************************************************************************************
 * DSurface::Create_Primary -- Creates a primary (visible) surface.                            *
 *                                                                                             *
 *    This routine is used to create the surface object that represents the currently          *
 *    visible display. The surface is not allocated, it is merely linked to the preexisting    *
 *    surface that the Windows GDI is also currently using.                                    *
 *                                                                                             *
 * INPUT:   backsurface -- Optional pointer to specify where the backpage (flip enabled)       *
 *                         pointer will be placed. If this parameter is NULL, then no          *
 *                         back surface will be created.                                       *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the primary surface.                                     *
 *                                                                                             *
 * WARNINGS:   There can be only one primary surface. If an additional call to this routine    *
 *             is made, another surface pointer will be returned, but it will point to the     *
 *             same surface as before.                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
DSurface * DSurface::Create_Primary(DSurface ** backsurface1)
{
	DSurface * surface = W3DNEW DSurface();
	int backcount = (backsurface1 != NULL) ? 1 : 0;

	/*
	**	Setup parameter for creating the primary surface. This will
	**	always be the visible surface plus optional back buffers of identical
	**	dimensions.
	*/
	surface->Description->dwFlags = DDSD_CAPS;
	surface->Description->ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if (backcount > 0) {
		surface->Description->ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		surface->Description->dwFlags |= DDSD_BACKBUFFERCOUNT;
		surface->Description->dwBackBufferCount = backcount;
	}
	HRESULT result = DirectDrawObject->CreateSurface(surface->Description, &surface->SurfacePtr, NULL);

	/*
	**	If the primary surface object was created, then fetch a pointer to the
	**	back buffer if there is one present.
	*/
	if (result == DD_OK) {
		if (backcount > 0) {
			LPDIRECTDRAWSURFACE back;
			DDSCAPS caps;
			caps.dwCaps = DDSCAPS_BACKBUFFER;
			result = surface->SurfacePtr->GetAttachedSurface(&caps, &back);
			if (result == DD_OK) {
				*backsurface1 = W3DNEW DSurface(back);
			}
		}

		/*
		**	Get a description of the surface that was just allocated.
		*/
		memset(surface->Description, '\0', sizeof(DDSURFACEDESC));
		surface->Description->dwSize = sizeof(DDSURFACEDESC);
		surface->SurfacePtr->GetSurfaceDesc(surface->Description);
		surface->BytesPerPixel = (surface->Description->ddpfPixelFormat.dwRGBBitCount+7)/8;
		surface->IsPrimary = true;

//		surface->Window.Set(Rect(0, 0, surface->Description->dwWidth, surface->Description->dwHeight));
		surface->Width = surface->Description->dwWidth;
		surface->Height = surface->Description->dwHeight;
		PaletteSurface = surface->SurfacePtr;

		/*
		**	Attach a clipper object to the surface so that it can cooperate
		**	with the system GDI. This only comes into play if there are going
		**	to be GDI graphical elements on top of the surface (normally this
		**	isn't the case for full screen games). It doesn't hurt to attach
		**	a clipper object anyway -- just in case.
		*/
		if (DirectDrawObject->CreateClipper(0, &Clipper, NULL) == DD_OK) {
			if (Clipper->SetHWnd(0, GetActiveWindow()) == DD_OK) {
				surface->SurfacePtr->SetClipper(Clipper);
			}
		}

		/*
		**	Fetch the pixel format for the surface.
		*/
		memcpy(&PixelFormat, &surface->Description->ddpfPixelFormat, sizeof(DDPIXELFORMAT));

		/*
		**	If this is a hicolor surface, then build the shift values for
		**	building and extracting the colors from the hicolor pixel.
		*/
		if (surface->Bytes_Per_Pixel() == 2) {
			int index;
			int shift = PixelFormat.dwRBitMask;
			RedRight = 0;
			RedLeft = 0;
			for (index = 0; index < 16; index++) {
				if (shift & 0x01) break;
				shift >>= 1;
				RedRight++;
			}
			for (index = 0; index < 8; index++) {
				if (shift & 0x80) break;
				shift <<= 1;
				RedLeft++;
			}

			shift = PixelFormat.dwGBitMask;
			GreenRight = 0;
			GreenLeft = 0;
			for (index = 0; index < 16; index++) {
				if (shift & 0x01) break;
				GreenRight++;
				shift >>= 1;
			}
			for (index = 0; index < 8; index++) {
				if (shift & 0x80) break;
				GreenLeft++;
				shift <<= 1;
			}

			shift = PixelFormat.dwBBitMask;
			BlueRight = 0;
			BlueLeft = 0;
			for (index = 0; index < 16; index++) {
				if (shift & 0x01) break;
				BlueRight++;
				shift >>= 1;
			}
			for (index = 0; index < 8; index++) {
				if (shift & 0x80) break;
				BlueLeft++;
				shift <<= 1;
			}


			/*
			**	Create the halfbright mask.
			*/
			HalfbrightMask = (unsigned short)Build_Hicolor_Pixel(127, 127, 127);
			QuarterbrightMask = (unsigned short)Build_Hicolor_Pixel(63, 63, 63);
			EighthbrightMask = (unsigned short)Build_Hicolor_Pixel(31, 31, 31);
		}

	} else {
		delete surface;
		surface = NULL;
	}

	return(surface);
}


/***********************************************************************************************
 * DSurface::DSurface -- Create a surface attached to specified DDraw Surface Object.          *
 *                                                                                             *
 *    If an existing Direct Draw Surface Object is available, use this constructor to create   *
 *    a DSurface object that is attached to the surface specified.                             *
 *                                                                                             *
 * INPUT:   surfaceptr  -- Pointer to a preexisting Direct Draw Surface Object.                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
DSurface::DSurface(LPDIRECTDRAWSURFACE surfaceptr) :
	BytesPerPixel(0),
	LockPtr(NULL),
	SurfacePtr(surfaceptr),
	Description(NULL)
{
	if (SurfacePtr != NULL) {
		Description = W3DNEW DDSURFACEDESC;
		memset(Description, '\0', sizeof(DDSURFACEDESC));
		Description->dwSize = sizeof(DDSURFACEDESC);
		HRESULT result = SurfacePtr->GetSurfaceDesc(Description);
		if (result == DD_OK) {
			BytesPerPixel = (Description->ddpfPixelFormat.dwRGBBitCount+7)/8;
//			Window.Set(Rect(0, 0, Description->dwWidth, Description->dwHeight));
			Width = Description->dwWidth;
			Height = Description->dwHeight;
		}
	}
}


/***********************************************************************************************
 * DSurface::Bytes_Per_Pixel -- Fetches the bytes per pixel of the surface.                    *
 *                                                                                             *
 *    This routine will return with the number of bytes that each pixel consumes. The value    *
 *    is dependant upon the graphic mode of the display.                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the bytes per pixel of the surface object.                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int DSurface::Bytes_Per_Pixel(void) const
{
	return(BytesPerPixel);
}


/***********************************************************************************************
 * DSurface::Stride -- Fetches the bytes between rows.                                         *
 *                                                                                             *
 *    This routine will return the number of bytes to add so that the pointer will be          *
 *    positioned at the same column, but one row down the screen. This value may very well     *
 *    NOT be equal to the width multiplied by the bytes per pixel.                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the byte difference between subsequent pixel rows.                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int DSurface::Stride(void) const
{
	return(Description->lPitch);
}


/***********************************************************************************************
 * DSurface::Lock -- Fetches a working pointer into surface memory.                            *
 *                                                                                             *
 *    This routine will return with a pointer to the pixel at the location specified. In order *
 *    to directly manipulate surface memory, the surface memory must be mapped into the        *
 *    program's logical address space. In addition, all blitter activity on the surface will   *
 *    be suspended. Every call to Lock must be have a corresponding call to Unlock if the      *
 *    pointer returned is not equal to NULL.                                                   *
 *                                                                                             *
 * INPUT:   point -- Pixel coordinate to return a pointer to.                                  *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the pixel specified. If the return value is NULL, then   *
 *          the surface could not be locked and no call to Unlock should be performed.         *
 *                                                                                             *
 * WARNINGS:   It is important not to keep a surface locked indefinately since the blitter     *
 *             will not be able to function. Due to the time that locking consumes, it is      *
 *             also important to not perform unnecessarily frequent Lock calls.                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void * DSurface::Lock(Point2D point) const
{
	Restore_Check();
	if (LockCount == 0) {
		DDSURFACEDESC desc;
		memset(&desc, '\0', sizeof(desc));
		desc.dwSize = sizeof(desc);
		HRESULT result = SurfacePtr->Lock(NULL, &desc, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT, NULL);
   	if (result != DD_OK) return(NULL);
		memcpy(Description, &desc, sizeof(DDSURFACEDESC));
		BytesPerPixel = (Description->ddpfPixelFormat.dwRGBBitCount+7)/8;
		LockPtr = Description->lpSurface;
	}
	XSurface::Lock();
	return(((char*)LockPtr) + point.Y * Stride() + point.X * Bytes_Per_Pixel());
}


/***********************************************************************************************
 * DSurface::Unlock -- Unlock a previously locked surface.                                     *
 *                                                                                             *
 *    After a surface has been successfully locked, a call to the Unlock() function is         *
 *    required.                                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the unlock successful?                                                   *
 *                                                                                             *
 * WARNINGS:   Only pair a call to Unlock if the prior Lock actually returned a non-NULL       *
 *             value.                                                                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool DSurface::Unlock(void) const
{
	Restore_Check();
	if (LockCount > 0) {
		XSurface::Unlock();
		if (LockCount == 0) {
			SurfacePtr->Unlock(LockPtr);
			LockPtr = NULL;
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * DSurface::Restore_Check -- Checks for and restores surface memory if necessary.             *
 *                                                                                             *
 *    This routine will check to see if surface memory has been lost to the surface. If it     *
 *    has, then the surface memory will be restored.                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void DSurface::Restore_Check(void) const
{
	if (SurfacePtr->IsLost() == DDERR_SURFACELOST) {
		SurfacePtr->Restore();
		if (LockCount > 0 && SurfacePtr->IsLost() != DDERR_SURFACELOST) {
			int oldlockcount = LockCount;
			LockCount = 0;
			Lock();
			LockCount++;
			Unlock();
			LockCount = oldlockcount;
		}
	}
}


/***********************************************************************************************
 * DSurface::Blit_From -- Blit graphic memory from one rectangle to another.                   *
 *                                                                                             *
 *    This routine will use the blitter (if possible) to blit a block of graphic memory from   *
 *    one screen rectangle to another. If the rectangles do no match in size, scaling may      *
 *    be performed.                                                                            *
 *                                                                                             *
 * INPUT:   destrect -- The destination rectangle.                                             *
 *                                                                                             *
 *          ssource  -- The source surface to blit from.                                       *
 *                                                                                             *
 *          sourecrect  -- The source rectangle.                                               *
 *                                                                                             *
 *          trans    -- Should transparency checking be performed?                             *
 *                                                                                             *
 * OUTPUT:  bool; Was the blit performed without error?                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool DSurface::Blit_From(Rect const & destrect, Surface const & ssource, Rect const & sourcerect, bool trans)
{
	return(Blit_From(Get_Rect(), destrect, ssource, ssource.Get_Rect(), sourcerect, trans));
}


/*********************************************************************************************** 
 * DSurface::Blit_From -- Blit from one surface to this one.                                   * 
 *                                                                                             * 
 *    Use this routine to blit a rectangle from the specified surface to this surface while    * 
 *    performing clipping upon the blit rectangles specified.                                  * 
 *                                                                                             * 
 * INPUT:   dcliprect   -- The clipping rectangle to use for this surface.                     * 
 *                                                                                             * 
 *          destrect    -- The destination rectangle of the blit. The is relative to the       * 
 *                         dcliprect parameter.                                                * 
 *                                                                                             * 
 *          ssource     -- The source surface of the blit.                                     * 
 *                                                                                             * 
 *          scliprect   -- The source clipping rectangle.                                      * 
 *                                                                                             * 
 *          sourcrect   -- The source rectangle of the blit. This rectangle is relative to     * 
 *                         the source clipping rectangle.                                      * 
 *                                                                                             * 
 *          trans       -- Is this a transparent blit request?                                 * 
 *                                                                                             * 
 * OUTPUT:  bool; Was there a blit performed? A 'false' return value would indicate that the   * 
 *                blit was clipped into nothing.                                               * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/27/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool DSurface::Blit_From(Rect const & dcliprect, Rect const & destrect, Surface const & ssource, Rect const & scliprect, Rect const & sourcerect, bool trans)
{
	if (!dcliprect.Is_Valid() || !scliprect.Is_Valid() || !destrect.Is_Valid() || !sourcerect.Is_Valid()) return(false);

	/*
	**	For non-direct draw surfaces, perform a manual blit operation. This is also
	**	necessary if any of the surfaces are currently locked. It is also necessary if the
	**	blit regions overlap and the blitter cannot handle overlapped regions.
	**
	** NOTE: Its legal to blit to a locked surface but not from a locked surface.
	** 	 	ST - 4/23/97 1:03AM
	*/
	if (!ssource.Is_Direct_Draw() || ((DSurface&)ssource).Is_Locked() || trans || Bytes_Per_Pixel() != ssource.Bytes_Per_Pixel()) {
		return(XSurface::Blit_From(destrect, ssource, sourcerect, trans));
	}

	Restore_Check();
	DSurface const & source = (DSurface const &)ssource;

	Rect drect = destrect;
	Rect srect = sourcerect;
	Rect swindow = scliprect.Intersect(ssource.Get_Rect());
	Rect dwindow = dcliprect.Intersect(Get_Rect());
	if (Blit_Clip(drect, dwindow, srect, swindow)) {
		RECT xdestrect;
		xdestrect.left = drect.X+dwindow.X;
		xdestrect.top = drect.Y+dwindow.Y;
		xdestrect.right = drect.X+dwindow.X+drect.Width;
		xdestrect.bottom = drect.Y+dwindow.Y+drect.Height;

		RECT xsrcrect;
		xsrcrect.left = srect.X+swindow.X;
		xsrcrect.top = srect.Y+swindow.Y;
		xsrcrect.right = srect.X+swindow.X+srect.Width;
		xsrcrect.bottom = srect.Y+swindow.Y+srect.Height;

		HRESULT result = SurfacePtr->Blt(&xdestrect, source.SurfacePtr, &xsrcrect, DDBLT_WAIT, NULL);
		return(result == DD_OK);
	}
	return(false);
}


/***********************************************************************************************
 * DSurface::Fill_Rect -- This routine will fill the specified rectangle.                      *
 *                                                                                             *
 *    This routine will fill the specified rectangle with a color.                             *
 *                                                                                             *
 * INPUT:   fillrect -- The rectangle to fill.                                                 *
 *                                                                                             *
 *          color    -- The color to fill with.                                                *
 *                                                                                             *
 * OUTPUT:  bool; Was the fill performed without error?                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool DSurface::Fill_Rect(Rect const & fillrect, int color)
{
	return(DSurface::Fill_Rect(Get_Rect(), fillrect, color));
}


/*********************************************************************************************** 
 * DSurface::Fill_Rect -- Fills a rectangle with clipping control.                             * 
 *                                                                                             * 
 *    This routine will fill a rectangle on this surface, but will clip the request against    * 
 *    a clipping rectangle first.                                                              * 
 *                                                                                             * 
 * INPUT:   cliprect -- The clipping rectangle to use for this surface.                        * 
 *                                                                                             * 
 *          fillrect -- The rectangle to fill with the specified color. The rectangle is       * 
 *                      relative to the clipping rectangle.                                    * 
 *                                                                                             * 
 *          color    -- The color (surface dependant format) to use when filling the rectangle * 
 *                      pixels.                                                                * 
 *                                                                                             * 
 * OUTPUT:  bool; Was a fill operation performed? A 'false' return value would mean that the   * 
 *                fill request was clipped into nothing.                                       * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/27/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
bool DSurface::Fill_Rect(Rect const & cliprect, Rect const & fillrect, int color)
{
	if (!fillrect.Is_Valid()) return(false);
	
	/*
	**	If the buffer is locked, then using the blitter to perform the fill is not possible.
	**	In such a case, perform a manual fill of the region.
	*/
	if (Is_Locked()) {
		return(XSurface::Fill_Rect(cliprect, fillrect, color));
	}

	Restore_Check();

	/*
	**	Ensure that the clipping rectangle is legal.
	*/
	Rect crect = cliprect.Intersect(Get_Rect());

	/*
	**	Bias the fill rect to the clipping rectangle.
	*/
	Rect frect = fillrect.Bias_To(cliprect);

	/*
	**	Find the region that should be filled after being clipped by the 
	**	clipping rectangle. This could result in no fill operation being performed
	**	if the desired fill rectangle has been completely clipped away.
	*/
	frect = frect.Intersect(crect);
	if (!frect.Is_Valid()) return(false);

	RECT rect;
	rect.left = frect.X;
	rect.top = frect.Y;
	rect.right = rect.left + frect.Width;
	rect.bottom = rect.top + frect.Height;

	DDBLTFX fx;
	memset(&fx, '\0', sizeof(fx));
	fx.dwSize = sizeof(fx);
	fx.dwFillColor = color;
	HRESULT result = SurfacePtr->Blt(&rect, NULL, NULL, DDBLT_WAIT|DDBLT_COLORFILL, &fx);
	return(result == DD_OK);
}


/*********************************************************************************************** 
 * DSurface::Build_Hicolor_Pixel -- Construct a hicolor pixel according to the surface pixel f * 
 *                                                                                             * 
 *    This routine will construct a pixel according to the highcolor pixel format for this     * 
 *    surface.                                                                                 * 
 *                                                                                             * 
 * INPUT:   red   -- The red component of the color (0..255).                                  * 
 *                                                                                             * 
 *          green -- The green component of the color (0..255).                                * 
 *                                                                                             * 
 *          blue  -- The blue component of the color (0..255).                                 * 
 *                                                                                             * 
 * OUTPUT:  Returns with a screen format pixel number that most closesly matches the color     * 
 *          specified.                                                                         * 
 *                                                                                             * 
 * WARNINGS:   The return value is card dependant and only applies to hicolor displays.        * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/27/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
int DSurface::Build_Hicolor_Pixel(int red, int green, int blue)
{
	return(((red >> RedLeft) << RedRight) | ((green >> GreenLeft) << GreenRight) | ((blue >> BlueLeft) << BlueRight));
}


/*********************************************************************************************** 
 * DSurface::Build_Remap_Table -- Build a highcolor remap table.                               * 
 *                                                                                             * 
 *    This will build a complete hicolor remap table for the palette specified. This table     * 
 *    can then be used to quickly fetch a pixel that matches the color index of the palette.   * 
 *                                                                                             * 
 * INPUT:   table -- The location to store the hicolor table. The buffer must be 256*2 bytes   * 
 *                   long.                                                                     * 
 *                                                                                             * 
 *          palette  -- The palette to use to create the remap table.                          * 
 *                                                                                             * 
 * OUTPUT:  none                                                                               * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/27/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
void DSurface::Build_Remap_Table(unsigned short * table, PaletteClass const & palette)
{
	assert(table != NULL);

	/*
	**	Build the hicolor index table according to the palette.
	*/
	for (int index = 0; index < 256; index++) {
		table[index] = (unsigned short)Build_Hicolor_Pixel(palette[index].Get_Red(), palette[index].Get_Green(), palette[index].Get_Blue());
	}
}


