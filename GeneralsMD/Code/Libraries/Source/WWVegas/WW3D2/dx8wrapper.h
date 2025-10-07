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
 *                     $Archive:: /Commando/Code/ww3d2/dx8wrapper.h                           $*
 *                                                                                             *
 *              Original Author:: Jani Penttinen                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 08/05/02 2:40p                                              $*
 *                                                                                             *
 *                    $Revision:: 92                                                          $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 * 06/27/02 KM Render to shadow buffer texture support														*
 * 08/05/02 KM Texture class redesign
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef DX8_WRAPPER_H
#define DX8_WRAPPER_H

#include "always.h"
#include "dllist.h"
#include "d3d8.h"
#include "matrix4.h"
#include "statistics.h"
#include "wwstring.h"
#include "lightenvironment.h"
#include "shader.h"
#include "vector4.h"
#include "cpudetect.h"
#include "dx8caps.h"

#include "texture.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "vertmaterial.h"

/*
** Registry value names
*/
#define	VALUE_NAME_RENDER_DEVICE_NAME					"RenderDeviceName"
#define	VALUE_NAME_RENDER_DEVICE_WIDTH				"RenderDeviceWidth"
#define	VALUE_NAME_RENDER_DEVICE_HEIGHT				"RenderDeviceHeight"
#define	VALUE_NAME_RENDER_DEVICE_DEPTH				"RenderDeviceDepth"
#define	VALUE_NAME_RENDER_DEVICE_WINDOWED			"RenderDeviceWindowed"
#define	VALUE_NAME_RENDER_DEVICE_TEXTURE_DEPTH		"RenderDeviceTextureDepth"

const unsigned MAX_TEXTURE_STAGES=8;
const unsigned MAX_VERTEX_STREAMS=2;
const unsigned MAX_VERTEX_SHADER_CONSTANTS=96;
const unsigned MAX_PIXEL_SHADER_CONSTANTS=8;
const unsigned MAX_SHADOW_MAPS=1;

#define prevVer
#define nextVer


enum {
	BUFFER_TYPE_DX8,
	BUFFER_TYPE_SORTING,
	BUFFER_TYPE_DYNAMIC_DX8,
	BUFFER_TYPE_DYNAMIC_SORTING,
	BUFFER_TYPE_INVALID
};

class VertexMaterialClass;
class CameraClass;
class LightEnvironmentClass;
class RenderDeviceDescClass;
class VertexBufferClass;
class DynamicVBAccessClass;
class IndexBufferClass;
class DynamicIBAccessClass;
class TextureClass;
class ZTextureClass;
class LightClass;
class SurfaceClass;
class DX8Caps;

#define DX8_RECORD_MATRIX_CHANGE()				matrix_changes++
#define DX8_RECORD_MATERIAL_CHANGE()			material_changes++
#define DX8_RECORD_VERTEX_BUFFER_CHANGE()		vertex_buffer_changes++
#define DX8_RECORD_INDEX_BUFFER_CHANGE()		index_buffer_changes++
#define DX8_RECORD_LIGHT_CHANGE()				light_changes++
#define DX8_RECORD_TEXTURE_CHANGE()				texture_changes++
#define DX8_RECORD_RENDER_STATE_CHANGE()		render_state_changes++
#define DX8_RECORD_TEXTURE_STAGE_STATE_CHANGE() texture_stage_state_changes++
#define DX8_RECORD_DRAW_CALLS()					draw_calls++

extern unsigned number_of_DX8_calls;
extern bool _DX8SingleThreaded;

void DX8_Assert();
void Log_DX8_ErrorCode(unsigned res);

WWINLINE void DX8_ErrorCode(unsigned res)
{
	if (res==D3D_OK) return;
	Log_DX8_ErrorCode(res);
}

#ifdef WWDEBUG
#define DX8CALL_HRES(x,res) DX8_Assert(); res = DX8Wrapper::_Get_D3D_Device8()->x; DX8_ErrorCode(res); number_of_DX8_calls++;
#define DX8CALL(x) DX8_Assert(); DX8_ErrorCode(DX8Wrapper::_Get_D3D_Device8()->x); number_of_DX8_calls++;
#define DX8CALL_D3D(x) DX8_Assert(); DX8_ErrorCode(DX8Wrapper::_Get_D3D8()->x); number_of_DX8_calls++;
#define DX8_THREAD_ASSERT() if (_DX8SingleThreaded) { WWASSERT_PRINT(DX8Wrapper::_Get_Main_Thread_ID()==ThreadClass::_Get_Current_Thread_ID(),"DX8Wrapper::DX8 calls must be called from the main thread!"); }
#else
#define DX8CALL_HRES(x,res) res = DX8Wrapper::_Get_D3D_Device8()->x; number_of_DX8_calls++;
#define DX8CALL(x) DX8Wrapper::_Get_D3D_Device8()->x; number_of_DX8_calls++;
#define DX8CALL_D3D(x) DX8Wrapper::_Get_D3D8()->x; number_of_DX8_calls++;
#define DX8_THREAD_ASSERT() ;
#endif


#define no_EXTENDED_STATS
// EXTENDED_STATS collects additional timing statistics by turning off parts
// of the 3D drawing system (terrain, objects, etc.)
#ifdef EXTENDED_STATS
class DX8_Stats
{
public:
	bool m_showingStats;
	bool m_disableTerrain;
	bool m_disableWater;
	bool m_disableObjects;
	bool m_disableOverhead;
	bool m_disableConsole;
	int  m_debugLinesToShow;
	int	 m_sleepTime;
public:
	DX8_Stats::DX8_Stats(void) {
		m_disableConsole = m_showingStats = m_disableTerrain = m_disableWater = m_disableOverhead = m_disableObjects = false;
		m_sleepTime = 0;
		m_debugLinesToShow = -1; // -1 means show all expected lines of output
	}
};
#endif


// This virtual interface was added for the Generals RTS.
// It is called before resetting the dx8 device to ensure
// that all dx8 resources are released.  Otherwise reset fails. jba.
class DX8_CleanupHook
{
public:
	virtual void ReleaseResources(void)=0;
	virtual void ReAcquireResources(void)=0;
};


struct RenderStateStruct
{
	ShaderClass shader;
	VertexMaterialClass* material;
	TextureBaseClass * Textures[MAX_TEXTURE_STAGES];
	D3DLIGHT8 Lights[4];
	bool LightEnable[4];
  //unsigned lightsHash;
	Matrix4x4 world;
	Matrix4x4 view;
	unsigned vertex_buffer_types[MAX_VERTEX_STREAMS];
	unsigned index_buffer_type;
	unsigned short vba_offset;
	unsigned short vba_count;
	unsigned short iba_offset;
	VertexBufferClass* vertex_buffers[MAX_VERTEX_STREAMS];
	IndexBufferClass* index_buffer;
	unsigned short index_base_offset;

	RenderStateStruct();
	~RenderStateStruct();

	RenderStateStruct& operator= (const RenderStateStruct& src);
};

/**
** DX8Wrapper
**
** DX8 interface wrapper class.  This encapsulates the DX8 interface; adding redundant state
** detection, stat tracking, etc etc.  In general, we will wrap all DX8 calls with at least
** an WWINLINE function so that we can add stat tracking, etc if needed.  Direct access to the
** D3D device will require "friend" status and should be granted only in extreme circumstances :-)
*/
class DX8Wrapper
{
	enum ChangedStates {
		WORLD_CHANGED	=	1<<0,
		VIEW_CHANGED	=	1<<1,
		LIGHT0_CHANGED	=	1<<2,
		LIGHT1_CHANGED	=	1<<3,
		LIGHT2_CHANGED	=	1<<4,
		LIGHT3_CHANGED	=	1<<5,
		TEXTURE0_CHANGED=	1<<6,
		TEXTURE1_CHANGED=	1<<7,
		TEXTURE2_CHANGED=	1<<8,
		TEXTURE3_CHANGED=	1<<9,
		MATERIAL_CHANGED=	1<<14,
		SHADER_CHANGED	=	1<<15,
		VERTEX_BUFFER_CHANGED = 1<<16,
		INDEX_BUFFER_CHANGED = 1 << 17,
		WORLD_IDENTITY=	1<<18,
		VIEW_IDENTITY=		1<<19,

		TEXTURES_CHANGED=
			TEXTURE0_CHANGED|TEXTURE1_CHANGED|TEXTURE2_CHANGED|TEXTURE3_CHANGED,
		LIGHTS_CHANGED=
			LIGHT0_CHANGED|LIGHT1_CHANGED|LIGHT2_CHANGED|LIGHT3_CHANGED,
	};

	static void Draw_Sorting_IB_VB(
		unsigned primitive_type,
		unsigned short start_index,
		unsigned short polygon_count,
		unsigned short min_vertex_index,
		unsigned short vertex_count);

	static void Draw(
		unsigned primitive_type,
		unsigned short start_index,
		unsigned short polygon_count,
		unsigned short min_vertex_index=0,
		unsigned short vertex_count=0);

public:
#ifdef EXTENDED_STATS
	static DX8_Stats stats;
#endif

	static bool Init(void * hwnd, bool lite = false);
	static void Shutdown(void);

	static void SetCleanupHook(DX8_CleanupHook *pCleanupHook) {m_pCleanupHook = pCleanupHook;};
	/*
	** Some WW3D sub-systems need to be initialized after the device is created and shutdown
	** before the device is released.
	*/
	static void	Do_Onetime_Device_Dependent_Inits(void);
	static void Do_Onetime_Device_Dependent_Shutdowns(void);

	static bool Is_Device_Lost() { return IsDeviceLost; }
	static bool Is_Initted(void) { return IsInitted; }

	static bool Has_Stencil (void);
	static void Get_Format_Name(unsigned int format, StringClass *tex_format);

	/*
	** Rendering
	*/
	static void Begin_Scene(void);
	static void End_Scene(bool flip_frame = true);

	// Flip until the primary buffer is visible.
	static void Flip_To_Primary(void);

	static void Clear(bool clear_color, bool clear_z_stencil, const Vector3 &color, float dest_alpha=0.0f, float z=1.0f, unsigned int stencil=0);

	static void	Set_Viewport(CONST D3DVIEWPORT8* pViewport);

	static void Set_Vertex_Buffer(const VertexBufferClass* vb, unsigned stream=0);
	static void Set_Vertex_Buffer(const DynamicVBAccessClass& vba);
	static void Set_Index_Buffer(const IndexBufferClass* ib,unsigned short index_base_offset);
	static void Set_Index_Buffer(const DynamicIBAccessClass& iba,unsigned short index_base_offset);
	static void Set_Index_Buffer_Index_Offset(unsigned offset);

	static void Get_Render_State(RenderStateStruct& state);
	static void Set_Render_State(const RenderStateStruct& state);
	static void Release_Render_State();

	static void Set_DX8_Material(const D3DMATERIAL8* mat);

	static void Set_Gamma(float gamma,float bright,float contrast,bool calibrate=true,bool uselimit=true);

	// Set_ and Get_Transform() functions take the matrix in Westwood convention format.

	static void Set_DX8_ZBias(int zbias);
	static void Set_Projection_Transform_With_Z_Bias(const Matrix4x4& matrix,float znear, float zfar);	// pointer to 16 matrices

	static void Set_Transform(D3DTRANSFORMSTATETYPE transform,const Matrix4x4& m);
	static void Set_Transform(D3DTRANSFORMSTATETYPE transform,const Matrix3D& m);
	static void Get_Transform(D3DTRANSFORMSTATETYPE transform, Matrix4x4& m);
	static void	Set_World_Identity();
	static void Set_View_Identity();
	static bool	Is_World_Identity();
	static bool Is_View_Identity();

