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

//----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:   Generals
//
// Module:    Video
//
// File name: W3DDevice/GameClient/W3DVideoBuffer.cpp
//
// Created:   10/23/01 TR
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes
//----------------------------------------------------------------------------

#include "Common/GameMemory.h"
#include "WW3D2/texture.h"
#include "WW3D2/textureloader.h"
#include "W3DDevice/GameClient/W3DVideoBuffer.h"

#ifndef _WIN32
// Phase 34: OpenGL support for video buffer upload
#include <glad/glad.h>
#endif

//----------------------------------------------------------------------------
//         Externals
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Types
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Data
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Data
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Prototypes
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Functions
//----------------------------------------------------------------------------


//============================================================================
// W3DVideoBuffer::W3DVideoBuffer
//============================================================================

W3DVideoBuffer::W3DVideoBuffer( VideoBuffer::Type format )
: VideoBuffer(format),
	m_texture(NULL),
	m_surface(NULL)
#ifndef _WIN32
	,m_lockedBuffer(NULL)
#endif
{

}


//============================================================================
// W3DVideoBuffer::SetBuffer
//============================================================================

Bool W3DVideoBuffer::allocate( UnsignedInt width, UnsignedInt height )
{
	printf("DEBUG: W3DVideoBuffer::allocate() - ENTRY (width=%d, height=%d)\n", width, height);
	fflush(stdout);
	
	free();

	m_width = width;
	m_height = height;
	m_textureWidth = width;;
	m_textureHeight = height;;
	unsigned int temp_depth=1;
	TextureLoader::Validate_Texture_Size( m_textureWidth, m_textureHeight, temp_depth);
	
	printf("DEBUG: allocate() - After Validate_Texture_Size: %dx%d\n", m_textureWidth, m_textureHeight);
	fflush(stdout);

	WW3DFormat w3dFormat = TypeToW3DFormat(  m_format );
	printf("DEBUG: allocate() - w3dFormat=%d (UNKNOWN=%d)\n", (int)w3dFormat, (int)WW3D_FORMAT_UNKNOWN);
	fflush(stdout);

	if ( w3dFormat == WW3D_FORMAT_UNKNOWN )
	{
		printf("ERROR: allocate() - w3dFormat is UNKNOWN!\n");
		fflush(stdout);
		return NULL;
	}

	m_texture  = MSGNEW("TextureClass") TextureClass ( m_textureWidth, m_textureHeight, w3dFormat, MIP_LEVELS_1 );
	printf("DEBUG: allocate() - TextureClass created: %p\n", m_texture);
	fflush(stdout);

	if ( m_texture == NULL )
	{
		printf("ERROR: allocate() - TextureClass creation FAILED!\n");
		fflush(stdout);
		return FALSE;
	}

#ifndef _WIN32
	// Phase 34: Metal/OpenGL backend path
	extern bool g_useMetalBackend;
	
	if (g_useMetalBackend) {
        printf("DEBUG: allocate() - Using Metal/OpenGL backend, skipping DirectX lock\n");
        // Calculate pitch based on width and format (assume RGBA8 = 4 bytes per pixel)
        m_pitch = width * 4;  // RGBA8 format
        size_t buffer_size = m_pitch * height;
        m_lockedBuffer = new uint8_t[buffer_size];
        memset(m_lockedBuffer, 0, buffer_size);  // Initialize to black
        printf("DEBUG: allocate() - Internal buffer allocated (%zu bytes, pitch=%d, %dx%d)\n", buffer_size, m_pitch, width, height);
        return TRUE;
    }
#endif
	
	printf("DEBUG: allocate() - About to call lock()\n");
	fflush(stdout);
	
	if ( lock() == NULL )
	{
		printf("ERROR: allocate() - lock() returned NULL!\n");
		fflush(stdout);
		free();
		return FALSE;
	}
	
	printf("DEBUG: allocate() - lock() succeeded\n");
	fflush(stdout);

	unlock();
	
	printf("DEBUG: allocate() - SUCCESS - returning TRUE\n");
	fflush(stdout);

	return TRUE;
}

//============================================================================
// W3DVideoBuffer::~W3DVideoBuffer
//============================================================================

W3DVideoBuffer::~W3DVideoBuffer()
{
	free();
}

//============================================================================
// W3DVideoBuffer::lock
//============================================================================

void*		W3DVideoBuffer::lock( void )
{
	printf("DEBUG: W3DVideoBuffer::lock() - ENTRY\n");
	fflush(stdout);
	
#ifndef _WIN32
	// Phase 34: Metal/OpenGL path - return internal buffer
	extern bool g_useMetalBackend;
	
	if (g_useMetalBackend) {
		if (m_lockedBuffer != NULL) {
			printf("DEBUG: lock() - Returning Metal/OpenGL internal buffer: %p\n", m_lockedBuffer);
			fflush(stdout);
			return m_lockedBuffer;
		} else {
			printf("ERROR: lock() - Metal/OpenGL buffer not allocated!\n");
			fflush(stdout);
			return NULL;
		}
	}
#endif
	
	void *mem = NULL;

	if ( m_surface != NULL )
	{
		printf("DEBUG: lock() - m_surface was NOT NULL, calling unlock()\n");
		fflush(stdout);
		unlock();
	}

	m_surface = m_texture->Get_Surface_Level();
	printf("DEBUG: lock() - Get_Surface_Level() returned: %p\n", m_surface);
	fflush(stdout);

	if ( m_surface )
	{
		mem = m_surface->Lock( (Int*) &m_pitch );
		printf("DEBUG: lock() - Lock() returned: %p, pitch=%d\n", mem, m_pitch);
		fflush(stdout);
	}
	else
	{
		printf("ERROR: lock() - Get_Surface_Level() returned NULL!\n");
		fflush(stdout);
	}

	return mem;
}

