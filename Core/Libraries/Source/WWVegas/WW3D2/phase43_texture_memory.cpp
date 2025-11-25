#include "texture.h"

#include "d3d8_vulkan_graphics_compat.h"
#include "dx8wrapper.h"

static unsigned CalculateTextureMemoryUsage(IDirect3DTexture8 *texture)
{
    if (texture == nullptr) {
        return 0;
    }

    unsigned total_bytes = 0;
    const unsigned levels = texture->GetLevelCount();
    for (unsigned level = 0; level < levels; ++level) {
        D3DSURFACE_DESC desc;
        DX8_ErrorCode(texture->GetLevelDesc(level, &desc));
        total_bytes += desc.Size;
    }

    return total_bytes;
}

unsigned TextureClass::Get_Texture_Memory_Usage() const
{
    return CalculateTextureMemoryUsage(Peek_D3D_Texture());
}

unsigned ZTextureClass::Get_Texture_Memory_Usage() const
{
    return CalculateTextureMemoryUsage(Peek_D3D_Texture());
}