	// Note that *_DX8_Transform() functions take the matrix in DX8 format - transposed from Westwood convention.

	static void _Set_DX8_Transform(D3DTRANSFORMSTATETYPE transform,const Matrix4x4& m);
	static void _Set_DX8_Transform(D3DTRANSFORMSTATETYPE transform,const Matrix3D& m);
	static void _Get_DX8_Transform(D3DTRANSFORMSTATETYPE transform, Matrix4x4& m);

	static void Set_DX8_Light(int index,D3DLIGHT8* light);
	static void Set_DX8_Render_State(D3DRENDERSTATETYPE state, unsigned value);
	static void Set_DX8_Clip_Plane(DWORD Index, CONST float* pPlane);
	static void Set_DX8_Texture_Stage_State(unsigned stage, D3DTEXTURESTAGESTATETYPE state, unsigned value);
	static void Set_DX8_Texture(unsigned int stage, IDirect3DBaseTexture8* texture);
	static void Set_Light_Environment(LightEnvironmentClass* light_env);
	static LightEnvironmentClass* Get_Light_Environment() { return Light_Environment; }
	static void Set_Fog(bool enable, const Vector3 &color, float start, float end);

	static WWINLINE const D3DLIGHT8& Peek_Light(unsigned index);
	static WWINLINE bool Is_Light_Enabled(unsigned index);

	static bool Validate_Device(void);

	// Deferred

	static void Set_Shader(const ShaderClass& shader);
	static void Get_Shader(ShaderClass& shader);
	static void Set_Texture(unsigned stage,TextureBaseClass* texture);
	static void Set_Material(const VertexMaterialClass* material);
	static void Set_Light(unsigned index,const D3DLIGHT8* light);
	static void Set_Light(unsigned index,const LightClass &light);

	static void Apply_Render_State_Changes();	// Apply deferred render state changes (will be called automatically by Draw...)

	static void Draw_Triangles(
		unsigned buffer_type,
		unsigned short start_index,
		unsigned short polygon_count,
		unsigned short min_vertex_index,
		unsigned short vertex_count);
	static void Draw_Triangles(
		unsigned short start_index,
		unsigned short polygon_count,
		unsigned short min_vertex_index,
		unsigned short vertex_count);
	static void Draw_Strip(
		unsigned short start_index,
		unsigned short index_count,
		unsigned short min_vertex_index,
		unsigned short vertex_count);

	/*
	** Resources
	*/

	static IDirect3DVolumeTexture8* _Create_DX8_Volume_Texture
	(
		unsigned int width,
		unsigned int height,
		unsigned int depth,
		WW3DFormat format,
		MipCountType mip_level_count,
		D3DPOOL pool=D3DPOOL_MANAGED
	);

	static IDirect3DCubeTexture8* _Create_DX8_Cube_Texture
	(
		unsigned int width,
		unsigned int height,
		WW3DFormat format,
		MipCountType mip_level_count,
		D3DPOOL pool=D3DPOOL_MANAGED,
		bool rendertarget=false
	);


	static IDirect3DTexture8* _Create_DX8_ZTexture
	(
		unsigned int width,
		unsigned int height,
		WW3DZFormat zformat,
		MipCountType mip_level_count,
		D3DPOOL pool=D3DPOOL_MANAGED
	);


	static IDirect3DTexture8 * _Create_DX8_Texture
	(
		unsigned int width,
		unsigned int height,
		WW3DFormat format,
		MipCountType mip_level_count,
		D3DPOOL pool=D3DPOOL_MANAGED,
		bool rendertarget=false
	);
	static IDirect3DTexture8 * _Create_DX8_Texture(const char *filename, MipCountType mip_level_count);
	static IDirect3DTexture8 * _Create_DX8_Texture(IDirect3DSurface8 *surface, MipCountType mip_level_count);

	// Phase 27.2.3: OpenGL texture creation
#ifndef _WIN32
	static unsigned int _Create_GL_Texture
	(
		unsigned int width,
		unsigned int height,
		WW3DFormat format,
		MipCountType mip_level_count,
		bool rendertarget=false
	);
#endif

	// Phase 27.2.4: OpenGL shader program management
#ifndef _WIN32
	static unsigned int _Load_And_Compile_Shader(const char *shader_path, unsigned int shader_type);
	static unsigned int _Create_Shader_Program(unsigned int vertex_shader, unsigned int fragment_shader);
	static bool _Check_Shader_Compile_Status(unsigned int shader, const char *shader_name);
	static bool _Check_Program_Link_Status(unsigned int program);
	static int _Get_Uniform_Location(unsigned int program, const char *uniform_name);
#endif

	// Phase 27.2.5: Vertex attribute setup
#ifndef _WIN32
	static void _Setup_Vertex_Attributes(unsigned int fvf, unsigned int vertex_stride);
#endif

	static IDirect3DSurface8 * _Create_DX8_Surface(unsigned int width, unsigned int height, WW3DFormat format);
	static IDirect3DSurface8 * _Create_DX8_Surface(const char *filename);
	static IDirect3DSurface8 * _Get_DX8_Front_Buffer();
	static SurfaceClass * _Get_DX8_Back_Buffer(unsigned int num=0);

	static void _Copy_DX8_Rects(
			IDirect3DSurface8* pSourceSurface,
			CONST RECT* pSourceRectsArray,
			UINT cRects,
			IDirect3DSurface8* pDestinationSurface,
			CONST POINT* pDestPointsArray
	);

	static void _Update_Texture(TextureClass *system, TextureClass *video);
	static void Flush_DX8_Resource_Manager(unsigned int bytes=0);
	static unsigned int Get_Free_Texture_RAM();

	static unsigned _Get_Main_Thread_ID() { return _MainThreadID; }
	static const D3DADAPTER_IDENTIFIER8& Get_Current_Adapter_Identifier() { return CurrentAdapterIdentifier; }

	/*
	** Statistics
	*/
	static void Begin_Statistics();
	static void End_Statistics();
	static unsigned Get_Last_Frame_Matrix_Changes();
	static unsigned Get_Last_Frame_Material_Changes();
	static unsigned Get_Last_Frame_Vertex_Buffer_Changes();
	static unsigned Get_Last_Frame_Index_Buffer_Changes();
	static unsigned Get_Last_Frame_Light_Changes();
	static unsigned Get_Last_Frame_Texture_Changes();
	static unsigned Get_Last_Frame_Render_State_Changes();
	static unsigned Get_Last_Frame_Texture_Stage_State_Changes();
	static unsigned Get_Last_Frame_DX8_Calls();
	static unsigned Get_Last_Frame_Draw_Calls();

	static unsigned long Get_FrameCount(void);

	// Needed by shader class
	static bool						Get_Fog_Enable() { return FogEnable; }
	static D3DCOLOR				Get_Fog_Color() { return FogColor; }

	// Utilities
	static Vector4 Convert_Color(unsigned color);
	static unsigned int Convert_Color(const Vector4& color);
	static unsigned int Convert_Color(const Vector3& color, const float alpha);
	static void Clamp_Color(Vector4& color);
	static unsigned int Convert_Color_Clamp(const Vector4& color);

	static void			  Set_Alpha (const float alpha, unsigned int &color);

	static void _Enable_Triangle_Draw(bool enable) { _EnableTriangleDraw=enable; }
	static bool _Is_Triangle_Draw_Enabled() { return _EnableTriangleDraw; }

	/*
	** Additional swap chain interface
	**
	**		Use this interface to render to multiple windows (in windowed mode).
	**	To render to an additional window, the sequence of calls should look
	**	something like this:
	**
	**	DX8Wrapper::Set_Render_Target (swap_chain_ptr);
	**
	**	WW3D::Begin_Render (true, true, Vector3 (0, 0, 0));
	**	WW3D::Render (scene, camera, FALSE, FALSE);
	**	WW3D::End_Render ();
	**
	**	swap_chain_ptr->Present (NULL, NULL, NULL, NULL);
	**
	**	DX8Wrapper::Set_Render_Target ((IDirect3DSurface8 *)NULL);
	**
	*/
	static IDirect3DSwapChain8 *	Create_Additional_Swap_Chain (HWND render_window);

	/*
	** Render target interface. If render target format is WW3D_FORMAT_UNKNOWN, current display format is used.
	*/
	static TextureClass *	Create_Render_Target (int width, int height, WW3DFormat format = WW3D_FORMAT_UNKNOWN);

	static void					Set_Render_Target (IDirect3DSurface8 *render_target, bool use_default_depth_buffer = false);
	static void					Set_Render_Target (IDirect3DSurface8* render_target, IDirect3DSurface8* dpeth_buffer);

	static void					Set_Render_Target (IDirect3DSwapChain8 *swap_chain);
	static bool					Is_Render_To_Texture(void) { return IsRenderToTexture; }

	// for depth map support KJM V
	static void Create_Render_Target
	(
		int width,
		int height,
		WW3DFormat format,
		WW3DZFormat zformat,
		TextureClass** target,
		ZTextureClass** depth_buffer
	);
	static void					Set_Render_Target_With_Z (TextureClass * texture, ZTextureClass* ztexture=NULL);

	static void Set_Shadow_Map(int idx, ZTextureClass* ztex) { Shadow_Map[idx]=ztex; }
	static ZTextureClass* Get_Shadow_Map(int idx) { return Shadow_Map[idx]; }
	// for depth map support KJM ^

	// shader system udpates KJM v
	static void Apply_Default_State();

	static void Set_Vertex_Shader(DWORD vertex_shader);
	static void Set_Pixel_Shader(DWORD pixel_shader);

	static void Set_Vertex_Shader_Constant(int reg, const void* data, int count);
	static void Set_Pixel_Shader_Constant(int reg, const void* data, int count);

	static DWORD Get_Vertex_Processing_Behavior() { return Vertex_Processing_Behavior; }

	// Needed by scene lighting class
	static void						Set_Ambient(const Vector3& color);
	static const Vector3&		Get_Ambient() { return Ambient_Color; }
	// shader system updates KJM ^




	static IDirect3DDevice8* _Get_D3D_Device8() { return D3DDevice; }
	static IDirect3D8* _Get_D3D8() { return D3DInterface; }
	/// Returns the display format - added by TR for video playback - not part of W3D
	static WW3DFormat	getBackBufferFormat( void );
	static bool Reset_Device(bool reload_assets=true);

	static const DX8Caps*	Get_Current_Caps() { WWASSERT(CurrentCaps); return CurrentCaps; }

	static bool Registry_Save_Render_Device( const char * sub_key );
	static bool Registry_Load_Render_Device( const char * sub_key, bool resize_window );

	static const char* Get_DX8_Render_State_Name(D3DRENDERSTATETYPE state);
	static const char* Get_DX8_Texture_Stage_State_Name(D3DTEXTURESTAGESTATETYPE state);
	static unsigned Get_DX8_Render_State(D3DRENDERSTATETYPE state) { return RenderStates[state]; }

	// Names of the specific values of render states and texture stage states
	static void Get_DX8_Texture_Stage_State_Value_Name(StringClass& name, D3DTEXTURESTAGESTATETYPE state, unsigned value);
	static void Get_DX8_Render_State_Value_Name(StringClass& name, D3DRENDERSTATETYPE state, unsigned value);

