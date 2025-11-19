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

// FILE: W3DMouse.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Mark W.
// Desc:   W3D Mouse cursor implementations
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "Common/GameMemory.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/rendobj.h"
#include "WW3D2/hanim.h"
#include "WW3D2/camera.h"

#include "assetmgr.h"

#include "W3DDevice/Common/W3DConvert.h"
#include "W3DDevice/GameClient/W3DMouse.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "GameClient/Display.h"
#include "GameClient/Image.h"
#include "GameClient/InGameUI.h"
#include "mutex.h"
#include "thread.h"


//Since there can't be more than 1 mouse, might as well keep these static.
static CriticalSectionClass mutex;
static Bool isThread;
static TextureClass *cursorTextures[Mouse::NUM_MOUSE_CURSORS][MAX_2D_CURSOR_ANIM_FRAMES];	///<Textures for each cursor type
static const Image *cursorImages[Mouse::NUM_MOUSE_CURSORS];			///<Images for use with the RM_POLYGON method.
static RenderObjClass *cursorModels[Mouse::NUM_MOUSE_CURSORS];	///< W3D models for each cursor type
static HAnimClass			*cursorAnims[Mouse::NUM_MOUSE_CURSORS];		///< W3D animations for each cursor type

///Mouse polling/update thread function
static class MouseThreadClass : public ThreadClass
{

public:
	MouseThreadClass() : ThreadClass() {}

	void Thread_Function();

} thread;

void MouseThreadClass::Thread_Function()
{

	//poll mouse and update position

	while (running)
	{
		isThread=TRUE;
		if (TheMouse)
			TheMouse->draw();
		isThread=FALSE;
		Switch_Thread();
	}
}

W3DMouse::W3DMouse( void )
{
	// zero our event list
	for (Int i=0; i<NUM_MOUSE_CURSORS; i++)
	{
		for (Int j=0; j<MAX_2D_CURSOR_ANIM_FRAMES; j++)
			cursorTextures[i][j]=NULL;
		cursorModels[i]=NULL;
		cursorAnims[i]=NULL;
	}

	m_currentD3DCursor=NONE;
	m_currentW3DCursor=NONE;
	m_currentPolygonCursor=NONE;
	m_currentAnimFrame = 0;
	m_currentD3DFrame = 0;
	m_currentFrames = 0;
	m_currentFMS= 1.0f/1000.0f;

	m_camera = NULL;
	m_drawing = FALSE;

}

W3DMouse::~W3DMouse( void )
{

	freeD3DAssets();
	freeW3DAssets();

	thread.Stop();

}

void W3DMouse::initPolygonAssets(void)
{
	CriticalSectionClass::LockClass m(mutex);

	//don't allow the mouse thread to initialize
	//wait for main app to do initialization.
	if (isThread)
		return;

	//Check if texture assets already loaded
	if (m_currentRedrawMode == RM_POLYGON && cursorImages[1] == NULL)
	{
		for (Int i=0; i<NUM_MOUSE_CURSORS; i++)
		{
			m_currentPolygonCursor = m_currentCursor;
			if (!m_cursorInfo[i].imageName.isEmpty())
				cursorImages[i]=TheMappedImageCollection->findImageByName(m_cursorInfo[i].imageName);
		}
	}
}

void W3DMouse::freePolygonAssets(void)
{

	for (Int i=0; i<NUM_MOUSE_CURSORS; i++)
	{
		cursorImages[i]=NULL;
	}
}

/**Release the textures required to display the selected cursor*/
Bool W3DMouse::releaseD3DCursorTextures(MouseCursor cursor)
{
	if (cursor == NONE || !cursorTextures[cursor][0])
		return TRUE;	//no texture for this cursor or texture never loaded

	for (Int i=0; i<MAX_2D_CURSOR_ANIM_FRAMES; i++)
	{
		REF_PTR_RELEASE(m_currentD3DSurface[i]);
		REF_PTR_RELEASE(cursorTextures[cursor][i]);
	}

	return TRUE;
}

