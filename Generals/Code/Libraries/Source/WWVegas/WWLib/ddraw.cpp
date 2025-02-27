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
 *                     $Archive:: /Commando/Code/Library/DDRAW.CPP                            $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 10/15/98 11:05a                                             $*
 *                                                                                             * 
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   Set_Video_Mode -- Initializes Direct Draw and sets the required Video Mode                *
 *   Process_DD_Result -- Does a message box based on the result of a DD command               *
 *   Reset_Video_Mode -- Resets video mode and deletes Direct Draw Object                      *
 *   Get_Free_Video_Memory -- returns amount of free video memory                              *
 *   Get_Video_Hardware_Caps -- returns bitmask of direct draw video hardware support          *
 *   Wait_Vert_Blank -- Waits for the start (leading edge) of a vertical blank                 *
 *   Set_Palette -- set a direct draw palette                                                  *
 *   Check_Overlapped_Blit_Capability -- See if video driver supports blitting overlapped regions*
 *   Wait_Blit -- waits for the DirectDraw blitter to become idle                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"misc.h"
#include	"dsurface.h"
#include	"data.h"
#include	"_timer.h"
#include	<assert.h>
#include <stdio.h>


LPDIRECTDRAW DirectDrawObject = NULL;	// Pointer to the direct draw object
LPDIRECTDRAW2 DirectDraw2Interface = NULL;  	// Pointer to direct draw 2 interface

static PALETTEENTRY PaletteEntries[256];		// 256 windows palette entries
static LPDIRECTDRAWPALETTE	PalettePtr;					// Pointer to direct draw palette object
static bool FirstPaletteSet = false;	// Is this the first time 'Set_Palette' has been called?
LPDIRECTDRAWSURFACE	PaletteSurface = NULL;
bool SurfacesRestored = false;
static bool CanVblankSync = true;

unsigned char CurrentPalette[768];
bool Debug_Windowed;

int (*DirectDrawErrorHandler)(HRESULT error) = NULL;

void Set_Palette(PaletteClass const & pal, int time, void (*callback)())
{
	CDTimerClass<SystemTimerClass> timer = time;
	PaletteClass original;
	memcpy(&original, CurrentPalette, sizeof(CurrentPalette));
	PaletteClass newpal = pal;

	while (timer) {

		/*
		**	Build an intermediate palette that is as close to the destination palette
		**	as the current time is proportional to the ending time.
		*/
		PaletteClass palette = original;
		int adjust = ((time - timer) * 256) / time;
		adjust = MIN(adjust, 255);
		palette.Adjust(adjust, newpal);

		/*
		**	Remember the current time so that multiple palette sets within the same game
		**	time tick won't occur. This is probably unnecessary since the palette setting
		**	code, at the time of this writing, delays at least one game tick in the process
		**	of setting the palette.
		*/
		int holdtime = timer;

		/*
		**	Set the palette to this intermediate palette and then loop back
		**	to calculate and set a new intermediate palette.
		*/
		Set_Palette((void*)&palette[0]);

		/*
		**	If the callback routine was specified, then call it once per palette
		**	setting loop.
		*/
		if (callback) {
			callback();
		}

		/*
		**	This loop ensures that the palette won't be set more than once per game tick. Setting
		**	the palette more than once per game tick will have no effect since the calculation will
		**	result in the same intermediate palette that was previously calculated.
		*/
		while (timer == holdtime && holdtime != 0) {
			if (callback) callback();
		}
	}

	/*
	**	Ensure that the final palette exactly matches the requested
	**	palette before exiting the fading routine.
	*/
	Set_Palette((void*)&newpal[0]);
}


