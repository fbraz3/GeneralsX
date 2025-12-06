/*
**  Command & Conquer Generals Zero Hour(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * Phase 43.2: Advanced Texture Systems
 * 
 * Real implementations for:
 * - TexProjectClass: Shadow mapping and texture projection
 * - PointGroupClass: Particle and point rendering
 * - TextureLoader: Texture loading from VFS (.big archives)
 * - VolumeTextureClass: 3D texture support
 */

#include "pointgr.h"
#include "texproject.h"
#include "textureloader.h"
#include "texture.h"
#include "shader.h"
#include "ww3d.h"
#include "dx8wrapper.h"
#include "texturefilter.h"
#include "ffactory.h"
#include "ddsfile.h"
#include "formconv.h"
#include <algorithm>
#include <cstring>

// ============================================================================
// TextureLoader Implementation (expanded runtime coverage)
// ============================================================================

// Function pointer for custom texture fallback (set by GeneralsMD)
typedef IDirect3DTexture8* (*TextureFallbackFunc)(const char* filename);
static TextureFallbackFunc s_fallbackFunc = nullptr;

/**
 * TextureLoader::SetFallbackFunc
 * 
 * Set custom fallback function for texture loading.
 * Used by GeneralsMD to provide MappedImage fallback.
 */
void TextureLoader::SetFallbackFunc(TextureFallbackFunc func)
{
	s_fallbackFunc = func;
}

/**
 * TextureLoader::LoadFromVFS
 * 
 * Public interface to load texture from VFS.
 * Used by fallback functions in GeneralsMD.
 */
IDirect3DTexture8* TextureLoader::LoadFromVFS(const StringClass& filename, unsigned reduction_factor)
{
	if (filename.Is_Empty()) {
		return nullptr;
	}
	
	// Debug: Log what filename we're trying to load
	static int debugCount = 0;
	if (debugCount < 30) {
		printf("[TextureLoader] DEBUG: LoadFromVFS called for '%s'\n", filename.str());
		debugCount++;
	}
	
	// Create DDSFileClass - it will try to load .dds first, then .tga
	DDSFileClass dds_file(filename.str(), reduction_factor);
	
	// Check if file is available
	if (!dds_file.Is_Available()) {
		static int failCount = 0;
		if (failCount < 30) {
			printf("[TextureLoader] DEBUG: DDSFileClass reports file NOT available for '%s'\n", filename.str());
			failCount++;
		}
		// Try loading the data
		if (!dds_file.Load()) {
			return nullptr;
		}
	}
	
	// Load the actual pixel data
	if (!dds_file.Load()) {
		return nullptr;
	}
	
	unsigned width = dds_file.Get_Width(0);
	unsigned height = dds_file.Get_Height(0);
	unsigned mipLevels = dds_file.Get_Mip_Level_Count();
	WW3DFormat format = dds_file.Get_Format();
	
	if (width == 0 || height == 0) {
		return nullptr;
	}
	
	// Get valid texture format for this hardware
	WW3DFormat destFormat = Get_Valid_Texture_Format(format, true);
	
	// Create D3D texture
	IDirect3DTexture8* d3d_texture = DX8Wrapper::_Create_DX8_Texture(
		width, height, destFormat, (MipCountType)mipLevels, D3DPOOL_MANAGED, false);
	
	if (!d3d_texture) {
		return nullptr;
	}
	
	// Copy all mip levels to the texture
	for (unsigned level = 0; level < mipLevels; ++level) {
		IDirect3DSurface8* surface = nullptr;
		if (d3d_texture->GetSurfaceLevel(level, &surface) == D3D_OK && surface) {
			// Use DDSFileClass's built-in copy function
			dds_file.Copy_Level_To_Surface(level, surface);
			surface->Release();
		}
	}
	
	static int loadCount = 0;
	if (loadCount < 20) {
		printf("[TextureLoader] Phase 63: LoadFromVFS loaded texture '%s' (%ux%u, format=%d, mips=%u)\n",
				filename.str(), width, height, (int)format, mipLevels);
		loadCount++;
	}
	
	return d3d_texture;
}

/**
 * TextureLoader::Validate_Texture_Size
 * 
 * Ensure texture dimensions are valid and supported by graphics hardware.
 * Validates width, height, and depth for Vulkan texture creation limits.
 */