/**Load the textures required to display the selected cursor*/
Bool W3DMouse::loadD3DCursorTextures(MouseCursor cursor)
{
	if (cursor == NONE || cursorTextures[cursor][0])
		return TRUE;	//no texture for this cursor or texture already loaded

	WW3DAssetManager *am=WW3DAssetManager::Get_Instance();
	Int animFrames=m_cursorInfo[cursor].numFrames;

	if (!animFrames)
		return FALSE;	//no animation frames defined.

	const char *baseName=m_cursorInfo[cursor].textureName.str();
	char FrameName[64];

	//Clamp to reasonable number
	if (animFrames > MAX_2D_CURSOR_ANIM_FRAMES)
		animFrames = MAX_2D_CURSOR_ANIM_FRAMES;

	m_currentFrames=0;

	if (animFrames == 1)
	{	//single animation frame without trailing numbers
		sprintf(FrameName,"%s.tga",baseName);
		cursorTextures[cursor][0]=	am->Get_Texture(FrameName);
		m_currentD3DSurface[0]=cursorTextures[cursor][0]->Get_Surface_Level();
		m_currentFrames = 1;
	}
	else
	for (Int i=0; i<animFrames; i++)
	{
		sprintf(FrameName,"%s%04d.tga",baseName,i);
		if ((cursorTextures[cursor][i]=am->Get_Texture(FrameName)) != NULL)
		{	m_currentD3DSurface[m_currentFrames]=cursorTextures[cursor][i]->Get_Surface_Level();
			m_currentFrames++;
		}
	}
	return TRUE;
}

void W3DMouse::initD3DAssets(void)
{
	//Nothing to do here unless we want to preload all possible cursors which would
	//probably not be practical for memory reasons.

	CriticalSectionClass::LockClass m(mutex);

	//don't allow the mouse thread to initialize
	//wait for main app to do initialization.
	if (isThread)
		return;

	WW3DAssetManager *am=WW3DAssetManager::Get_Instance();

	//Check if texture assets already loaded
	if (m_currentRedrawMode == RM_DX8 && cursorTextures[1] == NULL && am)
	{
		for (Int i=0; i<NUM_MOUSE_CURSORS; i++)
		{
			for (Int j=0; j < MAX_2D_CURSOR_ANIM_FRAMES; j++)
			{
				cursorTextures[i][j]=NULL;//am->Get_Texture(m_cursorInfo[i].textureName.str());
			}
		}

		for (Int x = 0; x < MAX_2D_CURSOR_ANIM_FRAMES; x++)
			m_currentD3DSurface[x]=NULL;
	}
}

void W3DMouse::freeD3DAssets(void)
{
	//free pointers to texture surfaces.
	Int i=0;
	for (; i<MAX_2D_CURSOR_ANIM_FRAMES; i++)
		REF_PTR_RELEASE(m_currentD3DSurface[i]);

	//free textures.
	for (i=0; i<NUM_MOUSE_CURSORS; i++)
	{
		for (Int j=0; j<MAX_2D_CURSOR_ANIM_FRAMES; j++)
			REF_PTR_RELEASE(cursorTextures[i][j]);
	}

}

void W3DMouse::initW3DAssets(void)
{
	CriticalSectionClass::LockClass m(mutex);

	//don't allow the mouse thread to initialize
	//wait for main app to do initialization.
	if (isThread)
		return;

	//Check if model assets already loaded
	if ((cursorModels[1] == NULL && W3DDisplay::m_assetManager))
	{
		for (Int i=1; i<NUM_MOUSE_CURSORS; i++)
		{
			if (!m_cursorInfo[i].W3DModelName.isEmpty())
			{
				if (m_orthoCamera)
					cursorModels[i] = W3DDisplay::m_assetManager->Create_Render_Obj(m_cursorInfo[i].W3DModelName.str(), m_cursorInfo[i].W3DScale*m_orthoZoom, 0);
				else
					cursorModels[i] = W3DDisplay::m_assetManager->Create_Render_Obj(m_cursorInfo[i].W3DModelName.str(), m_cursorInfo[i].W3DScale, 0);
				if (cursorModels[i])
				{
					cursorModels[i]->Set_Position(Vector3(0.0f, 0.0f, -1.0f));
					//W3DDisplay::m_3DInterfaceScene->Add_Render_Object(cursorModels[i]);
				}
			}
		}
	}
	if ((cursorAnims[1] == NULL && W3DDisplay::m_assetManager))
	{
		for (Int i=1; i<NUM_MOUSE_CURSORS; i++)
		{
			if (!m_cursorInfo[i].W3DAnimName.isEmpty())
			{
				DEBUG_ASSERTCRASH(cursorAnims[i] == NULL, ("hmm, leak festival"));
				cursorAnims[i] = W3DDisplay::m_assetManager->Get_HAnim(m_cursorInfo[i].W3DAnimName.str());
				if (cursorAnims[i] && cursorModels[i])
				{
					cursorModels[i]->Set_Animation(cursorAnims[i], 0, (m_cursorInfo[i].loop) ? RenderObjClass::ANIM_MODE_LOOP : RenderObjClass::ANIM_MODE_ONCE);
				}
			}
		}
	}

	// create the camera
	m_camera = NEW_REF( CameraClass, () );
	m_camera->Set_Position( Vector3( 0, 1, 1 ) );
	Vector2 min = Vector2( -1, -1 );
	Vector2 max = Vector2( +1, +1 );
	m_camera->Set_View_Plane( min, max );
	m_camera->Set_Clip_Planes( 0.995f, 20.0f );
	if (m_orthoCamera)
		m_camera->Set_Projection_Type( CameraClass::ORTHO );
}