/***********************************************************************************************
 * Process_DD_Result -- Does a message box based on the result of a DD command                 *
 *                                                                                             *
 * INPUT:		HRESULT result				- the result returned from the direct draw command		  *
 *             int     display_ok_msg	- should a message be displayed if command ok			  *                                                                                      *
 *                                                                                             *
 * OUTPUT:		none																									  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/27/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
void Process_DD_Result(HRESULT result, int display_ok_msg)
{
#ifdef _DEBUG
	static struct {
		HRESULT Error;
		char const * Message;
	} _errors[] = {
		{DDERR_ALREADYINITIALIZED, "This object is already initialized"},
		{DDERR_BLTFASTCANTCLIP, "Return if a clipper object is attached to the source surface passed into a BltFast call."},
		{DDERR_CANNOTATTACHSURFACE, "This surface can not be attached to the requested surface."},
		{DDERR_CANNOTDETACHSURFACE, "This surface can not be detached from the requested surface."},
		{DDERR_CANTCREATEDC, "Windows can not create any more DCs"},
		{DDERR_CANTDUPLICATE, "Can't duplicate primary & 3D surfaces, or surfaces that are implicitly created."},
		{DDERR_CANTLOCKSURFACE, "Unable to lock surface because no driver exists which can supply a pointer to the surface."},
		{DDERR_CLIPPERISUSINGHWND, "An attempt was made to set a cliplist for a clipper object that is already monitoring an hwnd."},
		{DDERR_COLORKEYNOTSET, "No src color key specified for this operation."},
		{DDERR_CURRENTLYNOTAVAIL, "Support is currently not available."},
		{DDERR_DIRECTDRAWALREADYCREATED, "A DirectDraw object representing this driver has already been created for this process."},
		{DDERR_EXCEPTION, "An exception was encountered while performing the requested operation."},
		{DDERR_EXCLUSIVEMODEALREADYSET, "An attempt was made to set the cooperative level when it was already set to exclusive."},
		{DDERR_GENERIC, "Generic failure."},
		{DDERR_HEIGHTALIGN, "Height of rectangle provided is not a multiple of reqd alignment."},
		{DDERR_HWNDALREADYSET, "The CooperativeLevel HWND has already been set. It can not be reset while the process has surfaces or palettes created."},
		{DDERR_HWNDSUBCLASSED, "HWND used by DirectDraw CooperativeLevel has been subclassed, this prevents DirectDraw from restoring state."},
		{DDERR_IMPLICITLYCREATED, "This surface can not be restored because it is an implicitly created surface."},
		{DDERR_INCOMPATIBLEPRIMARY, "Unable to match primary surface creation request with existing primary surface."},
		{DDERR_INVALIDCAPS, "One or more of the caps bits passed to the callback are incorrect."},
		{DDERR_INVALIDCLIPLIST, "DirectDraw does not support the provided cliplist."},
		{DDERR_INVALIDDIRECTDRAWGUID, "The GUID passed to DirectDrawCreate is not a valid DirectDraw driver identifier."},
		{DDERR_INVALIDMODE, "DirectDraw does not support the requested mode."},
		{DDERR_INVALIDOBJECT, "DirectDraw received a pointer that was an invalid DIRECTDRAW object."},
		{DDERR_INVALIDPARAMS, "One or more of the parameters passed to the function are incorrect."},
		{DDERR_INVALIDPIXELFORMAT, "The pixel format was invalid as specified."},
		{DDERR_INVALIDPOSITION, "Returned when the position of the overlay on the destination is no longer legal for that destination."},
		{DDERR_INVALIDRECT, "Rectangle provided was invalid."},
		{DDERR_INVALIDSURFACETYPE, "The requested action could not be performed because the surface was of the wrong type."},
		{DDERR_LOCKEDSURFACES, "Operation could not be carried out because one or more surfaces are locked."},
		{DDERR_NO3D, "There is no 3D present."},
		{DDERR_NOALPHAHW, "Operation could not be carried out because there is no alpha accleration hardware present or available."},
//		{DDERR_NOANTITEARHW, "Operation could not be carried out because there is no hardware support for synchronizing blts to avoid tearing.	"},
		{DDERR_NOBLTHW, "No blter hardware present."},
//		{DDERR_NOBLTQUEUEHW, "Operation could not be carried out because there is no hardware support for asynchronous blting."},
		{DDERR_NOCLIPLIST, "No cliplist available."},
		{DDERR_NOCLIPPERATTACHED, "No clipper object attached to surface object."},
		{DDERR_NOCOLORCONVHW, "Operation could not be carried out because there is no color conversion hardware present or available."},
		{DDERR_NOCOLORKEY, "Surface doesn't currently have a color key"},
		{DDERR_NOCOLORKEYHW, "Operation could not be carried out because there is no hardware support of the destination color key."},
		{DDERR_NOCOOPERATIVELEVELSET, "Create function called without DirectDraw object method SetCooperativeLevel being called."},
		{DDERR_NODC, "No DC was ever created for this surface."},
		{DDERR_NODDROPSHW, "No DirectDraw ROP hardware."},
		{DDERR_NODIRECTDRAWHW, "A hardware-only DirectDraw object creation was attempted but the driver did not support any hardware."},
		{DDERR_NODIRECTDRAWSUPPORT, "No DirectDraw support possible with current display driver."},
		{DDERR_NOEMULATION, "Software emulation not available."},
		{DDERR_NOEXCLUSIVEMODE, "Operation requires the application to have exclusive mode but the application does not have exclusive mode."},
		{DDERR_NOFLIPHW, "Flipping visible surfaces is not supported."},
		{DDERR_NOGDI, "There is no GDI present."},
		{DDERR_NOHWND, "Clipper notification requires an HWND or no HWND has previously been set as the CooperativeLevel HWND."},
		{DDERR_NOMIRRORHW, "Operation could not be carried out because there is no hardware present or available."},
		{DDERR_NOOVERLAYDEST, "Returned when GetOverlayPosition is called on an overlay that UpdateOverlay has never been called on to establish a destination."},
		{DDERR_NOOVERLAYHW, "Operation could not be carried out because there is no overlay hardware present or available."},
		{DDERR_NOPALETTEATTACHED, "No palette object attached to this surface.	"},
		{DDERR_NOPALETTEHW, "No hardware support for 16 or 256 color palettes."},
		{DDERR_NORASTEROPHW, "Operation could not be carried out because there is no appropriate raster op hardware present or available."},
		{DDERR_NOROTATIONHW, "Operation could not be carried out because there is no rotation hardware present or available."},
		{DDERR_NOSTRETCHHW, "Operation could not be carried out because there is no hardware support for stretching."},
		{DDERR_NOT4BITCOLOR, "DirectDrawSurface is not in 4 bit color palette and the requested operation requires 4 bit color palette."},
		{DDERR_NOT4BITCOLORINDEX, "DirectDrawSurface is not in 4 bit color index palette and the requested operation requires 4 bit color index palette."},
		{DDERR_NOT8BITCOLOR, "DirectDrawSurface is not in 8 bit color mode and the requested operation requires 8 bit color."},
		{DDERR_NOTAOVERLAYSURFACE, "Returned when an overlay member is called for a non-overlay surface."},
		{DDERR_NOTEXTUREHW, "Operation could not be carried out because there is no texture mapping hardware present or available."},
		{DDERR_NOTFLIPPABLE, "An attempt has been made to flip a surface that is not flippable."},
		{DDERR_NOTFOUND, "Requested item was not found."},
		{DDERR_NOTLOCKED, "Surface was not locked.  An attempt to unlock a surface that was not locked at all, or by this process, has been attempted."},
		{DDERR_NOTPALETTIZED, "The surface being used is not a palette-based surface."},
		{DDERR_NOVSYNCHW, "Operation could not be carried out because there is no hardware support for vertical blank synchronized operations."},
		{DDERR_NOZBUFFERHW, "Operation could not be carried out because there is no hardware support for zbuffer blting."},
		{DDERR_NOZOVERLAYHW, "Overlay surfaces could not be z layered based on their BltOrder because the hardware does not support z layering of overlays."},
		{DDERR_OUTOFCAPS, "The hardware needed for the requested operation has already been allocated."},
		{DDERR_OUTOFMEMORY, "DirectDraw does not have enough memory to perform the operation."},
		{DDERR_OUTOFVIDEOMEMORY, "DirectDraw does not have enough memory to perform the operation."},
		{DDERR_OVERLAYCANTCLIP, "The hardware does not support clipped overlays."},
		{DDERR_OVERLAYCOLORKEYONLYONEACTIVE, "Can only have ony color key active at one time for overlays."},
		{DDERR_OVERLAYNOTVISIBLE, "Returned when GetOverlayPosition is called on a hidden overlay."},
		{DDERR_PALETTEBUSY, "Access to this palette is being refused because the palette is already locked by another thread."},
		{DDERR_PRIMARYSURFACEALREADYEXISTS, "This process already has created a primary surface."},
		{DDERR_REGIONTOOSMALL, "Region passed to Clipper::GetClipList is too small."},
		{DDERR_SURFACEALREADYATTACHED, "This surface is already attached to the surface it is being attached to."},
		{DDERR_SURFACEALREADYDEPENDENT, "This surface is already a dependency of the surface it is being made a dependency of."},
		{DDERR_SURFACEBUSY, "Access to this surface is being refused because the surface is already locked by another thread."},
		{DDERR_SURFACEISOBSCURED, "Access to surface refused because the surface is obscured."},
		{DDERR_SURFACELOST, "Access to this surface is being refused because the surface memory is gone. The DirectDrawSurface object representing this surface should have Restore called on it."},
		{DDERR_SURFACENOTATTACHED, "The requested surface is not attached."},
		{DDERR_TOOBIGHEIGHT, "Height requested by DirectDraw is too large."},
		{DDERR_TOOBIGSIZE, "Size requested by DirectDraw is too large --	the individual height and width are OK."},
		{DDERR_TOOBIGWIDTH, "Width requested by DirectDraw is too large."},
		{DDERR_UNSUPPORTED, "Action not supported."},
		{DDERR_UNSUPPORTEDFORMAT, "FOURCC format requested is unsupported by DirectDraw."},
		{DDERR_UNSUPPORTEDMASK, "Bitmask in the pixel format requested is unsupported by DirectDraw."},
		{DDERR_VERTICALBLANKINPROGRESS, "Vertical blank is in progress."},
		{DDERR_WASSTILLDRAWING, "Informs DirectDraw that the previous Blt which is transfering information to or from this Surface is incomplete."},
		{DDERR_WRONGMODE, "This surface can not be restored because it was created in a different mode."},
		{DDERR_XALIGN, "Rectangle provided was not horizontally aligned on required boundary."}
	};
#endif
	/*
	**	If there iwas no error detected, then either bail out or display a message to
	**	this effect as indicated by the "display_ok_msg" parameter.
	*/
	if (result == DD_OK) {
		if (display_ok_msg) {
			MessageBox(MainWindow, "Direct Draw operation processed without error", "Note", MB_OK);
		}
		return;
	}

	if (DirectDrawErrorHandler) {
		DirectDrawErrorHandler(result);
		return;
	}
