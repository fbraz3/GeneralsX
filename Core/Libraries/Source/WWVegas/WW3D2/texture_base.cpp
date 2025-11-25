#include "texture.h"
#include "textureloader.h"
#include "DX8Wrapper_Stubs.h"

TextureBaseClass::~TextureBaseClass()
{
    delete TextureLoadTask;
    TextureLoadTask = nullptr;
    delete ThumbnailLoadTask;
    ThumbnailLoadTask = nullptr;

    if (D3DTexture)
    {
        D3DTexture->Release();
        D3DTexture = nullptr;
    }

    DX8TextureManagerClass::Remove(this);
}