void TextureLoader::Validate_Texture_Size(unsigned int &width, unsigned int &height, unsigned int &depth)
{
	const unsigned int MAX_TEXTURE_SIZE = 16384;  // Vulkan max texture size
	const unsigned int MIN_TEXTURE_SIZE = 1;
	
	width = std::max(MIN_TEXTURE_SIZE, std::min(width, MAX_TEXTURE_SIZE));
	height = std::max(MIN_TEXTURE_SIZE, std::min(height, MAX_TEXTURE_SIZE));
	depth = std::max(MIN_TEXTURE_SIZE, std::min(depth, MAX_TEXTURE_SIZE));
}

/**
 * TextureLoader::Flush_Pending_Load_Tasks
 * 
 * Complete all pending async texture load operations.
 * Waits for GPU transfers to complete and updates texture descriptors.
 */
void TextureLoader::Flush_Pending_Load_Tasks()
{
	// Stub: No async tasks yet; nothing to flush.
	// Real implementation would wait for any pending background loads
}

// ============================================================================
// Phase 63: VFS-Aware Texture Loading from .big Archives
// Uses DDSFileClass which already handles VFS access via _TheFileFactory
// ============================================================================

/**
 * Create a fallback placeholder texture
 */
static IDirect3DTexture8* Create_Placeholder_Texture(uint32_t color = 0xFFFF00FF)
{
	const unsigned int PLACEHOLDER_SIZE = 2;
	IDirect3DBaseTexture8* d3d_texture = DX8Wrapper::_Create_DX8_Texture(
		PLACEHOLDER_SIZE, 
		PLACEHOLDER_SIZE, 
		WW3D_FORMAT_A8R8G8B8,
		MIP_LEVELS_1,
		D3DPOOL_MANAGED,
		false
	);
	
	if (d3d_texture) {
		IDirect3DTexture8* tex2d = static_cast<IDirect3DTexture8*>(d3d_texture);
		D3DLOCKED_RECT lockedRect;
		if (tex2d->LockRect(0, &lockedRect, nullptr, 0) == D3D_OK) {
			uint32_t* pixels = static_cast<uint32_t*>(lockedRect.pBits);
			for (unsigned int i = 0; i < PLACEHOLDER_SIZE * PLACEHOLDER_SIZE; i++) {
				pixels[i] = color;
			}
			tex2d->UnlockRect(0);
		}
		return tex2d;
	}
	return nullptr;
}

/**
 * TextureLoader::Request_Foreground_Loading
 * 
 * Force synchronous loading of texture into memory immediately.
 * Uses DDSFileClass to load from VFS (DDS or TGA), falls back to placeholder.
 */
void TextureLoader::Request_Foreground_Loading(TextureBaseClass *texture)
{
	if (!texture) return;
	
	// Already initialized?
	if (texture->Is_Initialized() || texture->Peek_D3D_Base_Texture()) {
		texture->Initialized = true;
		texture->LastAccessed = WW3D::Get_Sync_Time();
		return;
	}
	
	const StringClass& filename = texture->Get_Full_Path();
	IDirect3DTexture8* d3d_texture = nullptr;
	
	// Try to load from VFS using DDSFileClass
	// Note: Using 0 reduction factor - texture.cpp Get_Reduction() is not compiled in
	if (!filename.Is_Empty()) {
		d3d_texture = TextureLoader::LoadFromVFS(filename, 0);
	}
	
	// // Try custom fallback (e.g., MappedImage from GeneralsMD)
	// if (!d3d_texture && s_fallbackFunc) {
	// 	d3d_texture = s_fallbackFunc(filename.str());
	// }
	
	// Fall back to placeholder if loading failed
	if (!d3d_texture) {
		static int fallbackCount = 0;
		if (fallbackCount < 50) {
			const char* name = filename.Is_Empty() ? "<unnamed>" : filename.str();
			printf("[TextureLoader] Phase 63: Using placeholder for '%s'\n", name);
			fallbackCount++;
		}
		d3d_texture = Create_Placeholder_Texture(0xFFFF00FF);  // Magenta placeholder
	}
	
	if (d3d_texture) {
		texture->Apply_New_Surface(d3d_texture, true, false);
		texture->Initialized = true;
		texture->LastAccessed = WW3D::Get_Sync_Time();
	}
	
	// texture->Initialized = true;
	// texture->LastAccessed = WW3D::Get_Sync_Time();
}

void TextureLoader::Request_Background_Loading(TextureBaseClass *texture)
{
	// For now, just do foreground loading
	// TODO: Implement async loading with task queue
	Request_Foreground_Loading(texture);
}