//============================================================================
// W3DVideoBuffer::unlock
//============================================================================

void		W3DVideoBuffer::unlock( void )
{
#ifndef _WIN32
	// Phase 34: Metal/OpenGL path - upload buffer to texture
	extern bool g_useMetalBackend;
	
	if (g_useMetalBackend) {
		printf("DEBUG: unlock() - Metal backend path (buffer=%p, texture=%p, pitch=%d)\n", 
		       m_lockedBuffer, m_texture, m_pitch);
		fflush(stdout);
		
		if (m_lockedBuffer == NULL) {
			printf("ERROR: unlock() - m_lockedBuffer is NULL!\n");
			fflush(stdout);
			return;
		}
		
		if (m_texture == NULL) {
			printf("ERROR: unlock() - m_texture is NULL!\n");
			fflush(stdout);
			return;
		}
		
		if (m_pitch == 0) {
			printf("ERROR: unlock() - m_pitch is 0!\n");
			fflush(stdout);
			return;
		}
		
		// CRITICAL: Use Metal texture upload, not OpenGL!
		// Get Metal texture handle from TextureClass
		void* metal_texture = m_texture->Get_Metal_Texture();
		printf("DEBUG: unlock() - Metal texture handle: %p\n", metal_texture);
		fflush(stdout);
		
		if (metal_texture == nullptr) {
			printf("ERROR: unlock() - Metal texture is NULL! Cannot upload video frame.\n");
			fflush(stdout);
			return;
		}
		
		// Upload pixel data directly to Metal texture
		// MetalWrapper should provide a function to update texture data
		printf("DEBUG: unlock() - Uploading %dx%d pixels (%d bytes) to Metal texture\n",
		       m_textureWidth, m_textureHeight, m_textureWidth * m_textureHeight * 4);
		fflush(stdout);
		
		// TODO Phase 34.2: Implement MetalWrapper::UpdateTextureData()
		// For now, just log that we would upload here
		printf("WARNING: unlock() - Metal texture upload not yet implemented! Video frame not uploaded.\n");
		fflush(stdout);
		
		return;
	}
	
	// OpenGL backend path (when USE_OPENGL=1)
	// TODO: Implement OpenGL texture upload if needed
	printf("DEBUG: unlock() - OpenGL backend not yet implemented for video textures\n");
	fflush(stdout);
#endif
	
	if ( m_surface != NULL )
	{
		printf("DEBUG: unlock() - About to call m_surface->Unlock() (m_surface=%p)\n", m_surface);
		fflush(stdout);
		
		// Check vtable validity before calling virtual method
		void* vtable_ptr = *(void**)m_surface;
		printf("DEBUG: unlock() - m_surface vtable ptr=%p\n", vtable_ptr);
		fflush(stdout);
		
		m_surface->Unlock();
		printf("DEBUG: unlock() - m_surface->Unlock() completed\n");
		fflush(stdout);
		
		printf("DEBUG: unlock() - About to call m_surface->Release_Ref()\n");
		fflush(stdout);
		m_surface->Release_Ref();
		printf("DEBUG: unlock() - m_surface->Release_Ref() completed\n");
		fflush(stdout);
		
		m_surface = NULL;
	}
}

//============================================================================
// W3DVideoBuffer::valid
//============================================================================

Bool		W3DVideoBuffer::valid( void )
{
	return m_texture != NULL;
}

//============================================================================
// W3DVideoBuffer::reset
//============================================================================

void	W3DVideoBuffer::free( void )
{
	unlock();

	if ( m_texture )
	{
		unlock();
		m_texture->Release_Ref();
		m_texture = NULL;
	}
	m_surface = NULL;

#ifndef _WIN32
	// Phase 34: Free Metal/OpenGL internal buffer
	if (m_lockedBuffer != NULL) {
		delete[] m_lockedBuffer;
		m_lockedBuffer = NULL;
	}
#endif

	VideoBuffer::free();
}


//============================================================================
// W3DVideoBuffer::TypeToW3DFormat
//============================================================================

WW3DFormat W3DVideoBuffer::TypeToW3DFormat( VideoBuffer::Type format )
{
	WW3DFormat w3dFormat = WW3D_FORMAT_UNKNOWN;
	switch ( format )
	{
		case TYPE_X8R8G8B8:
			w3dFormat = WW3D_FORMAT_X8R8G8B8;
			break;

 		case TYPE_R8G8B8:
			w3dFormat = WW3D_FORMAT_R8G8B8;
			break;

 		case TYPE_R5G6B5:
			w3dFormat = WW3D_FORMAT_R5G6B5;
			break;

 		case TYPE_X1R5G5B5:
			w3dFormat = WW3D_FORMAT_X1R5G5B5;
			break;
	}

	return w3dFormat;
}

//============================================================================
// W3DFormatToType
//============================================================================

VideoBuffer::Type W3DVideoBuffer::W3DFormatToType( WW3DFormat w3dFormat )
{
	Type format = TYPE_UNKNOWN;
	switch ( w3dFormat )
	{
		case WW3D_FORMAT_X8R8G8B8:
				format = VideoBuffer::TYPE_X8R8G8B8;
				break;
		case WW3D_FORMAT_R8G8B8:
				format = VideoBuffer::TYPE_R8G8B8;
				break;
		case WW3D_FORMAT_R5G6B5:
				format = VideoBuffer::TYPE_R5G6B5;
				break;
		case WW3D_FORMAT_X1R5G5B5:
				format = VideoBuffer::TYPE_X1R5G5B5;
				break;
	}

	return format;
}