#ifdef _DEBUG
	/*
	**	Scan for a matching error code and display the appropriate message.
	*/
	for (int index = 0; index < ARRAY_SIZE(_errors); index++) {
		if (_errors[index].Error == result) {
			MessageBox(MainWindow, _errors[index].Message, "Westwood Library Direct Draw Error", MB_ICONEXCLAMATION|MB_OK);
			return;
		}
	}
#endif

	/*
	**	Since it fell out of the above loop, this must be an unrecognized error code.
	*/
	char str[80];
	sprintf(str, "DDRAW.DLL Error code = %08X", result);
	MessageBox(MainWindow, str, "Direct X", MB_ICONEXCLAMATION|MB_OK);
}


/***********************************************************************************************
 * Check_Overlapped_Blit_Capability -- See if video driver supports blitting overlapped regions*
 *                                                                                             *
 *  We will check for this by drawing something to a video page and blitting it over itself.   *
 * If we end up with the top line repeating then overlapped region blits dont work.            *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    6/7/96 5:06PM ST : Created                                                               *
 *=============================================================================================*/
void Check_Overlapped_Blit_Capability(void)
{
//	OverlappedVideoBlits = false;

#ifdef NEVER
	/*
	** Assume we can until we find out otherwise
	*/
	OverlappedVideoBlits = true;

	GraphicBufferClass test_buffer;

	test_buffer.Init (64, 64, NULL, 0, (GBC_Enum)GBC_VIDEOMEM);

	test_buffer.Clear();

	/*
	** Plot a pixel in the top left corner of the buffer.
	*/
	test_buffer.Put_Pixel(0, 0, 255);

	/*
	** Blit the buffer down by one line. If we end up with a vertical strip of pixel 255's then
	** overlapped blits dont work
	*/

	test_buffer.Blit(test_buffer, 0, 0, 0, 1, test_buffer.Get_Width(), test_buffer.Get_Height()-1);

	if (test_buffer.Get_Pixel(0, 5) == 255) OverlappedVideoBlits = false;
#endif
}