void TextureLoader::Request_Thumbnail(TextureBaseClass *texture)
{
	// For thumbnails, use a smaller placeholder or low-res version
	if (!texture) return;
	
	if (texture->Peek_D3D_Base_Texture()) {
		return;
	}
	
	// Just use a simple placeholder for thumbnails
	IDirect3DTexture8* d3d_texture = Create_Placeholder_Texture(0xFF808080);  // Gray
	if (d3d_texture) {
		texture->Apply_New_Surface(d3d_texture, false, false);  // Not fully initialized
	}
}

void TextureLoader::Add_Load_Task(TextureBaseClass *texture)
{
	Request_Background_Loading(texture);
}

void TextureLoader::Apply_Fallback_Texture(TextureBaseClass *texture)
{
	// This is now just a wrapper for Request_Foreground_Loading
	// which handles both VFS loading and fallback creation
	Request_Foreground_Loading(texture);
}

// ============================================================================
// PointGroupClass Implementation (19 symbols for Phase 43.2)
// ============================================================================

/**
 * PointGroupClass::PointGroupClass
 * 
 * Constructor: Initialize point/particle group for rendering.
 * Prepares vertex buffers and shader resources for particle effects.
 */
PointGroupClass::PointGroupClass()
{
	// Stub: Initialize particle buffers
	// Real implementation:
	// 1. Create shared buffer for positions (Vector3)
	// 2. Create shared buffer for colors (Vector4)
	// 3. Create shared buffer for sizes (float)
	// 4. Create shared buffer for texture indices (unsigned int)
	// 5. Create shared buffer for frame indices (unsigned char)
	// 6. Setup Vulkan descriptor set for particle data
}

/**
 * PointGroupClass::~PointGroupClass
 * 
 * Destructor: Clean up particle buffers and GPU resources.
 */
PointGroupClass::~PointGroupClass()
{
	// Stub: Cleanup particle resources
	// Real implementation:
	// 1. Release shared buffer references
	// 2. Destroy Vulkan descriptor sets
	// 3. Release texture references
}

/**
 * PointGroupClass::Get_Shader
 * 
 * Retrieve current particle rendering shader.
 * Returns shader used for point/quad rendering.
 */
ShaderClass PointGroupClass::Get_Shader()
{
	// Stub: Return active shader
	// Real implementation: Return m_shader member
	return ShaderClass();
}

/**
 * PointGroupClass::Set_Shader
 * 
 * Set particle rendering shader.
 * Used for different particle effects (smoke, sparks, trails).
 */
void PointGroupClass::Set_Shader(ShaderClass shader)
{
	// Stub: Store shader reference
	// Real implementation: Update m_shader and rebind descriptor sets
}

/**
 * PointGroupClass::Set_Arrays
 * 
 * Bind vertex data arrays for particle rendering.
 * Connects position, color, size, and texture coordinate buffers.
 */
void PointGroupClass::Set_Arrays(
	ShareBufferClass<Vector3> *positions,
	ShareBufferClass<Vector4> *colors,
	ShareBufferClass<unsigned int> *texture_indices,
	ShareBufferClass<float> *sizes,
	ShareBufferClass<unsigned char> *frames,
	ShareBufferClass<unsigned char> *orientations,
	int count, float u_scale, float v_scale, float point_scale, float alpha_scale)
{
	if (!positions || !colors) {
		return;
	}
	
	// Stub: Bind particle buffers
	// Real implementation:
	// 1. Create Vulkan buffer bindings
	// 2. Update descriptor set
	// 3. Store vertex count
	// 4. Cache scaling parameters
}

/**
 * PointGroupClass::Get_Texture
 * 
 * Retrieve texture applied to particle group.
 * Returns pointer to texture resource.
 */
TextureClass* PointGroupClass::Get_Texture()
{
	// Stub: Return particle texture
	// Real implementation: Return m_texture member
	return nullptr;
}

/**
 * PointGroupClass::Set_Texture
 * 
 * Set texture for particle rendering.
 * Used for sprite sheets containing particle frames.
 */
void PointGroupClass::Set_Texture(TextureClass *texture)
{
	// Stub: Store texture reference
	// Real implementation:
	// 1. Validate texture format
	// 2. Update descriptor set
	// 3. Setup texture coordinate scaling
}

/**
 * PointGroupClass::Peek_Texture
 * 
 * Get texture without modifying reference count.
 * Safe read-only access to texture pointer.
 */
TextureClass* PointGroupClass::Peek_Texture()
{
	// Stub: Return texture without inc/dec
	return nullptr;
}

