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
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/dx8vertexbuffer.cpp                    $*
 *                                                                                             *
 *              Original Author:: Jani Penttinen                                               *
 *                                                                                             *
 *                      $Author:: Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/26/02 5:06p                                             $*
 *                                                                                             *
 *                    $Revision:: 39                                                          $*
 *                                                                                             *
 * 06/26/02 KM VB Vertex format size update for shaders                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//#define VERTEX_BUFFER_LOG

#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "dx8fvf.h"
#include "dx8caps.h"
#include "thread.h"
#include "wwmemlog.h"

#ifdef _WIN32
#include <d3dx8core.h>
#else
// Phase 27.2.1: OpenGL includes for vertex buffer implementation
#include <glad/glad.h>
#include <cstring>  // for memcpy
#endif

#define DEFAULT_VB_SIZE 5000

static bool _DynamicSortingVertexArrayInUse=false;
//static VertexFormatXYZNDUV2* _DynamicSortingVertexArray=NULL;
static SortingVertexBufferClass* _DynamicSortingVertexArray=NULL;
static unsigned short _DynamicSortingVertexArraySize=0;
static unsigned short _DynamicSortingVertexArrayOffset=0;

static bool _DynamicDX8VertexBufferInUse=false;
static DX8VertexBufferClass* _DynamicDX8VertexBuffer=NULL;
static unsigned short _DynamicDX8VertexBufferSize=DEFAULT_VB_SIZE;
static unsigned short _DynamicDX8VertexBufferOffset=0;

static const FVFInfoClass _DynamicFVFInfo(dynamic_fvf_type);

static int _DX8VertexBufferCount=0;

static int _VertexBufferCount;
static int _VertexBufferTotalVertices;
static int _VertexBufferTotalSize;

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

VertexBufferClass::VertexBufferClass(unsigned type_, unsigned FVF, unsigned short vertex_count_, unsigned vertex_size)
	:
	VertexCount(vertex_count_),
	type(type_),
	engine_refs(0)
{
	WWMEMLOG(MEM_RENDERER);
	WWASSERT(VertexCount);
	WWASSERT(type==BUFFER_TYPE_DX8 || type==BUFFER_TYPE_SORTING);
	WWASSERT((FVF!=0 && vertex_size==0) || (FVF==0 && vertex_size!=0));
	fvf_info=W3DNEW FVFInfoClass(FVF,vertex_size);

	_VertexBufferCount++;
	_VertexBufferTotalVertices+=VertexCount;
	_VertexBufferTotalSize+=VertexCount*fvf_info->Get_FVF_Size();
#ifdef VERTEX_BUFFER_LOG
	WWDEBUG_SAY(("New VB, %d vertices, size %d bytes",VertexCount,VertexCount*fvf_info->Get_FVF_Size()));
	WWDEBUG_SAY(("Total VB count: %d, total %d vertices, total size %d bytes",
		_VertexBufferCount,
		_VertexBufferTotalVertices,
		_VertexBufferTotalSize));
#endif
}

// ----------------------------------------------------------------------------

VertexBufferClass::~VertexBufferClass()
{
	_VertexBufferCount--;
	_VertexBufferTotalVertices-=VertexCount;
	_VertexBufferTotalSize-=VertexCount*fvf_info->Get_FVF_Size();

#ifdef VERTEX_BUFFER_LOG
	WWDEBUG_SAY(("Delete VB, %d vertices, size %d bytes",VertexCount,VertexCount*fvf_info->Get_FVF_Size()));
	WWDEBUG_SAY(("Total VB count: %d, total %d vertices, total size %d bytes",
		_VertexBufferCount,
		_VertexBufferTotalVertices,
		_VertexBufferTotalSize));
#endif
	delete fvf_info;
}

unsigned VertexBufferClass::Get_Total_Buffer_Count()
{
	return _VertexBufferCount;
}

unsigned VertexBufferClass::Get_Total_Allocated_Vertices()
{
	return _VertexBufferTotalVertices;
}

unsigned VertexBufferClass::Get_Total_Allocated_Memory()
{
	return _VertexBufferTotalSize;
}


// ----------------------------------------------------------------------------

void VertexBufferClass::Add_Engine_Ref() const
{
	engine_refs++;
}

// ----------------------------------------------------------------------------