	static const char* Get_DX8_Texture_Address_Name(unsigned value);
	static const char* Get_DX8_Texture_Filter_Name(unsigned value);
	static const char* Get_DX8_Texture_Arg_Name(unsigned value);
	static const char* Get_DX8_Texture_Op_Name(unsigned value);
	static const char* Get_DX8_Texture_Transform_Flag_Name(unsigned value);
	static const char* Get_DX8_ZBuffer_Type_Name(unsigned value);
	static const char* Get_DX8_Fill_Mode_Name(unsigned value);
	static const char* Get_DX8_Shade_Mode_Name(unsigned value);
	static const char* Get_DX8_Blend_Name(unsigned value);
	static const char* Get_DX8_Cull_Mode_Name(unsigned value);
	static const char* Get_DX8_Cmp_Func_Name(unsigned value);
	static const char* Get_DX8_Fog_Mode_Name(unsigned value);
	static const char* Get_DX8_Stencil_Op_Name(unsigned value);
	static const char* Get_DX8_Material_Source_Name(unsigned value);
	static const char* Get_DX8_Vertex_Blend_Flag_Name(unsigned value);
	static const char* Get_DX8_Patch_Edge_Style_Name(unsigned value);
	static const char* Get_DX8_Debug_Monitor_Token_Name(unsigned value);
	static const char* Get_DX8_Blend_Op_Name(unsigned value);

	static void Invalidate_Cached_Render_States(void);

	static void Set_Draw_Polygon_Low_Bound_Limit(unsigned n) { DrawPolygonLowBoundLimit=n; }

protected:

	static bool	Create_Device(void);
	static void Release_Device(void);

	static void Reset_Statistics();
	static void Enumerate_Devices();
	static void Set_Default_Global_Render_States(void);

	/*
	** Device Selection Code.
	** For backward compatibility, the public interface for these functions is in the ww3d.
	** header file.  These functions are protected so that we aren't exposing two interfaces.
	*/
	static bool Set_Any_Render_Device(void);
	static bool	Set_Render_Device(const char * dev_name,int width=-1,int height=-1,int bits=-1,int windowed=-1,bool resize_window=false);
	static bool	Set_Render_Device(int dev=-1,int resx=-1,int resy=-1,int bits=-1,int windowed=-1,bool resize_window = false, bool reset_device = false, bool restore_assets=true);
	static bool Set_Next_Render_Device(void);
	static bool Toggle_Windowed(void);

	static int	Get_Render_Device_Count(void);
	static int	Get_Render_Device(void);
	static const RenderDeviceDescClass & Get_Render_Device_Desc(int deviceidx);
	static const char * Get_Render_Device_Name(int device_index);
	static bool Set_Device_Resolution(int width=-1,int height=-1,int bits=-1,int windowed=-1, bool resize_window=false);
	static void Get_Device_Resolution(int & set_w,int & set_h,int & set_bits,bool & set_windowed);
	static void Get_Render_Target_Resolution(int & set_w,int & set_h,int & set_bits,bool & set_windowed);
	static int	Get_Device_Resolution_Width(void) { return ResolutionWidth; }
	static int	Get_Device_Resolution_Height(void) { return ResolutionHeight; }

	static bool Registry_Save_Render_Device( const char *sub_key, int device, int width, int height, int depth, bool windowed, int texture_depth);
	static bool Registry_Load_Render_Device( const char * sub_key, char *device, int device_len, int &width, int &height, int &depth, int &windowed, int &texture_depth);
	static bool Is_Windowed(void) { return IsWindowed; }

	static void	Set_Texture_Bitdepth(int depth)	{ WWASSERT(depth==16 || depth==32); TextureBitDepth = depth; }
	static int	Get_Texture_Bitdepth(void)			{ return TextureBitDepth; }

	static void	Set_Swap_Interval(int swap);
	static int	Get_Swap_Interval(void);
	static void Set_Polygon_Mode(int mode);

	/*
	** Internal functions
	*/
	static void Resize_And_Position_Window();
	static bool Find_Color_And_Z_Mode(int resx,int resy,int bitdepth,D3DFORMAT * set_colorbuffer,D3DFORMAT * set_backbuffer, D3DFORMAT * set_zmode);
	static bool Find_Color_Mode(D3DFORMAT colorbuffer, int resx, int resy, UINT *mode);
	static bool Find_Z_Mode(D3DFORMAT colorbuffer,D3DFORMAT backbuffer, D3DFORMAT *zmode);
	static bool Test_Z_Mode(D3DFORMAT colorbuffer,D3DFORMAT backbuffer, D3DFORMAT zmode);
	static void Compute_Caps(WW3DFormat display_format);

	/*
	** Protected Member Variables
	*/

	static DX8_CleanupHook *m_pCleanupHook;

	static RenderStateStruct			render_state;
	static unsigned						render_state_changed;
	static Matrix4x4						DX8Transforms[D3DTS_WORLD+1];

	static bool								IsInitted;
	static bool								IsDeviceLost;
	static void *							Hwnd;
	static unsigned						_MainThreadID;

	static bool								_EnableTriangleDraw;

	static int								CurRenderDevice;
	static int								ResolutionWidth;
	static int								ResolutionHeight;
	static int								BitDepth;
	static int								TextureBitDepth;
	static bool								IsWindowed;
	static D3DFORMAT					DisplayFormat;

	static D3DMATRIX						old_world;
	static D3DMATRIX						old_view;
	static D3DMATRIX						old_prj;

	// shader system updates KJM v
	static DWORD							Vertex_Shader;
	static DWORD							Pixel_Shader;

	static Vector4							Vertex_Shader_Constants[MAX_VERTEX_SHADER_CONSTANTS];
	static Vector4							Pixel_Shader_Constants[MAX_PIXEL_SHADER_CONSTANTS];

	// Phase 27.2.4: OpenGL shader program state
#ifndef _WIN32
	static unsigned int					GL_Shader_Program;
	static unsigned int					GL_Vertex_Shader;
	static unsigned int					GL_Fragment_Shader;
	static unsigned int					GL_VAO;  // Vertex Array Object for vertex attribute setup
#endif

	static LightEnvironmentClass*		Light_Environment;
	static RenderInfoClass*				Render_Info;

	static DWORD							Vertex_Processing_Behavior;

	static ZTextureClass*				Shadow_Map[MAX_SHADOW_MAPS];

	static Vector3							Ambient_Color;
	// shader system updates KJM ^

	static bool								world_identity;
	static unsigned						RenderStates[256];
	static unsigned						TextureStageStates[MAX_TEXTURE_STAGES][32];
	static IDirect3DBaseTexture8 *	Textures[MAX_TEXTURE_STAGES];

	// These fog settings are constant for all objects in a given scene,
	// unlike the matching renderstates which vary based on shader settings.
	static bool								FogEnable;
	static D3DCOLOR						FogColor;

	static unsigned						matrix_changes;
	static unsigned						material_changes;
	static unsigned						vertex_buffer_changes;
	static unsigned						index_buffer_changes;
	static unsigned						light_changes;
	static unsigned						texture_changes;
	static unsigned						render_state_changes;
	static unsigned						texture_stage_state_changes;
	static unsigned						draw_calls;
	static bool								CurrentDX8LightEnables[4];

	static unsigned long FrameCount;

	static DX8Caps*						CurrentCaps;

	static D3DADAPTER_IDENTIFIER8		CurrentAdapterIdentifier;

	static IDirect3D8 *					D3DInterface;			//d3d8;
	static IDirect3DDevice8 *			D3DDevice;				//d3ddevice8;

	static IDirect3DSurface8 *			CurrentRenderTarget;
	static IDirect3DSurface8 *			CurrentDepthBuffer;
	static IDirect3DSurface8 *			DefaultRenderTarget;
	static IDirect3DSurface8 *			DefaultDepthBuffer;

	static unsigned							DrawPolygonLowBoundLimit;

	static bool								IsRenderToTexture;

	static int								ZBias;
	static float							ZNear;
	static float							ZFar;
	static Matrix4x4						ProjectionMatrix;

	friend void DX8_Assert();
	friend class WW3D;
	friend class DX8IndexBufferClass;
	friend class DX8VertexBufferClass;
};

// shader system updates KJM v
WWINLINE void DX8Wrapper::Set_Vertex_Shader(DWORD vertex_shader)
{
#if 0 //(gth) some code is bypassing this acessor function so we can't count on this variable...
	// may be incorrect if shaders are created and destroyed dynamically
	if (Vertex_Shader==vertex_shader) return;
#endif

	Vertex_Shader=vertex_shader;
	DX8CALL(SetVertexShader(Vertex_Shader));
}

WWINLINE void DX8Wrapper::Set_Pixel_Shader(DWORD pixel_shader)
{
	// may be incorrect if shaders are created and destroyed dynamically
	if (Pixel_Shader==pixel_shader) return;

	Pixel_Shader=pixel_shader;
	DX8CALL(SetPixelShader(Pixel_Shader));
}

WWINLINE void DX8Wrapper::Set_Vertex_Shader_Constant(int reg, const void* data, int count)
{
	int memsize=sizeof(Vector4)*count;

	// may be incorrect if shaders are created and destroyed dynamically
	if (memcmp(data, &Vertex_Shader_Constants[reg],memsize)==0) return;

	memcpy(&Vertex_Shader_Constants[reg],data,memsize);
	DX8CALL(SetVertexShaderConstant(reg,data,count));
}

WWINLINE void DX8Wrapper::Set_Pixel_Shader_Constant(int reg, const void* data, int count)
{
	int memsize=sizeof(Vector4)*count;

	// may be incorrect if shaders are created and destroyed dynamically
	if (memcmp(data, &Pixel_Shader_Constants[reg],memsize)==0) return;

	memcpy(&Pixel_Shader_Constants[reg],data,memsize);
	DX8CALL(SetPixelShaderConstant(reg,data,count));
}
// shader system updates KJM ^


WWINLINE void DX8Wrapper::_Set_DX8_Transform(D3DTRANSFORMSTATETYPE transform,const Matrix4x4& m)
{
	WWASSERT(transform<=D3DTS_WORLD);
#if 0 // (gth) this optimization is breaking generals because they set the transform behind our backs.
	if (m!=DX8Transforms[transform])
#endif
	{
		DX8Transforms[transform]=m;
		SNAPSHOT_SAY(("DX8 - SetTransform %d [%f,%f,%f,%f][%f,%f,%f,%f][%f,%f,%f,%f][%f,%f,%f,%f]",transform,m[0][0],m[0][1],m[0][2],m[0][3],m[1][0],m[1][1],m[1][2],m[1][3],m[2][0],m[2][1],m[2][2],m[2][3],m[3][0],m[3][1],m[3][2],m[3][3]));
		DX8_RECORD_MATRIX_CHANGE();
		DX8CALL(SetTransform(transform,(D3DMATRIX*)&m));
	}
}


WWINLINE void DX8Wrapper::_Set_DX8_Transform(D3DTRANSFORMSTATETYPE transform,const Matrix3D& m)
{
	WWASSERT(transform<=D3DTS_WORLD);
	Matrix4x4 mtx(m);
#if 0 // (gth) this optimization is breaking generals because they set the transform behind our backs.
	if (mtx!=DX8Transforms[transform])
#endif
	{
		DX8Transforms[transform]=mtx;
		SNAPSHOT_SAY(("DX8 - SetTransform %d [%f,%f,%f,%f][%f,%f,%f,%f][%f,%f,%f,%f]",transform,m[0][0],m[0][1],m[0][2],m[0][3],m[1][0],m[1][1],m[1][2],m[1][3],m[2][0],m[2][1],m[2][2],m[2][3]));
		DX8_RECORD_MATRIX_CHANGE();
		DX8CALL(SetTransform(transform,(D3DMATRIX*)&m));
	}
}