void Prep_Direct_Draw(void)
{
	//
	// If there is not currently a direct draw object then we need to define one.
	//
	if ( DirectDrawObject == NULL ) {
		HRESULT result = DirectDrawCreate(NULL, &DirectDrawObject, NULL);
		Process_DD_Result(result, false);
		if (result == DD_OK) {
			if (Debug_Windowed) {
				result = DirectDrawObject->SetCooperativeLevel(MainWindow, DDSCL_NORMAL);
			} else {
				result = DirectDrawObject->SetCooperativeLevel(MainWindow, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
			}
			Process_DD_Result(result, false);
		}
	}
}



/***********************************************************************************************
 * Set_Video_Mode -- Initializes Direct Draw and sets the required Video Mode                  *
 *                                                                                             *
 * INPUT:  		int width   			- the width of the video mode in pixels						  *
 *					int height           - the height of the video mode in pixels                   *
 *					int bits_per_pixel	- the number of bits per pixel the video mode supports     *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/26/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
bool Set_Video_Mode(HWND , int w, int h, int bits_per_pixel)
{
	HRESULT result;

	Prep_Direct_Draw();

	//
	// Set the required display mode with 8 bits per pixel
	//
	//MessageBox(MainWindow, "In Set_Video_Mode. About to call call SetDisplayMode.","Note", MB_ICONEXCLAMATION|MB_OK);
	result = DirectDrawObject->SetDisplayMode(w, h, bits_per_pixel);
	if (result != DD_OK) {
//		Process_DD_Result(result, false);
		DirectDrawObject->Release();
		DirectDrawObject = NULL;
		return(false);
	}

	//
	// Create a direct draw palette object
	//
	//MessageBox(MainWindow, "In Set_Video_Mode. About to call CreatePalette.","Note", MB_ICONEXCLAMATION|MB_OK);
	result = DirectDrawObject->CreatePalette( DDPCAPS_8BIT | DDPCAPS_ALLOW256, &PaletteEntries[0], &PalettePtr, NULL);
	Process_DD_Result(result, false);
	if (result != DD_OK) {
		return (false);
	}

	Check_Overlapped_Blit_Capability();

	//MessageBox(MainWindow, "In Set_Video_Mode. About to return success.","Note", MB_ICONEXCLAMATION|MB_OK);
#if (0)
	/*
	** Find out if DirectX 2 extensions are available
	*/
	result = DirectDrawObject->QueryInterface (IID_IDirectDraw2, (LPVOID*)&DirectDraw2Interface);
	SystemToVideoBlits = false;
	VideoToSystemBlits = false;
	SystemToSystemBlits= false;
	if (result != DD_OK) {
		DirectDraw2Interface = NULL;
	} else {
		DDCAPS capabilities;
		DDCAPS emulated_capabilities;

		memset ((char*)&capabilities, 0, sizeof(capabilities));
		memset ((char*)&emulated_capabilities, 0, sizeof(emulated_capabilities));
		capabilities.dwSize = sizeof (capabilities);
		emulated_capabilities.dwSize = sizeof (emulated_capabilities);

		DirectDrawObject->GetCaps (&capabilities, &emulated_capabilities);

		if (capabilities.dwCaps & DDCAPS_CANBLTSYSMEM) {
			SystemToVideoBlits = (capabilities.dwSVBCaps & DDCAPS_BLT) ? true : false;
			VideoToSystemBlits = (capabilities.dwVSBCaps & DDCAPS_BLT) ? true : false;
			SystemToSystemBlits = (capabilities.dwSSBCaps & DDCAPS_BLT) ? true : false;
		}
	}
#endif	//(0)

	//MessageBox(MainWindow, "In Set_Video_Mode. About to return success.","Note", MB_ICONEXCLAMATION|MB_OK);

	return (true);

}