void VertexBufferClass::Release_Engine_Ref() const
{
	engine_refs--;
	WWASSERT(engine_refs>=0);
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

VertexBufferClass::WriteLockClass::WriteLockClass(VertexBufferClass* VertexBuffer, int flags)
	:
	VertexBufferLockClass(VertexBuffer)
{
	DX8_THREAD_ASSERT();
	WWASSERT(VertexBuffer);
	WWASSERT(!VertexBuffer->Engine_Refs());
	VertexBuffer->Add_Ref();
	switch (VertexBuffer->Type()) {
	case BUFFER_TYPE_DX8:
#ifdef _WIN32
#ifdef VERTEX_BUFFER_LOG
		{
		StringClass fvf_name;
		VertexBuffer->FVF_Info().Get_FVF_Name(fvf_name);
		WWDEBUG_SAY(("VertexBuffer->Lock(start_index: 0, index_range: 0(%d), fvf_size: %d, fvf: %s)",
			VertexBuffer->Get_Vertex_Count(),
			VertexBuffer->FVF_Info().Get_FVF_Size(),
			fvf_name));
		}
#endif
		DX8_Assert();
		DX8_ErrorCode(static_cast<DX8VertexBufferClass*>(VertexBuffer)->Get_DX8_Vertex_Buffer()->Lock(
			0,
			0,
			(void**)&Vertices,
			flags));	//flags
#else
		// Phase 27.2.1: OpenGL vertex buffer lock (CPU-side emulation)
		{
			DX8VertexBufferClass* vb = static_cast<DX8VertexBufferClass*>(VertexBuffer);
			Vertices = vb->GLVertexData;
			WWASSERT(Vertices != NULL);
#ifdef VERTEX_BUFFER_LOG
			StringClass fvf_name;
			VertexBuffer->FVF_Info().Get_FVF_Name(fvf_name);
			printf("Phase 27.2.1: GL VBO Lock - VBO id=%u, size=%u bytes, fvf=%s\n",
				vb->Get_GL_Vertex_Buffer(),
				VertexBuffer->Get_Vertex_Count() * VertexBuffer->FVF_Info().Get_FVF_Size(),
				fvf_name.Peek_Buffer());
#endif
		}
#endif
		break;
	case BUFFER_TYPE_SORTING:
		Vertices=static_cast<SortingVertexBufferClass*>(VertexBuffer)->VertexBuffer;
		break;
	default:
		WWASSERT(0);
		break;
	}
}

// ----------------------------------------------------------------------------

VertexBufferClass::WriteLockClass::~WriteLockClass()
{
	DX8_THREAD_ASSERT();
	switch (VertexBuffer->Type()) {
	case BUFFER_TYPE_DX8:
#ifdef _WIN32
#ifdef VERTEX_BUFFER_LOG
		WWDEBUG_SAY(("VertexBuffer->Unlock()"));
#endif
		DX8_Assert();
		DX8_ErrorCode(static_cast<DX8VertexBufferClass*>(VertexBuffer)->Get_DX8_Vertex_Buffer()->Unlock());
#else
		// Phase 27.2.1: OpenGL vertex buffer unlock (upload CPU data to GPU)
		// Phase 29.3: Skip OpenGL calls when Metal active (CPU buffer already updated)
		if (!g_useMetalBackend) {
			DX8VertexBufferClass* vb = static_cast<DX8VertexBufferClass*>(VertexBuffer);
			unsigned buffer_size = VertexBuffer->Get_Vertex_Count() * VertexBuffer->FVF_Info().Get_FVF_Size();
			
			glBindBuffer(GL_ARRAY_BUFFER, vb->Get_GL_Vertex_Buffer());
			glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size, vb->GLVertexData);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			
#ifdef VERTEX_BUFFER_LOG
			printf("Phase 27.2.1: GL VBO Unlock - Uploaded %u bytes to VBO id=%u\n",
				buffer_size, vb->Get_GL_Vertex_Buffer());
#endif
			
			// Check for errors
			GLenum error = glGetError();
			if (error != GL_NO_ERROR) {
				printf("ERROR Phase 27.2.1: GL VBO Unlock failed with error 0x%X\n", error);
			}
		}
		// Metal: CPU-side buffer already updated, no GPU upload needed in Phase 29.3
#endif
		break;
	case BUFFER_TYPE_SORTING:
		break;
	default:
		WWASSERT(0);
		break;
	}
	VertexBuffer->Release_Ref();
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

VertexBufferClass::AppendLockClass::AppendLockClass(VertexBufferClass* VertexBuffer,unsigned start_index, unsigned index_range)
	:
	VertexBufferLockClass(VertexBuffer)
{
	DX8_THREAD_ASSERT();
	WWASSERT(VertexBuffer);
	WWASSERT(!VertexBuffer->Engine_Refs());
	WWASSERT(start_index+index_range<=VertexBuffer->Get_Vertex_Count());
	VertexBuffer->Add_Ref();
	switch (VertexBuffer->Type()) {
	case BUFFER_TYPE_DX8:
#ifdef _WIN32
#ifdef VERTEX_BUFFER_LOG
		{
		StringClass fvf_name;
		VertexBuffer->FVF_Info().Get_FVF_Name(fvf_name);
		WWDEBUG_SAY(("VertexBuffer->Lock(start_index: %d, index_range: %d, fvf_size: %d, fvf: %s)",
			start_index,
			index_range,
			VertexBuffer->FVF_Info().Get_FVF_Size(),
			fvf_name));
		}
#endif
		DX8_Assert();
		DX8_ErrorCode(static_cast<DX8VertexBufferClass*>(VertexBuffer)->Get_DX8_Vertex_Buffer()->Lock(
			start_index*VertexBuffer->FVF_Info().Get_FVF_Size(),
			index_range*VertexBuffer->FVF_Info().Get_FVF_Size(),
			(void**)&Vertices,
			0));	// Default (no) flags
#else
		// Phase 27.2.1: OpenGL vertex buffer append lock (partial buffer access)
		{
			DX8VertexBufferClass* vb = static_cast<DX8VertexBufferClass*>(VertexBuffer);
			unsigned fvf_size = VertexBuffer->FVF_Info().Get_FVF_Size();
			unsigned char* base_ptr = static_cast<unsigned char*>(vb->GLVertexData);
			Vertices = base_ptr + (start_index * fvf_size);
			WWASSERT(Vertices != NULL);
#ifdef VERTEX_BUFFER_LOG
			StringClass fvf_name;
			VertexBuffer->FVF_Info().Get_FVF_Name(fvf_name);
			printf("Phase 27.2.1: GL VBO AppendLock - VBO id=%u, start=%u, range=%u, fvf_size=%u, fvf=%s\n",
				vb->Get_GL_Vertex_Buffer(),
				start_index,
				index_range,
				fvf_size,
				fvf_name.Peek_Buffer());
#endif
		}
#endif
		break;
	case BUFFER_TYPE_SORTING:
		Vertices=static_cast<SortingVertexBufferClass*>(VertexBuffer)->VertexBuffer+start_index;
		break;
	default:
		WWASSERT(0);
		break;
	}
}

// ----------------------------------------------------------------------------

VertexBufferClass::AppendLockClass::~AppendLockClass()
{
	DX8_THREAD_ASSERT();
	switch (VertexBuffer->Type()) {
	case BUFFER_TYPE_DX8:
#ifdef _WIN32
		DX8_Assert();
#ifdef VERTEX_BUFFER_LOG
		WWDEBUG_SAY(("VertexBuffer->Unlock()"));
#endif
		DX8_ErrorCode(static_cast<DX8VertexBufferClass*>(VertexBuffer)->Get_DX8_Vertex_Buffer()->Unlock());
#else
		// Phase 27.2.1: OpenGL vertex buffer append unlock
		// Phase 29.3: Skip OpenGL calls when Metal active (CPU buffer already updated)
		if (!g_useMetalBackend) {
			DX8VertexBufferClass* vb = static_cast<DX8VertexBufferClass*>(VertexBuffer);
			unsigned buffer_size = VertexBuffer->Get_Vertex_Count() * VertexBuffer->FVF_Info().Get_FVF_Size();
			
			glBindBuffer(GL_ARRAY_BUFFER, vb->Get_GL_Vertex_Buffer());
			glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size, vb->GLVertexData);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			
#ifdef VERTEX_BUFFER_LOG
			printf("Phase 27.2.1: GL VBO AppendUnlock - Uploaded %u bytes to VBO id=%u\n",
				buffer_size, vb->Get_GL_Vertex_Buffer());
#endif
			
			// Check for errors
			GLenum error = glGetError();
			if (error != GL_NO_ERROR) {
				printf("ERROR Phase 27.2.1: GL VBO AppendUnlock failed with error 0x%X\n", error);
			}
		}
		// Metal: CPU-side buffer already updated, no GPU upload needed in Phase 29.3
#endif
		break;
	case BUFFER_TYPE_SORTING:
		break;
	default:
		WWASSERT(0);
		break;
	}
	VertexBuffer->Release_Ref();
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

SortingVertexBufferClass::SortingVertexBufferClass(unsigned short VertexCount)
	:
	VertexBufferClass(BUFFER_TYPE_SORTING, dynamic_fvf_type, VertexCount)
{
	WWMEMLOG(MEM_RENDERER);
	VertexBuffer=W3DNEWARRAY VertexFormatXYZNDUV2[VertexCount];
}

// ----------------------------------------------------------------------------

SortingVertexBufferClass::~SortingVertexBufferClass()
{
	delete[] VertexBuffer;
}


// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

//	bool dynamic=false,bool softwarevp=false);

DX8VertexBufferClass::DX8VertexBufferClass(unsigned FVF, unsigned short vertex_count_, UsageType usage, unsigned vertex_size)
	:
	VertexBufferClass(BUFFER_TYPE_DX8, FVF, vertex_count_, vertex_size)
#ifdef _WIN32
	, VertexBuffer(NULL)
#else
	// Phase 27.2.1: Initialize OpenGL members
	, GLVertexBuffer(0)
	, GLVertexData(NULL)
	// Phase 30.1: Initialize Metal members
	#if defined(__APPLE__)
	, MetalVertexBuffer(nullptr)
	, MetalVertexData(NULL)
	#endif
#endif
{
	Create_Vertex_Buffer(usage);
}

// ----------------------------------------------------------------------------

DX8VertexBufferClass::DX8VertexBufferClass(
	const Vector3* vertices,
	const Vector3* normals,
	const Vector2* tex_coords,
	unsigned short VertexCount,
	UsageType usage)
	:
	VertexBufferClass(BUFFER_TYPE_DX8, D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_NORMAL, VertexCount)
#ifdef _WIN32
	, VertexBuffer(NULL)
#else
	, GLVertexBuffer(0), GLVertexData(NULL)
	#if defined(__APPLE__)
	, MetalVertexBuffer(nullptr), MetalVertexData(NULL)
	#endif
#endif
{
	WWASSERT(vertices);
	WWASSERT(normals);
	WWASSERT(tex_coords);

	Create_Vertex_Buffer(usage);
	Copy(vertices,normals,tex_coords,0,VertexCount);
}

// ----------------------------------------------------------------------------

DX8VertexBufferClass::DX8VertexBufferClass(
	const Vector3* vertices,
	const Vector3* normals,
	const Vector4* diffuse,
	const Vector2* tex_coords,
	unsigned short VertexCount,
	UsageType usage)
	:
	VertexBufferClass(BUFFER_TYPE_DX8, D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_NORMAL|D3DFVF_DIFFUSE, VertexCount)
#ifdef _WIN32
	, VertexBuffer(NULL)
#else
	, GLVertexBuffer(0), GLVertexData(NULL)
	#if defined(__APPLE__)
	, MetalVertexBuffer(nullptr), MetalVertexData(NULL)
	#endif
#endif
{
	WWASSERT(vertices);
	WWASSERT(normals);
	WWASSERT(tex_coords);
	WWASSERT(diffuse);

	Create_Vertex_Buffer(usage);
	Copy(vertices,normals,tex_coords,diffuse,0,VertexCount);
}

// ----------------------------------------------------------------------------

DX8VertexBufferClass::DX8VertexBufferClass(
	const Vector3* vertices,
	const Vector4* diffuse,
	const Vector2* tex_coords,
	unsigned short VertexCount,
	UsageType usage)
	:
	VertexBufferClass(BUFFER_TYPE_DX8, D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_DIFFUSE, VertexCount)
#ifdef _WIN32
	, VertexBuffer(NULL)
#else
	, GLVertexBuffer(0), GLVertexData(NULL)
	#if defined(__APPLE__)
	, MetalVertexBuffer(nullptr), MetalVertexData(NULL)
	#endif
#endif
{
	WWASSERT(vertices);
	WWASSERT(tex_coords);
	WWASSERT(diffuse);

	Create_Vertex_Buffer(usage);
	Copy(vertices,tex_coords,diffuse,0,VertexCount);
}

// ----------------------------------------------------------------------------

DX8VertexBufferClass::DX8VertexBufferClass(
	const Vector3* vertices,
	const Vector2* tex_coords,
	unsigned short VertexCount,
	UsageType usage)
	:
	VertexBufferClass(BUFFER_TYPE_DX8, D3DFVF_XYZ|D3DFVF_TEX1, VertexCount)
#ifdef _WIN32
	, VertexBuffer(NULL)
#else
	, GLVertexBuffer(0), GLVertexData(NULL)
	#if defined(__APPLE__)
	, MetalVertexBuffer(nullptr), MetalVertexData(NULL)
	#endif
#endif
{
	WWASSERT(vertices);
	WWASSERT(tex_coords);

	Create_Vertex_Buffer(usage);
	Copy(vertices,tex_coords,0,VertexCount);
}

// ----------------------------------------------------------------------------

DX8VertexBufferClass::~DX8VertexBufferClass()
{
#ifdef VERTEX_BUFFER_LOG
	WWDEBUG_SAY(("VertexBuffer->Release()"));
	_DX8VertexBufferCount--;
	WWDEBUG_SAY(("Current vertex buffer count: %d",_DX8VertexBufferCount));
#endif

#ifdef _WIN32
	VertexBuffer->Release();
#else
	// Phase 27.2.1/29.3: OpenGL vertex buffer cleanup (skip if using Metal)
	if (!g_useMetalBackend && GLVertexBuffer != 0) {
		glDeleteBuffers(1, &GLVertexBuffer);
		GLVertexBuffer = 0;
	}
	if (GLVertexData != NULL) {
		free(GLVertexData);
		GLVertexData = NULL;
	}

#if defined(__APPLE__)
	// Phase 30.1: Metal vertex buffer cleanup
	if (MetalVertexBuffer != nullptr) {
		MetalVertexBuffer = nullptr;  // ARC handles deallocation
	}
	if (MetalVertexData != NULL) {
		free(MetalVertexData);
		MetalVertexData = NULL;
	}
#endif
#endif

#ifdef VERTEX_BUFFER_LOG
	printf("Phase 27.2.1: Destroyed OpenGL VBO\n");
	_DX8VertexBufferCount--;
	printf("Current OpenGL vertex buffer count: %d\n", _DX8VertexBufferCount);
#endif
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void DX8VertexBufferClass::Create_Vertex_Buffer(UsageType usage)
{
	DX8_THREAD_ASSERT();
	
#ifdef _WIN32
	WWASSERT(!VertexBuffer);

#ifdef VERTEX_BUFFER_LOG
	StringClass fvf_name;
	FVF_Info().Get_FVF_Name(fvf_name);
	WWDEBUG_SAY(("CreateVertexBuffer(fvfsize=%d, vertex_count=%d, D3DUSAGE_WRITEONLY|%s|%s, fvf: %s, %s)",
		FVF_Info().Get_FVF_Size(),
		VertexCount,
		(usage&USAGE_DYNAMIC) ? "D3DUSAGE_DYNAMIC" : "-",
		(usage&USAGE_SOFTWAREPROCESSING) ? "D3DUSAGE_SOFTWAREPROCESSING" : "-",
		fvf_name,
		(usage&USAGE_DYNAMIC) ? "D3DPOOL_DEFAULT" : "D3DPOOL_MANAGED"));
	_DX8VertexBufferCount++;
	WWDEBUG_SAY(("Current vertex buffer count: %d",_DX8VertexBufferCount));
#endif

	unsigned usage_flags=
		D3DUSAGE_WRITEONLY|
		((usage&USAGE_DYNAMIC) ? D3DUSAGE_DYNAMIC : 0)|
		((usage&USAGE_NPATCHES) ? D3DUSAGE_NPATCHES : 0)|
		((usage&USAGE_SOFTWAREPROCESSING) ? D3DUSAGE_SOFTWAREPROCESSING : 0);
	if (!DX8Wrapper::Get_Current_Caps()->Support_TnL()) {
		usage_flags|=D3DUSAGE_SOFTWAREPROCESSING;
	}

	// New Code
	if (!DX8Wrapper::Get_Current_Caps()->Support_TnL()) {
		usage_flags|=D3DUSAGE_SOFTWAREPROCESSING;
	}

	HRESULT ret=DX8Wrapper::_Get_D3D_Device8()->CreateVertexBuffer(
		FVF_Info().Get_FVF_Size()*VertexCount,
		usage_flags,
		FVF_Info().Get_FVF(),
		(usage&USAGE_DYNAMIC) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
		&VertexBuffer);
	if (SUCCEEDED(ret)) {
		return;
	}

	WWDEBUG_SAY(("Vertex buffer creation failed, trying to release assets..."));

	// Vertex buffer creation failed, so try releasing least used textures and flushing the mesh cache.

	// Free all textures that haven't been used in the last 5 seconds
	TextureClass::Invalidate_Old_Unused_Textures(5000);

	// Invalidate the mesh cache
	WW3D::_Invalidate_Mesh_Cache();

	//@todo: Find some way to invalidate the textures too
	ret = DX8Wrapper::_Get_D3D_Device8()->ResourceManagerDiscardBytes(0);

	// Try again...
	ret=DX8Wrapper::_Get_D3D_Device8()->CreateVertexBuffer(
		FVF_Info().Get_FVF_Size()*VertexCount,
		usage_flags,
		FVF_Info().Get_FVF(),
		(usage&USAGE_DYNAMIC) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
		&VertexBuffer);

	if (SUCCEEDED(ret)) {
		WWDEBUG_SAY(("...Vertex buffer creation succesful"));
	}

	// If it still fails it is fatal
	DX8_ErrorCode(ret);

	/* Old Code
	DX8CALL(CreateVertexBuffer(
		FVF_Info().Get_FVF_Size()*VertexCount,
		usage_flags,
		FVF_Info().Get_FVF(),
		(usage&USAGE_DYNAMIC) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
		&VertexBuffer));
	*/
#else
	// Phase 29.3: Metal/OpenGL backend detection (using global flag)
	if (g_useMetalBackend) {
		// Phase 29.3: Metal CPU-side stub (GPU buffers in Phase 30)
		unsigned buffer_size = FVF_Info().Get_FVF_Size() * VertexCount;
		printf("Phase 29.3: Creating Metal CPU-side vertex buffer (%d vertices, %u bytes)\n", 
			VertexCount, buffer_size);
		
		GLVertexData = malloc(buffer_size);
		
		if (!GLVertexData) {
			printf("Phase 29.3: FATAL - Failed to allocate vertex buffer (%u bytes)\n", buffer_size);
			GLVertexBuffer = 0;
			return;
		}
		
		memset(GLVertexData, 0, buffer_size);
		GLVertexBuffer = 1; // Fake handle for Metal stub
		
		printf("Phase 29.3: Metal vertex buffer created (CPU-side %u bytes)\n", buffer_size);
	} else {
		// Phase 27.2.1: OpenGL vertex buffer creation
		WWASSERT(GLVertexBuffer == 0);
		WWASSERT(GLVertexData == NULL);
		
		// Generate OpenGL Vertex Buffer Object
		glGenBuffers(1, &GLVertexBuffer);
		WWASSERT(GLVertexBuffer != 0);
		
		// Allocate CPU-side memory for lock/unlock emulation
		unsigned buffer_size = FVF_Info().Get_FVF_Size() * VertexCount;
		GLVertexData = malloc(buffer_size);
		WWASSERT(GLVertexData != NULL);
		
		// Bind and allocate GPU memory
		glBindBuffer(GL_ARRAY_BUFFER, GLVertexBuffer);
		
		// Map usage types to OpenGL hints
		GLenum gl_usage = GL_STATIC_DRAW;  // Default
		if (usage & USAGE_DYNAMIC) {
			gl_usage = GL_DYNAMIC_DRAW;
		}
		
		// Allocate buffer (NULL data for now, will be filled via Lock/Unlock)
		glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, gl_usage);
		
		// Unbind for safety
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		// Check for OpenGL errors
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			printf("ERROR Phase 27.2.1: OpenGL VBO creation failed with error 0x%X\n", error);
			WWASSERT(0);  // Fatal error
		}
		
#ifdef VERTEX_BUFFER_LOG
		StringClass fvf_name;
		FVF_Info().Get_FVF_Name(fvf_name);
		printf("Phase 27.2.1: Created OpenGL VBO id=%u, size=%u bytes, usage=%s, fvf=%s\n",
			GLVertexBuffer,
			buffer_size,
			(usage & USAGE_DYNAMIC) ? "DYNAMIC" : "STATIC",
			fvf_name.Peek_Buffer());
		_DX8VertexBufferCount++;
		printf("Current OpenGL vertex buffer count: %d\n", _DX8VertexBufferCount);
#endif
	}
#endif
}