WWINLINE void DX8Wrapper::_Get_DX8_Transform(D3DTRANSFORMSTATETYPE transform, Matrix4x4& m)
{
	DX8CALL(GetTransform(transform,(D3DMATRIX*)&m));
}

// ----------------------------------------------------------------------------
//
// Set the index offset for the current index buffer
//
// ----------------------------------------------------------------------------

WWINLINE void DX8Wrapper::Set_Index_Buffer_Index_Offset(unsigned offset)
{
	if (render_state.index_base_offset==offset) return;
	render_state.index_base_offset=offset;
	render_state_changed|=INDEX_BUFFER_CHANGED;
}

// ----------------------------------------------------------------------------
// Set the fog settings. This function should be used, rather than setting the
// appropriate renderstates directly, because the shader sets some of the
// renderstates on a per-mesh / per-pass basis depending on global fog states
// (stored in the wrapper) as well as the shader settings.
// This function should be called rarely - once per scene would be appropriate.
// ----------------------------------------------------------------------------

WWINLINE void DX8Wrapper::Set_Fog(bool enable, const Vector3 &color, float start, float end)
{
	// Set global states
	FogEnable = enable;
	FogColor = Convert_Color(color,0.0f);

	// Invalidate the current shader (since the renderstates set by the shader
	// depend on the global fog settings as well as the actual shader settings)
	ShaderClass::Invalidate();

	// Set renderstates which are not affected by the shader
	Set_DX8_Render_State(D3DRS_FOGSTART, *(DWORD *)(&start));
	Set_DX8_Render_State(D3DRS_FOGEND,   *(DWORD *)(&end));
}


WWINLINE void DX8Wrapper::Set_Ambient(const Vector3& color)
{
	Ambient_Color=color;
	Set_DX8_Render_State(D3DRS_AMBIENT, DX8Wrapper::Convert_Color(color,0.0f));
}

// ----------------------------------------------------------------------------
//
// Set vertex buffer to be used in the subsequent render calls. If there was
// a vertex buffer being used earlier, release the reference to it. Passing
// NULL just will release the vertex buffer.
//
// ----------------------------------------------------------------------------

WWINLINE void DX8Wrapper::Set_DX8_Material(const D3DMATERIAL8* mat)
{
	DX8_RECORD_MATERIAL_CHANGE();
	WWASSERT(mat);
	SNAPSHOT_SAY(("DX8 - SetMaterial"));
#ifdef _WIN32
	DX8CALL(SetMaterial(mat));
#else
	// Phase 27.3.2: Update material uniforms in OpenGL shader
	// Note: Our basic shader doesn't currently use material properties,
	// but we can add them here for future expansion
	// The shader currently uses vertex colors instead of material colors
	if (GL_Shader_Program != 0 && mat != NULL) {
		glUseProgram(GL_Shader_Program);
		
		// For now, we'll just log that materials are being set
		// Future shaders can use these uniforms: uMaterialAmbient, uMaterialDiffuse, etc.
		// mat->Diffuse is float[4] with RGBA values
		printf("Phase 27.3.2: Material set (diffuse: %.2f,%.2f,%.2f,%.2f)\n", 
			mat->Diffuse[0], mat->Diffuse[1], mat->Diffuse[2], mat->Diffuse[3]);
	}
#endif
}

WWINLINE void DX8Wrapper::Set_DX8_Light(int index, D3DLIGHT8* light)
{
	if (light) {
		DX8_RECORD_LIGHT_CHANGE();
#ifdef _WIN32
		DX8CALL(SetLight(index,light));
		DX8CALL(LightEnable(index,TRUE));
#else
		// Phase 27.3.3: Update lighting uniforms in OpenGL shader
		if (GL_Shader_Program != 0) {
			glUseProgram(GL_Shader_Program);
			
			// For now, we only support one directional light (index 0)
			// Our basic shader has: uLightDirection, uLightColor, uAmbientColor
			if (index == 0 && light->Type == D3DLIGHT_DIRECTIONAL) {
				// Light direction
				GLint loc = glGetUniformLocation(GL_Shader_Program, "uLightDirection");
				if (loc != -1) {
					glUniform3f(loc, light->Direction.x, light->Direction.y, light->Direction.z);
					printf("Phase 27.3.3: Updated uLightDirection (%.2f, %.2f, %.2f)\n",
						light->Direction.x, light->Direction.y, light->Direction.z);
				}
				
				// Light diffuse color
				loc = glGetUniformLocation(GL_Shader_Program, "uLightColor");
				if (loc != -1) {
					glUniform3f(loc, light->Diffuse.r, light->Diffuse.g, light->Diffuse.b);
					printf("Phase 27.3.3: Updated uLightColor (%.2f, %.2f, %.2f)\n",
						light->Diffuse.r, light->Diffuse.g, light->Diffuse.b);
				}
				
				// Ambient color
				loc = glGetUniformLocation(GL_Shader_Program, "uAmbientColor");
				if (loc != -1) {
					glUniform3f(loc, light->Ambient.r, light->Ambient.g, light->Ambient.b);
					printf("Phase 27.3.3: Updated uAmbientColor (%.2f, %.2f, %.2f)\n",
						light->Ambient.r, light->Ambient.g, light->Ambient.b);
				}
				
				// Enable lighting uniform
				loc = glGetUniformLocation(GL_Shader_Program, "uUseLighting");
				if (loc != -1) {
					glUniform1i(loc, 1);
				}
			}
		}
#endif
		CurrentDX8LightEnables[index]=true;
		SNAPSHOT_SAY(("DX8 - SetLight %d",index));
	}
	else if (CurrentDX8LightEnables[index]) {
		DX8_RECORD_LIGHT_CHANGE();
		CurrentDX8LightEnables[index]=false;
#ifdef _WIN32
		DX8CALL(LightEnable(index,FALSE));
#else
		// Phase 27.3.3: Disable lighting in OpenGL shader
		if (GL_Shader_Program != 0 && index == 0) {
			glUseProgram(GL_Shader_Program);
			GLint loc = glGetUniformLocation(GL_Shader_Program, "uUseLighting");
			if (loc != -1) {
				glUniform1i(loc, 0);
				printf("Phase 27.3.3: Disabled lighting\n");
			}
		}
#endif
		SNAPSHOT_SAY(("DX8 - DisableLight %d",index));
	}
}