/***********************************************************************************************
 * Reset_Video_Mode -- Resets video mode and deletes Direct Draw Object                        *
 *                                                                                             *
 * INPUT:		none                                                                            *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/26/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
void Reset_Video_Mode(void)
{
	HRESULT result;

	//
	// If a direct draw object has been declared and a video mode has been set
	// then reset the video mode and release the direct draw object.
	//
	if ( DirectDrawObject ) {
		result = DirectDrawObject->RestoreDisplayMode();
		Process_DD_Result(result, false);
		result = DirectDrawObject->Release();
		Process_DD_Result(result, false);

		DirectDrawObject = NULL;
	}
}


/***********************************************************************************************
 * Get_Free_Video_Memory -- returns amount of free video memory                                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   bytes of available video RAM                                                      *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    11/29/95 12:52PM ST : Created                                                            *
 *=============================================================================================*/
unsigned int Get_Free_Video_Memory(void)
{
	DDCAPS	video_capabilities;

	if (DirectDrawObject) {

		video_capabilities.dwSize = sizeof (video_capabilities);

		if (DD_OK == DirectDrawObject->GetCaps (&video_capabilities, NULL)) {
			char string [256];
			wsprintf (string, "In Get_Free_Video_Memory. About to return %d bytes",video_capabilities.dwVidMemFree);
			return (video_capabilities.dwVidMemFree);
		}
	}

	return (0);
}