// ----------------------------------------------------------------------------

void DX8VertexBufferClass::Copy(const Vector3* loc, const Vector3* norm, const Vector2* uv, unsigned first_vertex,unsigned count)
{
	WWASSERT(loc);
	WWASSERT(norm);
	WWASSERT(uv);
	WWASSERT(count<=VertexCount);
	WWASSERT(FVF_Info().Get_FVF()==DX8_FVF_XYZNUV1);

	if (first_vertex) {
		VertexBufferClass::AppendLockClass l(this,first_vertex,count);
		VertexFormatXYZNUV1* verts=(VertexFormatXYZNUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].nx=(*norm)[0];
			verts[v].ny=(*norm)[1];
			verts[v].nz=(*norm++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
		}
	}
	else {
		VertexBufferClass::WriteLockClass l(this);
		VertexFormatXYZNUV1* verts=(VertexFormatXYZNUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].nx=(*norm)[0];
			verts[v].ny=(*norm)[1];
			verts[v].nz=(*norm++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
		}
	}
}

// ----------------------------------------------------------------------------

void DX8VertexBufferClass::Copy(const Vector3* loc, unsigned first_vertex, unsigned count)
{
	WWASSERT(loc);
	WWASSERT(count<=VertexCount);
	WWASSERT(FVF_Info().Get_FVF()==DX8_FVF_XYZ);

	if (first_vertex) {
		VertexBufferClass::AppendLockClass l(this,first_vertex,count);
		VertexFormatXYZ* verts=(VertexFormatXYZ*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
		}
	}
	else {
		VertexBufferClass::WriteLockClass l(this);
		VertexFormatXYZ* verts=(VertexFormatXYZ*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
		}
	}

}

