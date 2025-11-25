#include "texture.h"

#include "d3d8_vulkan_graphics_compat.h"
#include "dx8wrapper.h"
#include "formconv.h"
#include "textureloader.h"
#include "ww3d.h"
#include "statistics.h"

#include <cstring>

namespace {

bool IsCompressedFormat(WW3DFormat format)
{
	switch (format) {
	case WW3D_FORMAT_DXT1:
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
		return true;
	default:
		return false;
	}
}

}

void TextureBaseClass::Load_Locked_Surface()
{
	Set_D3D_Base_Texture(nullptr);
	Initialized = false;
	TextureLoader::Request_Thumbnail(this);
}

void TextureClass::Init()
{
	if (Initialized) {
		return;
	}

	if (!WW3D::Is_Texturing_Enabled()) {
		Set_D3D_Base_Texture(nullptr);
		Initialized = true;
		LastAccessed = WW3D::Get_Sync_Time();
		return;
	}

	if (!WW3D::Get_Thumbnail_Enabled() || MipLevelCount == MIP_LEVELS_1) {
		TextureLoader::Request_Foreground_Loading(this);
	} else {
		TextureLoader::Request_Background_Loading(this);
	}

	LastAccessed = WW3D::Get_Sync_Time();
}

void TextureClass::Apply(unsigned int stage)
{
	if (!WW3D::Is_Texturing_Enabled()) {
		DX8Wrapper::Set_DX8_Texture(stage, nullptr);
		Filter.Apply(stage);
		return;
	}

	if (!Initialized) {
		Init();
	}

	LastAccessed = WW3D::Get_Sync_Time();

	DX8_RECORD_TEXTURE(this);

	DX8Wrapper::Set_DX8_Texture(stage, Peek_D3D_Base_Texture());
	Filter.Apply(stage);
}

void ZTextureClass::Apply(unsigned int stage)
{
	if (!WW3D::Is_Texturing_Enabled()) {
		DX8Wrapper::Set_DX8_Texture(stage, nullptr);
		return;
	}

	if (!Initialized) {
		Init();
	}

	LastAccessed = WW3D::Get_Sync_Time();

	DX8Wrapper::Set_DX8_Texture(stage, Peek_D3D_Base_Texture());
}

void TextureBaseClass::Apply_Null(unsigned int stage)
{
	DX8Wrapper::Set_DX8_Texture(stage, nullptr);
}

void TextureClass::Apply_New_Surface(IDirect3DBaseTexture8* d3d_texture, bool initialized, bool disable_auto_invalidation)
{
	WWASSERT(d3d_texture != nullptr);
	if (d3d_texture == nullptr) {
		return;
	}

	Set_D3D_Base_Texture(d3d_texture);

	if (initialized) {
		Initialized = true;
	}
	if (disable_auto_invalidation) {
		InactivationTime = 0;
	}
	LastAccessed = WW3D::Get_Sync_Time();

	if (!initialized) {
		return;
	}

	IDirect3DSurface8* surface = nullptr;
	IDirect3DTexture8* d3d_texture8 = Peek_D3D_Texture();
	if (d3d_texture8 != nullptr) {
		DX8_ErrorCode(d3d_texture8->GetSurfaceLevel(0, &surface));
	}
	if (surface != nullptr) {
		D3DSURFACE_DESC desc;
		std::memset(&desc, 0, sizeof(desc));
		DX8_ErrorCode(surface->GetDesc(&desc));
		TextureFormat = D3DFormat_To_WW3DFormat(desc.Format);
		Width = desc.Width;
		Height = desc.Height;
		IsCompressionAllowed = IsCompressedFormat(TextureFormat);
		surface->Release();
	}
}

void ZTextureClass::Apply_New_Surface(IDirect3DBaseTexture8* d3d_texture, bool initialized, bool disable_auto_invalidation)
{
	WWASSERT(d3d_texture != nullptr);
	if (d3d_texture == nullptr) {
		return;
	}

	Set_D3D_Base_Texture(d3d_texture);

	if (initialized) {
		Initialized = true;
	}
	if (disable_auto_invalidation) {
		InactivationTime = 0;
	}
	LastAccessed = WW3D::Get_Sync_Time();

	if (!initialized) {
		return;
	}

	IDirect3DSurface8* surface = nullptr;
	IDirect3DTexture8* d3d_texture8 = Peek_D3D_Texture();
	if (d3d_texture8 != nullptr) {
		DX8_ErrorCode(d3d_texture8->GetSurfaceLevel(0, &surface));
	}
	if (surface != nullptr) {
		D3DSURFACE_DESC desc;
		std::memset(&desc, 0, sizeof(desc));
		DX8_ErrorCode(surface->GetDesc(&desc));
		DepthStencilTextureFormat = D3DFormat_To_WW3DZFormat(desc.Format);
		Width = desc.Width;
		Height = desc.Height;
		surface->Release();
	}
}

void CubeTextureClass::Apply_New_Surface(IDirect3DBaseTexture8* d3d_texture, bool initialized, bool disable_auto_invalidation)
{
	WWASSERT(d3d_texture != nullptr);
	if (d3d_texture == nullptr) {
		return;
	}

	Set_D3D_Base_Texture(d3d_texture);

	if (initialized) {
		Initialized = true;
	}
	if (disable_auto_invalidation) {
		InactivationTime = 0;
	}
	LastAccessed = WW3D::Get_Sync_Time();

	if (!initialized) {
		return;
	}

	IDirect3DCubeTexture8* cube_texture = Peek_D3D_CubeTexture();
	if (cube_texture != nullptr) {
		D3DSURFACE_DESC desc;
		std::memset(&desc, 0, sizeof(desc));
		DX8_ErrorCode(cube_texture->GetLevelDesc(0, &desc));
		TextureFormat = D3DFormat_To_WW3DFormat(desc.Format);
		Width = desc.Width;
		Height = desc.Height;
		IsCompressionAllowed = IsCompressedFormat(TextureFormat);
	}
}

void VolumeTextureClass::Apply_New_Surface(IDirect3DBaseTexture8* d3d_texture, bool initialized, bool disable_auto_invalidation)
{
	WWASSERT(d3d_texture != nullptr);
	if (d3d_texture == nullptr) {
		return;
	}

	Set_D3D_Base_Texture(d3d_texture);

	if (initialized) {
		Initialized = true;
	}
	if (disable_auto_invalidation) {
		InactivationTime = 0;
	}
	LastAccessed = WW3D::Get_Sync_Time();

	if (!initialized) {
		return;
	}

	IDirect3DVolumeTexture8* volume_texture = Peek_D3D_VolumeTexture();
	if (volume_texture != nullptr) {
		D3DVOLUME_DESC desc;
		std::memset(&desc, 0, sizeof(desc));
		DX8_ErrorCode(volume_texture->GetLevelDesc(0, &desc));
		TextureFormat = D3DFormat_To_WW3DFormat(desc.Format);
		Width = desc.Width;
		Height = desc.Height;
		Depth = desc.Depth;
		IsCompressionAllowed = IsCompressedFormat(TextureFormat);
	}
}