WWINLINE void DX8Wrapper::Set_DX8_Render_State(D3DRENDERSTATETYPE state, unsigned value)
{
	// Can't monitor state changes because setShader call to GERD may change the states!
	if (RenderStates[state]==value) return;

#ifdef MESH_RENDER_SNAPSHOT_ENABLED
	if (WW3D::Is_Snapshot_Activated()) {
		StringClass value_name(0,true);
		Get_DX8_Render_State_Value_Name(value_name,state,value);
		SNAPSHOT_SAY(("DX8 - SetRenderState(state: %s, value: %s)",
			Get_DX8_Render_State_Name(state),
			value_name.str()));
	}
#endif

	RenderStates[state]=value;

#ifdef _WIN32
	DX8CALL(SetRenderState( state, value ));
	DX8_RECORD_RENDER_STATE_CHANGE();
#else
	// Phase 27.4.2: OpenGL render state management
	switch (state) {
		case D3DRS_CULLMODE:
			// Face culling
			if (value == D3DCULL_NONE) {
				glDisable(GL_CULL_FACE);
				printf("Phase 27.4.2: Disabled face culling\n");
			} else {
				glEnable(GL_CULL_FACE);
				// D3D: CW = clockwise, CCW = counter-clockwise
				// OpenGL: GL_CW = clockwise, GL_CCW = counter-clockwise (default)
				// D3DCULL_CW culls clockwise faces, D3DCULL_CCW culls counter-clockwise
				if (value == D3DCULL_CW) {
					glCullFace(GL_FRONT);  // D3D culls CW (front faces in D3D default)
					glFrontFace(GL_CW);    // Define CW as front-facing
					printf("Phase 27.4.2: Cull mode CW (cull front faces)\n");
				} else if (value == D3DCULL_CCW) {
					glCullFace(GL_BACK);   // D3D culls CCW (back faces in D3D default)
					glFrontFace(GL_CCW);   // Define CCW as front-facing  
					printf("Phase 27.4.2: Cull mode CCW (cull back faces)\n");
				}
			}
			break;

		case D3DRS_ZENABLE:
			// Depth testing
			if (value == D3DZB_TRUE || value == D3DZB_USEW) {
				glEnable(GL_DEPTH_TEST);
				printf("Phase 27.4.2: Enabled depth testing\n");
			} else {
				glDisable(GL_DEPTH_TEST);
				printf("Phase 27.4.2: Disabled depth testing\n");
			}
			break;

		case D3DRS_ZWRITEENABLE:
			// Depth write mask
			glDepthMask(value ? GL_TRUE : GL_FALSE);
			printf("Phase 27.4.2: Depth write %s\n", value ? "enabled" : "disabled");
			break;

		case D3DRS_ZFUNC:
			// Depth comparison function
			{
				GLenum gl_func = GL_LEQUAL;  // Default
				switch (value) {
					case D3DCMP_NEVER:        gl_func = GL_NEVER; break;
					case D3DCMP_LESS:         gl_func = GL_LESS; break;
					case D3DCMP_EQUAL:        gl_func = GL_EQUAL; break;
					case D3DCMP_LESSEQUAL:    gl_func = GL_LEQUAL; break;
					case D3DCMP_GREATER:      gl_func = GL_GREATER; break;
					case D3DCMP_NOTEQUAL:     gl_func = GL_NOTEQUAL; break;
					case D3DCMP_GREATEREQUAL: gl_func = GL_GEQUAL; break;
					case D3DCMP_ALWAYS:       gl_func = GL_ALWAYS; break;
				}
				glDepthFunc(gl_func);
				printf("Phase 27.4.2: Depth func set to 0x%x\n", gl_func);
			}
			break;

		case D3DRS_ALPHABLENDENABLE:
			// Alpha blending
			if (value) {
				glEnable(GL_BLEND);
				printf("Phase 27.4.2: Enabled alpha blending\n");
			} else {
				glDisable(GL_BLEND);
				printf("Phase 27.4.2: Disabled alpha blending\n");
			}
			break;

		case D3DRS_SRCBLEND:
		case D3DRS_DESTBLEND:
			// Blend function - need both SRC and DEST to set glBlendFunc
			{
				GLenum src_blend = GL_ONE;
				GLenum dest_blend = GL_ZERO;
				
				// Map D3D blend modes to OpenGL
				auto map_blend = [](unsigned int d3d_blend) -> GLenum {
					switch (d3d_blend) {
						case D3DBLEND_ZERO:           return GL_ZERO;
						case D3DBLEND_ONE:            return GL_ONE;
						case D3DBLEND_SRCCOLOR:       return GL_SRC_COLOR;
						case D3DBLEND_INVSRCCOLOR:    return GL_ONE_MINUS_SRC_COLOR;
						case D3DBLEND_SRCALPHA:       return GL_SRC_ALPHA;
						case D3DBLEND_INVSRCALPHA:    return GL_ONE_MINUS_SRC_ALPHA;
						case D3DBLEND_DESTALPHA:      return GL_DST_ALPHA;
						case D3DBLEND_INVDESTALPHA:   return GL_ONE_MINUS_DST_ALPHA;
						case D3DBLEND_DESTCOLOR:      return GL_DST_COLOR;
						case D3DBLEND_INVDESTCOLOR:   return GL_ONE_MINUS_DST_COLOR;
						case D3DBLEND_SRCALPHASAT:    return GL_SRC_ALPHA_SATURATE;
						default:                      return GL_ONE;
					}
				};
				
				// Get current blend modes from RenderStates array
				src_blend = map_blend(RenderStates[D3DRS_SRCBLEND]);
				dest_blend = map_blend(RenderStates[D3DRS_DESTBLEND]);
				
				glBlendFunc(src_blend, dest_blend);
				printf("Phase 27.4.2: Blend func set (src: 0x%x, dest: 0x%x)\n", src_blend, dest_blend);
			}
			break;

		// Phase 27.4.4: Alpha testing (shader-based)
		case D3DRS_ALPHATESTENABLE:
		{
			// Update uAlphaTestEnabled uniform
			if (GL_Shader_Program != 0) {
				glUseProgram(GL_Shader_Program);
				GLint loc = glGetUniformLocation(GL_Shader_Program, "uAlphaTestEnabled");
				if (loc != -1) {
					glUniform1i(loc, value ? 1 : 0);
					printf("Phase 27.4.4: Alpha test %s\n", value ? "enabled" : "disabled");
				}
			}
			break;
		}
		
		case D3DRS_ALPHAREF:
		{
			// Update uAlphaRef uniform (D3D uses 0-255, shader uses 0.0-1.0)
			if (GL_Shader_Program != 0) {
				glUseProgram(GL_Shader_Program);
				GLint loc = glGetUniformLocation(GL_Shader_Program, "uAlphaRef");
				if (loc != -1) {
					float alpha_ref = value / 255.0f;
					glUniform1f(loc, alpha_ref);
					printf("Phase 27.4.4: Alpha reference set to %.3f (D3D value: %u)\n", alpha_ref, value);
				}
			}
			break;
		}
		
		case D3DRS_ALPHAFUNC:
		{
			// Update uAlphaTestFunc uniform (D3DCMP_*)
			if (GL_Shader_Program != 0) {
				glUseProgram(GL_Shader_Program);
				GLint loc = glGetUniformLocation(GL_Shader_Program, "uAlphaTestFunc");
				if (loc != -1) {
					glUniform1i(loc, value);
					const char* func_names[] = {"", "NEVER", "LESS", "EQUAL", "LESSEQUAL", 
						"GREATER", "NOTEQUAL", "GREATEREQUAL", "ALWAYS"};
					const char* func_name = (value >= 1 && value <= 8) ? func_names[value] : "UNKNOWN";
					printf("Phase 27.4.4: Alpha test func set to %s (value: %u)\n", func_name, value);
				}
			}
			break;
		}
		
		// Phase 27.4.5: Fog rendering (shader-based)
		case D3DRS_FOGENABLE:
		{
			// Update uFogEnabled uniform
			if (GL_Shader_Program != 0) {
				glUseProgram(GL_Shader_Program);
				GLint loc = glGetUniformLocation(GL_Shader_Program, "uFogEnabled");
				if (loc != -1) {
					glUniform1i(loc, value ? 1 : 0);
					printf("Phase 27.4.5: Fog %s\n", value ? "enabled" : "disabled");
				}
			}
			break;
		}
		
		case D3DRS_FOGCOLOR:
		{
			// Update uFogColor uniform (D3DCOLOR ARGB → RGB float)
			if (GL_Shader_Program != 0) {
				glUseProgram(GL_Shader_Program);
				GLint loc = glGetUniformLocation(GL_Shader_Program, "uFogColor");
				if (loc != -1) {
					float r = ((value >> 16) & 0xFF) / 255.0f;
					float g = ((value >> 8) & 0xFF) / 255.0f;
					float b = (value & 0xFF) / 255.0f;
					glUniform3f(loc, r, g, b);
					printf("Phase 27.4.5: Fog color set to RGB(%.2f, %.2f, %.2f)\n", r, g, b);
				}
			}
			break;
		}
		
		case D3DRS_FOGSTART:
		{
			// Update uFogStart uniform (D3D uses float in DWORD)
			if (GL_Shader_Program != 0) {
				glUseProgram(GL_Shader_Program);
				GLint loc = glGetUniformLocation(GL_Shader_Program, "uFogStart");
				if (loc != -1) {
					float fog_start = *reinterpret_cast<const float*>(&value);
					glUniform1f(loc, fog_start);
					printf("Phase 27.4.5: Fog start set to %.2f\n", fog_start);
				}
			}
			break;
		}
		
		case D3DRS_FOGEND:
		{
			// Update uFogEnd uniform (D3D uses float in DWORD)
			if (GL_Shader_Program != 0) {
				glUseProgram(GL_Shader_Program);
				GLint loc = glGetUniformLocation(GL_Shader_Program, "uFogEnd");
				if (loc != -1) {
					float fog_end = *reinterpret_cast<const float*>(&value);
					glUniform1f(loc, fog_end);
					printf("Phase 27.4.5: Fog end set to %.2f\n", fog_end);
				}
			}
			break;
		}
		
		case D3DRS_FOGDENSITY:
		{
			// Update uFogDensity uniform (D3D uses float in DWORD)
			if (GL_Shader_Program != 0) {
				glUseProgram(GL_Shader_Program);
				GLint loc = glGetUniformLocation(GL_Shader_Program, "uFogDensity");
				if (loc != -1) {
					float fog_density = *reinterpret_cast<const float*>(&value);
					glUniform1f(loc, fog_density);
					printf("Phase 27.4.5: Fog density set to %.4f\n", fog_density);
				}
			}
			break;
		}
		
		case D3DRS_FOGTABLEMODE:
		case D3DRS_FOGVERTEXMODE:
		{
			// Update uFogMode uniform
			// D3DFOG_NONE (0), D3DFOG_EXP (1), D3DFOG_EXP2 (2), D3DFOG_LINEAR (3)
			if (GL_Shader_Program != 0) {
				glUseProgram(GL_Shader_Program);
				GLint loc = glGetUniformLocation(GL_Shader_Program, "uFogMode");
				if (loc != -1) {
					glUniform1i(loc, value);
					const char* fog_modes[] = {"NONE", "EXP", "EXP2", "LINEAR"};
					const char* mode_name = (value <= 3) ? fog_modes[value] : "UNKNOWN";
					printf("Phase 27.4.5: Fog mode set to %s (value: %u)\n", mode_name, value);
				}
			}
			break;
		}
		
		// Phase 27.4.6: Stencil buffer operations
		case D3DRS_STENCILENABLE:
		{
			if (value) {
				glEnable(GL_STENCIL_TEST);
				printf("Phase 27.4.6: Stencil test enabled\n");
			} else {
				glDisable(GL_STENCIL_TEST);
				printf("Phase 27.4.6: Stencil test disabled\n");
			}
			break;
		}
		
		case D3DRS_STENCILFUNC:
		{
			// Map D3DCMP_* to GL comparison functions
			GLenum gl_func = GL_ALWAYS;
			switch (value) {
				case 1: gl_func = GL_NEVER; break;     // D3DCMP_NEVER
				case 2: gl_func = GL_LESS; break;      // D3DCMP_LESS
				case 3: gl_func = GL_EQUAL; break;     // D3DCMP_EQUAL
				case 4: gl_func = GL_LEQUAL; break;    // D3DCMP_LESSEQUAL
				case 5: gl_func = GL_GREATER; break;   // D3DCMP_GREATER
				case 6: gl_func = GL_NOTEQUAL; break;  // D3DCMP_NOTEQUAL
				case 7: gl_func = GL_GEQUAL; break;    // D3DCMP_GREATEREQUAL
				case 8: gl_func = GL_ALWAYS; break;    // D3DCMP_ALWAYS
				default:
					printf("Phase 27.4.6 WARNING: Unknown stencil func %u\n", value);
					break;
			}
			
			// Use stored ref and mask values
			glStencilFunc(gl_func, RenderStates[D3DRS_STENCILREF], RenderStates[D3DRS_STENCILMASK]);
			printf("Phase 27.4.6: Stencil func set to 0x%04X (D3D: %u)\n", gl_func, value);
			break;
		}
		
		case D3DRS_STENCILREF:
		{
			// Update stencil reference value
			glStencilFunc(
				GL_ALWAYS,  // Will be updated by D3DRS_STENCILFUNC
				value,
				RenderStates[D3DRS_STENCILMASK]
			);
			printf("Phase 27.4.6: Stencil ref set to %u\n", value);
			break;
		}
		
		case D3DRS_STENCILMASK:
		{
			// Update stencil read mask
			glStencilFunc(
				GL_ALWAYS,  // Will be updated by D3DRS_STENCILFUNC
				RenderStates[D3DRS_STENCILREF],
				value
			);
			printf("Phase 27.4.6: Stencil mask set to 0x%08X\n", value);
			break;
		}
		
		case D3DRS_STENCILWRITEMASK:
		{
			// Update stencil write mask
			glStencilMask(value);
			printf("Phase 27.4.6: Stencil write mask set to 0x%08X\n", value);
			break;
		}
		
		case D3DRS_STENCILFAIL:
		case D3DRS_STENCILZFAIL:
		case D3DRS_STENCILPASS:
		{
			// Lambda to map D3DSTENCILOP to GL stencil operation
			auto map_stencil_op = [](unsigned int d3d_op) -> GLenum {
				switch (d3d_op) {
					case 1: return GL_KEEP;      // D3DSTENCILOP_KEEP
					case 2: return GL_ZERO;      // D3DSTENCILOP_ZERO
					case 3: return GL_REPLACE;   // D3DSTENCILOP_REPLACE
					case 4: return GL_INCR;      // D3DSTENCILOP_INCRSAT (clamped)
					case 5: return GL_DECR;      // D3DSTENCILOP_DECRSAT (clamped)
					case 6: return GL_INVERT;    // D3DSTENCILOP_INVERT
					case 7: return GL_INCR_WRAP; // D3DSTENCILOP_INCR (wrapping)
					case 8: return GL_DECR_WRAP; // D3DSTENCILOP_DECR (wrapping)
					default: return GL_KEEP;
				}
			};
			
			// Get current operations
			GLenum sfail = map_stencil_op(RenderStates[D3DRS_STENCILFAIL]);
			GLenum dpfail = map_stencil_op(RenderStates[D3DRS_STENCILZFAIL]);
			GLenum dppass = map_stencil_op(RenderStates[D3DRS_STENCILPASS]);
			
			// Update the one that changed
			if (state == D3DRS_STENCILFAIL) sfail = map_stencil_op(value);
			else if (state == D3DRS_STENCILZFAIL) dpfail = map_stencil_op(value);
			else if (state == D3DRS_STENCILPASS) dppass = map_stencil_op(value);
			
			glStencilOp(sfail, dpfail, dppass);
			
			const char* state_names[] = {"", "FAIL", "ZFAIL", "PASS"};
			const char* op_names[] = {"", "KEEP", "ZERO", "REPLACE", "INCRSAT", "DECRSAT", "INVERT", "INCR", "DECR"};
			const char* state_name = (state >= D3DRS_STENCILFAIL && state <= D3DRS_STENCILPASS) ? 
				state_names[state - D3DRS_STENCILFAIL + 1] : "UNKNOWN";
			const char* op_name = (value >= 1 && value <= 8) ? op_names[value] : "UNKNOWN";
			
			printf("Phase 27.4.6: Stencil %s operation set to %s (GL: 0x%04X)\n", 
				state_name, op_name, map_stencil_op(value));
			break;
		}
		
		// Phase 27.4.7: Scissor test (D3D9 feature, limited support in D3D8)
		// Note: D3D8 doesn't have D3DRS_SCISSORTESTENABLE. In D3D9, this would be state 174.
		// For D3D8 compatibility, we provide basic scissor support that can be enabled
		// when the engine is upgraded to D3D9 or when custom render targets are used.
		// The implementation below is prepared for future use.
		case 174:  // D3DRS_SCISSORTESTENABLE (D3D9 constant, not in D3D8)
		{
			if (value) {
				glEnable(GL_SCISSOR_TEST);
				printf("Phase 27.4.7: Scissor test enabled (D3D9 extension)\n");
			} else {
				glDisable(GL_SCISSOR_TEST);
				printf("Phase 27.4.7: Scissor test disabled (D3D9 extension)\n");
			}
			break;
		}


	// Phase 27.4.8: Point sprite render states
	case D3DRS_POINTSPRITEENABLE:  // 156
	{
		if (GL_Shader_Program != 0) {
			glUseProgram(GL_Shader_Program);
			GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointSpriteEnabled");
			if (loc != -1) {
				glUniform1i(loc, value ? 1 : 0);
				printf("Phase 27.4.8: Point sprite %s\n", value ? "enabled" : "disabled");
			}
			
			// Enable OpenGL point sprite mode
			if (value) {
				glEnable(GL_PROGRAM_POINT_SIZE);  // Allow shader to control point size
				printf("Phase 27.4.8: GL_PROGRAM_POINT_SIZE enabled\n");
			} else {
				glDisable(GL_PROGRAM_POINT_SIZE);
			}
		}
		break;
	}

	case D3DRS_POINTSIZE:  // 154
	{
		if (GL_Shader_Program != 0) {
			glUseProgram(GL_Shader_Program);
			GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointSize");
			if (loc != -1) {
				float pointSize = *reinterpret_cast<const float*>(&value);
				glUniform1f(loc, pointSize);
				printf("Phase 27.4.8: Point size set to %.2f\n", pointSize);
			}
		}
		break;
	}

	case D3DRS_POINTSCALEENABLE:  // 157
	{
		if (GL_Shader_Program != 0) {
			glUseProgram(GL_Shader_Program);
			GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointScaleEnabled");
			if (loc != -1) {
				glUniform1i(loc, value ? 1 : 0);
				printf("Phase 27.4.8: Point scale %s\n", value ? "enabled" : "disabled");
			}
		}
		break;
	}

	case D3DRS_POINTSCALE_A:  // 158
	{
		if (GL_Shader_Program != 0) {
			glUseProgram(GL_Shader_Program);
			GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointScaleA");
			if (loc != -1) {
				float scaleA = *reinterpret_cast<const float*>(&value);
				glUniform1f(loc, scaleA);
				printf("Phase 27.4.8: Point scale A = %.4f\n", scaleA);
			}
		}
		break;
	}

	case D3DRS_POINTSCALE_B:  // 159
	{
		if (GL_Shader_Program != 0) {
			glUseProgram(GL_Shader_Program);
			GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointScaleB");
			if (loc != -1) {
				float scaleB = *reinterpret_cast<const float*>(&value);
				glUniform1f(loc, scaleB);
				printf("Phase 27.4.8: Point scale B = %.4f\n", scaleB);
			}
		}
		break;
	}

	case D3DRS_POINTSCALE_C:  // 160
	{
		if (GL_Shader_Program != 0) {
			glUseProgram(GL_Shader_Program);
			GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointScaleC");
			if (loc != -1) {
				float scaleC = *reinterpret_cast<const float*>(&value);
				glUniform1f(loc, scaleC);
				printf("Phase 27.4.8: Point scale C = %.4f\n", scaleC);
			}
		}
		break;
	}

	case D3DRS_POINTSIZE_MIN:  // 155
	{
		if (GL_Shader_Program != 0) {
			glUseProgram(GL_Shader_Program);
			GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointSizeMin");
			if (loc != -1) {
				float minSize = *reinterpret_cast<const float*>(&value);
				glUniform1f(loc, minSize);
				printf("Phase 27.4.8: Point size min = %.2f\n", minSize);
			}
		}
		break;
	}

	case D3DRS_POINTSIZE_MAX:  // 166
	{
		if (GL_Shader_Program != 0) {
			glUseProgram(GL_Shader_Program);
			GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointSizeMax");
			if (loc != -1) {
				float maxSize = *reinterpret_cast<const float*>(&value);
				glUniform1f(loc, maxSize);
				printf("Phase 27.4.8: Point size max = %.2f\n", maxSize);
			}
		}
		break;
	}
		default:
			// Other states - store but don't translate yet
			break;
	}
#endif
}