// ----------------------------------------------------------------------------

void DX8VertexBufferClass::Copy(const Vector3* loc, const Vector2* uv, unsigned first_vertex, unsigned count)
{
	WWASSERT(loc);
	WWASSERT(uv);
	WWASSERT(count<=VertexCount);
	WWASSERT(FVF_Info().Get_FVF()==DX8_FVF_XYZUV1);

	if (first_vertex) {
		VertexBufferClass::AppendLockClass l(this,first_vertex,count);
		VertexFormatXYZUV1* verts=(VertexFormatXYZUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
		}
	}
	else {
		VertexBufferClass::WriteLockClass l(this);
		VertexFormatXYZUV1* verts=(VertexFormatXYZUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
		}
	}
}

// ----------------------------------------------------------------------------

void DX8VertexBufferClass::Copy(const Vector3* loc, const Vector3* norm, unsigned first_vertex, unsigned count)
{
	WWASSERT(loc);
	WWASSERT(norm);
	WWASSERT(count<=VertexCount);
	WWASSERT(FVF_Info().Get_FVF()==DX8_FVF_XYZN);

	if (first_vertex) {
		VertexBufferClass::AppendLockClass l(this,first_vertex,count);
		VertexFormatXYZN* verts=(VertexFormatXYZN*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].nx=(*norm)[0];
			verts[v].ny=(*norm)[1];
			verts[v].nz=(*norm++)[2];
		}
	}
	else {
		VertexBufferClass::WriteLockClass l(this);
		VertexFormatXYZN* verts=(VertexFormatXYZN*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].nx=(*norm)[0];
			verts[v].ny=(*norm)[1];
			verts[v].nz=(*norm++)[2];
		}
	}
}