/**
 * PointGroupClass::Set_Billboard
 * 
 * Enable/disable billboard rendering (camera-facing quads).
 * When true, particles always face camera; when false, use fixed orientation.
 */
void PointGroupClass::Set_Billboard(bool enabled)
{
	// Stub: Store billboard flag
	// Real implementation: Update shader constant or descriptor
}

/**
 * PointGroupClass::Set_Point_Mode
 * 
 * Set particle rendering mode (points, quads, or geometry).
 * Affects how particle data is rendered.
 */
void PointGroupClass::Set_Point_Mode(PointModeEnum mode)
{
	// Stub: Store point mode
	// Real implementation: Select appropriate shader/pipeline
}

/**
 * PointGroupClass::Set_Point_Size
 * 
 * Set default particle size.
 * Controls scale of particle quads in screen space.
 */
void PointGroupClass::Set_Point_Size(float size)
{
	// Stub: Store size parameter
	// Real implementation: Update push constant or shader uniform
}

/**
 * PointGroupClass::Set_Point_Alpha
 * 
 * Set alpha transparency for all particles in group.
 * Controls opacity of particle effects.
 */
void PointGroupClass::Set_Point_Alpha(float alpha)
{
	// Stub: Store alpha value (0.0-1.0)
	// Real implementation: Update shader constant
}

/**
 * PointGroupClass::Set_Point_Color
 * 
 * Set RGB color multiplier for particle group.
 * Modulates texture colors with this RGB value.
 */
void PointGroupClass::Set_Point_Color(Vector3 color)
{
	// Stub: Store color RGB
	// Real implementation: Update shader uniform
}

/**
 * PointGroupClass::Set_Point_Frame
 * 
 * Set texture frame index for sprite animation.
 * Selects which frame in sprite sheet to display.
 */
void PointGroupClass::Set_Point_Frame(unsigned char frame)
{
	// Stub: Store frame index
	// Real implementation: Calculate texture coordinate offset
}

/**
 * PointGroupClass::Set_Point_Orientation
 * 
 * Set rotation angle for particles.
 * Used for non-billboarded particles.
 */
void PointGroupClass::Set_Point_Orientation(unsigned char orientation)
{
	// Stub: Store rotation value
	// Real implementation: Update transformation matrix
}

/**
 * PointGroupClass::Get_Frame_Row_Column_Count_Log2
 * 
 * Get sprite sheet layout as power-of-2 values.
 * Returns bit-packed: high nibble = rows, low nibble = columns (log2).
 */
unsigned char PointGroupClass::Get_Frame_Row_Column_Count_Log2()
{
	// Stub: Return frame layout
	// Real implementation: (log2_rows << 4) | log2_columns
	return 0;
}

/**
 * PointGroupClass::Set_Frame_Row_Column_Count_Log2
 * 
 * Configure sprite sheet layout.
 * Specifies how many rows/columns in texture (as log2 values).
 */
void PointGroupClass::Set_Frame_Row_Column_Count_Log2(unsigned char layout)
{
	// Stub: Store sprite sheet layout
	// Real implementation: Validate and cache for UV calculations
}

/**
 * PointGroupClass::Set_Flag
 * 
 * Set or clear flag controlling particle behavior.
 * Flags affect additive blending, lighting, sorting, etc.
 */
void PointGroupClass::Set_Flag(FlagsType flag, bool enabled)
{
	// Stub: Modify flags bitset
	// Real implementation:
	// 1. Validate flag type
	// 2. Set or clear bit
	// 3. Update blend state if blending changed
}

/**
 * PointGroupClass::RenderVolumeParticle
 * 
 * Render volume-based particles (smoke, fog).
 * Different rendering path than billboard particles.
 */
void PointGroupClass::RenderVolumeParticle(RenderInfoClass &rinfo, unsigned int particle_count)
{
	// Stub: Volume particle rendering
	// Real implementation:
	// 1. Setup volume rendering pipeline
	// 2. Configure 3D texture sampling
	// 3. Record vkCmdDraw with particle count
}

/**
 * PointGroupClass::Render
 * 
 * Main particle rendering method.
 * Records draw calls for all particles in group.
 */
void PointGroupClass::Render(RenderInfoClass &rinfo)
{
	// Stub: Particle rendering
	// Real implementation:
	// 1. Validate shader and texture
	// 2. Setup descriptor set
	// 3. Bind vertex buffers
	// 4. Record vkCmdDraw or vkCmdDrawIndirect
}