WWINLINE void DX8Wrapper::Set_DX8_Clip_Plane(DWORD Index, CONST float* pPlane)
{
	DX8CALL(SetClipPlane( Index, pPlane ));
}

WWINLINE void DX8Wrapper::Set_DX8_Texture_Stage_State(unsigned stage, D3DTEXTURESTAGESTATETYPE state, unsigned value)
{
  	if (stage >= MAX_TEXTURE_STAGES)
  	{	
#ifdef _WIN32
		DX8CALL(SetTextureStageState( stage, state, value ));
#endif
  		return;
  	}

	// Can't monitor state changes because setShader call to GERD may change the states!
	if (TextureStageStates[stage][(unsigned int)state]==value) return;
#ifdef MESH_RENDER_SNAPSHOT_ENABLED
	if (WW3D::Is_Snapshot_Activated()) {
		StringClass value_name(0,true);
		Get_DX8_Texture_Stage_State_Value_Name(value_name,state,value);
		SNAPSHOT_SAY(("DX8 - SetTextureStageState(stage: %d, state: %s, value: %s)",
			stage,
			Get_DX8_Texture_Stage_State_Name(state),
			value_name.str()));
	}
#endif

	TextureStageStates[stage][(unsigned int)state]=value;
	
#ifdef _WIN32
	DX8CALL(SetTextureStageState( stage, state, value ));
#else
	// Phase 27.4.3: OpenGL texture stage state management
	// Note: In OpenGL 3.3 Core Profile, texture combiners are handled in fragment shader
	// We store these states for potential shader uniform updates
	
	switch (state) {
		// Texture operations (stored for shader use)
		case D3DTSS_COLOROP:
		case D3DTSS_ALPHAOP:
			printf("Phase 27.4.3: Texture stage %u operation state %d = %u (stored for shader)\n", 
				stage, state, value);
			// These will be used to configure fragment shader texture combiners
			// D3DTOP_DISABLE (1), D3DTOP_SELECTARG1 (2), D3DTOP_MODULATE (4), etc.
			break;
			
		case D3DTSS_COLORARG1:
		case D3DTSS_COLORARG2:
		case D3DTSS_COLORARG0:
		case D3DTSS_ALPHAARG1:
		case D3DTSS_ALPHAARG2:
		case D3DTSS_ALPHAARG0:
			// Texture arguments (D3DTA_DIFFUSE, D3DTA_TEXTURE, D3DTA_CURRENT, etc.)
			printf("Phase 27.4.3: Texture stage %u argument state %d = %u (stored for shader)\n", 
				stage, state, value);
			break;
		
		// Texture filtering and addressing (can be mapped to OpenGL sampler states)
		case D3DTSS_ADDRESSU:
		case D3DTSS_ADDRESSV:
		case D3DTSS_ADDRESSW:
		{
			// Map D3D texture address modes to OpenGL wrap modes
			// D3DTADDRESS_WRAP (1) → GL_REPEAT
			// D3DTADDRESS_CLAMP (3) → GL_CLAMP_TO_EDGE
			// D3DTADDRESS_MIRROR (2) → GL_MIRRORED_REPEAT
			// D3DTADDRESS_BORDER (4) → GL_CLAMP_TO_BORDER
			
			GLenum gl_wrap_mode = GL_REPEAT;
			switch (value) {
				case 1: gl_wrap_mode = GL_REPEAT; break;           // D3DTADDRESS_WRAP
				case 2: gl_wrap_mode = GL_MIRRORED_REPEAT; break;  // D3DTADDRESS_MIRROR
				case 3: gl_wrap_mode = GL_CLAMP_TO_EDGE; break;    // D3DTADDRESS_CLAMP
				case 4: gl_wrap_mode = GL_CLAMP_TO_BORDER; break;  // D3DTADDRESS_BORDER
				default: 
					printf("Phase 27.4.3 WARNING: Unknown texture address mode %u\n", value);
					break;
			}
			
			// Apply to active texture unit
			if (Textures[stage] != nullptr) {
				glActiveTexture(GL_TEXTURE0 + stage);
				
				GLenum gl_wrap_param = (state == D3DTSS_ADDRESSU) ? GL_TEXTURE_WRAP_S :
				                       (state == D3DTSS_ADDRESSV) ? GL_TEXTURE_WRAP_T :
				                       GL_TEXTURE_WRAP_R;
				
				glTexParameteri(GL_TEXTURE_2D, gl_wrap_param, gl_wrap_mode);
				
				printf("Phase 27.4.3: Texture stage %u wrap mode set: %s = 0x%04X\n", 
					stage, 
					(state == D3DTSS_ADDRESSU) ? "WRAP_S" : 
					(state == D3DTSS_ADDRESSV) ? "WRAP_T" : "WRAP_R",
					gl_wrap_mode);
			}
			break;
		}
		
		case D3DTSS_MAGFILTER:
		case D3DTSS_MINFILTER:
		{
			// Map D3D texture filters to OpenGL
			// D3DTEXF_POINT (1) → GL_NEAREST
			// D3DTEXF_LINEAR (2) → GL_LINEAR
			
			GLenum gl_filter = (value == 1) ? GL_NEAREST : GL_LINEAR;
			
			if (Textures[stage] != nullptr) {
				glActiveTexture(GL_TEXTURE0 + stage);
				GLenum gl_filter_param = (state == D3DTSS_MAGFILTER) ? GL_TEXTURE_MAG_FILTER : GL_TEXTURE_MIN_FILTER;
				glTexParameteri(GL_TEXTURE_2D, gl_filter_param, gl_filter);
				
				printf("Phase 27.4.3: Texture stage %u filter set: %s = 0x%04X\n", 
					stage,
					(state == D3DTSS_MAGFILTER) ? "MAG_FILTER" : "MIN_FILTER",
					gl_filter);
			}
			break;
		}
		
		case D3DTSS_MIPFILTER:
		{
			// Mipmap filtering
			// D3DTEXF_NONE (0) → GL_LINEAR (no mipmap)
			// D3DTEXF_POINT (1) → GL_LINEAR_MIPMAP_NEAREST
			// D3DTEXF_LINEAR (2) → GL_LINEAR_MIPMAP_LINEAR
			
			if (Textures[stage] != nullptr) {
				glActiveTexture(GL_TEXTURE0 + stage);
				GLenum gl_mip_filter = GL_LINEAR;
				
				if (value == 1) gl_mip_filter = GL_LINEAR_MIPMAP_NEAREST;
				else if (value == 2) gl_mip_filter = GL_LINEAR_MIPMAP_LINEAR;
				
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_mip_filter);
				
				printf("Phase 27.4.3: Texture stage %u mipmap filter set: 0x%04X\n", stage, gl_mip_filter);
			}
			break;
		}
		
		case D3DTSS_BORDERCOLOR:
		{
			// Border color for GL_CLAMP_TO_BORDER
			if (Textures[stage] != nullptr) {
				glActiveTexture(GL_TEXTURE0 + stage);
				
				// Extract ARGB components from D3DCOLOR
				float border_color[4] = {
					((value >> 16) & 0xFF) / 255.0f,  // R
					((value >> 8) & 0xFF) / 255.0f,   // G
					(value & 0xFF) / 255.0f,          // B
					((value >> 24) & 0xFF) / 255.0f   // A
				};
				
				glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
				
				printf("Phase 27.4.3: Texture stage %u border color set: ARGB(%.2f, %.2f, %.2f, %.2f)\n", 
					stage, border_color[3], border_color[0], border_color[1], border_color[2]);
			}
			break;
		}
		
		case D3DTSS_MAXANISOTROPY:
		{
			// Anisotropic filtering level
			if (Textures[stage] != nullptr) {
				glActiveTexture(GL_TEXTURE0 + stage);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);
				printf("Phase 27.4.3: Texture stage %u max anisotropy set: %u\n", stage, value);
			}
			break;
		}
		
		case D3DTSS_TEXCOORDINDEX:
		case D3DTSS_TEXTURETRANSFORMFLAGS:
		case D3DTSS_BUMPENVMAT00:
		case D3DTSS_BUMPENVMAT01:
		case D3DTSS_BUMPENVMAT10:
		case D3DTSS_BUMPENVMAT11:
		case D3DTSS_BUMPENVLSCALE:
		case D3DTSS_BUMPENVLOFFSET:
		case D3DTSS_RESULTARG:
		case D3DTSS_CONSTANT:
		case D3DTSS_MIPMAPLODBIAS:
		case D3DTSS_MAXMIPLEVEL:
			// Advanced states - store but don't translate yet
			printf("Phase 27.4.3: Texture stage %u advanced state %d = %u (stored)\n", stage, state, value);
			break;
			
		default:
			// Unknown state
			printf("Phase 27.4.3 WARNING: Unknown texture stage state %d = %u\n", state, value);
			break;
	}