// ----------------------------------------------------------------------------

void DX8VertexBufferClass::Copy(const Vector3* loc, const Vector3* norm, const Vector2* uv, const Vector4* diffuse, unsigned first_vertex, unsigned count)
{
	WWASSERT(loc);
	WWASSERT(norm);
	WWASSERT(uv);
	WWASSERT(diffuse);
	WWASSERT(count<=VertexCount);
	WWASSERT(FVF_Info().Get_FVF()==DX8_FVF_XYZNDUV1);

	if (first_vertex) {
		VertexBufferClass::AppendLockClass l(this,first_vertex,count);
		VertexFormatXYZNDUV1* verts=(VertexFormatXYZNDUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].nx=(*norm)[0];
			verts[v].ny=(*norm)[1];
			verts[v].nz=(*norm++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
			verts[v].diffuse=DX8Wrapper::Convert_Color(diffuse[v]);
		}
	}
	else {
		VertexBufferClass::WriteLockClass l(this);
		VertexFormatXYZNDUV1* verts=(VertexFormatXYZNDUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].nx=(*norm)[0];
			verts[v].ny=(*norm)[1];
			verts[v].nz=(*norm++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
			verts[v].diffuse=DX8Wrapper::Convert_Color(diffuse[v]);
		}
	}
}

