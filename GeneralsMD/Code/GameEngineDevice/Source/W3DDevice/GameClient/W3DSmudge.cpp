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

// W3DSmudge.cpp ////////////////////////////////////////////////////////////////////////////////
// Smudge System implementation
// Author: Mark Wilczynski, June 2003
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Lib/BaseType.h"
#include "always.h"
#include "W3DDevice/GameClient/W3DSmudge.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "Common/GameMemory.h"
#include "GameClient/View.h"
#include "GameClient/Display.h"
#include "WW3D2/texture.h"
// #include "WW3D2/dx8indexbuffer.h" // Phase 39.4: Removed with DirectX 8 cleanup
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/rinfo.h"
#include "WW3D2/camera.h"
#include "WW3D2/sortingrenderer.h"


SmudgeManager *TheSmudgeManager=NULL;

W3DSmudgeManager::W3DSmudgeManager(void)
{
}

W3DSmudgeManager::~W3DSmudgeManager()
{
	ReleaseResources();
}

void W3DSmudgeManager::init(void)
{
	SmudgeManager::init();
	ReAcquireResources();
}

void W3DSmudgeManager::reset (void)
{
	SmudgeManager::reset();	//base
}

void W3DSmudgeManager::ReleaseResources(void)
{
#ifdef USE_COPY_RECTS
	REF_PTR_RELEASE(m_backgroundTexture);
#endif
	REF_PTR_RELEASE(m_indexBuffer);
}

//Make sure (SMUDGE_DRAW_SIZE * 12) < 65535 because that's the max index buffer size.
#define SMUDGE_DRAW_SIZE	500	//draw at most 50 smudges per call. Tweak value to improve CPU/GPU parallelism.

void W3DSmudgeManager::ReAcquireResources(void)
{
	ReleaseResources();

	SurfaceClass *surface=DX8Wrapper::_Get_DX8_Back_Buffer();
	SurfaceClass::SurfaceDescription surface_desc;

	surface->Get_Description(surface_desc);
	REF_PTR_RELEASE(surface);

	#ifdef USE_COPY_RECTS
	m_backgroundTexture = MSGNEW("TextureClass") TextureClass(TheTacticalView->getWidth(),TheTacticalView->getHeight(),surface_desc.Format,MIP_LEVELS_1,TextureClass::POOL_DEFAULT, true);
	#endif

	m_backBufferWidth = surface_desc.Width;
	m_backBufferHeight = surface_desc.Height;

	m_indexBuffer=NEW_REF(DX8IndexBufferClass,(SMUDGE_DRAW_SIZE*4*3));	//allocate 4 triangles per smudge, each with 3 indices.

	// Fill up the IB with static vertex indices that will be used for all smudges.
	{
		DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexBuffer);
		UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
		//quad of 4 triangles:
		//	0-----3
		//  |\   /|
		//  |  4  |
		//	|/   \|
		//  1-----2
		Int vbCount=0;
		for (Int i=0; i<SMUDGE_DRAW_SIZE; i++)
		{
			//Top
			ib[0]=vbCount;
			ib[1]=vbCount+4;
			ib[2]=vbCount+3;
			//Right
			ib[3]=vbCount+3;
			ib[4]=vbCount+4;
			ib[5]=vbCount+2;
			//Bottom
			ib[6]=vbCount+2;
			ib[7]=vbCount+4;
			ib[8]=vbCount+1;
			//Left
			ib[9]=vbCount+1;
			ib[10]=vbCount+4;
			ib[11]=vbCount+0;

			vbCount += 5;
			ib+=12;
		}
	}
}

/*Copies a portion of the current render target into a specified buffer*/
Int copyRect(unsigned char *buf, Int bufSize, int oX, int oY, int width, int height)
{
	// Non-Windows: Cannot copy render target (DirectX-only feature)
	return 0;
}

#define UNIQUE_COLOR	(0x12345678)
#define BLOCK_SIZE	(8)

Bool W3DSmudgeManager::testHardwareSupport(void)
{
	// Non-Windows: Smudge hardware support not available
	m_hardwareSupportStatus = SMUDGE_SUPPORT_NO;
	return FALSE;
}

void W3DSmudgeManager::render(RenderInfoClass &rinfo)
{
#ifdef USE_COPY_RECTS
#else
#endif
#ifdef USE_COPY_RECTS
#endif
#ifdef USE_COPY_RECTS
#endif
#ifdef USE_COPY_RECTS
#else
#endif
	// Non-Windows: Smudge rendering not available (DirectX-only feature)
}