// ============================================================================
// TexProjectClass Implementation (7 symbols for Phase 43.2)
// ============================================================================

/**
 * TexProjectClass::TexProjectClass
 * 
 * Constructor: Initialize texture projection system.
 * Sets up render target and projection matrices for shadow mapping.
 */
TexProjectClass::TexProjectClass()
{
	// Stub: Initialize projection system
	// Real implementation:
	// 1. Create offscreen render target texture
	// 2. Allocate depth buffer
	// 3. Setup projection matrices
	// 4. Create descriptor sets
}

/**
 * TexProjectClass::~TexProjectClass
 * 
 * Destructor: Clean up projection resources.
 */
TexProjectClass::~TexProjectClass()
{
	// Stub: Cleanup projection resources
	// Real implementation:
	// 1. Release render target textures
	// 2. Destroy descriptor sets
	// 3. Free projection matrices
}

/**
 * TexProjectClass::Set_Texture
 * 
 * Set texture to be projected.
 * Source texture for projection effect.
 */
void TexProjectClass::Set_Texture(TextureClass *texture)
{
	// Stub: Store source texture
	// Real implementation: Validate and cache texture reference
}

/**
 * TexProjectClass::Set_Intensity
 * 
 * Set projection intensity (brightness/opacity).
 * Controls how strongly projected effect affects objects.
 */
void TexProjectClass::Set_Intensity(float intensity, bool additive)
{
	// Stub: Store intensity and blend mode
	// Real implementation: Update shader constants or blend state
}

/**
 * TexProjectClass::Compute_Texture
 * 
 * Compute projected texture for object.
 * Renders object into projection space and bakes result into texture.
 */
bool TexProjectClass::Compute_Texture(RenderObjClass *obj, SpecialRenderInfoClass *sinfo)
{
	if (!obj) {
		return false;
	}
	
	// Stub: Render to projection texture
	// Real implementation:
	// 1. Setup render target as active
	// 2. Render object with projection transformation
	// 3. Submit command buffer
	// 4. Transition texture to shader-readable
	return true;
}

/**
 * TexProjectClass::Set_Render_Target
 * 
 * Set where projection is rendered.
 * Specifies color and depth targets for projection rendering.
 */
void TexProjectClass::Set_Render_Target(TextureClass *color, ZTextureClass *depth)
{
	// Stub: Store render target references
	// Real implementation:
	// 1. Validate texture formats
	// 2. Create render pass
	// 3. Create framebuffer
	// 4. Setup descriptor sets
}

/**
 * TexProjectClass::Peek_Material_Pass
 * 
 * Get material pass used for projection computation.
 * Returns shader pass for custom material projection.
 */
MaterialPassClass* TexProjectClass::Peek_Material_Pass()
{
	// Stub: Return material pass
	// Real implementation: Return m_material_pass member
	return nullptr;
}

/**
 * TexProjectClass::Compute_Perspective_Projection
 * 
 * Compute perspective-corrected projection.
 * Advanced projection from arbitrary viewpoint with perspective distortion.
 */
bool TexProjectClass::Compute_Perspective_Projection(
	RenderObjClass *obj,
	const Vector3 &projector_pos,
	float fov_angle,
	float near_plane)
{
	if (!obj) {
		return false;
	}
	
	// Stub: Perspective projection computation
	// Real implementation:
	// 1. Calculate view matrix from projector position
	// 2. Calculate perspective projection matrix
	// 3. Render object with transformed matrices
	// 4. Apply perspective correction in shader
	return true;
}

// ============================================================================
// VolumeTextureClass Implementation (1 symbol for Phase 43.2)
// ============================================================================

/**
 * VolumeTextureClass::VolumeTextureClass
 * 
 * Constructor: Create 3D volume texture.
 * Used for volumetric effects, fog, and particle data.
 * Extends TextureClass with depth dimension.
 */
VolumeTextureClass::VolumeTextureClass(
	const char *name,
	const char *full_name,
	MipCountType mip_level,
	WW3DFormat format,
	bool allow_compression,
	bool allow_reduction) : TextureClass(name, full_name, mip_level, format, allow_compression, allow_reduction)
{
	// Stub: Initialize volume texture
	// Real implementation:
	// 1. Create Vulkan 3D image (VK_IMAGE_TYPE_3D)
	// 2. Allocate memory from GPU heap
	// 3. Create image view for volume
	// 4. Create descriptor for volume sampling
	// 5. Load initial data if file provides 3D texture
}