// ----------------------------------------------------------------------------

void DX8VertexBufferClass::Copy(const Vector3* loc, const Vector2* uv, const Vector4* diffuse, unsigned first_vertex, unsigned count)
{
	WWASSERT(loc);
	WWASSERT(uv);
	WWASSERT(diffuse);
	WWASSERT(count<=VertexCount);
	WWASSERT(FVF_Info().Get_FVF()==DX8_FVF_XYZDUV1);

	if (first_vertex) {
		VertexBufferClass::AppendLockClass l(this,first_vertex,count);
		VertexFormatXYZDUV1* verts=(VertexFormatXYZDUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
			verts[v].diffuse=DX8Wrapper::Convert_Color(diffuse[v]);
		}
	}
	else {
		VertexBufferClass::WriteLockClass l(this);
		VertexFormatXYZDUV1* verts=(VertexFormatXYZDUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
			verts[v].diffuse=DX8Wrapper::Convert_Color(diffuse[v]);
		}
	}
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

DynamicVBAccessClass::DynamicVBAccessClass(unsigned t,unsigned fvf,unsigned short vertex_count_)
	:
	Type(t),
	FVFInfo(_DynamicFVFInfo),
	VertexCount(vertex_count_),
	VertexBuffer(0)
{
	WWASSERT(fvf==dynamic_fvf_type);
	WWASSERT(Type==BUFFER_TYPE_DYNAMIC_DX8 || Type==BUFFER_TYPE_DYNAMIC_SORTING);

	if (Type==BUFFER_TYPE_DYNAMIC_DX8) {
		Allocate_DX8_Dynamic_Buffer();
	}
	else {
		Allocate_Sorting_Dynamic_Buffer();
	}
}

DynamicVBAccessClass::~DynamicVBAccessClass()
{
	if (Type==BUFFER_TYPE_DYNAMIC_DX8) {
		_DynamicDX8VertexBufferInUse=false;
		_DynamicDX8VertexBufferOffset+=(unsigned) VertexCount;
	}
	else {
		_DynamicSortingVertexArrayInUse=false;
		_DynamicSortingVertexArrayOffset+=VertexCount;
	}

	REF_PTR_RELEASE (VertexBuffer);
}

// ----------------------------------------------------------------------------

void DynamicVBAccessClass::_Deinit()
{
	WWASSERT ((_DynamicDX8VertexBuffer == NULL) || (_DynamicDX8VertexBuffer->Num_Refs() == 1));
	REF_PTR_RELEASE(_DynamicDX8VertexBuffer);
	_DynamicDX8VertexBufferInUse=false;
	_DynamicDX8VertexBufferSize=DEFAULT_VB_SIZE;
	_DynamicDX8VertexBufferOffset=0;

	WWASSERT ((_DynamicSortingVertexArray == NULL) || (_DynamicSortingVertexArray->Num_Refs() == 1));
	REF_PTR_RELEASE(_DynamicSortingVertexArray);
	WWASSERT(!_DynamicSortingVertexArrayInUse);
	_DynamicSortingVertexArrayInUse=false;
	_DynamicSortingVertexArraySize=0;
	_DynamicSortingVertexArrayOffset=0;
}

void DynamicVBAccessClass::Allocate_DX8_Dynamic_Buffer()
{
	WWMEMLOG(MEM_RENDERER);
	WWASSERT(!_DynamicDX8VertexBufferInUse);
	_DynamicDX8VertexBufferInUse=true;

	// If requesting more vertices than dynamic vertex buffer can fit, delete the vb
	// and adjust the size to the new count.
	if (VertexCount>_DynamicDX8VertexBufferSize) {
		REF_PTR_RELEASE(_DynamicDX8VertexBuffer);
		_DynamicDX8VertexBufferSize=VertexCount;
		if (_DynamicDX8VertexBufferSize<DEFAULT_VB_SIZE) _DynamicDX8VertexBufferSize=DEFAULT_VB_SIZE;
	}

	// Create a new vb if one doesn't exist currently
	if (!_DynamicDX8VertexBuffer) {
		unsigned usage=DX8VertexBufferClass::USAGE_DYNAMIC;
		if (DX8Wrapper::Get_Current_Caps()->Support_NPatches()) {
			usage|=DX8VertexBufferClass::USAGE_NPATCHES;
		}

		_DynamicDX8VertexBuffer=NEW_REF(DX8VertexBufferClass,(
			dynamic_fvf_type,
			_DynamicDX8VertexBufferSize,
			(DX8VertexBufferClass::UsageType)usage));
		_DynamicDX8VertexBufferOffset=0;
	}

	// Any room at the end of the buffer?
	if (((unsigned)VertexCount+_DynamicDX8VertexBufferOffset)>_DynamicDX8VertexBufferSize) {
		_DynamicDX8VertexBufferOffset=0;
	}

	REF_PTR_SET(VertexBuffer,_DynamicDX8VertexBuffer);
	VertexBufferOffset=_DynamicDX8VertexBufferOffset;
}

void DynamicVBAccessClass::Allocate_Sorting_Dynamic_Buffer()
{
	WWMEMLOG(MEM_RENDERER);
	WWASSERT(!_DynamicSortingVertexArrayInUse);
	_DynamicSortingVertexArrayInUse=true;

	unsigned new_vertex_count=_DynamicSortingVertexArrayOffset+VertexCount;
	WWASSERT(new_vertex_count<65536);
	if (new_vertex_count>_DynamicSortingVertexArraySize) {
		REF_PTR_RELEASE(_DynamicSortingVertexArray);
		_DynamicSortingVertexArraySize=new_vertex_count;
		if (_DynamicSortingVertexArraySize<DEFAULT_VB_SIZE) _DynamicSortingVertexArraySize=DEFAULT_VB_SIZE;
	}

	if (!_DynamicSortingVertexArray) {
		_DynamicSortingVertexArray=NEW_REF(SortingVertexBufferClass,(_DynamicSortingVertexArraySize));
		_DynamicSortingVertexArrayOffset=0;
	}

	REF_PTR_SET(VertexBuffer,_DynamicSortingVertexArray);
	VertexBufferOffset=_DynamicSortingVertexArrayOffset;
}

// ----------------------------------------------------------------------------
static int dx8_lock;
DynamicVBAccessClass::WriteLockClass::WriteLockClass(DynamicVBAccessClass* dynamic_vb_access_)
	:
	DynamicVBAccess(dynamic_vb_access_)
{
	DX8_THREAD_ASSERT();
	switch (DynamicVBAccess->Get_Type()) {
	case BUFFER_TYPE_DYNAMIC_DX8:
#ifdef VERTEX_BUFFER_LOG
/*		{
		WWASSERT(!dx8_lock);
		dx8_lock++;
		StringClass fvf_name;
		DynamicVBAccess->VertexBuffer->FVF_Info().Get_FVF_Name(fvf_name);
		WWDEBUG_SAY(("DynamicVertexBuffer->Lock(start_index: %d, index_range: %d, fvf_size: %d, fvf: %s)",
			DynamicVBAccess->VertexBufferOffset,
			DynamicVBAccess->Get_Vertex_Count(),
			DynamicVBAccess->VertexBuffer->FVF_Info().Get_FVF_Size(),
			fvf_name));
		}
*/
#endif
		WWASSERT(_DynamicDX8VertexBuffer);
//		WWASSERT(!_DynamicDX8VertexBuffer->Engine_Refs());

#ifdef _WIN32
		DX8_Assert();
		// Lock with discard contents if the buffer offset is zero
		DX8_ErrorCode(static_cast<DX8VertexBufferClass*>(DynamicVBAccess->VertexBuffer)->Get_DX8_Vertex_Buffer()->Lock(
			DynamicVBAccess->VertexBufferOffset*_DynamicDX8VertexBuffer->FVF_Info().Get_FVF_Size(),
			DynamicVBAccess->Get_Vertex_Count()*DynamicVBAccess->VertexBuffer->FVF_Info().Get_FVF_Size(),
			(void**)&Vertices,
			D3DLOCK_NOSYSLOCK | (!DynamicVBAccess->VertexBufferOffset ? D3DLOCK_DISCARD : D3DLOCK_NOOVERWRITE)));
#else
		// Phase 27.2.1: OpenGL dynamic vertex buffer lock (CPU-side emulation with offset)
		{
			DX8VertexBufferClass* vb = static_cast<DX8VertexBufferClass*>(DynamicVBAccess->VertexBuffer);
			unsigned fvf_size = _DynamicDX8VertexBuffer->FVF_Info().Get_FVF_Size();
			unsigned offset_bytes = DynamicVBAccess->VertexBufferOffset * fvf_size;
			Vertices = static_cast<VertexFormatXYZNDUV2*>((void*)((char*)vb->GLVertexData + offset_bytes));
			WWASSERT(Vertices != NULL);
#ifdef VERTEX_BUFFER_LOG
			printf("Phase 27.2.1: GL Dynamic VBO Lock - VBO id=%u, offset=%u, size=%u bytes\n",
				vb->Get_GL_Vertex_Buffer(),
				offset_bytes,
				DynamicVBAccess->Get_Vertex_Count() * fvf_size);
#endif
		}
#endif
		break;
	case BUFFER_TYPE_DYNAMIC_SORTING:
		Vertices=static_cast<SortingVertexBufferClass*>(DynamicVBAccess->VertexBuffer)->VertexBuffer;
		Vertices+=DynamicVBAccess->VertexBufferOffset;
//		vertices=_DynamicSortingVertexArray+_DynamicSortingVertexArrayOffset;
		break;
	default:
		WWASSERT(0);
		break;
	}
}

// ----------------------------------------------------------------------------

DynamicVBAccessClass::WriteLockClass::~WriteLockClass()
{
	DX8_THREAD_ASSERT();
	switch (DynamicVBAccess->Get_Type()) {
	case BUFFER_TYPE_DYNAMIC_DX8:
#ifdef VERTEX_BUFFER_LOG
/*		dx8_lock--;
		WWASSERT(!dx8_lock);
		WWDEBUG_SAY(("DynamicVertexBuffer->Unlock()"));
*/
#endif
#ifdef _WIN32
		DX8_Assert();
		DX8_ErrorCode(static_cast<DX8VertexBufferClass*>(DynamicVBAccess->VertexBuffer)->Get_DX8_Vertex_Buffer()->Unlock());
#else
		// Phase 27.2.1: OpenGL dynamic vertex buffer unlock (upload CPU data to GPU)
		// Phase 29.3: Skip OpenGL calls when Metal active (CPU buffer already updated)
		if (!g_useMetalBackend) {
			DX8VertexBufferClass* vb = static_cast<DX8VertexBufferClass*>(DynamicVBAccess->VertexBuffer);
			unsigned fvf_size = vb->FVF_Info().Get_FVF_Size();
			unsigned offset_bytes = DynamicVBAccess->VertexBufferOffset * fvf_size;
			unsigned size_bytes = DynamicVBAccess->Get_Vertex_Count() * fvf_size;

			glBindBuffer(GL_ARRAY_BUFFER, vb->Get_GL_Vertex_Buffer());
			glBufferSubData(GL_ARRAY_BUFFER, offset_bytes, size_bytes, Vertices);
#ifdef VERTEX_BUFFER_LOG
			printf("Phase 27.2.1: GL Dynamic VBO Unlock - VBO id=%u, uploaded %u bytes at offset %u\n",
				vb->Get_GL_Vertex_Buffer(), size_bytes, offset_bytes);
#endif
		}
		// Metal: CPU-side buffer already updated, no GPU upload needed in Phase 29.3
#endif
		break;
	case BUFFER_TYPE_DYNAMIC_SORTING:
		break;
	default:
		WWASSERT(0);
		break;
	}
}

// ----------------------------------------------------------------------------

void DynamicVBAccessClass::_Reset(bool frame_changed)
{
	_DynamicSortingVertexArrayOffset=0;
	if (frame_changed) _DynamicDX8VertexBufferOffset=0;
}

unsigned short DynamicVBAccessClass::Get_Default_Vertex_Count(void)
{
	return _DynamicDX8VertexBufferSize;
}