/***********************************************************************************************
 * Get_Video_Hardware_Caps -- returns bitmask of direct draw video hardware support            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   hardware flags                                                                    *
 *                                                                                             *
 * WARNINGS: Must call Set_Video_Mode 1st to create the direct draw object                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    1/12/96 9:14AM ST : Created                                                              *
 *=============================================================================================*/
unsigned Get_Video_Hardware_Capabilities(void)
{
	DDCAPS	video_capabilities;
	unsigned	video;

	/*
	** Fail if the direct draw object has not been initialised
	*/
	if (!DirectDrawObject) return (0);

	/*
	** Get the capabilities of the direct draw object
	*/
	video_capabilities.dwSize = sizeof(video_capabilities);
	//MessageBox(MainWindow, "In Get_Video_Hardware_Capabilities. About to call GetCaps","Note", MB_ICONEXCLAMATION|MB_OK);
	HRESULT result = DirectDrawObject->GetCaps (&video_capabilities, NULL);
	if (result != DD_OK) {
		Process_DD_Result(result, false);
		return (0);
	}

	/*
	** Set flags to indicate the presence of the features we are interested in
	*/
	video = 0;

	/* Hardware blits supported? */
	if (video_capabilities.dwCaps & DDCAPS_BLT) 				video |= VIDEO_BLITTER;

	/* Hardware blits asyncronous? */
	if (video_capabilities.dwCaps & DDCAPS_BLTQUEUE) 		video |= VIDEO_BLITTER_ASYNC;

	/* Can palette changes be synced to vertical refresh? */
	if (video_capabilities.dwCaps & DDCAPS_PALETTEVSYNC) 	video |= VIDEO_SYNC_PALETTE;

	/* Is the video cards memory bank switched? */
	if (video_capabilities.dwCaps & DDCAPS_BANKSWITCHED) 	video |= VIDEO_BANK_SWITCHED;

	/* Can the blitter do filled rectangles? */
	if (video_capabilities.dwCaps & DDCAPS_BLTCOLORFILL)	video |= VIDEO_COLOR_FILL;

	/* Is there no hardware assistance avaailable at all? */
	if (video_capabilities.dwCaps & DDCAPS_NOHARDWARE) 	video |= VIDEO_NO_HARDWARE_ASSIST;

	//MessageBox(MainWindow, "In Get_Video_Hardware_Capabilities. About to return success.","Note", MB_ICONEXCLAMATION|MB_OK);
	return (video);
}


/***********************************************************************************************
 * Wait_Vert_Blank -- Waits for the start (leading edge) of a vertical blank                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
void Wait_Vert_Blank(void)
{
	if (CanVblankSync) {
		HRESULT result = DirectDrawObject->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
		if (result == E_NOTIMPL) {
			CanVblankSync = false;
			return;
		}
		Process_DD_Result(result, false);
	}
}


/***********************************************************************************************
 * Set_Palette -- set a direct draw palette                                                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ptr to 768 rgb palette bytes                                                      *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    10/11/95 3:33PM ST : Created                                                             *
 *=============================================================================================*/
void Set_Palette(void const * palette)
{
	assert(palette != NULL);

	if (&CurrentPalette[0] != palette) {
		memmove(CurrentPalette, palette, sizeof(CurrentPalette));
	}

	if (DirectDrawObject != NULL && PaletteSurface != NULL) {
		unsigned char * palette_get = (unsigned char *)palette;
		for (int index = 0; index < 256; index++) {

			int red = *palette_get++;
			int green = *palette_get++;
			int blue = *palette_get++;

			PaletteEntries[index].peRed = (unsigned char)red;
			PaletteEntries[index].peGreen = (unsigned char)green;
			PaletteEntries[index].peBlue = (unsigned char)blue;
		}

		if (PalettePtr != NULL) {
			if (!FirstPaletteSet) {
				PaletteSurface->SetPalette(PalettePtr);
				FirstPaletteSet = true;
			}

			PalettePtr->SetEntries(0, 0, 256, &PaletteEntries[0]);
		}
	}
}


/***********************************************************************************************
 * Wait_Blit -- waits for the DirectDraw blitter to become idle                                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07-25-95 03:53pm ST : Created                                                             *
 *=============================================================================================*/
void Wait_Blit (void)
{
	HRESULT	return_code;

	do {
		return_code=PaletteSurface->GetBltStatus (DDGBS_ISBLTDONE);
	} while (return_code != DD_OK && return_code != DDERR_SURFACELOST);
}