#endif
	
	DX8_RECORD_TEXTURE_STAGE_STATE_CHANGE();
}

WWINLINE void DX8Wrapper::Set_DX8_Texture(unsigned int stage, IDirect3DBaseTexture8* texture)
{
  	if (stage >= MAX_TEXTURE_STAGES)
  	{	DX8CALL(SetTexture(stage, texture));
  		return;
  	}

	if (Textures[stage]==texture) return;

	SNAPSHOT_SAY(("DX8 - SetTexture(%x) ",texture));

	if (Textures[stage]) Textures[stage]->Release();
	Textures[stage] = texture;
	if (Textures[stage]) Textures[stage]->AddRef();
	DX8CALL(SetTexture(stage, texture));
	DX8_RECORD_TEXTURE_CHANGE();
}

WWINLINE void DX8Wrapper::_Copy_DX8_Rects(
  IDirect3DSurface8* pSourceSurface,
  CONST RECT* pSourceRectsArray,
  UINT cRects,
  IDirect3DSurface8* pDestinationSurface,
  CONST POINT* pDestPointsArray
)
{
	DX8CALL(CopyRects(
  pSourceSurface,
  pSourceRectsArray,
  cRects,
  pDestinationSurface,
  pDestPointsArray));
}

WWINLINE Vector4 DX8Wrapper::Convert_Color(unsigned color)
{
	Vector4 col;
	col[3]=((color&0xff000000)>>24)/255.0f;
	col[0]=((color&0xff0000)>>16)/255.0f;
	col[1]=((color&0xff00)>>8)/255.0f;
	col[2]=((color&0xff)>>0)/255.0f;
//	col=Vector4(1.0f,1.0f,1.0f,1.0f);
	return col;
}

#if 0
WWINLINE unsigned int DX8Wrapper::Convert_Color(const Vector3& color, const float alpha)
{
	WWASSERT(color.X<=1.0f);
	WWASSERT(color.Y<=1.0f);
	WWASSERT(color.Z<=1.0f);
	WWASSERT(alpha<=1.0f);
	WWASSERT(color.X>=0.0f);
	WWASSERT(color.Y>=0.0f);
	WWASSERT(color.Z>=0.0f);
	WWASSERT(alpha>=0.0f);

	return D3DCOLOR_COLORVALUE(color.X,color.Y,color.Z,alpha);
}
WWINLINE unsigned int DX8Wrapper::Convert_Color(const Vector4& color)
{
	WWASSERT(color.X<=1.0f);
	WWASSERT(color.Y<=1.0f);
	WWASSERT(color.Z<=1.0f);
	WWASSERT(color.W<=1.0f);
	WWASSERT(color.X>=0.0f);
	WWASSERT(color.Y>=0.0f);
	WWASSERT(color.Z>=0.0f);
	WWASSERT(color.W>=0.0f);

	return D3DCOLOR_COLORVALUE(color.X,color.Y,color.Z,color.W);
}
#else

// ----------------------------------------------------------------------------
//
// Convert RGBA color from float vector to 32 bit integer
// Note: Color vector needs to be clamped to [0...1] range!
//
// ----------------------------------------------------------------------------

WWINLINE unsigned int DX8Wrapper::Convert_Color(const Vector3& color,float alpha)
{
#if defined(_MSC_VER) && _MSC_VER < 1300
	const float scale = 255.0;
	unsigned int col;

	// Multiply r, g, b and a components (0.0,...,1.0) by 255 and convert to integer. Or the integer values togerher
	// such that 32 bit ingeger has AAAAAAAARRRRRRRRGGGGGGGGBBBBBBBB.
	__asm
	{
		sub	esp,20					// space for a, r, g and b float plus fpu rounding mode

		// Store the fpu rounding mode

		fwait
		fstcw		[esp+16]				// store control word to stack
		mov		eax,[esp+16]		// load it to eax
		mov		edi,eax				// take copy
		and		eax,~(1024|2048)	// mask out certain bits
		or			eax,(1024|2048)	// or with precision control value "truncate"
		sub		edi,eax				// did it change?
		jz			skip					// .. if not, skip
		mov		[esp],eax			// .. change control word
		fldcw		[esp]
skip:

		// Convert the color

		mov	esi,dword ptr color
		fld	dword ptr[scale]

		fld	dword ptr[esi]			// r
		fld	dword ptr[esi+4]		// g
		fld	dword ptr[esi+8]		// b
		fld	dword ptr[alpha]		// a
		fld	st(4)
		fmul	st(4),st
		fmul	st(3),st
		fmul	st(2),st
		fmulp	st(1),st
		fistp	dword ptr[esp+0]		// a
		fistp	dword ptr[esp+4]		// b
		fistp	dword ptr[esp+8]		// g
		fistp	dword ptr[esp+12]		// r
		mov	ecx,[esp]				// a
		mov	eax,[esp+4]				// b
		mov	edx,[esp+8]				// g
		mov	ebx,[esp+12]			// r
		shl	ecx,24					// a << 24
		shl	ebx,16					// r << 16
		shl	edx,8						//	g << 8
		or		eax,ecx					// (a << 24) | b
		or		eax,ebx					// (a << 24) | (r << 16) | b
		or		eax,edx					// (a << 24) | (r << 16) | (g << 8) | b

		fstp	st(0)

		// Restore fpu rounding mode

		cmp	edi,0					// did we change the value?
		je		not_changed			// nope... skip now...
		fwait
		fldcw	[esp+16];
not_changed:
		add	esp,20

		mov	col,eax
	}
	return col;
#else
	return color.Convert_To_ARGB(alpha);
#endif // defined(_MSC_VER) && _MSC_VER < 1300
}

// ----------------------------------------------------------------------------
//
// Clamp color vector to [0...1] range
//
// ----------------------------------------------------------------------------

WWINLINE void DX8Wrapper::Clamp_Color(Vector4& color)
{
#if defined(_MSC_VER) && _MSC_VER < 1300
	if (CPUDetectClass::Has_CMOV_Instruction()) {
	__asm
	{
		mov	esi,dword ptr color

		mov edx,0x3f800000

		mov edi,dword ptr[esi]
		mov ebx,edi
		sar edi,31
		not edi			// mask is now zero if negative value
		and edi,ebx
		cmp edi,edx		// if no less than 1.0 set to 1.0
		cmovnb edi,edx
		mov dword ptr[esi],edi

		mov edi,dword ptr[esi+4]
		mov ebx,edi
		sar edi,31
		not edi			// mask is now zero if negative value
		and edi,ebx
		cmp edi,edx		// if no less than 1.0 set to 1.0
		cmovnb edi,edx
		mov dword ptr[esi+4],edi

		mov edi,dword ptr[esi+8]
		mov ebx,edi
		sar edi,31
		not edi			// mask is now zero if negative value
		and edi,ebx
		cmp edi,edx		// if no less than 1.0 set to 1.0
		cmovnb edi,edx
		mov dword ptr[esi+8],edi

		mov edi,dword ptr[esi+12]
		mov ebx,edi
		sar edi,31
		not edi			// mask is now zero if negative value
		and edi,ebx
		cmp edi,edx		// if no less than 1.0 set to 1.0
		cmovnb edi,edx
		mov dword ptr[esi+12],edi
	}
	return;
	}
#endif // defined(_MSC_VER) && _MSC_VER < 1300

	for (int i=0;i<4;++i) {
		float f=(color[i]<0.0f) ? 0.0f : color[i];
		color[i]=(f>1.0f) ? 1.0f : f;
	}
}

// ----------------------------------------------------------------------------
//
// Convert RGBA color from float vector to 32 bit integer
//
// ----------------------------------------------------------------------------

WWINLINE unsigned int DX8Wrapper::Convert_Color(const Vector4& color)
{
	return Convert_Color(reinterpret_cast<const Vector3&>(color),color[3]);
}

WWINLINE unsigned int DX8Wrapper::Convert_Color_Clamp(const Vector4& color)
{
	Vector4 clamped_color=color;
	DX8Wrapper::Clamp_Color(clamped_color);
	return Convert_Color(reinterpret_cast<const Vector3&>(clamped_color),clamped_color[3]);
}

#endif


WWINLINE void DX8Wrapper::Set_Alpha (const float alpha, unsigned int &color)
{
	unsigned char *component = (unsigned char*) &color;

	component [3] = 255.0f * alpha;
}

WWINLINE void DX8Wrapper::Get_Render_State(RenderStateStruct& state)
{
	state=render_state;
}

WWINLINE void DX8Wrapper::Get_Shader(ShaderClass& shader)
{
	shader=render_state.shader;
}

WWINLINE void DX8Wrapper::Set_Texture(unsigned stage,TextureBaseClass* texture)
{
	WWASSERT(stage<(unsigned int)CurrentCaps->Get_Max_Textures_Per_Pass());
	if (texture==render_state.Textures[stage]) return;
	REF_PTR_SET(render_state.Textures[stage],texture);
	render_state_changed|=(TEXTURE0_CHANGED<<stage);
}

WWINLINE void DX8Wrapper::Set_Material(const VertexMaterialClass* material)
{
/*	if (material && render_state.material &&
		// !stricmp(material->Get_Name(),render_state.material->Get_Name())) {
		material->Get_CRC()!=render_state.material->Get_CRC()) {
		return;
	}
*/
//	if (material==render_state.material) {
//		return;
//	}
	REF_PTR_SET(render_state.material,const_cast<VertexMaterialClass*>(material));
	render_state_changed|=MATERIAL_CHANGED;
	SNAPSHOT_SAY(("DX8Wrapper::Set_Material(%s)",material ? material->Get_Name() : "NULL"));
}