void W3DMouse::freeW3DAssets(void)
{

	for (Int i=0; i<NUM_MOUSE_CURSORS; i++)
	{
		if (W3DDisplay::m_3DInterfaceScene && cursorModels[i])
		{
			W3DDisplay::m_3DInterfaceScene->Remove_Render_Object(cursorModels[i]);
		}
		REF_PTR_RELEASE(cursorModels[i]);
		REF_PTR_RELEASE(cursorAnims[i]);
	}

	REF_PTR_RELEASE(m_camera);
}

//-------------------------------------------------------------------------------------------------
/** Initialize our device */
//-------------------------------------------------------------------------------------------------
void W3DMouse::init( void )
{

	//check if system already initialized and texture assets loaded.
	Win32Mouse::init();
	setCursor(ARROW);	//set default starting cursor image

	WWASSERT(!thread.Is_Running());


	isThread=FALSE;
	if (m_currentRedrawMode == RM_DX8)
		thread.Execute();
	thread.Set_Priority(0);

}

//-------------------------------------------------------------------------------------------------
/** Reset */
//-------------------------------------------------------------------------------------------------
void W3DMouse::reset( void )
{

	// extend
	Win32Mouse::reset();

}

//-------------------------------------------------------------------------------------------------
/** Super basic simplistic cursor */
//-------------------------------------------------------------------------------------------------
void W3DMouse::setCursor( MouseCursor cursor )
{
	// Non-Windows stub
	m_currentCursor = cursor;
}

extern HWND ApplicationHWnd;

void W3DMouse::draw(void)
{
	// Non-Windows stub
	m_drawing = FALSE;
}

void W3DMouse::setRedrawMode(RedrawMode mode)
{
	MouseCursor cursor = getMouseCursor();

	//Turn off the previous cursor mode
	setCursor(NONE);

	m_currentRedrawMode=mode;

	switch (mode)
	{
		case RM_WINDOWS:
		{	//Windows mouse doesn't need an update thread.
			if (thread.Is_Running())
				thread.Stop();
			freeD3DAssets();	//using Windows resources
			freeW3DAssets();
			freePolygonAssets();
			m_currentD3DCursor = NONE;
			m_currentW3DCursor = NONE;
			m_currentPolygonCursor = NONE;
		}
		break;

		case RM_W3D:
		{	//Model mouse updated only at render time so doesn't
			//require thread.
			if (thread.Is_Running())
				thread.Stop();
			freeD3DAssets();	//using packed Image data, not textures.
			freePolygonAssets();
			m_currentD3DCursor = NONE;
			m_currentPolygonCursor = NONE;
			initW3DAssets();
		}
		break;

		case RM_POLYGON:
		{	//Polygon mouse updated only at render time so doesn't
			//require thread.
			if (thread.Is_Running())
				thread.Stop();
			freeD3DAssets();	//using packed Image data, not textures.
			freeW3DAssets();
			m_currentD3DCursor = NONE;
			m_currentW3DCursor = NONE;
			m_currentPolygonCursor = NONE;
			initPolygonAssets();
		}
		break;

		case RM_DX8:
		{	//this cursor type is drawn by DX8 and can be refreshed
			//independent of rendering rate.  Uses another thread to do
			//position updates.
			initD3DAssets();	//make sure textures loaded.
			freeW3DAssets();
			freePolygonAssets();
			if (!thread.Is_Running())
				thread.Execute();
			m_currentW3DCursor = NONE;
			m_currentPolygonCursor = NONE;
			break;
		}
	}

	setCursor(NONE);
	setCursor(cursor);
}

void W3DMouse::setCursorDirection(MouseCursor cursor)
{
	Coord2D offset = {0, 0};
	//Check if we have a directional cursor that needs different images for each direction
	if (m_cursorInfo[cursor].numDirections > 1 && TheInGameUI && TheInGameUI->isScrolling())
	{
		offset = TheInGameUI->getScrollAmount();
		if (offset.x || offset.y)
		{
			offset.normalize();
			Real theta = atan2(offset.y, offset.x);
			theta = fmod(theta+M_PI*2,M_PI*2);
			Int numDirections=m_cursorInfo[m_currentCursor].numDirections;
			//Figure out which of our predrawn cursor orientations best matches the
			//actual cursor direction.  Frame 0 is assumed to point right and continue
			//clockwise.
			m_directionFrame=(Int)(theta/(2.0f*M_PI/(Real)numDirections)+0.5f);
			if (m_directionFrame >= numDirections)
				m_directionFrame = 0;
		}
		else
		{
			m_directionFrame=0;
		}
	}
	else
		m_directionFrame = 0;
}