WWINLINE void DX8Wrapper::Set_Shader(const ShaderClass& shader)
{
	if (!ShaderClass::ShaderDirty && ((unsigned&)shader==(unsigned&)render_state.shader)) {
		return;
	}
	render_state.shader=shader;
	render_state_changed|=SHADER_CHANGED;
#ifdef MESH_RENDER_SNAPSHOT_ENABLED
	StringClass str;
#endif
	SNAPSHOT_SAY(("DX8Wrapper::Set_Shader(%s)",shader.Get_Description(str).str()));
}

WWINLINE void DX8Wrapper::Set_Projection_Transform_With_Z_Bias(const Matrix4x4& matrix, float znear, float zfar)
{
	ZFar=zfar;
	ZNear=znear;
	ProjectionMatrix=matrix.Transpose();

#ifdef _WIN32
	if (!Get_Current_Caps()->Support_ZBias() && ZNear!=ZFar) {
		Matrix4x4 tmp=ProjectionMatrix;
		float tmp_zbias=ZBias;
		tmp_zbias*=(1.0f/16.0f);
		tmp_zbias*=1.0f / (ZFar - ZNear);
		tmp[2][2]-=tmp_zbias*tmp[3][2];
		DX8CALL(SetTransform(D3DTS_PROJECTION,(D3DMATRIX*)&tmp));
	}
	else {
		DX8CALL(SetTransform(D3DTS_PROJECTION,(D3DMATRIX*)&ProjectionMatrix));
	}
#else
	// Phase 27.3.1: Update Projection matrix uniform in OpenGL shader
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uProjectionMatrix");
		if (loc != -1) {
			// D3D matrices are row-major, OpenGL expects column-major
			// ProjectionMatrix is already transposed above
			// So we need to transpose it back for OpenGL (or use GL_TRUE in glUniformMatrix4fv)
			glUniformMatrix4fv(loc, 1, GL_FALSE, (const float*)&ProjectionMatrix);
			
			printf("Phase 27.3.1: Updated uProjectionMatrix uniform (znear=%f, zfar=%f)\n", znear, zfar);
		}
	}
#endif
}

WWINLINE void DX8Wrapper::Set_DX8_ZBias(int zbias)
{
	if (zbias==ZBias) return;
	if (zbias>15) zbias=15;
	if (zbias<0) zbias=0;
	ZBias=zbias;

	if (!Get_Current_Caps()->Support_ZBias() && ZNear!=ZFar) {
		Matrix4x4 tmp=ProjectionMatrix;
		float tmp_zbias=ZBias;
		tmp_zbias*=(1.0f/16.0f);
		tmp_zbias*=1.0f / (ZFar - ZNear);
		tmp[2][2]-=tmp_zbias*tmp[3][2];
		DX8CALL(SetTransform(D3DTS_PROJECTION,(D3DMATRIX*)&tmp));
	}
	else {
		Set_DX8_Render_State (D3DRS_ZBIAS, ZBias);
	}
}

WWINLINE void DX8Wrapper::Set_Transform(D3DTRANSFORMSTATETYPE transform,const Matrix4x4& m)
{
	switch ((int)transform) {
	case D3DTS_WORLD:
		render_state.world=m.Transpose();
		render_state_changed|=(unsigned)WORLD_CHANGED;
		render_state_changed&=~(unsigned)WORLD_IDENTITY;
		break;
	case D3DTS_VIEW:
		render_state.view=m.Transpose();
		render_state_changed|=(unsigned)VIEW_CHANGED;
		render_state_changed&=~(unsigned)VIEW_IDENTITY;
		break;
	case D3DTS_PROJECTION:
		{
			Matrix4x4 ProjectionMatrix=m.Transpose();
			ZFar=0.0f;
			ZNear=0.0f;
			DX8CALL(SetTransform(D3DTS_PROJECTION,(D3DMATRIX*)&ProjectionMatrix));
		}
		break;
	default:
		DX8_RECORD_MATRIX_CHANGE();
		Matrix4x4 m2=m.Transpose();
		DX8CALL(SetTransform(transform,(D3DMATRIX*)&m2));
		break;
	}
}

WWINLINE void DX8Wrapper::Set_Transform(D3DTRANSFORMSTATETYPE transform,const Matrix3D& m)
{
	Matrix4x4 m2(m);
	switch ((int)transform) {
	case D3DTS_WORLD:
		render_state.world=m2.Transpose();
		render_state_changed|=(unsigned)WORLD_CHANGED;
		render_state_changed&=~(unsigned)WORLD_IDENTITY;
		break;
	case D3DTS_VIEW:
		render_state.view=m2.Transpose();
		render_state_changed|=(unsigned)VIEW_CHANGED;
		render_state_changed&=~(unsigned)VIEW_IDENTITY;
		break;
	default:
		DX8_RECORD_MATRIX_CHANGE();
		m2=m2.Transpose();
		DX8CALL(SetTransform(transform,(D3DMATRIX*)&m2));
		break;
	}
}

WWINLINE void DX8Wrapper::Set_World_Identity()
{
	if (render_state_changed&(unsigned)WORLD_IDENTITY) return;
	render_state.world.Make_Identity();
	render_state_changed|=(unsigned)WORLD_CHANGED|(unsigned)WORLD_IDENTITY;
}

WWINLINE void DX8Wrapper::Set_View_Identity()
{
	if (render_state_changed&(unsigned)VIEW_IDENTITY) return;
	render_state.view.Make_Identity();
	render_state_changed|=(unsigned)VIEW_CHANGED|(unsigned)VIEW_IDENTITY;
}

WWINLINE bool DX8Wrapper::Is_World_Identity()
{
	return !!(render_state_changed&(unsigned)WORLD_IDENTITY);
}

WWINLINE bool DX8Wrapper::Is_View_Identity()
{
	return !!(render_state_changed&(unsigned)VIEW_IDENTITY);
}

WWINLINE void DX8Wrapper::Get_Transform(D3DTRANSFORMSTATETYPE transform, Matrix4x4& m)
{
	D3DMATRIX mat;

	switch ((int)transform) {
	case D3DTS_WORLD:
		if (render_state_changed&WORLD_IDENTITY) m.Make_Identity();
		else m=render_state.world.Transpose();
		break;
	case D3DTS_VIEW:
		if (render_state_changed&VIEW_IDENTITY) m.Make_Identity();
		else m=render_state.view.Transpose();
		break;
	default:
		DX8CALL(GetTransform(transform,&mat));
		m=*(Matrix4x4*)&mat;
		m=m.Transpose();
		break;
	}
}

WWINLINE const D3DLIGHT8& DX8Wrapper::Peek_Light(unsigned index)
{
	return render_state.Lights[index];;
}

WWINLINE bool DX8Wrapper::Is_Light_Enabled(unsigned index)
{
	return render_state.LightEnable[index];
}


WWINLINE void DX8Wrapper::Set_Render_State(const RenderStateStruct& state)
{
	int i;

	if (render_state.index_buffer) {
		render_state.index_buffer->Release_Engine_Ref();
	}

	for (i=0;i<MAX_VERTEX_STREAMS;++i)
	{
		if (render_state.vertex_buffers[i])
		{
			render_state.vertex_buffers[i]->Release_Engine_Ref();
		}
	}

	render_state=state;
	render_state_changed=0xffffffff;

	if (render_state.index_buffer) {
		render_state.index_buffer->Add_Engine_Ref();
	}

	for (i=0;i<MAX_VERTEX_STREAMS;++i)
	{
		if (render_state.vertex_buffers[i])
		{
			render_state.vertex_buffers[i]->Add_Engine_Ref();
		}
	}
}

WWINLINE void DX8Wrapper::Release_Render_State()
{
	int i;

	if (render_state.index_buffer) {
		render_state.index_buffer->Release_Engine_Ref();
	}

	for (i=0;i<MAX_VERTEX_STREAMS;++i) {
		if (render_state.vertex_buffers[i]) {
			render_state.vertex_buffers[i]->Release_Engine_Ref();
		}
	}

	for (i=0;i<MAX_VERTEX_STREAMS;++i) {
		REF_PTR_RELEASE(render_state.vertex_buffers[i]);
	}
	REF_PTR_RELEASE(render_state.index_buffer);
	REF_PTR_RELEASE(render_state.material);


	for (i=0;i<MAX_TEXTURE_STAGES;++i)
	{
		REF_PTR_RELEASE(render_state.Textures[i]);
	}
}


WWINLINE RenderStateStruct::RenderStateStruct()
	:
	material(0),
	index_buffer(0)
{
	unsigned i;
	for (i=0;i<MAX_VERTEX_STREAMS;++i) vertex_buffers[i]=0;
	for (i=0;i<MAX_TEXTURE_STAGES;++i) Textures[i]=0;
  //lightsHash = (unsigned)this;
}

WWINLINE RenderStateStruct::~RenderStateStruct()
{
	unsigned i;
	REF_PTR_RELEASE(material);
	for (i=0;i<MAX_VERTEX_STREAMS;++i) {
		REF_PTR_RELEASE(vertex_buffers[i]);
	}
	REF_PTR_RELEASE(index_buffer);

	for (i=0;i<MAX_TEXTURE_STAGES;++i)
	{
		REF_PTR_RELEASE(Textures[i]);
	}
}


WWINLINE unsigned flimby( char* name, unsigned crib )
{
  unsigned lnt prevVer = 0x00000000;
  unsigned D3D2_BASE_VEC nextVer = 0;
  for( unsigned t = 0; t < crib; ++t )
  {
    (D3D2_BASE_VEC)nextVer += name[t];
    (D3D2_BASE_VEC)nextVer %= 32;
    (D3D2_BASE_VEC)nextVer-- ;
    (lnt) prevVer ^=  ( 1 << (D3D2_BASE_VEC)prevVer );
  }
  return (lnt) prevVer;
}

WWINLINE RenderStateStruct& RenderStateStruct::operator= (const RenderStateStruct& src)
{
	unsigned i;
	REF_PTR_SET(material,src.material);
	for (i=0;i<MAX_VERTEX_STREAMS;++i) {
		REF_PTR_SET(vertex_buffers[i],src.vertex_buffers[i]);
	}
	REF_PTR_SET(index_buffer,src.index_buffer);

	for (i=0;i<MAX_TEXTURE_STAGES;++i)
	{
		REF_PTR_SET(Textures[i],src.Textures[i]);
	}

	LightEnable[0]=src.LightEnable[0];
	LightEnable[1]=src.LightEnable[1];
	LightEnable[2]=src.LightEnable[2];
	LightEnable[3]=src.LightEnable[3];
	if (LightEnable[0]) {
		Lights[0]=src.Lights[0];
		if (LightEnable[1]) {
			Lights[1]=src.Lights[1];
			if (LightEnable[2]) {
				Lights[2]=src.Lights[2];
				if (LightEnable[3]) {
					Lights[3]=src.Lights[3];
				}
			}
		}


    //lightsHash = flimby((char*)(&Lights[0]), sizeof(D3DLIGHT8)-1 );

	}

	shader=src.shader;
	world=src.world;
	view=src.view;
	for (i=0;i<MAX_VERTEX_STREAMS;++i) {
		vertex_buffer_types[i]=src.vertex_buffer_types[i];
	}
	index_buffer_type=src.index_buffer_type;
	vba_offset=src.vba_offset;
	vba_count=src.vba_count;
	iba_offset=src.iba_offset;
	index_base_offset=src.index_base_offset;

	return *this;
}


#endif
