#include "WebGPUDevice/WebGPUD3D8.h"

#include "WebGPUDevice/WebGPUContext.h"

#include <d3d8.h>
#include <emscripten.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
constexpr UINT kDefaultWidth = 1024;
constexpr UINT kDefaultHeight = 768;
constexpr UINT kTextureStageCount = 8;
constexpr UINT kTextureStateCount = 32;
constexpr size_t kUploadFrameCount = 3;
constexpr size_t kVertexUploadCapacity = 32 * 1024 * 1024;
constexpr size_t kIndexUploadCapacity = 4 * 1024 * 1024;
constexpr size_t kUniformUploadCapacity = 2 * 1024 * 1024;
constexpr size_t kUniformOffsetAlignment = 256;

WGPUStringView MakeStringView(const char *value)
{
	return { value, WGPU_STRLEN };
}

template<typename T>
HRESULT ReturnObject(T *object, T **output)
{
	if (!output) {
		return D3DERR_INVALIDCALL;
	}
	*output = object;
	if (object) {
		object->AddRef();
	}
	return D3D_OK;
}

UINT FormatRowPitch(D3DFORMAT format, UINT width)
{
	switch (format) {
		case D3DFMT_DXT1:
			return std::max(1U, (width + 3) / 4) * 8;
		case D3DFMT_DXT2:
		case D3DFMT_DXT3:
		case D3DFMT_DXT4:
		case D3DFMT_DXT5:
			return std::max(1U, (width + 3) / 4) * 16;
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
		case D3DFMT_R8G8B8:
			return width * 4;
		case D3DFMT_R5G6B5:
		case D3DFMT_A4R4G4B4:
		case D3DFMT_X4R4G4B4:
		case D3DFMT_A1R5G5B5:
		case D3DFMT_X1R5G5B5:
		case D3DFMT_A8L8:
			return width * 2;
		default:
			return width;
	}
}

UINT FormatRowCount(D3DFORMAT format, UINT height)
{
	if (format == D3DFMT_DXT1 || format == D3DFMT_DXT2 || format == D3DFMT_DXT3 ||
		format == D3DFMT_DXT4 || format == D3DFMT_DXT5) {
		return std::max(1U, (height + 3) / 4);
	}
	return height;
}

WGPUTextureFormat CompressedTextureFormat(D3DFORMAT format)
{
	switch (format) {
		case D3DFMT_DXT1: return WGPUTextureFormat_BC1RGBAUnorm;
		case D3DFMT_DXT2:
		case D3DFMT_DXT3: return WGPUTextureFormat_BC2RGBAUnorm;
		case D3DFMT_DXT4:
		case D3DFMT_DXT5: return WGPUTextureFormat_BC3RGBAUnorm;
		default: return WGPUTextureFormat_Undefined;
	}
}

void Decode565(uint16_t value, uint8_t *color)
{
	color[0] = static_cast<uint8_t>(((value >> 11) & 31) * 255 / 31);
	color[1] = static_cast<uint8_t>(((value >> 5) & 63) * 255 / 63);
	color[2] = static_cast<uint8_t>((value & 31) * 255 / 31);
	color[3] = 255;
}

void DecodeColorBlock(const uint8_t *block, bool allowTransparent, uint8_t colors[4][4])
{
	const uint16_t first = static_cast<uint16_t>(block[0] | (block[1] << 8));
	const uint16_t second = static_cast<uint16_t>(block[2] | (block[3] << 8));
	Decode565(first, colors[0]);
	Decode565(second, colors[1]);
	if (first > second || !allowTransparent) {
		for (int channel = 0; channel < 3; ++channel) {
			colors[2][channel] = static_cast<uint8_t>((2 * colors[0][channel] + colors[1][channel]) / 3);
			colors[3][channel] = static_cast<uint8_t>((colors[0][channel] + 2 * colors[1][channel]) / 3);
		}
		colors[2][3] = 255;
		colors[3][3] = 255;
	} else {
		for (int channel = 0; channel < 3; ++channel) {
			colors[2][channel] = static_cast<uint8_t>((colors[0][channel] + colors[1][channel]) / 2);
			colors[3][channel] = 0;
		}
		colors[2][3] = 255;
		colors[3][3] = 0;
	}
}

void StoreBlockPixel(std::vector<uint8_t> &rgba, UINT width, UINT height, UINT x, UINT y, const uint8_t *color)
{
	if (x >= width || y >= height) {
		return;
	}
	std::memcpy(rgba.data() + (static_cast<size_t>(y) * width + x) * 4, color, 4);
}

void DecodeDXT(const uint8_t *source, UINT width, UINT height, D3DFORMAT format, std::vector<uint8_t> &rgba)
{
	const bool dxt1 = format == D3DFMT_DXT1;
	const UINT blockBytes = dxt1 ? 8 : 16;
	const UINT blocksWide = std::max(1U, (width + 3) / 4);
	const UINT blocksHigh = std::max(1U, (height + 3) / 4);
	for (UINT blockY = 0; blockY < blocksHigh; ++blockY) {
		for (UINT blockX = 0; blockX < blocksWide; ++blockX) {
			const uint8_t *block = source + (static_cast<size_t>(blockY) * blocksWide + blockX) * blockBytes;
			const uint8_t *colorBlock = dxt1 ? block : block + 8;
			uint8_t colors[4][4] = {};
			DecodeColorBlock(colorBlock, dxt1, colors);
			uint32_t colorIndices = static_cast<uint32_t>(colorBlock[4]) |
				(static_cast<uint32_t>(colorBlock[5]) << 8) |
				(static_cast<uint32_t>(colorBlock[6]) << 16) |
				(static_cast<uint32_t>(colorBlock[7]) << 24);

			uint8_t alpha[16] = {};
			if (dxt1) {
				std::fill(std::begin(alpha), std::end(alpha), 255);
			} else if (format == D3DFMT_DXT2 || format == D3DFMT_DXT3) {
				for (UINT pixel = 0; pixel < 16; ++pixel) {
					alpha[pixel] = static_cast<uint8_t>(((block[pixel / 2] >> ((pixel & 1) * 4)) & 15) * 17);
				}
			} else {
				uint8_t values[8] = { block[0], block[1] };
				if (values[0] > values[1]) {
					for (int i = 1; i <= 6; ++i) {
						values[i + 1] = static_cast<uint8_t>(((7 - i) * values[0] + i * values[1]) / 7);
					}
				} else {
					for (int i = 1; i <= 4; ++i) {
						values[i + 1] = static_cast<uint8_t>(((5 - i) * values[0] + i * values[1]) / 5);
					}
					values[6] = 0;
					values[7] = 255;
				}
				uint64_t indices = 0;
				for (int i = 0; i < 6; ++i) {
					indices |= static_cast<uint64_t>(block[2 + i]) << (i * 8);
				}
				for (UINT pixel = 0; pixel < 16; ++pixel) {
					alpha[pixel] = values[(indices >> (pixel * 3)) & 7];
				}
			}

			for (UINT y = 0; y < 4; ++y) {
				for (UINT x = 0; x < 4; ++x) {
					const UINT pixel = y * 4 + x;
					uint8_t decoded[4];
					std::memcpy(decoded, colors[(colorIndices >> (pixel * 2)) & 3], 4);
					decoded[3] = static_cast<uint8_t>((static_cast<UINT>(decoded[3]) * alpha[pixel]) / 255);
					StoreBlockPixel(rgba, width, height, blockX * 4 + x, blockY * 4 + y, decoded);
				}
			}
		}
	}
}

void ConvertToRGBA(const uint8_t *source, UINT width, UINT height, D3DFORMAT format, std::vector<uint8_t> &rgba)
{
	rgba.assign(static_cast<size_t>(width) * height * 4, 255);
	if (format == D3DFMT_DXT1 || format == D3DFMT_DXT2 || format == D3DFMT_DXT3 ||
		format == D3DFMT_DXT4 || format == D3DFMT_DXT5) {
		DecodeDXT(source, width, height, format, rgba);
		return;
	}

	for (UINT y = 0; y < height; ++y) {
		for (UINT x = 0; x < width; ++x) {
			uint8_t *out = rgba.data() + (static_cast<size_t>(y) * width + x) * 4;
			switch (format) {
				case D3DFMT_A8R8G8B8:
				case D3DFMT_X8R8G8B8: {
					const uint8_t *in = source + (static_cast<size_t>(y) * width + x) * 4;
					out[0] = in[2];
					out[1] = in[1];
					out[2] = in[0];
					out[3] = format == D3DFMT_A8R8G8B8 ? in[3] : 255;
					break;
				}
				case D3DFMT_R5G6B5: {
					const uint16_t value = reinterpret_cast<const uint16_t *>(source)[static_cast<size_t>(y) * width + x];
					Decode565(value, out);
					break;
				}
				case D3DFMT_A4R4G4B4:
				case D3DFMT_X4R4G4B4: {
					const uint16_t value = reinterpret_cast<const uint16_t *>(source)[static_cast<size_t>(y) * width + x];
					out[0] = static_cast<uint8_t>(((value >> 8) & 15) * 17);
					out[1] = static_cast<uint8_t>(((value >> 4) & 15) * 17);
					out[2] = static_cast<uint8_t>((value & 15) * 17);
					out[3] = format == D3DFMT_A4R4G4B4 ? static_cast<uint8_t>(((value >> 12) & 15) * 17) : 255;
					break;
				}
				case D3DFMT_A1R5G5B5:
				case D3DFMT_X1R5G5B5: {
					const uint16_t value = reinterpret_cast<const uint16_t *>(source)[static_cast<size_t>(y) * width + x];
					out[0] = static_cast<uint8_t>(((value >> 10) & 31) * 255 / 31);
					out[1] = static_cast<uint8_t>(((value >> 5) & 31) * 255 / 31);
					out[2] = static_cast<uint8_t>((value & 31) * 255 / 31);
					out[3] = format == D3DFMT_A1R5G5B5 && !(value & 0x8000) ? 0 : 255;
					break;
				}
				case D3DFMT_A8L8: {
					const uint8_t *in = source + (static_cast<size_t>(y) * width + x) * 2;
					out[0] = out[1] = out[2] = in[0];
					out[3] = in[1];
					break;
				}
				case D3DFMT_A8:
					out[0] = out[1] = out[2] = 255;
					out[3] = source[static_cast<size_t>(y) * width + x];
					break;
				case D3DFMT_L8:
				default:
					out[0] = out[1] = out[2] = source[static_cast<size_t>(y) * width + x];
					out[3] = 255;
					break;
			}
		}
	}
}

struct CanonicalVertex
{
	float position[4];
	float color[4];
	float uv[4][2];
	float fog;
};

struct TextureCombinerUniforms
{
	DWORD colorOp0;
	DWORD colorArg10;
	DWORD colorArg20;
	DWORD alphaOp0;
	DWORD alphaArg10;
	DWORD alphaArg20;
	DWORD colorOp1;
	DWORD colorArg11;
	DWORD colorArg21;
	DWORD alphaOp1;
	DWORD alphaArg11;
	DWORD alphaArg21;
	DWORD colorOp2;
	DWORD colorArg12;
	DWORD colorArg22;
	DWORD alphaOp2;
	DWORD alphaArg12;
	DWORD alphaArg22;
	DWORD colorOp3;
	DWORD colorArg13;
	DWORD colorArg23;
	DWORD alphaOp3;
	DWORD alphaArg13;
	DWORD alphaArg23;
	float textureFactor[4];
	float fogColor[4];
	float alphaRef;
	DWORD alphaFunc;
	DWORD alphaTestEnable;
	DWORD padding;
	DWORD pixelShaderMode;
	DWORD pixelShaderTextureCount;
	DWORD shaderPadding0;
	DWORD shaderPadding1;
};

static_assert(sizeof(TextureCombinerUniforms) == 160);

struct PixelShaderProgram
{
	DWORD mode;
	DWORD textureCount;
};

struct PixelShaderInstruction
{
	DWORD opcode;
	std::array<DWORD, 4> parameters;
	UINT parameterCount;
};

struct VertexDeclarationElement
{
	DWORD vertexRegister;
	DWORD dataType;
	UINT offset;
};

struct VertexShaderInstruction
{
	DWORD opcode;
	DWORD destination;
	std::array<DWORD, 3> sources;
	UINT sourceCount;
};

struct VertexShaderProgram
{
	std::vector<DWORD> declaration;
	std::vector<DWORD> function;
	std::vector<VertexDeclarationElement> elements;
	std::vector<VertexShaderInstruction> instructions;
};

class WebGPUD3DDevice;
class WebGPUTexture;

class WebGPUVertexBuffer final : public IDirect3DVertexBuffer8
{
public:
	WebGPUVertexBuffer(WebGPUD3DDevice *device, UINT length, DWORD usage, DWORD fvf, D3DPOOL pool);
	~WebGPUVertexBuffer();

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void **output) override;
	ULONG STDMETHODCALLTYPE AddRef() override;
	ULONG STDMETHODCALLTYPE Release() override;
	HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice8 **device) override;
	HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, const void *, DWORD, DWORD) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, void *, DWORD *) override { return D3DERR_NOTFOUND; }
	HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID) override { return D3DERR_NOTFOUND; }
	DWORD STDMETHODCALLTYPE SetPriority(DWORD priority) override { return std::exchange(m_priority, priority); }
	DWORD STDMETHODCALLTYPE GetPriority() override { return m_priority; }
	void STDMETHODCALLTYPE PreLoad() override {}
	D3DRESOURCETYPE STDMETHODCALLTYPE GetType() override { return D3DRTYPE_VERTEXBUFFER; }
	HRESULT STDMETHODCALLTYPE Lock(UINT offset, UINT size, BYTE **data, DWORD) override;
	HRESULT STDMETHODCALLTYPE Unlock() override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetDesc(D3DVERTEXBUFFER_DESC *desc) override;

	const uint8_t *data() const { return m_data.data(); }
	UINT size() const { return static_cast<UINT>(m_data.size()); }
	DWORD fvf() const { return m_fvf; }

private:
	ULONG m_refs = 1;
	WebGPUD3DDevice *m_device;
	std::vector<uint8_t> m_data;
	DWORD m_usage;
	DWORD m_fvf;
	D3DPOOL m_pool;
	DWORD m_priority = 0;
};

class WebGPUIndexBuffer final : public IDirect3DIndexBuffer8
{
public:
	WebGPUIndexBuffer(WebGPUD3DDevice *device, UINT length, DWORD usage, D3DFORMAT format, D3DPOOL pool);
	~WebGPUIndexBuffer();

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void **output) override;
	ULONG STDMETHODCALLTYPE AddRef() override;
	ULONG STDMETHODCALLTYPE Release() override;
	HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice8 **device) override;
	HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, const void *, DWORD, DWORD) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, void *, DWORD *) override { return D3DERR_NOTFOUND; }
	HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID) override { return D3DERR_NOTFOUND; }
	DWORD STDMETHODCALLTYPE SetPriority(DWORD priority) override { return std::exchange(m_priority, priority); }
	DWORD STDMETHODCALLTYPE GetPriority() override { return m_priority; }
	void STDMETHODCALLTYPE PreLoad() override {}
	D3DRESOURCETYPE STDMETHODCALLTYPE GetType() override { return D3DRTYPE_INDEXBUFFER; }
	HRESULT STDMETHODCALLTYPE Lock(UINT offset, UINT size, BYTE **data, DWORD) override;
	HRESULT STDMETHODCALLTYPE Unlock() override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetDesc(D3DINDEXBUFFER_DESC *desc) override;

	const uint8_t *data() const { return m_data.data(); }
	UINT size() const { return static_cast<UINT>(m_data.size()); }
	D3DFORMAT format() const { return m_format; }

private:
	ULONG m_refs = 1;
	WebGPUD3DDevice *m_device;
	std::vector<uint8_t> m_data;
	DWORD m_usage;
	D3DFORMAT m_format;
	D3DPOOL m_pool;
	DWORD m_priority = 0;
};

struct TextureLevel
{
	UINT width = 0;
	UINT height = 0;
	UINT pitch = 0;
	std::vector<uint8_t> data;
};

class WebGPUSurface final : public IDirect3DSurface8
{
public:
	WebGPUSurface(WebGPUD3DDevice *device, UINT width, UINT height, D3DFORMAT format, DWORD usage, D3DPOOL pool, bool retainDevice = true);
	WebGPUSurface(WebGPUTexture *texture, UINT level);
	~WebGPUSurface();

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void **output) override;
	ULONG STDMETHODCALLTYPE AddRef() override;
	ULONG STDMETHODCALLTYPE Release() override;
	HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice8 **device) override;
	HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, const void *, DWORD, DWORD) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, void *, DWORD *) override { return D3DERR_NOTFOUND; }
	HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID) override { return D3DERR_NOTFOUND; }
	HRESULT STDMETHODCALLTYPE GetContainer(REFIID, void **container) override;
	HRESULT STDMETHODCALLTYPE GetDesc(D3DSURFACE_DESC *desc) override;
	HRESULT STDMETHODCALLTYPE LockRect(D3DLOCKED_RECT *lockedRect, const RECT *rect, DWORD flags) override;
	HRESULT STDMETHODCALLTYPE UnlockRect() override;

	TextureLevel &level();
	const TextureLevel &level() const;
	D3DFORMAT format() const { return m_format; }
	UINT width() const { return level().width; }
	UINT height() const { return level().height; }
	WGPUTextureView renderView();
	void markDirty();

private:
	ULONG m_refs = 1;
	WebGPUD3DDevice *m_device = nullptr;
	WebGPUTexture *m_texture = nullptr;
	UINT m_level = 0;
	D3DFORMAT m_format;
	DWORD m_usage;
	D3DPOOL m_pool;
	TextureLevel m_storage;
	bool m_retainsDevice = true;
	WGPUTexture m_gpuTexture = nullptr;
	WGPUTextureView m_gpuView = nullptr;
};

class WebGPUTexture final : public IDirect3DTexture8
{
public:
	WebGPUTexture(WebGPUD3DDevice *device, UINT width, UINT height, UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool);
	~WebGPUTexture();

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void **output) override;
	ULONG STDMETHODCALLTYPE AddRef() override;
	ULONG STDMETHODCALLTYPE Release() override;
	HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice8 **device) override;
	HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, const void *, DWORD, DWORD) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, void *, DWORD *) override { return D3DERR_NOTFOUND; }
	HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID) override { return D3DERR_NOTFOUND; }
	DWORD STDMETHODCALLTYPE SetPriority(DWORD priority) override { return std::exchange(m_priority, priority); }
	DWORD STDMETHODCALLTYPE GetPriority() override { return m_priority; }
	void STDMETHODCALLTYPE PreLoad() override;
	D3DRESOURCETYPE STDMETHODCALLTYPE GetType() override { return D3DRTYPE_TEXTURE; }
	DWORD STDMETHODCALLTYPE SetLOD(DWORD lod) override { return std::exchange(m_lod, lod); }
	DWORD STDMETHODCALLTYPE GetLOD() override { return m_lod; }
	DWORD STDMETHODCALLTYPE GetLevelCount() override { return static_cast<DWORD>(m_levels.size()); }
	HRESULT STDMETHODCALLTYPE GetLevelDesc(UINT level, D3DSURFACE_DESC *desc) override;
	HRESULT STDMETHODCALLTYPE GetSurfaceLevel(UINT level, IDirect3DSurface8 **surface) override;
	HRESULT STDMETHODCALLTYPE LockRect(UINT level, D3DLOCKED_RECT *lockedRect, const RECT *rect, DWORD flags) override;
	HRESULT STDMETHODCALLTYPE UnlockRect(UINT level) override;
	HRESULT STDMETHODCALLTYPE AddDirtyRect(const RECT *) override { m_dirty = true; return D3D_OK; }

	TextureLevel &level(UINT index) { return m_levels[index]; }
	const TextureLevel &level(UINT index) const { return m_levels[index]; }
	D3DFORMAT format() const { return m_format; }
	bool isDepthStencil() const
	{
		return (m_usage & D3DUSAGE_DEPTHSTENCIL) != 0 ||
			m_format == D3DFMT_D16 ||
			m_format == D3DFMT_D15S1 ||
			m_format == D3DFMT_D24S8 ||
			m_format == D3DFMT_D24X8 ||
			m_format == D3DFMT_D24X4S4 ||
			m_format == D3DFMT_D32;
	}
	WGPUTextureView view();
	WGPUTextureView createRenderView(UINT level);
	void copyFrom(const WebGPUTexture &source);
	void markDirty() { m_dirty = true; }

private:
	bool upload();

	ULONG m_refs = 1;
	WebGPUD3DDevice *m_device;
	std::vector<TextureLevel> m_levels;
	DWORD m_usage;
	D3DFORMAT m_format;
	D3DPOOL m_pool;
	DWORD m_priority = 0;
	DWORD m_lod = 0;
	bool m_dirty = true;
	WGPUTexture m_texture = nullptr;
	WGPUTextureView m_view = nullptr;
};

class WebGPUDirect3D8;
class WebGPUD3DDevice;

WebGPUD3DDevice *g_activeWebGPUDevice = nullptr;

class WebGPUD3DDevice final : public IDirect3DDevice8
{
public:
	WebGPUD3DDevice(WebGPUDirect3D8 *parent, WebGPUContext *context, const D3DPRESENT_PARAMETERS &parameters);
	~WebGPUD3DDevice();
	bool initialize();
	bool canSubmitFrame() const { return !m_uploadFrames[m_nextUploadFrame].busy; }

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void **output) override;
	ULONG STDMETHODCALLTYPE AddRef() override;
	ULONG STDMETHODCALLTYPE Release() override;
	HRESULT STDMETHODCALLTYPE TestCooperativeLevel() override { return D3D_OK; }
	UINT STDMETHODCALLTYPE GetAvailableTextureMem() override { return 256U * 1024U * 1024U; }
	HRESULT STDMETHODCALLTYPE ResourceManagerDiscardBytes(DWORD) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetDirect3D(IDirect3D8 **direct3D) override;
	HRESULT STDMETHODCALLTYPE GetDeviceCaps(D3DCAPS8 *caps) override;
	HRESULT STDMETHODCALLTYPE GetDisplayMode(D3DDISPLAYMODE *mode) override;
	HRESULT STDMETHODCALLTYPE GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *parameters) override;
	HRESULT STDMETHODCALLTYPE SetCursorProperties(UINT, UINT, IDirect3DSurface8 *) override { return D3D_OK; }
	void STDMETHODCALLTYPE SetCursorPosition(UINT, UINT, DWORD) override {}
	WINBOOL STDMETHODCALLTYPE ShowCursor(WINBOOL show) override { return show; }
	HRESULT STDMETHODCALLTYPE CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS *, IDirect3DSwapChain8 **) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE Reset(D3DPRESENT_PARAMETERS *parameters) override;
	HRESULT STDMETHODCALLTYPE Present(const RECT *, const RECT *, HWND, const RGNDATA *) override;
	HRESULT STDMETHODCALLTYPE GetBackBuffer(UINT, D3DBACKBUFFER_TYPE, IDirect3DSurface8 **surface) override;
	HRESULT STDMETHODCALLTYPE GetRasterStatus(D3DRASTER_STATUS *status) override;
	void STDMETHODCALLTYPE SetGammaRamp(DWORD, const D3DGAMMARAMP *) override {}
	void STDMETHODCALLTYPE GetGammaRamp(D3DGAMMARAMP *ramp) override;
	HRESULT STDMETHODCALLTYPE CreateTexture(UINT width, UINT height, UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DTexture8 **texture) override;
	HRESULT STDMETHODCALLTYPE CreateVolumeTexture(UINT, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DVolumeTexture8 **) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE CreateCubeTexture(UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DCubeTexture8 **) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE CreateVertexBuffer(UINT length, DWORD usage, DWORD fvf, D3DPOOL pool, IDirect3DVertexBuffer8 **buffer) override;
	HRESULT STDMETHODCALLTYPE CreateIndexBuffer(UINT length, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DIndexBuffer8 **buffer) override;
	HRESULT STDMETHODCALLTYPE CreateRenderTarget(UINT width, UINT height, D3DFORMAT format, D3DMULTISAMPLE_TYPE, WINBOOL, IDirect3DSurface8 **surface) override;
	HRESULT STDMETHODCALLTYPE CreateDepthStencilSurface(UINT width, UINT height, D3DFORMAT format, D3DMULTISAMPLE_TYPE, IDirect3DSurface8 **surface) override;
	HRESULT STDMETHODCALLTYPE CreateImageSurface(UINT width, UINT height, D3DFORMAT format, IDirect3DSurface8 **surface) override;
	HRESULT STDMETHODCALLTYPE CopyRects(IDirect3DSurface8 *source, const RECT *sourceRects, UINT rectCount, IDirect3DSurface8 *destination, const POINT *destinationPoints) override;
	HRESULT STDMETHODCALLTYPE UpdateTexture(IDirect3DBaseTexture8 *source, IDirect3DBaseTexture8 *destination) override;
	HRESULT STDMETHODCALLTYPE GetFrontBuffer(IDirect3DSurface8 *) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE SetRenderTarget(IDirect3DSurface8 *renderTarget, IDirect3DSurface8 *depthStencil) override;
	HRESULT STDMETHODCALLTYPE GetRenderTarget(IDirect3DSurface8 **renderTarget) override;
	HRESULT STDMETHODCALLTYPE GetDepthStencilSurface(IDirect3DSurface8 **depthStencil) override;
	HRESULT STDMETHODCALLTYPE BeginScene() override { return ensurePass(0, 0, 1.0f, 0); }
	HRESULT STDMETHODCALLTYPE EndScene() override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE Clear(DWORD, const D3DRECT *, DWORD flags, D3DCOLOR color, float z, DWORD stencil) override;
	HRESULT STDMETHODCALLTYPE SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX *matrix) override;
	HRESULT STDMETHODCALLTYPE GetTransform(D3DTRANSFORMSTATETYPE state, D3DMATRIX *matrix) override;
	HRESULT STDMETHODCALLTYPE MultiplyTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX *matrix) override;
	HRESULT STDMETHODCALLTYPE SetViewport(const D3DVIEWPORT8 *viewport) override;
	HRESULT STDMETHODCALLTYPE GetViewport(D3DVIEWPORT8 *viewport) override;
	HRESULT STDMETHODCALLTYPE SetMaterial(const D3DMATERIAL8 *material) override;
	HRESULT STDMETHODCALLTYPE GetMaterial(D3DMATERIAL8 *material) override;
	HRESULT STDMETHODCALLTYPE SetLight(DWORD index, const D3DLIGHT8 *light) override;
	HRESULT STDMETHODCALLTYPE GetLight(DWORD index, D3DLIGHT8 *light) override;
	HRESULT STDMETHODCALLTYPE LightEnable(DWORD index, WINBOOL enable) override;
	HRESULT STDMETHODCALLTYPE GetLightEnable(DWORD index, WINBOOL *enable) override;
	HRESULT STDMETHODCALLTYPE SetClipPlane(DWORD, const float *) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetClipPlane(DWORD, float *) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE SetRenderState(D3DRENDERSTATETYPE state, DWORD value) override;
	HRESULT STDMETHODCALLTYPE GetRenderState(D3DRENDERSTATETYPE state, DWORD *value) override;
	HRESULT STDMETHODCALLTYPE BeginStateBlock() override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE EndStateBlock(DWORD *token) override { if (token) *token = 1; return token ? D3D_OK : D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE ApplyStateBlock(DWORD) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE CaptureStateBlock(DWORD) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE DeleteStateBlock(DWORD) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE CreateStateBlock(D3DSTATEBLOCKTYPE, DWORD *token) override { if (token) *token = 1; return token ? D3D_OK : D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE SetClipStatus(const D3DCLIPSTATUS8 *) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetClipStatus(D3DCLIPSTATUS8 *) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE GetTexture(DWORD stage, IDirect3DBaseTexture8 **texture) override;
	HRESULT STDMETHODCALLTYPE SetTexture(DWORD stage, IDirect3DBaseTexture8 *texture) override;
	HRESULT STDMETHODCALLTYPE GetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD *value) override;
	HRESULT STDMETHODCALLTYPE SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value) override;
	HRESULT STDMETHODCALLTYPE ValidateDevice(DWORD *passes) override { if (passes) *passes = 1; return passes ? D3D_OK : D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE GetInfo(DWORD, void *, DWORD) override { return S_FALSE; }
	HRESULT STDMETHODCALLTYPE SetPaletteEntries(UINT, const PALETTEENTRY *) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE GetPaletteEntries(UINT, PALETTEENTRY *) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE SetCurrentTexturePalette(UINT) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE GetCurrentTexturePalette(UINT *) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE DrawPrimitive(D3DPRIMITIVETYPE type, UINT startVertex, UINT primitiveCount) override;
	HRESULT STDMETHODCALLTYPE DrawIndexedPrimitive(D3DPRIMITIVETYPE type, UINT minIndex, UINT vertexCount, UINT startIndex, UINT primitiveCount) override;
	HRESULT STDMETHODCALLTYPE DrawPrimitiveUP(D3DPRIMITIVETYPE type, UINT primitiveCount, const void *data, UINT stride) override;
	HRESULT STDMETHODCALLTYPE DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE type, UINT minVertexIndex, UINT vertexCount, UINT primitiveCount, const void *indexData, D3DFORMAT indexFormat, const void *data, UINT stride) override;
	HRESULT STDMETHODCALLTYPE ProcessVertices(UINT, UINT, UINT, IDirect3DVertexBuffer8 *, DWORD) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE CreateVertexShader(const DWORD *declaration, const DWORD *function, DWORD *handle, DWORD usage) override;
	HRESULT STDMETHODCALLTYPE SetVertexShader(DWORD handle) override;
	HRESULT STDMETHODCALLTYPE GetVertexShader(DWORD *handle) override { if (handle) *handle = m_vertexShader; return handle ? D3D_OK : D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE DeleteVertexShader(DWORD handle) override;
	HRESULT STDMETHODCALLTYPE SetVertexShaderConstant(DWORD startRegister, const void *data, DWORD count) override;
	HRESULT STDMETHODCALLTYPE GetVertexShaderConstant(DWORD startRegister, void *data, DWORD count) override;
	HRESULT STDMETHODCALLTYPE GetVertexShaderDeclaration(DWORD handle, void *data, DWORD *size) override;
	HRESULT STDMETHODCALLTYPE GetVertexShaderFunction(DWORD handle, void *data, DWORD *size) override;
	HRESULT STDMETHODCALLTYPE SetStreamSource(UINT stream, IDirect3DVertexBuffer8 *buffer, UINT stride) override;
	HRESULT STDMETHODCALLTYPE GetStreamSource(UINT stream, IDirect3DVertexBuffer8 **buffer, UINT *stride) override;
	HRESULT STDMETHODCALLTYPE SetIndices(IDirect3DIndexBuffer8 *indices, UINT baseVertexIndex) override;
	HRESULT STDMETHODCALLTYPE GetIndices(IDirect3DIndexBuffer8 **indices, UINT *baseVertexIndex) override;
	HRESULT STDMETHODCALLTYPE CreatePixelShader(const DWORD *function, DWORD *handle) override;
	HRESULT STDMETHODCALLTYPE SetPixelShader(DWORD handle) override;
	HRESULT STDMETHODCALLTYPE GetPixelShader(DWORD *handle) override { if (handle) *handle = m_pixelShader; return handle ? D3D_OK : D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE DeletePixelShader(DWORD handle) override;
	HRESULT STDMETHODCALLTYPE SetPixelShaderConstant(DWORD, const void *, DWORD) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetPixelShaderConstant(DWORD, void *, DWORD) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE GetPixelShaderFunction(DWORD, void *, DWORD *) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE DrawRectPatch(UINT, const float *, const D3DRECTPATCH_INFO *) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE DrawTriPatch(UINT, const float *, const D3DTRIPATCH_INFO *) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE DeletePatch(UINT) override { return D3D_OK; }

	WGPUDevice webGPUDevice() const { return m_context->device(); }
	WGPUQueue webGPUQueue() const { return m_context->queue(); }

private:
	struct DrawBindings
	{
		WGPUBindGroup group = nullptr;
		uint32_t uniformOffset = 0;
	};

	struct UploadFrame
	{
		WGPUBuffer vertexBuffer = nullptr;
		WGPUBuffer indexBuffer = nullptr;
		WGPUBuffer uniformBuffer = nullptr;
		std::vector<CanonicalVertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<uint8_t> uniforms;
		std::map<std::array<uintptr_t, 8>, WGPUBindGroup> bindGroups;
		bool busy = false;
	};

	friend class WebGPUDirect3D8;
	HRESULT ensurePass(DWORD clearFlags, D3DCOLOR color, float depth, DWORD stencil);
	void finishPass();
	void releaseFrame();
	bool prepareUploadFrame();
	void releaseUploadFrame(UploadFrame &frame);
	void createDepthBuffer();
	uint32_t renderSampleCount() const;
	WGPURenderPipeline pipeline(D3DPRIMITIVETYPE type);
	DrawBindings bindGroup();
	WGPUSampler sampler(UINT stage);
	WGPUTextureFormat colorTargetFormat() const;
	bool hasDepthTarget() const;
	HRESULT draw(D3DPRIMITIVETYPE type, const void *vertexData, UINT vertexCount, UINT stride, const void *indexData, D3DFORMAT indexFormat, UINT indexCount, UINT baseVertex);
	CanonicalVertex convertVertex(const uint8_t *source, UINT stride) const;
	CanonicalVertex executeVertexShader(const VertexShaderProgram &program, const uint8_t *source, UINT stride) const;
	UINT vertexCountFor(D3DPRIMITIVETYPE type, UINT primitiveCount) const;
	WGPUBlendFactor blendFactor(DWORD factor) const;
	WGPUCompareFunction compareFunction(DWORD function) const;
	WGPUStencilOperation stencilOperation(DWORD operation) const;
	D3DMATRIX identityMatrix() const;
	static void onSubmittedWorkDone(WGPUQueueWorkDoneStatus status, WGPUStringView message, void *userdata1, void *userdata2);

	ULONG m_refs = 1;
	WebGPUDirect3D8 *m_parent;
	WebGPUContext *m_context;
	D3DPRESENT_PARAMETERS m_parameters;
	D3DDEVICE_CREATION_PARAMETERS m_creation = {};
	D3DVIEWPORT8 m_viewport = {};
	D3DMATERIAL8 m_material = {};
	std::unordered_map<DWORD, D3DMATRIX> m_transforms;
	std::unordered_map<DWORD, D3DLIGHT8> m_lights;
	std::unordered_map<DWORD, WINBOOL> m_lightEnabled;
	std::array<DWORD, 256> m_renderStates = {};
	std::array<std::array<DWORD, kTextureStateCount>, kTextureStageCount> m_textureStates = {};
	std::array<WebGPUTexture *, kTextureStageCount> m_textures = {};
	WebGPUVertexBuffer *m_vertexBuffer = nullptr;
	WebGPUIndexBuffer *m_indexBuffer = nullptr;
	UINT m_vertexStride = 0;
	UINT m_baseVertex = 0;
	DWORD m_vertexShader = D3DFVF_XYZ;
	DWORD m_nextVertexShader = 1;
	std::unordered_map<DWORD, VertexShaderProgram> m_vertexShaders;
	std::array<std::array<float, 4>, 96> m_vertexShaderConstants = {};
	DWORD m_pixelShader = 0;
	DWORD m_nextPixelShader = 1;
	std::unordered_map<DWORD, PixelShaderProgram> m_pixelShaders;
	WebGPUSurface *m_backBuffer = nullptr;
	WebGPUSurface *m_depthSurface = nullptr;
	WebGPUSurface *m_renderTarget = nullptr;
	WebGPUSurface *m_depthTarget = nullptr;
	WGPUShaderModule m_shader = nullptr;
	WGPUBindGroupLayout m_bindGroupLayout = nullptr;
	WGPUPipelineLayout m_pipelineLayout = nullptr;
	std::unordered_map<uint64_t, WGPUSampler> m_samplers;
	WGPUTexture m_whiteTexture = nullptr;
	WGPUTextureView m_whiteView = nullptr;
	WGPUTexture m_depthTexture = nullptr;
	WGPUTextureView m_depthView = nullptr;
	WGPUTexture m_multisampleTexture = nullptr;
	WGPUTextureView m_multisampleView = nullptr;
	std::unordered_map<uint64_t, WGPURenderPipeline> m_pipelines;
	WGPUSurfaceTexture m_surfaceTexture = WGPU_SURFACE_TEXTURE_INIT;
	WGPUTextureView m_surfaceView = nullptr;
	WGPUCommandEncoder m_encoder = nullptr;
	WGPURenderPassEncoder m_pass = nullptr;
	bool m_frameLoaded = false;
	std::array<UploadFrame, kUploadFrameCount> m_uploadFrames;
	size_t m_activeUploadFrame = 0;
	size_t m_nextUploadFrame = 0;
	unsigned int m_pendingSubmissions = 0;
};

class WebGPUDirect3D8 final : public IDirect3D8
{
public:
	WebGPUDirect3D8() = default;
	~WebGPUDirect3D8() = default;
	bool initialize();

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void **output) override;
	ULONG STDMETHODCALLTYPE AddRef() override { return ++m_refs; }
	ULONG STDMETHODCALLTYPE Release() override;
	HRESULT STDMETHODCALLTYPE RegisterSoftwareDevice(void *) override { return D3DERR_INVALIDCALL; }
	UINT STDMETHODCALLTYPE GetAdapterCount() override { return 1; }
	HRESULT STDMETHODCALLTYPE GetAdapterIdentifier(UINT adapter, DWORD, D3DADAPTER_IDENTIFIER8 *identifier) override;
	UINT STDMETHODCALLTYPE GetAdapterModeCount(UINT adapter) override { return adapter == 0 ? 6 : 0; }
	HRESULT STDMETHODCALLTYPE EnumAdapterModes(UINT adapter, UINT mode, D3DDISPLAYMODE *displayMode) override;
	HRESULT STDMETHODCALLTYPE GetAdapterDisplayMode(UINT adapter, D3DDISPLAYMODE *displayMode) override;
	HRESULT STDMETHODCALLTYPE CheckDeviceType(UINT adapter, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, WINBOOL) override { return adapter == 0 ? D3D_OK : D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE CheckDeviceFormat(UINT adapter, D3DDEVTYPE, D3DFORMAT, DWORD usage, D3DRESOURCETYPE type, D3DFORMAT format) override;
	HRESULT STDMETHODCALLTYPE CheckDeviceMultiSampleType(UINT, D3DDEVTYPE, D3DFORMAT, WINBOOL, D3DMULTISAMPLE_TYPE type) override { return type == D3DMULTISAMPLE_NONE || type == D3DMULTISAMPLE_4_SAMPLES ? D3D_OK : D3DERR_NOTAVAILABLE; }
	HRESULT STDMETHODCALLTYPE CheckDepthStencilMatch(UINT adapter, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, D3DFORMAT depthFormat) override;
	HRESULT STDMETHODCALLTYPE GetDeviceCaps(UINT adapter, D3DDEVTYPE type, D3DCAPS8 *caps) override;
	HMONITOR STDMETHODCALLTYPE GetAdapterMonitor(UINT) override { return nullptr; }
	HRESULT STDMETHODCALLTYPE CreateDevice(UINT adapter, D3DDEVTYPE type, HWND window, DWORD behavior, D3DPRESENT_PARAMETERS *parameters, IDirect3DDevice8 **device) override;

	WebGPUContext &context() { return m_context; }

private:
	ULONG m_refs = 1;
	WebGPUContext m_context;
};

} // namespace

namespace
{
void WebGPUD3DDevice::createDepthBuffer()
{
	if (m_multisampleView) {
		wgpuTextureViewRelease(m_multisampleView);
		m_multisampleView = nullptr;
	}
	if (m_multisampleTexture) {
		wgpuTextureRelease(m_multisampleTexture);
		m_multisampleTexture = nullptr;
	}
	if (m_depthView) {
		wgpuTextureViewRelease(m_depthView);
		m_depthView = nullptr;
	}
	if (m_depthTexture) {
		wgpuTextureRelease(m_depthTexture);
		m_depthTexture = nullptr;
	}
	const uint32_t sampleCount = m_parameters.MultiSampleType == D3DMULTISAMPLE_4_SAMPLES ? 4 : 1;
	if (sampleCount > 1) {
		WGPUTextureDescriptor colorDescriptor = WGPU_TEXTURE_DESCRIPTOR_INIT;
		colorDescriptor.usage = WGPUTextureUsage_RenderAttachment;
		colorDescriptor.dimension = WGPUTextureDimension_2D;
		colorDescriptor.size = { m_parameters.BackBufferWidth, m_parameters.BackBufferHeight, 1 };
		colorDescriptor.format = m_context->surfaceFormat();
		colorDescriptor.sampleCount = sampleCount;
		m_multisampleTexture = wgpuDeviceCreateTexture(webGPUDevice(), &colorDescriptor);
		m_multisampleView = m_multisampleTexture ? wgpuTextureCreateView(m_multisampleTexture, nullptr) : nullptr;
	}
	WGPUTextureDescriptor descriptor = WGPU_TEXTURE_DESCRIPTOR_INIT;
	descriptor.usage = WGPUTextureUsage_RenderAttachment;
	descriptor.dimension = WGPUTextureDimension_2D;
	descriptor.size = { m_parameters.BackBufferWidth, m_parameters.BackBufferHeight, 1 };
	descriptor.format = WGPUTextureFormat_Depth24PlusStencil8;
	descriptor.sampleCount = sampleCount;
	m_depthTexture = wgpuDeviceCreateTexture(webGPUDevice(), &descriptor);
	m_depthView = m_depthTexture ? wgpuTextureCreateView(m_depthTexture, nullptr) : nullptr;
}

uint32_t WebGPUD3DDevice::renderSampleCount() const
{
	const bool customColorTarget = m_renderTarget && m_renderTarget != m_backBuffer;
	return !customColorTarget && m_parameters.MultiSampleType == D3DMULTISAMPLE_4_SAMPLES ? 4 : 1;
}

HRESULT WebGPUD3DDevice::ensurePass(DWORD clearFlags, D3DCOLOR color, float depth, DWORD stencil)
{
	if (clearFlags != 0 && m_pass) {
		finishPass();
	}
	if (!m_surfaceTexture.texture) {
		if (!prepareUploadFrame()) {
			return D3DERR_DEVICELOST;
		}
		m_surfaceTexture = WGPU_SURFACE_TEXTURE_INIT;
		wgpuSurfaceGetCurrentTexture(m_context->surface(), &m_surfaceTexture);
		if (m_surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
			m_surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
			return D3DERR_DEVICELOST;
		}
		m_surfaceView = wgpuTextureCreateView(m_surfaceTexture.texture, nullptr);
		m_encoder = wgpuDeviceCreateCommandEncoder(webGPUDevice(), nullptr);
		m_frameLoaded = false;
	}
	if (m_pass) {
		return D3D_OK;
	}

	const bool customColorTarget = m_renderTarget && m_renderTarget != m_backBuffer;
	WGPUTextureView colorView = customColorTarget ? m_renderTarget->renderView() :
		(renderSampleCount() > 1 ? m_multisampleView : m_surfaceView);
	WGPUTextureView depthView = nullptr;
	if (customColorTarget) {
		if (m_depthTarget && m_depthTarget->width() == m_renderTarget->width() && m_depthTarget->height() == m_renderTarget->height()) {
			depthView = m_depthTarget == m_depthSurface ? m_depthView : m_depthTarget->renderView();
		}
	} else {
		depthView = !m_depthTarget || m_depthTarget == m_depthSurface ? m_depthView : m_depthTarget->renderView();
	}
	if (!colorView || (hasDepthTarget() && !depthView)) {
		return D3DERR_DRIVERINTERNALERROR;
	}

	const bool mustClearColor = !m_frameLoaded || (clearFlags & D3DCLEAR_TARGET) != 0;
	const bool mustClearDepth = !m_frameLoaded || (clearFlags & D3DCLEAR_ZBUFFER) != 0;
	const bool mustClearStencil = !m_frameLoaded || (clearFlags & D3DCLEAR_STENCIL) != 0;
	WGPURenderPassColorAttachment colorAttachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
	colorAttachment.view = colorView;
	colorAttachment.resolveTarget = !customColorTarget && renderSampleCount() > 1 ? m_surfaceView : nullptr;
	colorAttachment.loadOp = mustClearColor ? WGPULoadOp_Clear : WGPULoadOp_Load;
	colorAttachment.storeOp = WGPUStoreOp_Store;
	colorAttachment.clearValue = {
		static_cast<double>((color >> 16) & 0xff) / 255.0,
		static_cast<double>((color >> 8) & 0xff) / 255.0,
		static_cast<double>(color & 0xff) / 255.0,
		static_cast<double>((color >> 24) & 0xff) / 255.0,
	};
	WGPURenderPassDepthStencilAttachment depthAttachment = WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT;
	depthAttachment.view = depthView;
	depthAttachment.depthLoadOp = mustClearDepth ? WGPULoadOp_Clear : WGPULoadOp_Load;
	depthAttachment.depthStoreOp = WGPUStoreOp_Store;
	depthAttachment.depthClearValue = depth;
	depthAttachment.depthReadOnly = WGPU_FALSE;
	depthAttachment.stencilLoadOp = mustClearStencil ? WGPULoadOp_Clear : WGPULoadOp_Load;
	depthAttachment.stencilStoreOp = WGPUStoreOp_Store;
	depthAttachment.stencilClearValue = stencil;
	WGPURenderPassDescriptor passDescriptor = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
	passDescriptor.colorAttachmentCount = 1;
	passDescriptor.colorAttachments = &colorAttachment;
	passDescriptor.depthStencilAttachment = depthView ? &depthAttachment : nullptr;
	m_pass = wgpuCommandEncoderBeginRenderPass(m_encoder, &passDescriptor);
	if (!m_pass) {
		return D3DERR_DRIVERINTERNALERROR;
	}
	wgpuRenderPassEncoderSetViewport(m_pass, static_cast<float>(m_viewport.X), static_cast<float>(m_viewport.Y), static_cast<float>(m_viewport.Width), static_cast<float>(m_viewport.Height), m_viewport.MinZ, m_viewport.MaxZ);
	m_frameLoaded = true;
	return D3D_OK;
}

void WebGPUD3DDevice::finishPass()
{
	if (!m_pass) {
		return;
	}
	wgpuRenderPassEncoderEnd(m_pass);
	wgpuRenderPassEncoderRelease(m_pass);
	m_pass = nullptr;
}

void WebGPUD3DDevice::releaseFrame()
{
	finishPass();
	if (m_encoder) {
		wgpuCommandEncoderRelease(m_encoder);
		m_encoder = nullptr;
	}
	if (m_surfaceView) {
		wgpuTextureViewRelease(m_surfaceView);
		m_surfaceView = nullptr;
	}
	if (m_surfaceTexture.texture) {
		wgpuTextureRelease(m_surfaceTexture.texture);
		m_surfaceTexture = WGPU_SURFACE_TEXTURE_INIT;
	}
	m_frameLoaded = false;
}

bool WebGPUD3DDevice::prepareUploadFrame()
{
	UploadFrame &frame = m_uploadFrames[m_nextUploadFrame];
	if (frame.busy) {
		return false;
	}
	for (const auto &[key, group] : frame.bindGroups) {
		(void)key;
		wgpuBindGroupRelease(group);
	}
	frame.bindGroups.clear();
	frame.vertices.clear();
	frame.indices.clear();
	frame.uniforms.clear();
	m_activeUploadFrame = m_nextUploadFrame;
	return true;
}

void WebGPUD3DDevice::releaseUploadFrame(UploadFrame &frame)
{
	for (const auto &[key, group] : frame.bindGroups) {
		(void)key;
		wgpuBindGroupRelease(group);
	}
	frame.bindGroups.clear();
	if (frame.indexBuffer) {
		wgpuBufferRelease(frame.indexBuffer);
		frame.indexBuffer = nullptr;
	}
	if (frame.uniformBuffer) {
		wgpuBufferRelease(frame.uniformBuffer);
		frame.uniformBuffer = nullptr;
	}
	if (frame.vertexBuffer) {
		wgpuBufferRelease(frame.vertexBuffer);
		frame.vertexBuffer = nullptr;
	}
}

WGPUBlendFactor WebGPUD3DDevice::blendFactor(DWORD factor) const
{
	switch (factor) {
		case D3DBLEND_ZERO: return WGPUBlendFactor_Zero;
		case D3DBLEND_ONE: return WGPUBlendFactor_One;
		case D3DBLEND_SRCCOLOR: return WGPUBlendFactor_Src;
		case D3DBLEND_INVSRCCOLOR: return WGPUBlendFactor_OneMinusSrc;
		case D3DBLEND_SRCALPHA: return WGPUBlendFactor_SrcAlpha;
		case D3DBLEND_INVSRCALPHA: return WGPUBlendFactor_OneMinusSrcAlpha;
		case D3DBLEND_DESTALPHA: return WGPUBlendFactor_DstAlpha;
		case D3DBLEND_INVDESTALPHA: return WGPUBlendFactor_OneMinusDstAlpha;
		case D3DBLEND_DESTCOLOR: return WGPUBlendFactor_Dst;
		case D3DBLEND_INVDESTCOLOR: return WGPUBlendFactor_OneMinusDst;
		default: return WGPUBlendFactor_One;
	}
}

WGPUCompareFunction WebGPUD3DDevice::compareFunction(DWORD function) const
{
	switch (function) {
		case D3DCMP_NEVER: return WGPUCompareFunction_Never;
		case D3DCMP_LESS: return WGPUCompareFunction_Less;
		case D3DCMP_EQUAL: return WGPUCompareFunction_Equal;
		case D3DCMP_LESSEQUAL: return WGPUCompareFunction_LessEqual;
		case D3DCMP_GREATER: return WGPUCompareFunction_Greater;
		case D3DCMP_NOTEQUAL: return WGPUCompareFunction_NotEqual;
		case D3DCMP_GREATEREQUAL: return WGPUCompareFunction_GreaterEqual;
		default: return WGPUCompareFunction_Always;
	}
}

WGPUStencilOperation WebGPUD3DDevice::stencilOperation(DWORD operation) const
{
	switch (operation) {
		case D3DSTENCILOP_ZERO: return WGPUStencilOperation_Zero;
		case D3DSTENCILOP_REPLACE: return WGPUStencilOperation_Replace;
		case D3DSTENCILOP_INCRSAT: return WGPUStencilOperation_IncrementClamp;
		case D3DSTENCILOP_DECRSAT: return WGPUStencilOperation_DecrementClamp;
		case D3DSTENCILOP_INVERT: return WGPUStencilOperation_Invert;
		case D3DSTENCILOP_INCR: return WGPUStencilOperation_IncrementWrap;
		case D3DSTENCILOP_DECR: return WGPUStencilOperation_DecrementWrap;
		default: return WGPUStencilOperation_Keep;
	}
}

WGPUTextureFormat WebGPUD3DDevice::colorTargetFormat() const
{
	return m_renderTarget && m_renderTarget != m_backBuffer ? WGPUTextureFormat_RGBA8Unorm : m_context->surfaceFormat();
}

bool WebGPUD3DDevice::hasDepthTarget() const
{
	if (!m_renderTarget || m_renderTarget == m_backBuffer) {
		return true;
	}
	return m_depthTarget && m_depthTarget->width() == m_renderTarget->width() && m_depthTarget->height() == m_renderTarget->height();
}

WGPURenderPipeline WebGPUD3DDevice::pipeline(D3DPRIMITIVETYPE type)
{
	WGPUPrimitiveTopology topology = WGPUPrimitiveTopology_TriangleList;
	uint64_t topologyKey = 2;
	switch (type) {
		case D3DPT_POINTLIST:
			topology = WGPUPrimitiveTopology_PointList;
			topologyKey = 0;
			break;
		case D3DPT_LINELIST:
		case D3DPT_LINESTRIP:
			topology = WGPUPrimitiveTopology_LineList;
			topologyKey = 1;
			break;
		default:
			topology = WGPUPrimitiveTopology_TriangleList;
			topologyKey = 2;
			break;
	}
	const uint64_t key = topologyKey |
		(static_cast<uint64_t>(m_renderStates[D3DRS_ALPHABLENDENABLE] != FALSE) << 2) |
		(static_cast<uint64_t>(m_renderStates[D3DRS_ZENABLE] != D3DZB_FALSE) << 3) |
		(static_cast<uint64_t>(m_renderStates[D3DRS_ZWRITEENABLE] != FALSE) << 4) |
		(static_cast<uint64_t>(m_renderStates[D3DRS_CULLMODE] & 3) << 5) |
		(static_cast<uint64_t>(m_renderStates[D3DRS_SRCBLEND] & 15) << 7) |
		(static_cast<uint64_t>(m_renderStates[D3DRS_DESTBLEND] & 15) << 11) |
		(static_cast<uint64_t>(colorTargetFormat()) & 63) << 15 |
		(static_cast<uint64_t>(m_renderStates[D3DRS_COLORWRITEENABLE] & 15) << 21) |
		(static_cast<uint64_t>(hasDepthTarget()) << 25) |
		(static_cast<uint64_t>(m_renderStates[D3DRS_ZFUNC] & 15) << 26) |
		(static_cast<uint64_t>(m_renderStates[D3DRS_STENCILENABLE] != FALSE) << 30) |
		(static_cast<uint64_t>(m_renderStates[D3DRS_STENCILFAIL] & 15) << 31) |
		(static_cast<uint64_t>(m_renderStates[D3DRS_STENCILZFAIL] & 15) << 35) |
		(static_cast<uint64_t>(m_renderStates[D3DRS_STENCILPASS] & 15) << 39) |
		(static_cast<uint64_t>(m_renderStates[D3DRS_STENCILFUNC] & 15) << 43) |
		(static_cast<uint64_t>(m_renderStates[D3DRS_STENCILMASK] & 0xff) << 47) |
		(static_cast<uint64_t>(m_renderStates[D3DRS_STENCILWRITEMASK] & 0xff) << 55) |
		(static_cast<uint64_t>(renderSampleCount() > 1) << 63);
	const auto existing = m_pipelines.find(key);
	if (existing != m_pipelines.end()) {
		return existing->second;
	}

	std::array<WGPUVertexAttribute, 7> attributes = {
		WGPU_VERTEX_ATTRIBUTE_INIT,
		WGPU_VERTEX_ATTRIBUTE_INIT,
		WGPU_VERTEX_ATTRIBUTE_INIT,
		WGPU_VERTEX_ATTRIBUTE_INIT,
		WGPU_VERTEX_ATTRIBUTE_INIT,
		WGPU_VERTEX_ATTRIBUTE_INIT,
		WGPU_VERTEX_ATTRIBUTE_INIT,
	};
	attributes[0].format = WGPUVertexFormat_Float32x4;
	attributes[0].offset = offsetof(CanonicalVertex, position);
	attributes[0].shaderLocation = 0;
	attributes[1].format = WGPUVertexFormat_Float32x4;
	attributes[1].offset = offsetof(CanonicalVertex, color);
	attributes[1].shaderLocation = 1;
	attributes[2].format = WGPUVertexFormat_Float32x2;
	attributes[2].offset = offsetof(CanonicalVertex, uv);
	attributes[2].shaderLocation = 2;
	attributes[3].format = WGPUVertexFormat_Float32x2;
	attributes[3].offset = offsetof(CanonicalVertex, uv) + sizeof(float) * 2;
	attributes[3].shaderLocation = 3;
	attributes[4].format = WGPUVertexFormat_Float32x2;
	attributes[4].offset = offsetof(CanonicalVertex, uv) + sizeof(float) * 4;
	attributes[4].shaderLocation = 4;
	attributes[5].format = WGPUVertexFormat_Float32x2;
	attributes[5].offset = offsetof(CanonicalVertex, uv) + sizeof(float) * 6;
	attributes[5].shaderLocation = 5;
	attributes[6].format = WGPUVertexFormat_Float32;
	attributes[6].offset = offsetof(CanonicalVertex, fog);
	attributes[6].shaderLocation = 6;
	WGPUVertexBufferLayout vertexLayout = WGPU_VERTEX_BUFFER_LAYOUT_INIT;
	vertexLayout.stepMode = WGPUVertexStepMode_Vertex;
	vertexLayout.arrayStride = sizeof(CanonicalVertex);
	vertexLayout.attributeCount = attributes.size();
	vertexLayout.attributes = attributes.data();

	WGPUBlendState blend = WGPU_BLEND_STATE_INIT;
	blend.color.operation = WGPUBlendOperation_Add;
	blend.color.srcFactor = blendFactor(m_renderStates[D3DRS_SRCBLEND]);
	blend.color.dstFactor = blendFactor(m_renderStates[D3DRS_DESTBLEND]);
	blend.alpha = blend.color;
	WGPUColorTargetState target = WGPU_COLOR_TARGET_STATE_INIT;
	target.format = colorTargetFormat();
	target.blend = m_renderStates[D3DRS_ALPHABLENDENABLE] ? &blend : nullptr;
	target.writeMask = WGPUColorWriteMask_None;
	if (m_renderStates[D3DRS_COLORWRITEENABLE] & D3DCOLORWRITEENABLE_RED) target.writeMask |= WGPUColorWriteMask_Red;
	if (m_renderStates[D3DRS_COLORWRITEENABLE] & D3DCOLORWRITEENABLE_GREEN) target.writeMask |= WGPUColorWriteMask_Green;
	if (m_renderStates[D3DRS_COLORWRITEENABLE] & D3DCOLORWRITEENABLE_BLUE) target.writeMask |= WGPUColorWriteMask_Blue;
	if (m_renderStates[D3DRS_COLORWRITEENABLE] & D3DCOLORWRITEENABLE_ALPHA) target.writeMask |= WGPUColorWriteMask_Alpha;
	WGPUFragmentState fragment = WGPU_FRAGMENT_STATE_INIT;
	fragment.module = m_shader;
	fragment.entryPoint = MakeStringView("fragment_main");
	fragment.targetCount = 1;
	fragment.targets = &target;

	WGPUDepthStencilState depth = WGPU_DEPTH_STENCIL_STATE_INIT;
	depth.format = WGPUTextureFormat_Depth24PlusStencil8;
	depth.depthWriteEnabled = m_renderStates[D3DRS_ZWRITEENABLE] ? WGPUOptionalBool_True : WGPUOptionalBool_False;
	depth.depthCompare = m_renderStates[D3DRS_ZENABLE] == D3DZB_FALSE
		? WGPUCompareFunction_Always : compareFunction(m_renderStates[D3DRS_ZFUNC]);
	depth.stencilFront.compare = m_renderStates[D3DRS_STENCILENABLE]
		? compareFunction(m_renderStates[D3DRS_STENCILFUNC]) : WGPUCompareFunction_Always;
	depth.stencilFront.failOp = stencilOperation(m_renderStates[D3DRS_STENCILFAIL]);
	depth.stencilFront.depthFailOp = stencilOperation(m_renderStates[D3DRS_STENCILZFAIL]);
	depth.stencilFront.passOp = stencilOperation(m_renderStates[D3DRS_STENCILPASS]);
	depth.stencilBack = depth.stencilFront;
	depth.stencilReadMask = m_renderStates[D3DRS_STENCILENABLE] ? m_renderStates[D3DRS_STENCILMASK] & 0xff : 0xff;
	depth.stencilWriteMask = m_renderStates[D3DRS_STENCILENABLE] ? m_renderStates[D3DRS_STENCILWRITEMASK] & 0xff : 0;
	WGPURenderPipelineDescriptor descriptor = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
	descriptor.layout = m_pipelineLayout;
	descriptor.vertex.module = m_shader;
	descriptor.vertex.entryPoint = MakeStringView("vertex_main");
	descriptor.vertex.bufferCount = 1;
	descriptor.vertex.buffers = &vertexLayout;
	descriptor.primitive.topology = topology;
	// GeneralsX @port Codex 04/08/2026 Match Direct3D screen-space winding after converting Y-down coordinates to WebGPU NDC.
	descriptor.primitive.frontFace = WGPUFrontFace_CW;
	switch (m_renderStates[D3DRS_CULLMODE]) {
		case D3DCULL_CW: descriptor.primitive.cullMode = WGPUCullMode_Front; break;
		case D3DCULL_CCW: descriptor.primitive.cullMode = WGPUCullMode_Back; break;
		default: descriptor.primitive.cullMode = WGPUCullMode_None; break;
	}
	descriptor.depthStencil = hasDepthTarget() ? &depth : nullptr;
	descriptor.multisample.count = renderSampleCount();
	descriptor.multisample.mask = 0xffffffff;
	descriptor.multisample.alphaToCoverageEnabled = WGPU_FALSE;
	descriptor.fragment = &fragment;
	WGPURenderPipeline created = wgpuDeviceCreateRenderPipeline(webGPUDevice(), &descriptor);
	if (created) {
		m_pipelines.emplace(key, created);
	}
	return created;
}

WebGPUD3DDevice::DrawBindings WebGPUD3DDevice::bindGroup()
{
	UploadFrame &frame = m_uploadFrames[m_activeUploadFrame];
	std::array<WGPUTextureView, 4> textureViews = {};
	std::array<WGPUSampler, 4> samplers = {};
	for (UINT stage = 0; stage < textureViews.size(); ++stage) {
		textureViews[stage] = m_whiteView;
		if (m_textures[stage] && (m_pixelShader || m_textureStates[stage][D3DTSS_COLOROP] != D3DTOP_DISABLE)) {
			textureViews[stage] = m_textures[stage]->view();
		}
		if (!textureViews[stage]) {
			textureViews[stage] = m_whiteView;
		}
		samplers[stage] = sampler(stage);
		if (!samplers[stage]) {
			return {};
		}
	}
	std::array<uintptr_t, 8> bindingKey = {};
	for (UINT stage = 0; stage < textureViews.size(); ++stage) {
		bindingKey[stage * 2] = reinterpret_cast<uintptr_t>(samplers[stage]);
		bindingKey[stage * 2 + 1] = reinterpret_cast<uintptr_t>(textureViews[stage]);
	}
	PixelShaderProgram pixelShader = {};
	if (m_pixelShader) {
		const auto program = m_pixelShaders.find(m_pixelShader);
		if (program == m_pixelShaders.end()) {
			return {};
		}
		pixelShader = program->second;
	}
	const DWORD textureFactor = m_renderStates[D3DRS_TEXTUREFACTOR];
	const DWORD fogColor = m_renderStates[D3DRS_FOGCOLOR];
	TextureCombinerUniforms uniforms = {
		m_textureStates[0][D3DTSS_COLOROP],
		m_textureStates[0][D3DTSS_COLORARG1],
		m_textureStates[0][D3DTSS_COLORARG2],
		m_textureStates[0][D3DTSS_ALPHAOP],
		m_textureStates[0][D3DTSS_ALPHAARG1],
		m_textureStates[0][D3DTSS_ALPHAARG2],
		m_textureStates[1][D3DTSS_COLOROP],
		m_textureStates[1][D3DTSS_COLORARG1],
		m_textureStates[1][D3DTSS_COLORARG2],
		m_textureStates[1][D3DTSS_ALPHAOP],
		m_textureStates[1][D3DTSS_ALPHAARG1],
		m_textureStates[1][D3DTSS_ALPHAARG2],
		m_textureStates[2][D3DTSS_COLOROP],
		m_textureStates[2][D3DTSS_COLORARG1],
		m_textureStates[2][D3DTSS_COLORARG2],
		m_textureStates[2][D3DTSS_ALPHAOP],
		m_textureStates[2][D3DTSS_ALPHAARG1],
		m_textureStates[2][D3DTSS_ALPHAARG2],
		m_textureStates[3][D3DTSS_COLOROP],
		m_textureStates[3][D3DTSS_COLORARG1],
		m_textureStates[3][D3DTSS_COLORARG2],
		m_textureStates[3][D3DTSS_ALPHAOP],
		m_textureStates[3][D3DTSS_ALPHAARG1],
		m_textureStates[3][D3DTSS_ALPHAARG2],
		{
			static_cast<float>((textureFactor >> 16) & 0xff) / 255.0f,
			static_cast<float>((textureFactor >> 8) & 0xff) / 255.0f,
			static_cast<float>(textureFactor & 0xff) / 255.0f,
			static_cast<float>((textureFactor >> 24) & 0xff) / 255.0f,
		},
		{
			static_cast<float>((fogColor >> 16) & 0xff) / 255.0f,
			static_cast<float>((fogColor >> 8) & 0xff) / 255.0f,
			static_cast<float>(fogColor & 0xff) / 255.0f,
			1.0f,
		},
		static_cast<float>(m_renderStates[D3DRS_ALPHAREF] & 0xff) / 255.0f,
		m_renderStates[D3DRS_ALPHAFUNC],
		m_renderStates[D3DRS_ALPHATESTENABLE],
		0,
		pixelShader.mode,
		pixelShader.textureCount,
		0,
		0,
	};
	const size_t uniformOffset = (frame.uniforms.size() + kUniformOffsetAlignment - 1) & ~(kUniformOffsetAlignment - 1);
	if (uniformOffset + sizeof(uniforms) > kUniformUploadCapacity) {
		return {};
	}
	frame.uniforms.resize(uniformOffset + sizeof(uniforms));
	std::memcpy(frame.uniforms.data() + uniformOffset, &uniforms, sizeof(uniforms));
	const auto existing = frame.bindGroups.find(bindingKey);
	if (existing != frame.bindGroups.end()) {
		return { existing->second, static_cast<uint32_t>(uniformOffset) };
	}
	std::array<WGPUBindGroupEntry, 9> entries = {
		WGPU_BIND_GROUP_ENTRY_INIT,
		WGPU_BIND_GROUP_ENTRY_INIT,
		WGPU_BIND_GROUP_ENTRY_INIT,
		WGPU_BIND_GROUP_ENTRY_INIT,
		WGPU_BIND_GROUP_ENTRY_INIT,
		WGPU_BIND_GROUP_ENTRY_INIT,
		WGPU_BIND_GROUP_ENTRY_INIT,
		WGPU_BIND_GROUP_ENTRY_INIT,
		WGPU_BIND_GROUP_ENTRY_INIT,
	};
	for (UINT stage = 0; stage < textureViews.size(); ++stage) {
		entries[stage * 2].binding = stage * 2;
		entries[stage * 2].sampler = samplers[stage];
		entries[stage * 2 + 1].binding = stage * 2 + 1;
		entries[stage * 2 + 1].textureView = textureViews[stage];
	}
	entries[8].binding = 8;
	entries[8].buffer = frame.uniformBuffer;
	entries[8].offset = 0;
	entries[8].size = sizeof(uniforms);
	WGPUBindGroupDescriptor descriptor = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
	descriptor.layout = m_bindGroupLayout;
	descriptor.entryCount = entries.size();
	descriptor.entries = entries.data();
	WGPUBindGroup group = wgpuDeviceCreateBindGroup(webGPUDevice(), &descriptor);
	if (group) {
		frame.bindGroups.emplace(bindingKey, group);
	}
	return { group, static_cast<uint32_t>(uniformOffset) };
}

WGPUSampler WebGPUD3DDevice::sampler(UINT stage)
{
	const auto &states = m_textureStates[stage];
	const DWORD addressU = states[D3DTSS_ADDRESSU];
	const DWORD addressV = states[D3DTSS_ADDRESSV];
	const DWORD magFilter = states[D3DTSS_MAGFILTER];
	const DWORD minFilter = states[D3DTSS_MINFILTER];
	const DWORD mipFilter = states[D3DTSS_MIPFILTER];
	const DWORD maxMipLevel = states[D3DTSS_MAXMIPLEVEL];
	const DWORD maxAnisotropy = std::clamp(states[D3DTSS_MAXANISOTROPY], DWORD{ 1 }, DWORD{ 16 });
	const uint64_t key = static_cast<uint64_t>(addressU & 7) |
		(static_cast<uint64_t>(addressV & 7) << 3) |
		(static_cast<uint64_t>(magFilter & 7) << 6) |
		(static_cast<uint64_t>(minFilter & 7) << 9) |
		(static_cast<uint64_t>(mipFilter & 7) << 12) |
		(static_cast<uint64_t>(maxMipLevel & 15) << 15) |
		(static_cast<uint64_t>(maxAnisotropy & 31) << 19);
	const auto existing = m_samplers.find(key);
	if (existing != m_samplers.end()) {
		return existing->second;
	}

	auto addressMode = [](DWORD address) {
		switch (address) {
			case D3DTADDRESS_MIRROR: return WGPUAddressMode_MirrorRepeat;
			case D3DTADDRESS_CLAMP:
			case D3DTADDRESS_BORDER:
			case D3DTADDRESS_MIRRORONCE: return WGPUAddressMode_ClampToEdge;
			default: return WGPUAddressMode_Repeat;
		}
	};
	auto filterMode = [](DWORD filter) {
		return filter == D3DTEXF_LINEAR || filter == D3DTEXF_ANISOTROPIC
			? WGPUFilterMode_Linear : WGPUFilterMode_Nearest;
	};
	const bool anisotropic = minFilter == D3DTEXF_ANISOTROPIC;
	WGPUSamplerDescriptor descriptor = WGPU_SAMPLER_DESCRIPTOR_INIT;
	descriptor.addressModeU = addressMode(addressU);
	descriptor.addressModeV = addressMode(addressV);
	descriptor.addressModeW = WGPUAddressMode_Repeat;
	descriptor.magFilter = anisotropic ? WGPUFilterMode_Linear : filterMode(magFilter);
	descriptor.minFilter = anisotropic ? WGPUFilterMode_Linear : filterMode(minFilter);
	descriptor.mipmapFilter = mipFilter == D3DTEXF_LINEAR
		? WGPUMipmapFilterMode_Linear : WGPUMipmapFilterMode_Nearest;
	descriptor.lodMinClamp = static_cast<float>(maxMipLevel);
	descriptor.lodMaxClamp = mipFilter == D3DTEXF_NONE ? static_cast<float>(maxMipLevel) : 32.0f;
	descriptor.maxAnisotropy = anisotropic ? maxAnisotropy : 1;
	WGPUSampler created = wgpuDeviceCreateSampler(webGPUDevice(), &descriptor);
	if (created) {
		m_samplers.emplace(key, created);
	}
	return created;
}

UINT WebGPUD3DDevice::vertexCountFor(D3DPRIMITIVETYPE type, UINT primitiveCount) const
{
	switch (type) {
		case D3DPT_POINTLIST: return primitiveCount;
		case D3DPT_LINELIST: return primitiveCount * 2;
		case D3DPT_LINESTRIP: return primitiveCount + 1;
		case D3DPT_TRIANGLELIST: return primitiveCount * 3;
		case D3DPT_TRIANGLESTRIP:
		case D3DPT_TRIANGLEFAN: return primitiveCount + 2;
		default: return 0;
	}
}

D3DMATRIX WebGPUD3DDevice::identityMatrix() const
{
	D3DMATRIX matrix = {};
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

CanonicalVertex WebGPUD3DDevice::executeVertexShader(const VertexShaderProgram &program, const uint8_t *source, UINT stride) const
{
	using Vector = std::array<float, 4>;
	std::array<Vector, 16> inputs = {};
	std::array<Vector, 12> temporaries = {};
	std::array<Vector, 3> rasterOutputs = {};
	std::array<Vector, 2> attributeOutputs = {};
	std::array<Vector, 8> textureOutputs = {};
	Vector address = {};
	rasterOutputs[0][3] = 1.0f;
	attributeOutputs[0] = { 1.0f, 1.0f, 1.0f, 1.0f };
	for (const VertexDeclarationElement &element : program.elements) {
		if (element.vertexRegister >= inputs.size() || element.offset >= stride) continue;
		Vector value = { 0.0f, 0.0f, 0.0f, 1.0f };
		const uint8_t *data = source + element.offset;
		switch (element.dataType) {
			case D3DVSDT_FLOAT1:
			case D3DVSDT_FLOAT2:
			case D3DVSDT_FLOAT3:
			case D3DVSDT_FLOAT4: {
				const UINT count = element.dataType + 1;
				if (element.offset + count * sizeof(float) > stride) continue;
				std::copy_n(reinterpret_cast<const float *>(data), count, value.begin());
				break;
			}
			case D3DVSDT_D3DCOLOR: {
				if (element.offset + sizeof(DWORD) > stride) continue;
				const DWORD color = *reinterpret_cast<const DWORD *>(data);
				value = {
					static_cast<float>((color >> 16) & 0xff) / 255.0f,
					static_cast<float>((color >> 8) & 0xff) / 255.0f,
					static_cast<float>(color & 0xff) / 255.0f,
					static_cast<float>((color >> 24) & 0xff) / 255.0f
				};
				break;
			}
			case D3DVSDT_UBYTE4:
				if (element.offset + 4 > stride) continue;
				for (UINT component = 0; component < 4; ++component) value[component] = data[component];
				break;
			case D3DVSDT_SHORT2:
			case D3DVSDT_SHORT4: {
				const UINT count = element.dataType == D3DVSDT_SHORT2 ? 2 : 4;
				if (element.offset + count * sizeof(int16_t) > stride) continue;
				const auto *values = reinterpret_cast<const int16_t *>(data);
				for (UINT component = 0; component < count; ++component) value[component] = values[component];
				break;
			}
			default:
				continue;
		}
		inputs[element.vertexRegister] = value;
	}
	auto readSource = [&](DWORD token, int constantOffset = 0) {
		Vector value = {};
		const DWORD type = token & D3DSP_REGTYPE_MASK;
		UINT index = token & 0x7ff;
		if (type == D3DSPR_TEMP && index < temporaries.size()) value = temporaries[index];
		else if (type == D3DSPR_INPUT && index < inputs.size()) value = inputs[index];
		else if (type == D3DSPR_CONST) {
			if (token & D3DVS_ADDRESSMODE_MASK) index += static_cast<UINT>(std::max(0.0f, std::floor(address[0] + 0.5f)));
			index += constantOffset;
			if (index < m_vertexShaderConstants.size()) value = m_vertexShaderConstants[index];
		} else if (type == D3DSPR_ADDR) {
			value = address;
		}
		Vector swizzled = {};
		for (UINT component = 0; component < 4; ++component) {
			swizzled[component] = value[(token >> (D3DVS_SWIZZLE_SHIFT + component * 2)) & 3];
		}
		if ((token & D3DSP_SRCMOD_MASK) == D3DSPSM_NEG) {
			for (float &component : swizzled) component = -component;
		}
		return swizzled;
	};
	auto writeDestination = [&](DWORD token, const Vector &value) {
		Vector *destination = nullptr;
		const DWORD type = token & D3DSP_REGTYPE_MASK;
		const UINT index = token & 0x7ff;
		if (type == D3DSPR_TEMP && index < temporaries.size()) destination = &temporaries[index];
		else if (type == D3DSPR_ADDR) destination = &address;
		else if (type == D3DSPR_RASTOUT && index < rasterOutputs.size()) destination = &rasterOutputs[index];
		else if (type == D3DSPR_ATTROUT && index < attributeOutputs.size()) destination = &attributeOutputs[index];
		else if (type == D3DSPR_TEXCRDOUT && index < textureOutputs.size()) destination = &textureOutputs[index];
		if (!destination) return;
		for (UINT component = 0; component < 4; ++component) {
			if (token & (D3DSP_WRITEMASK_0 << component)) (*destination)[component] = value[component];
		}
	};
	for (const VertexShaderInstruction &instruction : program.instructions) {
		Vector result = {};
		const Vector first = readSource(instruction.sources[0]);
		const Vector second = instruction.sourceCount > 1 ? readSource(instruction.sources[1]) : Vector{};
		const Vector third = instruction.sourceCount > 2 ? readSource(instruction.sources[2]) : Vector{};
		switch (instruction.opcode) {
			case D3DSIO_MOV:
				result = first;
				break;
			case D3DSIO_ADD:
				for (UINT component = 0; component < 4; ++component) result[component] = first[component] + second[component];
				break;
			case D3DSIO_SUB:
				for (UINT component = 0; component < 4; ++component) result[component] = first[component] - second[component];
				break;
			case D3DSIO_MUL:
				for (UINT component = 0; component < 4; ++component) result[component] = first[component] * second[component];
				break;
			case D3DSIO_MAD:
				for (UINT component = 0; component < 4; ++component) result[component] = first[component] * second[component] + third[component];
				break;
			case D3DSIO_M4x4:
				for (UINT row = 0; row < 4; ++row) {
					const Vector matrixRow = readSource(instruction.sources[1], row);
					for (UINT component = 0; component < 4; ++component) result[row] += first[component] * matrixRow[component];
				}
				break;
			default:
				continue;
		}
		writeDestination(instruction.destination, result);
	}
	CanonicalVertex output = {};
	std::copy(rasterOutputs[0].begin(), rasterOutputs[0].end(), output.position);
	output.position[0] += output.position[3] / static_cast<float>(m_viewport.Width);
	output.position[1] -= output.position[3] / static_cast<float>(m_viewport.Height);
	std::copy(attributeOutputs[0].begin(), attributeOutputs[0].end(), output.color);
	for (UINT stage = 0; stage < 4; ++stage) {
		output.uv[stage][0] = textureOutputs[stage][0];
		output.uv[stage][1] = textureOutputs[stage][1];
	}
	output.fog = rasterOutputs[1][0] == 0.0f ? 1.0f : rasterOutputs[1][0];
	return output;
}

CanonicalVertex WebGPUD3DDevice::convertVertex(const uint8_t *source, UINT stride) const
{
	const auto programmableShader = m_vertexShaders.find(m_vertexShader);
	if (programmableShader != m_vertexShaders.end()) {
		return executeVertexShader(programmableShader->second, source, stride);
	}
	CanonicalVertex output = {};
	std::fill(std::begin(output.color), std::end(output.color), 1.0f);
	output.fog = 1.0f;
	const DWORD fvf = m_vertexShader ? m_vertexShader : (m_vertexBuffer ? m_vertexBuffer->fvf() : D3DFVF_XYZ);
	const DWORD positionType = fvf & D3DFVF_POSITION_MASK;
	UINT offset = 0;
	std::array<float, 4> cameraPosition = {};
	const auto worldTransform = m_transforms.find(D3DTS_WORLD);
	const auto viewTransform = m_transforms.find(D3DTS_VIEW);
	const D3DMATRIX &worldMatrix = worldTransform == m_transforms.end() ? identityMatrix() : worldTransform->second;
	const D3DMATRIX &viewMatrix = viewTransform == m_transforms.end() ? identityMatrix() : viewTransform->second;
	auto transform = [](const std::array<float, 4> &input, const D3DMATRIX &matrix) {
		std::array<float, 4> result = {};
		for (int column = 0; column < 4; ++column) {
			for (int row = 0; row < 4; ++row) {
				result[column] += input[row] * matrix.m[row][column];
			}
		}
		return result;
	};
	if (positionType == D3DFVF_XYZRHW) {
		const float *position = reinterpret_cast<const float *>(source);
		// GeneralsX @port Codex 04/08/2026 Convert screen-space vertices against the active render-target viewport.
		output.position[0] = ((position[0] - static_cast<float>(m_viewport.X)) / static_cast<float>(m_viewport.Width)) * 2.0f - 1.0f;
		output.position[1] = 1.0f - ((position[1] - static_cast<float>(m_viewport.Y)) / static_cast<float>(m_viewport.Height)) * 2.0f;
		output.position[2] = position[2];
		output.position[3] = 1.0f;
		offset = 4 * sizeof(float);
	} else {
		std::array<float, 4> position = {
			reinterpret_cast<const float *>(source)[0],
			reinterpret_cast<const float *>(source)[1],
			reinterpret_cast<const float *>(source)[2],
			1.0f,
		};
		const auto projection = m_transforms.find(D3DTS_PROJECTION);
		position = transform(position, worldMatrix);
		cameraPosition = transform(position, viewMatrix);
		position = transform(cameraPosition, projection == m_transforms.end() ? identityMatrix() : projection->second);
		std::copy(position.begin(), position.end(), output.position);
		offset = 3 * sizeof(float);
		if (positionType >= D3DFVF_XYZB1 && positionType <= D3DFVF_XYZB5) {
			offset += ((positionType - D3DFVF_XYZB1) / 2 + 1) * sizeof(float);
		}
	}
	output.position[0] += output.position[3] / static_cast<float>(m_viewport.Width);
	output.position[1] -= output.position[3] / static_cast<float>(m_viewport.Height);
	std::array<float, 3> cameraNormal = {};
	if (fvf & D3DFVF_NORMAL) {
		const float *normal = reinterpret_cast<const float *>(source + offset);
		float worldView[3][3] = {};
		for (int row = 0; row < 3; ++row) {
			for (int column = 0; column < 3; ++column) {
				for (int index = 0; index < 3; ++index) {
					worldView[row][column] += worldMatrix.m[row][index] * viewMatrix.m[index][column];
				}
			}
		}
		const float determinant =
			worldView[0][0] * (worldView[1][1] * worldView[2][2] - worldView[1][2] * worldView[2][1]) -
			worldView[0][1] * (worldView[1][0] * worldView[2][2] - worldView[1][2] * worldView[2][0]) +
			worldView[0][2] * (worldView[1][0] * worldView[2][1] - worldView[1][1] * worldView[2][0]);
		if (std::abs(determinant) > 1.0e-20f) {
			const float inverseDeterminant = 1.0f / determinant;
			const float inverse[3][3] = {
				{ (worldView[1][1] * worldView[2][2] - worldView[1][2] * worldView[2][1]) * inverseDeterminant,
				  (worldView[0][2] * worldView[2][1] - worldView[0][1] * worldView[2][2]) * inverseDeterminant,
				  (worldView[0][1] * worldView[1][2] - worldView[0][2] * worldView[1][1]) * inverseDeterminant },
				{ (worldView[1][2] * worldView[2][0] - worldView[1][0] * worldView[2][2]) * inverseDeterminant,
				  (worldView[0][0] * worldView[2][2] - worldView[0][2] * worldView[2][0]) * inverseDeterminant,
				  (worldView[0][2] * worldView[1][0] - worldView[0][0] * worldView[1][2]) * inverseDeterminant },
				{ (worldView[1][0] * worldView[2][1] - worldView[1][1] * worldView[2][0]) * inverseDeterminant,
				  (worldView[0][1] * worldView[2][0] - worldView[0][0] * worldView[2][1]) * inverseDeterminant,
				  (worldView[0][0] * worldView[1][1] - worldView[0][1] * worldView[1][0]) * inverseDeterminant }
			};
			for (int column = 0; column < 3; ++column) {
				for (int row = 0; row < 3; ++row) {
					cameraNormal[column] += normal[row] * inverse[column][row];
				}
			}
		}
		offset += 3 * sizeof(float);
		if (m_renderStates[D3DRS_NORMALIZENORMALS] != FALSE) {
			const float length = std::sqrt(cameraNormal[0] * cameraNormal[0] + cameraNormal[1] * cameraNormal[1] + cameraNormal[2] * cameraNormal[2]);
			if (length > 0.0f) {
				for (float &component : cameraNormal) component /= length;
			}
		}
	}
	if (positionType != D3DFVF_XYZRHW && m_renderStates[D3DRS_FOGENABLE] != FALSE) {
		const DWORD fogMode = m_renderStates[D3DRS_FOGVERTEXMODE];
		const float fogDistance = m_renderStates[D3DRS_RANGEFOGENABLE] != FALSE
			? std::sqrt(cameraPosition[0] * cameraPosition[0] + cameraPosition[1] * cameraPosition[1] + cameraPosition[2] * cameraPosition[2])
			: std::abs(cameraPosition[2]);
		const float fogDensity = std::bit_cast<float>(m_renderStates[D3DRS_FOGDENSITY]);
		switch (fogMode) {
			case D3DFOG_LINEAR: {
				const float fogStart = std::bit_cast<float>(m_renderStates[D3DRS_FOGSTART]);
				const float fogEnd = std::bit_cast<float>(m_renderStates[D3DRS_FOGEND]);
				output.fog = fogEnd == fogStart ? (fogDistance <= fogEnd ? 1.0f : 0.0f)
					: (fogEnd - fogDistance) / (fogEnd - fogStart);
				break;
			}
			case D3DFOG_EXP:
				output.fog = std::exp(-fogDensity * fogDistance);
				break;
			case D3DFOG_EXP2: {
				const float densityDistance = fogDensity * fogDistance;
				output.fog = std::exp(-densityDistance * densityDistance);
				break;
			}
			default:
				break;
		}
		output.fog = std::clamp(output.fog, 0.0f, 1.0f);
	}
	if (fvf & D3DFVF_PSIZE) {
		offset += sizeof(float);
	}
	std::array<float, 4> diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	std::array<float, 4> specular = {};
	if (fvf & D3DFVF_DIFFUSE) {
		const DWORD color = *reinterpret_cast<const DWORD *>(source + offset);
		diffuse[0] = static_cast<float>((color >> 16) & 0xff) / 255.0f;
		diffuse[1] = static_cast<float>((color >> 8) & 0xff) / 255.0f;
		diffuse[2] = static_cast<float>(color & 0xff) / 255.0f;
		diffuse[3] = static_cast<float>((color >> 24) & 0xff) / 255.0f;
		offset += sizeof(DWORD);
	}
	if (fvf & D3DFVF_SPECULAR) {
		const DWORD color = *reinterpret_cast<const DWORD *>(source + offset);
		specular[0] = static_cast<float>((color >> 16) & 0xff) / 255.0f;
		specular[1] = static_cast<float>((color >> 8) & 0xff) / 255.0f;
		specular[2] = static_cast<float>(color & 0xff) / 255.0f;
		specular[3] = static_cast<float>((color >> 24) & 0xff) / 255.0f;
		if (m_renderStates[D3DRS_FOGENABLE] != FALSE && m_renderStates[D3DRS_FOGVERTEXMODE] == D3DFOG_NONE) {
			output.fog = specular[3];
		}
		offset += sizeof(DWORD);
	}
	std::copy(diffuse.begin(), diffuse.end(), output.color);
	if (positionType != D3DFVF_XYZRHW && (fvf & D3DFVF_NORMAL) && m_renderStates[D3DRS_LIGHTING] != FALSE) {
		auto colorValue = [](const D3DCOLORVALUE &value) {
			return std::array<float, 4>{ value.r, value.g, value.b, value.a };
		};
		auto materialSource = [&](DWORD state, const D3DCOLORVALUE &material) {
			if (m_renderStates[D3DRS_COLORVERTEX] != FALSE) {
				if (state == D3DMCS_COLOR1 && (fvf & D3DFVF_DIFFUSE)) return diffuse;
				if (state == D3DMCS_COLOR2 && (fvf & D3DFVF_SPECULAR)) return specular;
			}
			return colorValue(material);
		};
		std::array<float, 4> lightAmbient = {};
		std::array<float, 4> lightDiffuse = {};
		for (DWORD index = 0; index < 8; ++index) {
			const auto enabled = m_lightEnabled.find(index);
			const auto found = m_lights.find(index);
			if (enabled == m_lightEnabled.end() || enabled->second == FALSE || found == m_lights.end()) continue;
			const D3DLIGHT8 &light = found->second;
			std::array<float, 4> position = { light.Position.x, light.Position.y, light.Position.z, 1.0f };
			std::array<float, 4> direction = { light.Direction.x, light.Direction.y, light.Direction.z, 0.0f };
			position = transform(position, viewMatrix);
			direction = transform(direction, viewMatrix);
			float directionLength = std::sqrt(direction[0] * direction[0] + direction[1] * direction[1] + direction[2] * direction[2]);
			if (directionLength > 0.0f) {
				for (int component = 0; component < 3; ++component) direction[component] /= directionLength;
			}
			std::array<float, 3> hitDirection = {
				position[0] - cameraPosition[0], position[1] - cameraPosition[1], position[2] - cameraPosition[2]
			};
			const float distance = std::sqrt(hitDirection[0] * hitDirection[0] + hitDirection[1] * hitDirection[1] + hitDirection[2] * hitDirection[2]);
			if (light.Type == D3DLIGHT_DIRECTIONAL) {
				hitDirection = { -direction[0], -direction[1], -direction[2] };
			} else if (distance > 0.0f) {
				for (float &component : hitDirection) component /= distance;
			}
			float attenuation = 1.0f;
			if (light.Type != D3DLIGHT_DIRECTIONAL) {
				const float denominator = light.Attenuation0 + distance * light.Attenuation1 + distance * distance * light.Attenuation2;
				attenuation = denominator > 0.0f ? 1.0f / denominator : std::numeric_limits<float>::max();
				if (distance > light.Range) attenuation = 0.0f;
			}
			if (light.Type == D3DLIGHT_SPOT) {
				const float rho = -(hitDirection[0] * direction[0] + hitDirection[1] * direction[1] + hitDirection[2] * direction[2]);
				const float theta = std::cos(light.Theta * 0.5f);
				const float phi = std::cos(light.Phi * 0.5f);
				float spot = 0.0f;
				if (rho > theta) spot = 1.0f;
				else if (rho > phi && theta != phi) spot = std::pow((rho - phi) / (theta - phi), light.Falloff);
				attenuation *= std::clamp(spot, 0.0f, 1.0f);
			}
			const float diffuseFactor = std::clamp(
				cameraNormal[0] * hitDirection[0] + cameraNormal[1] * hitDirection[1] + cameraNormal[2] * hitDirection[2], 0.0f, 1.0f) * attenuation;
			const auto ambient = colorValue(light.Ambient);
			const auto direct = colorValue(light.Diffuse);
			for (int component = 0; component < 4; ++component) {
				lightAmbient[component] += ambient[component] * attenuation;
				lightDiffuse[component] += direct[component] * diffuseFactor;
			}
		}
		const auto materialDiffuse = materialSource(m_renderStates[D3DRS_DIFFUSEMATERIALSOURCE], m_material.Diffuse);
		const auto materialAmbient = materialSource(m_renderStates[D3DRS_AMBIENTMATERIALSOURCE], m_material.Ambient);
		const auto materialEmissive = materialSource(m_renderStates[D3DRS_EMISSIVEMATERIALSOURCE], m_material.Emissive);
		const DWORD ambientColor = m_renderStates[D3DRS_AMBIENT];
		const std::array<float, 4> globalAmbient = {
			static_cast<float>((ambientColor >> 16) & 0xff) / 255.0f,
			static_cast<float>((ambientColor >> 8) & 0xff) / 255.0f,
			static_cast<float>(ambientColor & 0xff) / 255.0f,
			static_cast<float>((ambientColor >> 24) & 0xff) / 255.0f
		};
		for (int component = 0; component < 4; ++component) {
			output.color[component] = std::clamp(
				materialEmissive[component] + materialAmbient[component] * (globalAmbient[component] + lightAmbient[component]) +
				materialDiffuse[component] * lightDiffuse[component], 0.0f, 1.0f);
		}
		output.color[3] = materialDiffuse[3];
	}
	const UINT textureCount = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
	std::array<std::array<float, 2>, 4> sourceUV = {};
	for (UINT stage = 0; stage < std::min(textureCount, 4U); ++stage) {
		const UINT sizeBits = (fvf >> (16 + stage * 2)) & 3;
		const UINT coordinateCount = sizeBits == 0 ? 2 : sizeBits == 1 ? 3 : sizeBits == 2 ? 4 : 1;
		if (offset + coordinateCount * sizeof(float) > stride) {
			break;
		}
		const float *coordinates = reinterpret_cast<const float *>(source + offset);
		sourceUV[stage][0] = coordinates[0];
		sourceUV[stage][1] = coordinateCount > 1 ? coordinates[1] : 0.0f;
		offset += coordinateCount * sizeof(float);
	}
	for (UINT stage = 0; stage < 4; ++stage) {
		const DWORD coordinateState = m_textureStates[stage][D3DTSS_TEXCOORDINDEX];
		const DWORD generationMode = coordinateState & 0xffff0000;
		std::array<float, 4> coordinates = {};
		if (positionType != D3DFVF_XYZRHW && generationMode == D3DTSS_TCI_CAMERASPACEPOSITION) {
			coordinates = cameraPosition;
		} else {
			const UINT coordinateIndex = coordinateState & 0xffff;
			const auto &selected = sourceUV[coordinateIndex < sourceUV.size() ? coordinateIndex : 0];
			coordinates = { selected[0], selected[1], 0.0f, 1.0f };
		}
		const DWORD transformFlags = m_textureStates[stage][D3DTSS_TEXTURETRANSFORMFLAGS];
		if (transformFlags != D3DTTFF_DISABLE) {
			const auto textureTransform = m_transforms.find(static_cast<DWORD>(D3DTS_TEXTURE0) + stage);
			if (textureTransform != m_transforms.end()) {
				std::array<float, 4> transformed = {};
				for (int column = 0; column < 4; ++column) {
					for (int row = 0; row < 4; ++row) {
						transformed[column] += coordinates[row] * textureTransform->second.m[row][column];
					}
				}
				coordinates = transformed;
			}
		}
		output.uv[stage][0] = coordinates[0];
		output.uv[stage][1] = coordinates[1];
	}
	return output;
}

HRESULT WebGPUD3DDevice::draw(D3DPRIMITIVETYPE type, const void *vertexData, UINT vertexCount, UINT stride, const void *indexData, D3DFORMAT indexFormat, UINT indexCount, UINT baseVertex)
{
	if (!vertexData || !vertexCount || !stride || FAILED(ensurePass(0, 0, 1.0f, 0))) {
		return D3DERR_INVALIDCALL;
	}
	const UINT sourceCount = indexData ? indexCount : vertexCount;
	size_t expandedCount = sourceCount;
	switch (type) {
		case D3DPT_POINTLIST:
		case D3DPT_LINELIST:
		case D3DPT_TRIANGLELIST:
			break;
		case D3DPT_LINESTRIP:
			expandedCount = sourceCount > 1 ? static_cast<size_t>(sourceCount - 1) * 2 : 0;
			break;
		case D3DPT_TRIANGLESTRIP:
		case D3DPT_TRIANGLEFAN:
			expandedCount = sourceCount > 2 ? static_cast<size_t>(sourceCount - 2) * 3 : 0;
			break;
		default:
			return D3DERR_INVALIDCALL;
	}
	if (!expandedCount) return D3D_OK;
	UploadFrame &frame = m_uploadFrames[m_activeUploadFrame];
	const size_t vertexOffset = frame.vertices.size() * sizeof(CanonicalVertex);
	const size_t firstVertex = frame.vertices.size();
	const auto *bytes = static_cast<const uint8_t *>(vertexData);
	auto sourceIndex = [&](UINT index) -> UINT {
		if (!indexData) return index;
		if (indexFormat == D3DFMT_INDEX32) return reinterpret_cast<const uint32_t *>(indexData)[index];
		return reinterpret_cast<const uint16_t *>(indexData)[index];
	};
	auto forEachExpandedIndex = [&](auto &&append) {
		switch (type) {
			case D3DPT_POINTLIST:
			case D3DPT_LINELIST:
			case D3DPT_TRIANGLELIST:
				for (UINT index = 0; index < sourceCount; ++index) if (!append(sourceIndex(index))) return false;
				break;
			case D3DPT_LINESTRIP:
				for (UINT index = 0; index + 1 < sourceCount; ++index) {
					if (!append(sourceIndex(index)) || !append(sourceIndex(index + 1))) return false;
				}
				break;
			case D3DPT_TRIANGLESTRIP:
				for (UINT index = 0; index + 2 < sourceCount; ++index) {
					if (!append(sourceIndex(index + (index & 1)))
						|| !append(sourceIndex(index + 1 - (index & 1)))
						|| !append(sourceIndex(index + 2))) return false;
				}
				break;
			case D3DPT_TRIANGLEFAN:
				for (UINT index = 1; index + 1 < sourceCount; ++index) {
					if (!append(sourceIndex(0)) || !append(sourceIndex(index))
						|| !append(sourceIndex(index + 1))) return false;
				}
				break;
			default:
				return false;
		}
		return true;
	};
	const size_t expandedVertexBytes = expandedCount * sizeof(CanonicalVertex);
	const size_t compactVertexBytes = static_cast<size_t>(vertexCount) * sizeof(CanonicalVertex);
	const size_t indexOffset = frame.indices.size() * sizeof(uint32_t);
	const size_t indexBytes = expandedCount * sizeof(uint32_t);
	const bool expandedFits = vertexOffset + expandedVertexBytes <= kVertexUploadCapacity;
	const bool indexedFits = vertexOffset + compactVertexBytes <= kVertexUploadCapacity
		&& indexOffset + indexBytes <= kIndexUploadCapacity;
	const bool useIndices = indexedFits
		&& (compactVertexBytes + indexBytes < expandedVertexBytes || !expandedFits);
	size_t vertexBytes = expandedVertexBytes;
	if (useIndices) {
		const size_t firstIndex = frame.indices.size();
		auto appendIndex = [&](UINT index) {
			if (index < baseVertex || index - baseVertex >= vertexCount) return false;
			frame.indices.push_back(index - baseVertex);
			return true;
		};
		if (!forEachExpandedIndex(appendIndex)) {
			frame.indices.resize(firstIndex);
			return D3DERR_INVALIDCALL;
		}
		for (UINT index = 0; index < vertexCount; ++index) {
			frame.vertices.push_back(convertVertex(bytes + static_cast<size_t>(index) * stride, stride));
		}
		vertexBytes = compactVertexBytes;
	} else {
		if (!expandedFits) return D3DERR_OUTOFVIDEOMEMORY;
		auto appendVertex = [&](UINT index) {
			if (index < baseVertex || index - baseVertex >= vertexCount) return false;
			frame.vertices.push_back(convertVertex(bytes + static_cast<size_t>(index - baseVertex) * stride, stride));
			return true;
		};
		if (!forEachExpandedIndex(appendVertex)) {
			frame.vertices.resize(firstVertex);
			return D3DERR_INVALIDCALL;
		}
	}
	WGPURenderPipeline renderPipeline = pipeline(type);
	const DrawBindings bindings = bindGroup();
	if (!renderPipeline || !bindings.group) {
		return D3DERR_DRIVERINTERNALERROR;
	}
	wgpuRenderPassEncoderSetPipeline(m_pass, renderPipeline);
	wgpuRenderPassEncoderSetStencilReference(m_pass, m_renderStates[D3DRS_STENCILREF] & 0xff);
	wgpuRenderPassEncoderSetBindGroup(m_pass, 0, bindings.group, 1, &bindings.uniformOffset);
	wgpuRenderPassEncoderSetVertexBuffer(m_pass, 0, frame.vertexBuffer, vertexOffset, vertexBytes);
	if (useIndices) {
		wgpuRenderPassEncoderSetIndexBuffer(m_pass, frame.indexBuffer, WGPUIndexFormat_Uint32, indexOffset, indexBytes);
		wgpuRenderPassEncoderDrawIndexed(m_pass, static_cast<uint32_t>(expandedCount), 1, 0, 0, 0);
	} else {
		wgpuRenderPassEncoderDraw(m_pass, static_cast<uint32_t>(expandedCount), 1, 0, 0);
	}
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::DrawPrimitive(D3DPRIMITIVETYPE type, UINT startVertex, UINT primitiveCount)
{
	if (!m_vertexBuffer || !m_vertexStride) return D3DERR_INVALIDCALL;
	const UINT count = vertexCountFor(type, primitiveCount);
	const size_t offset = static_cast<size_t>(startVertex) * m_vertexStride;
	if (offset + static_cast<size_t>(count) * m_vertexStride > m_vertexBuffer->size()) return D3DERR_INVALIDCALL;
	return draw(type, m_vertexBuffer->data() + offset, count, m_vertexStride, nullptr, D3DFMT_UNKNOWN, count, 0);
}

HRESULT WebGPUD3DDevice::DrawIndexedPrimitive(D3DPRIMITIVETYPE type, UINT minIndex, UINT vertexCount, UINT startIndex, UINT primitiveCount)
{
	if (!m_vertexBuffer || !m_indexBuffer || !m_vertexStride) return D3DERR_INVALIDCALL;
	const UINT indexCount = vertexCountFor(type, primitiveCount);
	const UINT indexSize = m_indexBuffer->format() == D3DFMT_INDEX32 ? 4 : 2;
	const size_t indexOffset = static_cast<size_t>(startIndex) * indexSize;
	const size_t vertexOffset = static_cast<size_t>(m_baseVertex + minIndex) * m_vertexStride;
	if (indexOffset + static_cast<size_t>(indexCount) * indexSize > m_indexBuffer->size() ||
		vertexOffset + static_cast<size_t>(vertexCount) * m_vertexStride > m_vertexBuffer->size()) return D3DERR_INVALIDCALL;
	return draw(type, m_vertexBuffer->data() + vertexOffset, vertexCount, m_vertexStride, m_indexBuffer->data() + indexOffset, m_indexBuffer->format(), indexCount, minIndex);
}

HRESULT WebGPUD3DDevice::DrawPrimitiveUP(D3DPRIMITIVETYPE type, UINT primitiveCount, const void *data, UINT stride)
{
	const UINT count = vertexCountFor(type, primitiveCount);
	return draw(type, data, count, stride, nullptr, D3DFMT_UNKNOWN, count, 0);
}

HRESULT WebGPUD3DDevice::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE type, UINT minVertexIndex, UINT vertexCount, UINT primitiveCount, const void *indexData, D3DFORMAT indexFormat, const void *data, UINT stride)
{
	return draw(type, data, vertexCount, stride, indexData, indexFormat, vertexCountFor(type, primitiveCount), minVertexIndex);
}

} // namespace

IDirect3D8 *CreateWebGPUDirect3D8()
{
	auto *direct3D = new WebGPUDirect3D8();
	if (!direct3D->initialize()) {
		direct3D->Release();
		return nullptr;
	}
	fprintf(stderr, "INFO: Direct3D 8 compatibility initialized with Chrome WebGPU\n");
	return direct3D;
}

bool WebGPUDeviceCanSubmitFrame()
{
	return !g_activeWebGPUDevice || g_activeWebGPUDevice->canSubmitFrame();
}

namespace
{
WebGPUD3DDevice::WebGPUD3DDevice(WebGPUDirect3D8 *parent, WebGPUContext *context, const D3DPRESENT_PARAMETERS &parameters) :
	m_parent(parent),
	m_context(context),
	m_parameters(parameters)
{
	m_parent->AddRef();
	g_activeWebGPUDevice = this;
	if (!m_parameters.BackBufferWidth) {
		m_parameters.BackBufferWidth = context->width();
	}
	if (!m_parameters.BackBufferHeight) {
		m_parameters.BackBufferHeight = context->height();
	}
	m_viewport = { 0, 0, m_parameters.BackBufferWidth, m_parameters.BackBufferHeight, 0.0f, 1.0f };
	m_material.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_material.Ambient = m_material.Diffuse;
	m_transforms[D3DTS_WORLD] = identityMatrix();
	m_transforms[D3DTS_VIEW] = identityMatrix();
	m_transforms[D3DTS_PROJECTION] = identityMatrix();
	m_renderStates[D3DRS_ZENABLE] = D3DZB_TRUE;
	m_renderStates[D3DRS_ZWRITEENABLE] = TRUE;
	m_renderStates[D3DRS_ZFUNC] = D3DCMP_LESSEQUAL;
	m_renderStates[D3DRS_CULLMODE] = D3DCULL_CCW;
	m_renderStates[D3DRS_ALPHABLENDENABLE] = FALSE;
	m_renderStates[D3DRS_ALPHATESTENABLE] = FALSE;
	m_renderStates[D3DRS_ALPHAFUNC] = D3DCMP_ALWAYS;
	m_renderStates[D3DRS_SRCBLEND] = D3DBLEND_ONE;
	m_renderStates[D3DRS_COLORWRITEENABLE] = D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
		D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA;
	m_renderStates[D3DRS_STENCILENABLE] = FALSE;
	m_renderStates[D3DRS_STENCILFAIL] = D3DSTENCILOP_KEEP;
	m_renderStates[D3DRS_STENCILZFAIL] = D3DSTENCILOP_KEEP;
	m_renderStates[D3DRS_STENCILPASS] = D3DSTENCILOP_KEEP;
	m_renderStates[D3DRS_STENCILFUNC] = D3DCMP_ALWAYS;
	m_renderStates[D3DRS_STENCILMASK] = 0xffffffff;
	m_renderStates[D3DRS_STENCILWRITEMASK] = 0xffffffff;
	m_renderStates[D3DRS_DESTBLEND] = D3DBLEND_ZERO;
	m_renderStates[D3DRS_TEXTUREFACTOR] = 0xffffffff;
	m_renderStates[D3DRS_LIGHTING] = TRUE;
	m_renderStates[D3DRS_SPECULARENABLE] = FALSE;
	m_renderStates[D3DRS_AMBIENT] = 0;
	m_renderStates[D3DRS_COLORVERTEX] = TRUE;
	m_renderStates[D3DRS_LOCALVIEWER] = TRUE;
	m_renderStates[D3DRS_NORMALIZENORMALS] = FALSE;
	m_renderStates[D3DRS_DIFFUSEMATERIALSOURCE] = D3DMCS_COLOR1;
	m_renderStates[D3DRS_SPECULARMATERIALSOURCE] = D3DMCS_COLOR2;
	m_renderStates[D3DRS_AMBIENTMATERIALSOURCE] = D3DMCS_MATERIAL;
	m_renderStates[D3DRS_EMISSIVEMATERIALSOURCE] = D3DMCS_MATERIAL;
	m_renderStates[D3DRS_FOGENABLE] = FALSE;
	m_renderStates[D3DRS_FOGCOLOR] = 0;
	m_renderStates[D3DRS_FOGSTART] = std::bit_cast<DWORD>(0.0f);
	m_renderStates[D3DRS_FOGEND] = std::bit_cast<DWORD>(1.0f);
	m_renderStates[D3DRS_FOGDENSITY] = std::bit_cast<DWORD>(1.0f);
	m_renderStates[D3DRS_FOGTABLEMODE] = D3DFOG_NONE;
	m_renderStates[D3DRS_FOGVERTEXMODE] = D3DFOG_NONE;
	m_renderStates[D3DRS_RANGEFOGENABLE] = FALSE;
	for (UINT stageIndex = 0; stageIndex < m_textureStates.size(); ++stageIndex) {
		auto &stage = m_textureStates[stageIndex];
		stage[D3DTSS_ADDRESSU] = D3DTADDRESS_WRAP;
		stage[D3DTSS_ADDRESSV] = D3DTADDRESS_WRAP;
		stage[D3DTSS_MAGFILTER] = D3DTEXF_POINT;
		stage[D3DTSS_MINFILTER] = D3DTEXF_POINT;
		stage[D3DTSS_MIPFILTER] = D3DTEXF_NONE;
		stage[D3DTSS_MAXANISOTROPY] = 1;
		stage[D3DTSS_COLOROP] = stageIndex == 0 ? D3DTOP_MODULATE : D3DTOP_DISABLE;
		stage[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
		stage[D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
		stage[D3DTSS_ALPHAOP] = stageIndex == 0 ? D3DTOP_MODULATE : D3DTOP_DISABLE;
		stage[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
		stage[D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
	}
}

WebGPUD3DDevice::~WebGPUD3DDevice()
{
	if (g_activeWebGPUDevice == this) {
		g_activeWebGPUDevice = nullptr;
	}
	releaseFrame();
	for (WebGPUTexture *texture : m_textures) {
		if (texture) {
			texture->Release();
		}
	}
	if (m_vertexBuffer) {
		m_vertexBuffer->Release();
	}
	if (m_indexBuffer) {
		m_indexBuffer->Release();
	}
	if (m_renderTarget) {
		m_renderTarget->Release();
	}
	if (m_depthTarget) {
		m_depthTarget->Release();
	}
	if (m_backBuffer) {
		m_backBuffer->Release();
	}
	if (m_depthSurface) {
		m_depthSurface->Release();
	}
	for (const auto &[key, pipelineObject] : m_pipelines) {
		(void)key;
		wgpuRenderPipelineRelease(pipelineObject);
	}
	if (m_depthView) {
		wgpuTextureViewRelease(m_depthView);
	}
	if (m_depthTexture) {
		wgpuTextureRelease(m_depthTexture);
	}
	if (m_multisampleView) {
		wgpuTextureViewRelease(m_multisampleView);
	}
	if (m_multisampleTexture) {
		wgpuTextureRelease(m_multisampleTexture);
	}
	if (m_whiteView) {
		wgpuTextureViewRelease(m_whiteView);
	}
	if (m_whiteTexture) {
		wgpuTextureRelease(m_whiteTexture);
	}
	for (const auto &[key, samplerObject] : m_samplers) {
		(void)key;
		wgpuSamplerRelease(samplerObject);
	}
	for (UploadFrame &frame : m_uploadFrames) {
		releaseUploadFrame(frame);
	}
	if (m_pipelineLayout) {
		wgpuPipelineLayoutRelease(m_pipelineLayout);
	}
	if (m_bindGroupLayout) {
		wgpuBindGroupLayoutRelease(m_bindGroupLayout);
	}
	if (m_shader) {
		wgpuShaderModuleRelease(m_shader);
	}
	m_parent->Release();
}

void WebGPUD3DDevice::onSubmittedWorkDone(WGPUQueueWorkDoneStatus, WGPUStringView, void *userdata1, void *userdata2)
{
	auto *device = static_cast<WebGPUD3DDevice *>(userdata1);
	static_cast<UploadFrame *>(userdata2)->busy = false;
	if (device->m_pendingSubmissions > 0) {
		--device->m_pendingSubmissions;
	}
	device->Release();
}

bool WebGPUD3DDevice::initialize()
{
	static constexpr const char *shaderSource = R"(
struct VertexInput {
    @location(0) position: vec4<f32>,
    @location(1) color: vec4<f32>,
    @location(2) uv0: vec2<f32>,
    @location(3) uv1: vec2<f32>,
    @location(4) uv2: vec2<f32>,
    @location(5) uv3: vec2<f32>,
    @location(6) fog: f32,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) uv0: vec2<f32>,
    @location(2) uv1: vec2<f32>,
    @location(3) uv2: vec2<f32>,
    @location(4) uv3: vec2<f32>,
    @location(5) fog: f32,
};

@vertex
fn vertex_main(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;
    output.position = input.position;
    output.color = input.color;
    output.uv0 = input.uv0;
    output.uv1 = input.uv1;
    output.uv2 = input.uv2;
    output.uv3 = input.uv3;
    output.fog = input.fog;
    return output;
}

@group(0) @binding(0) var texture_sampler0: sampler;
@group(0) @binding(1) var color_texture0: texture_2d<f32>;
@group(0) @binding(2) var texture_sampler1: sampler;
@group(0) @binding(3) var color_texture1: texture_2d<f32>;
@group(0) @binding(4) var texture_sampler2: sampler;
@group(0) @binding(5) var color_texture2: texture_2d<f32>;
@group(0) @binding(6) var texture_sampler3: sampler;
@group(0) @binding(7) var color_texture3: texture_2d<f32>;

struct TextureCombiner {
    color_op0: u32,
    color_arg10: u32,
    color_arg20: u32,
    alpha_op0: u32,
    alpha_arg10: u32,
    alpha_arg20: u32,
    color_op1: u32,
    color_arg11: u32,
    color_arg21: u32,
    alpha_op1: u32,
    alpha_arg11: u32,
    alpha_arg21: u32,
    color_op2: u32,
    color_arg12: u32,
    color_arg22: u32,
    alpha_op2: u32,
    alpha_arg12: u32,
    alpha_arg22: u32,
    color_op3: u32,
    color_arg13: u32,
    color_arg23: u32,
    alpha_op3: u32,
    alpha_arg13: u32,
    alpha_arg23: u32,
    texture_factor: vec4<f32>,
    fog_color: vec4<f32>,
    alpha_ref: f32,
    alpha_func: u32,
    alpha_test_enable: u32,
    padding: u32,
    pixel_shader_mode: u32,
    pixel_shader_texture_count: u32,
    shader_padding0: u32,
    shader_padding1: u32,
};

@group(0) @binding(8) var<uniform> combiner: TextureCombiner;

fn texture_argument(selector: u32, texture_color: vec4<f32>, diffuse: vec4<f32>, current: vec4<f32>) -> vec4<f32> {
    var value: vec4<f32>;
    switch selector & 15u {
        case 1u: { value = current; }
        case 2u: { value = texture_color; }
        case 3u: { value = combiner.texture_factor; }
        default: { value = diffuse; }
    }
    if ((selector & 32u) != 0u) {
        value = vec4<f32>(value.a);
    }
    if ((selector & 16u) != 0u) {
        value = vec4<f32>(1.0) - value;
    }
    return value;
}

fn combine_color(op: u32, first: vec4<f32>, second: vec4<f32>, texture_color: vec4<f32>, diffuse: vec4<f32>, current: vec4<f32>) -> vec3<f32> {
    switch op {
        case 2u: { return first.rgb; }
        case 3u: { return second.rgb; }
        case 5u: { return 2.0 * first.rgb * second.rgb; }
        case 6u: { return 4.0 * first.rgb * second.rgb; }
        case 7u: { return first.rgb + second.rgb; }
        case 8u: { return first.rgb + second.rgb - vec3<f32>(0.5); }
        case 9u: { return 2.0 * (first.rgb + second.rgb - vec3<f32>(0.5)); }
        case 10u: { return first.rgb - second.rgb; }
        case 11u: { return first.rgb + second.rgb - first.rgb * second.rgb; }
        case 12u: { return mix(second.rgb, first.rgb, diffuse.a); }
        case 13u: { return mix(second.rgb, first.rgb, texture_color.a); }
        case 14u: { return mix(second.rgb, first.rgb, combiner.texture_factor.a); }
        case 15u: { return first.rgb + second.rgb * (1.0 - first.a); }
        case 16u: { return mix(second.rgb, first.rgb, current.a); }
        case 18u: { return first.rgb + second.rgb * first.a; }
        case 19u: { return first.rgb * second.a + second.rgb; }
        case 20u: { return (1.0 - first.a) * second.rgb + first.rgb; }
        case 21u: { return (1.0 - second.a) * first.rgb + second.rgb; }
        case 25u: { return first.rgb * second.rgb + current.rgb; }
        case 26u: { return mix(second.rgb, first.rgb, current.rgb); }
        default: { return first.rgb * second.rgb; }
    }
}

fn combine_alpha(op: u32, first: vec4<f32>, second: vec4<f32>, texture_color: vec4<f32>, diffuse: vec4<f32>, current: vec4<f32>) -> f32 {
    switch op {
        case 1u: { return current.a; }
        case 2u: { return first.a; }
        case 3u: { return second.a; }
        case 5u: { return 2.0 * first.a * second.a; }
        case 6u: { return 4.0 * first.a * second.a; }
        case 7u: { return first.a + second.a; }
        case 8u: { return first.a + second.a - 0.5; }
        case 9u: { return 2.0 * (first.a + second.a - 0.5); }
        case 10u: { return first.a - second.a; }
        case 11u: { return first.a + second.a - first.a * second.a; }
        case 12u: { return mix(second.a, first.a, diffuse.a); }
        case 13u: { return mix(second.a, first.a, texture_color.a); }
        case 14u: { return mix(second.a, first.a, combiner.texture_factor.a); }
        case 16u: { return mix(second.a, first.a, current.a); }
        default: { return first.a * second.a; }
    }
}

fn apply_stage(current: vec4<f32>, diffuse: vec4<f32>, texture_color: vec4<f32>, color_op: u32, color_arg1: u32, color_arg2: u32, alpha_op: u32, alpha_arg1: u32, alpha_arg2: u32) -> vec4<f32> {
    if (color_op == 1u) {
        return current;
    }
    let first = texture_argument(color_arg1, texture_color, diffuse, current);
    let second = texture_argument(color_arg2, texture_color, diffuse, current);
    let alpha_first = texture_argument(alpha_arg1, texture_color, diffuse, current);
    let alpha_second = texture_argument(alpha_arg2, texture_color, diffuse, current);
    return vec4<f32>(
        clamp(combine_color(color_op, first, second, texture_color, diffuse, current), vec3<f32>(0.0), vec3<f32>(1.0)),
        clamp(combine_alpha(alpha_op, alpha_first, alpha_second, texture_color, diffuse, current), 0.0, 1.0)
    );
}

fn alpha_compare(value: f32) -> bool {
    switch combiner.alpha_func {
        case 1u: { return false; }
        case 2u: { return value < combiner.alpha_ref; }
        case 3u: { return value == combiner.alpha_ref; }
        case 4u: { return value <= combiner.alpha_ref; }
        case 5u: { return value > combiner.alpha_ref; }
        case 6u: { return value != combiner.alpha_ref; }
        case 7u: { return value >= combiner.alpha_ref; }
        default: { return true; }
    }
}

@fragment
fn fragment_main(input: VertexOutput) -> @location(0) vec4<f32> {
    let texture0 = textureSample(color_texture0, texture_sampler0, input.uv0);
    let texture1 = textureSample(color_texture1, texture_sampler1, input.uv1);
    let texture2 = textureSample(color_texture2, texture_sampler2, input.uv2);
    let texture3 = textureSample(color_texture3, texture_sampler3, input.uv3);
    var color: vec4<f32>;
    if (combiner.pixel_shader_mode == 1u) {
        color = ((texture1 - texture0) * input.color.a + texture0) * input.color;
        if (combiner.pixel_shader_texture_count > 2u) {
            color *= texture2;
        }
        if (combiner.pixel_shader_texture_count > 3u) {
            color *= texture3;
        }
    } else if (combiner.pixel_shader_mode == 2u) {
        color = texture0 * texture1 * input.color;
        if (combiner.pixel_shader_texture_count > 2u) {
            color *= texture2;
        }
        if (combiner.pixel_shader_texture_count > 3u) {
            color *= texture3;
        }
    } else if (combiner.pixel_shader_mode == 3u) {
        color = texture1 * input.color;
        if (combiner.pixel_shader_texture_count > 2u) {
            color *= texture2;
        }
        if (combiner.pixel_shader_texture_count > 3u) {
            color *= texture3;
        }
    } else if (combiner.pixel_shader_mode == 4u) {
        color = texture0 * texture1;
        color *= texture2;
        color *= input.color;
    } else {
        color = apply_stage(input.color, input.color, texture0, combiner.color_op0, combiner.color_arg10, combiner.color_arg20, combiner.alpha_op0, combiner.alpha_arg10, combiner.alpha_arg20);
        if (combiner.color_op1 != 1u) {
            color = apply_stage(color, input.color, texture1, combiner.color_op1, combiner.color_arg11, combiner.color_arg21, combiner.alpha_op1, combiner.alpha_arg11, combiner.alpha_arg21);
            if (combiner.color_op2 != 1u) {
                color = apply_stage(color, input.color, texture2, combiner.color_op2, combiner.color_arg12, combiner.color_arg22, combiner.alpha_op2, combiner.alpha_arg12, combiner.alpha_arg22);
                if (combiner.color_op3 != 1u) {
                    color = apply_stage(color, input.color, texture3, combiner.color_op3, combiner.color_arg13, combiner.color_arg23, combiner.alpha_op3, combiner.alpha_arg13, combiner.alpha_arg23);
                }
            }
        }
    }
    if (combiner.alpha_test_enable != 0u && !alpha_compare(color.a)) {
        discard;
    }
    color = vec4<f32>(mix(combiner.fog_color.rgb, color.rgb, input.fog), color.a);
    return color;
}
)";

	WGPUShaderSourceWGSL source = WGPU_SHADER_SOURCE_WGSL_INIT;
	source.code = MakeStringView(shaderSource);
	WGPUShaderModuleDescriptor shaderDescriptor = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
	shaderDescriptor.nextInChain = &source.chain;
	m_shader = wgpuDeviceCreateShaderModule(webGPUDevice(), &shaderDescriptor);
	if (!m_shader) {
		return false;
	}

	std::array<WGPUBindGroupLayoutEntry, 9> layoutEntries = {
		WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT,
		WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT,
		WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT,
		WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT,
		WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT,
		WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT,
		WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT,
		WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT,
		WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT,
	};
	layoutEntries[0].binding = 0;
	layoutEntries[0].visibility = WGPUShaderStage_Fragment;
	layoutEntries[0].sampler.type = WGPUSamplerBindingType_Filtering;
	layoutEntries[1].binding = 1;
	layoutEntries[1].visibility = WGPUShaderStage_Fragment;
	layoutEntries[1].texture.sampleType = WGPUTextureSampleType_Float;
	layoutEntries[1].texture.viewDimension = WGPUTextureViewDimension_2D;
	layoutEntries[1].texture.multisampled = WGPU_FALSE;
	layoutEntries[2] = layoutEntries[0];
	layoutEntries[2].binding = 2;
	layoutEntries[3] = layoutEntries[1];
	layoutEntries[3].binding = 3;
	layoutEntries[4] = layoutEntries[0];
	layoutEntries[4].binding = 4;
	layoutEntries[5] = layoutEntries[1];
	layoutEntries[5].binding = 5;
	layoutEntries[6] = layoutEntries[0];
	layoutEntries[6].binding = 6;
	layoutEntries[7] = layoutEntries[1];
	layoutEntries[7].binding = 7;
	layoutEntries[8].binding = 8;
	layoutEntries[8].visibility = WGPUShaderStage_Fragment;
	layoutEntries[8].buffer.type = WGPUBufferBindingType_Uniform;
	layoutEntries[8].buffer.hasDynamicOffset = WGPU_TRUE;
	layoutEntries[8].buffer.minBindingSize = sizeof(TextureCombinerUniforms);
	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
	bindGroupLayoutDescriptor.entryCount = layoutEntries.size();
	bindGroupLayoutDescriptor.entries = layoutEntries.data();
	m_bindGroupLayout = wgpuDeviceCreateBindGroupLayout(webGPUDevice(), &bindGroupLayoutDescriptor);
	if (!m_bindGroupLayout) {
		return false;
	}

	WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
	pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
	pipelineLayoutDescriptor.bindGroupLayouts = &m_bindGroupLayout;
	m_pipelineLayout = wgpuDeviceCreatePipelineLayout(webGPUDevice(), &pipelineLayoutDescriptor);
	for (UploadFrame &frame : m_uploadFrames) {
		WGPUBufferDescriptor vertexDescriptor = WGPU_BUFFER_DESCRIPTOR_INIT;
		vertexDescriptor.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
		vertexDescriptor.size = kVertexUploadCapacity;
		frame.vertexBuffer = wgpuDeviceCreateBuffer(webGPUDevice(), &vertexDescriptor);
		WGPUBufferDescriptor indexDescriptor = WGPU_BUFFER_DESCRIPTOR_INIT;
		indexDescriptor.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
		indexDescriptor.size = kIndexUploadCapacity;
		frame.indexBuffer = wgpuDeviceCreateBuffer(webGPUDevice(), &indexDescriptor);
		WGPUBufferDescriptor uniformDescriptor = WGPU_BUFFER_DESCRIPTOR_INIT;
		uniformDescriptor.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
		uniformDescriptor.size = kUniformUploadCapacity;
		frame.uniformBuffer = wgpuDeviceCreateBuffer(webGPUDevice(), &uniformDescriptor);
		if (!frame.vertexBuffer || !frame.indexBuffer || !frame.uniformBuffer) {
			return false;
		}
	}

	WGPUTextureDescriptor whiteDescriptor = WGPU_TEXTURE_DESCRIPTOR_INIT;
	whiteDescriptor.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
	whiteDescriptor.dimension = WGPUTextureDimension_2D;
	whiteDescriptor.size = { 1, 1, 1 };
	whiteDescriptor.format = WGPUTextureFormat_RGBA8Unorm;
	m_whiteTexture = wgpuDeviceCreateTexture(webGPUDevice(), &whiteDescriptor);
	m_whiteView = wgpuTextureCreateView(m_whiteTexture, nullptr);
	const std::array<uint8_t, 4> white = { 255, 255, 255, 255 };
	WGPUTexelCopyTextureInfo whiteDestination = WGPU_TEXEL_COPY_TEXTURE_INFO_INIT;
	whiteDestination.texture = m_whiteTexture;
	whiteDestination.aspect = WGPUTextureAspect_All;
	WGPUTexelCopyBufferLayout whiteLayout = WGPU_TEXEL_COPY_BUFFER_LAYOUT_INIT;
	whiteLayout.bytesPerRow = 4;
	whiteLayout.rowsPerImage = 1;
	WGPUExtent3D whiteExtent = { 1, 1, 1 };
	wgpuQueueWriteTexture(webGPUQueue(), &whiteDestination, white.data(), white.size(), &whiteLayout, &whiteExtent);

	m_backBuffer = new WebGPUSurface(this, m_parameters.BackBufferWidth, m_parameters.BackBufferHeight, m_parameters.BackBufferFormat, D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT, false);
	m_depthSurface = new WebGPUSurface(this, m_parameters.BackBufferWidth, m_parameters.BackBufferHeight, m_parameters.AutoDepthStencilFormat, D3DUSAGE_DEPTHSTENCIL, D3DPOOL_DEFAULT, false);
	createDepthBuffer();
	return m_pipelineLayout && m_whiteTexture && m_whiteView && m_depthTexture && m_depthView;
}

HRESULT WebGPUD3DDevice::QueryInterface(REFIID, void **output)
{
	if (!output) {
		return E_POINTER;
	}
	*output = this;
	AddRef();
	return S_OK;
}

ULONG WebGPUD3DDevice::AddRef()
{
	return ++m_refs;
}

ULONG WebGPUD3DDevice::Release()
{
	const ULONG refs = --m_refs;
	if (!refs) {
		delete this;
	}
	return refs;
}

HRESULT WebGPUD3DDevice::GetDirect3D(IDirect3D8 **direct3D)
{
	return ReturnObject<IDirect3D8>(m_parent, direct3D);
}

HRESULT WebGPUD3DDevice::GetDeviceCaps(D3DCAPS8 *caps)
{
	return m_parent->GetDeviceCaps(0, m_creation.DeviceType, caps);
}

HRESULT WebGPUD3DDevice::GetDisplayMode(D3DDISPLAYMODE *mode)
{
	if (!mode) {
		return D3DERR_INVALIDCALL;
	}
	mode->Width = m_parameters.BackBufferWidth;
	mode->Height = m_parameters.BackBufferHeight;
	mode->RefreshRate = 60;
	mode->Format = m_parameters.BackBufferFormat;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *parameters)
{
	if (!parameters) {
		return D3DERR_INVALIDCALL;
	}
	*parameters = m_creation;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::Reset(D3DPRESENT_PARAMETERS *parameters)
{
	if (!parameters) {
		return D3DERR_INVALIDCALL;
	}
	releaseFrame();
	m_parameters = *parameters;
	if (!m_parameters.BackBufferWidth) m_parameters.BackBufferWidth = m_context->width();
	if (!m_parameters.BackBufferHeight) m_parameters.BackBufferHeight = m_context->height();
	// GeneralsX @port Codex 04/08/2026 Keep the WebGPU surface and viewport aligned with Direct3D reset dimensions.
	if (!m_context->resize(m_parameters.BackBufferWidth, m_parameters.BackBufferHeight)) {
		return D3DERR_DEVICELOST;
	}
	if (m_renderTarget) {
		m_renderTarget->Release();
		m_renderTarget = nullptr;
	}
	if (m_depthTarget) {
		m_depthTarget->Release();
		m_depthTarget = nullptr;
	}
	if (m_backBuffer) {
		m_backBuffer->Release();
	}
	if (m_depthSurface) {
		m_depthSurface->Release();
	}
	m_backBuffer = new WebGPUSurface(this, m_parameters.BackBufferWidth, m_parameters.BackBufferHeight, m_parameters.BackBufferFormat, D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT, false);
	m_depthSurface = new WebGPUSurface(this, m_parameters.BackBufferWidth, m_parameters.BackBufferHeight, m_parameters.AutoDepthStencilFormat, D3DUSAGE_DEPTHSTENCIL, D3DPOOL_DEFAULT, false);
	m_viewport = { 0, 0, m_parameters.BackBufferWidth, m_parameters.BackBufferHeight, 0.0f, 1.0f };
	createDepthBuffer();
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::Present(const RECT *, const RECT *, HWND, const RGNDATA *)
{
	if (!m_encoder && FAILED(ensurePass(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0))) {
		return D3DERR_DEVICELOST;
	}
	finishPass();
	UploadFrame &uploadFrame = m_uploadFrames[m_activeUploadFrame];
	if (!uploadFrame.vertices.empty()) {
		wgpuQueueWriteBuffer(webGPUQueue(), uploadFrame.vertexBuffer, 0, uploadFrame.vertices.data(), uploadFrame.vertices.size() * sizeof(CanonicalVertex));
	}
	if (!uploadFrame.indices.empty()) {
		wgpuQueueWriteBuffer(webGPUQueue(), uploadFrame.indexBuffer, 0, uploadFrame.indices.data(), uploadFrame.indices.size() * sizeof(uint32_t));
	}
	if (!uploadFrame.uniforms.empty()) {
		wgpuQueueWriteBuffer(webGPUQueue(), uploadFrame.uniformBuffer, 0, uploadFrame.uniforms.data(), uploadFrame.uniforms.size());
	}
	WGPUCommandBuffer commands = wgpuCommandEncoderFinish(m_encoder, nullptr);
	wgpuQueueSubmit(webGPUQueue(), 1, &commands);
	uploadFrame.busy = true;
	++m_pendingSubmissions;
	AddRef();
	wgpuQueueOnSubmittedWorkDone(webGPUQueue(), WGPUQueueWorkDoneCallbackInfo {
		.mode = WGPUCallbackMode_AllowSpontaneous,
		.callback = onSubmittedWorkDone,
		.userdata1 = this,
		.userdata2 = &uploadFrame,
	});
	m_nextUploadFrame = (m_activeUploadFrame + 1) % kUploadFrameCount;
	#if !defined(__EMSCRIPTEN__)
	// GeneralsX @port Codex 04/08/2026 Browsers present WebGPU surfaces when requestAnimationFrame returns.
	wgpuSurfacePresent(m_context->surface());
	#endif
	wgpuCommandBufferRelease(commands);
	wgpuCommandEncoderRelease(m_encoder);
	m_encoder = nullptr;
	releaseFrame();
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetBackBuffer(UINT backBuffer, D3DBACKBUFFER_TYPE, IDirect3DSurface8 **surface)
{
	if (backBuffer || !surface) {
		return D3DERR_INVALIDCALL;
	}
	return ReturnObject<IDirect3DSurface8>(m_backBuffer, surface);
}

HRESULT WebGPUD3DDevice::GetRasterStatus(D3DRASTER_STATUS *status)
{
	if (!status) {
		return D3DERR_INVALIDCALL;
	}
	status->InVBlank = FALSE;
	status->ScanLine = 0;
	return D3D_OK;
}

void WebGPUD3DDevice::GetGammaRamp(D3DGAMMARAMP *ramp)
{
	if (!ramp) {
		return;
	}
	for (UINT index = 0; index < 256; ++index) {
		const WORD value = static_cast<WORD>(index * 257);
		ramp->red[index] = value;
		ramp->green[index] = value;
		ramp->blue[index] = value;
	}
}

HRESULT WebGPUD3DDevice::CreateTexture(UINT width, UINT height, UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DTexture8 **texture)
{
	if (!width || !height || !texture) {
		return D3DERR_INVALIDCALL;
	}
	*texture = new WebGPUTexture(this, width, height, levels, usage, format, pool);
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::CreateVertexBuffer(UINT length, DWORD usage, DWORD fvf, D3DPOOL pool, IDirect3DVertexBuffer8 **buffer)
{
	if (!length || !buffer) {
		return D3DERR_INVALIDCALL;
	}
	*buffer = new WebGPUVertexBuffer(this, length, usage, fvf, pool);
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::CreateIndexBuffer(UINT length, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DIndexBuffer8 **buffer)
{
	if (!length || !buffer || (format != D3DFMT_INDEX16 && format != D3DFMT_INDEX32)) {
		return D3DERR_INVALIDCALL;
	}
	*buffer = new WebGPUIndexBuffer(this, length, usage, format, pool);
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::CreateRenderTarget(UINT width, UINT height, D3DFORMAT format, D3DMULTISAMPLE_TYPE, WINBOOL, IDirect3DSurface8 **surface)
{
	if (!surface) {
		return D3DERR_INVALIDCALL;
	}
	*surface = new WebGPUSurface(this, width, height, format, D3DUSAGE_RENDERTARGET, D3DPOOL_DEFAULT);
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::CreateDepthStencilSurface(UINT width, UINT height, D3DFORMAT format, D3DMULTISAMPLE_TYPE, IDirect3DSurface8 **surface)
{
	if (!surface) {
		return D3DERR_INVALIDCALL;
	}
	*surface = new WebGPUSurface(this, width, height, format, D3DUSAGE_DEPTHSTENCIL, D3DPOOL_DEFAULT);
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::CreateImageSurface(UINT width, UINT height, D3DFORMAT format, IDirect3DSurface8 **surface)
{
	if (!surface) {
		return D3DERR_INVALIDCALL;
	}
	*surface = new WebGPUSurface(this, width, height, format, 0, D3DPOOL_SYSTEMMEM);
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::CopyRects(IDirect3DSurface8 *sourceInterface, const RECT *sourceRects, UINT rectCount, IDirect3DSurface8 *destinationInterface, const POINT *destinationPoints)
{
	auto *source = static_cast<WebGPUSurface *>(sourceInterface);
	auto *destination = static_cast<WebGPUSurface *>(destinationInterface);
	if (!source || !destination || source->format() != destination->format()) {
		return D3DERR_INVALIDCALL;
	}
	TextureLevel &sourceLevel = source->level();
	TextureLevel &destinationLevel = destination->level();
	if (!sourceRects || !rectCount) {
		destinationLevel.data = sourceLevel.data;
		destination->markDirty();
		return D3D_OK;
	}
	const UINT bytesPerPixel = sourceLevel.width ? sourceLevel.pitch / sourceLevel.width : 0;
	for (UINT rectIndex = 0; rectIndex < rectCount; ++rectIndex) {
		const RECT &sourceRect = sourceRects[rectIndex];
		const POINT destinationPoint = destinationPoints ? destinationPoints[rectIndex] : POINT { 0, 0 };
		const UINT width = static_cast<UINT>(sourceRect.right - sourceRect.left);
		const UINT height = static_cast<UINT>(sourceRect.bottom - sourceRect.top);
		for (UINT y = 0; y < height; ++y) {
			const uint8_t *sourceRow = sourceLevel.data.data() + static_cast<size_t>(sourceRect.top + y) * sourceLevel.pitch + sourceRect.left * bytesPerPixel;
			uint8_t *destinationRow = destinationLevel.data.data() + static_cast<size_t>(destinationPoint.y + y) * destinationLevel.pitch + destinationPoint.x * bytesPerPixel;
			std::memcpy(destinationRow, sourceRow, static_cast<size_t>(width) * bytesPerPixel);
		}
	}
	destination->markDirty();
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::UpdateTexture(IDirect3DBaseTexture8 *source, IDirect3DBaseTexture8 *destination)
{
	if (!source || !destination || source->GetType() != D3DRTYPE_TEXTURE || destination->GetType() != D3DRTYPE_TEXTURE) {
		return D3DERR_INVALIDCALL;
	}
	static_cast<WebGPUTexture *>(destination)->copyFrom(*static_cast<WebGPUTexture *>(source));
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::SetRenderTarget(IDirect3DSurface8 *renderTarget, IDirect3DSurface8 *depthStencil)
{
	finishPass();
	if (m_renderTarget) {
		m_renderTarget->Release();
	}
	m_renderTarget = static_cast<WebGPUSurface *>(renderTarget);
	if (m_renderTarget) {
		m_renderTarget->AddRef();
	}
	if (m_depthTarget) {
		m_depthTarget->Release();
	}
	m_depthTarget = static_cast<WebGPUSurface *>(depthStencil);
	if (m_depthTarget) {
		m_depthTarget->AddRef();
	}
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetRenderTarget(IDirect3DSurface8 **renderTarget)
{
	return ReturnObject<IDirect3DSurface8>(m_renderTarget ? m_renderTarget : m_backBuffer, renderTarget);
}

HRESULT WebGPUD3DDevice::GetDepthStencilSurface(IDirect3DSurface8 **depthStencil)
{
	return ReturnObject<IDirect3DSurface8>(m_depthTarget ? m_depthTarget : m_depthSurface, depthStencil);
}

HRESULT WebGPUD3DDevice::Clear(DWORD, const D3DRECT *, DWORD flags, D3DCOLOR color, float depth, DWORD stencil)
{
	return ensurePass(flags & (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL), color, depth, stencil);
}

HRESULT WebGPUD3DDevice::SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX *matrix)
{
	if (!matrix) {
		return D3DERR_INVALIDCALL;
	}
	m_transforms[state] = *matrix;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetTransform(D3DTRANSFORMSTATETYPE state, D3DMATRIX *matrix)
{
	if (!matrix) {
		return D3DERR_INVALIDCALL;
	}
	const auto found = m_transforms.find(state);
	*matrix = found == m_transforms.end() ? identityMatrix() : found->second;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::MultiplyTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX *matrix)
{
	if (!matrix) {
		return D3DERR_INVALIDCALL;
	}
	D3DMATRIX current;
	GetTransform(state, &current);
	D3DMATRIX result = {};
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			for (int index = 0; index < 4; ++index) {
				result.m[row][column] += current.m[row][index] * matrix->m[index][column];
			}
		}
	}
	m_transforms[state] = result;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::SetViewport(const D3DVIEWPORT8 *viewport)
{
	if (!viewport) {
		return D3DERR_INVALIDCALL;
	}
	m_viewport = *viewport;
	if (m_pass) {
		wgpuRenderPassEncoderSetViewport(m_pass, static_cast<float>(viewport->X), static_cast<float>(viewport->Y), static_cast<float>(viewport->Width), static_cast<float>(viewport->Height), viewport->MinZ, viewport->MaxZ);
	}
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetViewport(D3DVIEWPORT8 *viewport)
{
	if (!viewport) {
		return D3DERR_INVALIDCALL;
	}
	*viewport = m_viewport;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::SetMaterial(const D3DMATERIAL8 *material)
{
	if (!material) return D3DERR_INVALIDCALL;
	m_material = *material;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetMaterial(D3DMATERIAL8 *material)
{
	if (!material) return D3DERR_INVALIDCALL;
	*material = m_material;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::SetLight(DWORD index, const D3DLIGHT8 *light)
{
	if (!light) return D3DERR_INVALIDCALL;
	m_lights[index] = *light;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetLight(DWORD index, D3DLIGHT8 *light)
{
	if (!light || !m_lights.contains(index)) return D3DERR_INVALIDCALL;
	*light = m_lights[index];
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::LightEnable(DWORD index, WINBOOL enable)
{
	m_lightEnabled[index] = enable;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetLightEnable(DWORD index, WINBOOL *enable)
{
	if (!enable) return D3DERR_INVALIDCALL;
	*enable = m_lightEnabled[index];
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
	if (static_cast<UINT>(state) >= m_renderStates.size()) return D3DERR_INVALIDCALL;
	m_renderStates[state] = value;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetRenderState(D3DRENDERSTATETYPE state, DWORD *value)
{
	if (!value || static_cast<UINT>(state) >= m_renderStates.size()) return D3DERR_INVALIDCALL;
	*value = m_renderStates[state];
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetTexture(DWORD stage, IDirect3DBaseTexture8 **texture)
{
	if (stage >= m_textures.size() || !texture) return D3DERR_INVALIDCALL;
	*texture = m_textures[stage];
	if (*texture) (*texture)->AddRef();
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::SetTexture(DWORD stage, IDirect3DBaseTexture8 *texture)
{
	if (stage >= m_textures.size()) return D3DERR_INVALIDCALL;
	if (texture && texture->GetType() != D3DRTYPE_TEXTURE) return D3DERR_INVALIDCALL;
	if (texture) texture->AddRef();
	if (m_textures[stage]) m_textures[stage]->Release();
	m_textures[stage] = static_cast<WebGPUTexture *>(texture);
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD *value)
{
	if (!value || stage >= kTextureStageCount || static_cast<UINT>(type) >= kTextureStateCount) return D3DERR_INVALIDCALL;
	*value = m_textureStates[stage][type];
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
{
	if (stage >= kTextureStageCount || static_cast<UINT>(type) >= kTextureStateCount) return D3DERR_INVALIDCALL;
	m_textureStates[stage][type] = value;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::CreateVertexShader(const DWORD *declaration, const DWORD *function, DWORD *handle, DWORD)
{
	if (!declaration || !function || !handle || function[0] != D3DVS_VERSION(1, 1)) return D3DERR_INVALIDCALL;
	VertexShaderProgram program;
	UINT streamOffset = 0;
	bool hasStream = false;
	bool declarationEnded = false;
	for (UINT index = 0; index < 128; ++index) {
		const DWORD token = declaration[index];
		program.declaration.push_back(token);
		if (token == D3DVSD_END()) {
			declarationEnded = true;
			break;
		}
		const DWORD tokenType = (token & D3DVSD_TOKENTYPEMASK) >> D3DVSD_TOKENTYPESHIFT;
		if (tokenType == D3DVSD_TOKEN_STREAM) {
			if ((token & D3DVSD_STREAMTESSMASK) || (token & D3DVSD_STREAMNUMBERMASK) != 0) return D3DERR_INVALIDCALL;
			hasStream = true;
			streamOffset = 0;
			continue;
		}
		if (tokenType != D3DVSD_TOKEN_STREAMDATA || !hasStream) return D3DERR_INVALIDCALL;
		if (token & D3DVSD_DATALOADTYPEMASK) {
			streamOffset += ((token & D3DVSD_SKIPCOUNTMASK) >> D3DVSD_SKIPCOUNTSHIFT) * sizeof(DWORD);
			continue;
		}
		const DWORD dataType = (token & D3DVSD_DATATYPEMASK) >> D3DVSD_DATATYPESHIFT;
		UINT dataSize = 0;
		switch (dataType) {
			case D3DVSDT_FLOAT1: dataSize = sizeof(float); break;
			case D3DVSDT_FLOAT2: dataSize = 2 * sizeof(float); break;
			case D3DVSDT_FLOAT3: dataSize = 3 * sizeof(float); break;
			case D3DVSDT_FLOAT4: dataSize = 4 * sizeof(float); break;
			case D3DVSDT_D3DCOLOR:
			case D3DVSDT_UBYTE4:
			case D3DVSDT_SHORT2: dataSize = sizeof(DWORD); break;
			case D3DVSDT_SHORT4: dataSize = 2 * sizeof(DWORD); break;
			default: return D3DERR_INVALIDCALL;
		}
		program.elements.push_back({ token & D3DVSD_VERTEXREGMASK, dataType, streamOffset });
		streamOffset += dataSize;
	}
	if (!declarationEnded) return D3DERR_INVALIDCALL;
	program.function.push_back(function[0]);
	bool functionEnded = false;
	UINT offset = 1;
	for (UINT instructionIndex = 0; instructionIndex < 256; ++instructionIndex) {
		const DWORD token = function[offset++];
		program.function.push_back(token);
		const DWORD opcode = token & D3DSI_OPCODE_MASK;
		if (opcode == D3DSIO_END) {
			functionEnded = true;
			break;
		}
		if (opcode == D3DSIO_COMMENT) {
			const UINT commentSize = (token & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT;
			for (UINT index = 0; index < commentSize; ++index) program.function.push_back(function[offset++]);
			continue;
		}
		UINT sourceCount = 0;
		switch (opcode) {
			case D3DSIO_MOV: sourceCount = 1; break;
			case D3DSIO_ADD:
			case D3DSIO_SUB:
			case D3DSIO_MUL:
			case D3DSIO_M4x4: sourceCount = 2; break;
			case D3DSIO_MAD: sourceCount = 3; break;
			case D3DSIO_NOP: continue;
			default: return D3DERR_INVALIDCALL;
		}
		VertexShaderInstruction instruction = { opcode, function[offset++], {}, sourceCount };
		program.function.push_back(instruction.destination);
		for (UINT sourceIndex = 0; sourceIndex < sourceCount; ++sourceIndex) {
			instruction.sources[sourceIndex] = function[offset++];
			program.function.push_back(instruction.sources[sourceIndex]);
		}
		program.instructions.push_back(instruction);
	}
	if (!functionEnded) return D3DERR_INVALIDCALL;
	const DWORD shaderHandle = 0x80000000u | m_nextVertexShader++;
	m_vertexShaders.emplace(shaderHandle, std::move(program));
	*handle = shaderHandle;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::SetVertexShader(DWORD handle)
{
	if ((handle & 0x80000000u) && !m_vertexShaders.contains(handle)) return D3DERR_INVALIDCALL;
	m_vertexShader = handle;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::DeleteVertexShader(DWORD handle)
{
	if (!m_vertexShaders.erase(handle)) return D3DERR_INVALIDCALL;
	if (m_vertexShader == handle) m_vertexShader = D3DFVF_XYZ;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::SetVertexShaderConstant(DWORD startRegister, const void *data, DWORD count)
{
	if (!data || startRegister + count > m_vertexShaderConstants.size()) return D3DERR_INVALIDCALL;
	std::memcpy(m_vertexShaderConstants.data() + startRegister, data, count * sizeof(m_vertexShaderConstants[0]));
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetVertexShaderConstant(DWORD startRegister, void *data, DWORD count)
{
	if (!data || startRegister + count > m_vertexShaderConstants.size()) return D3DERR_INVALIDCALL;
	std::memcpy(data, m_vertexShaderConstants.data() + startRegister, count * sizeof(m_vertexShaderConstants[0]));
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetVertexShaderDeclaration(DWORD handle, void *data, DWORD *size)
{
	const auto found = m_vertexShaders.find(handle);
	if (found == m_vertexShaders.end() || !size) return D3DERR_INVALIDCALL;
	const DWORD requiredSize = found->second.declaration.size() * sizeof(DWORD);
	if (!data) {
		*size = requiredSize;
		return D3D_OK;
	}
	if (*size < requiredSize) {
		*size = requiredSize;
		return D3DERR_MOREDATA;
	}
	std::memcpy(data, found->second.declaration.data(), requiredSize);
	*size = requiredSize;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetVertexShaderFunction(DWORD handle, void *data, DWORD *size)
{
	const auto found = m_vertexShaders.find(handle);
	if (found == m_vertexShaders.end() || !size) return D3DERR_INVALIDCALL;
	const DWORD requiredSize = found->second.function.size() * sizeof(DWORD);
	if (!data) {
		*size = requiredSize;
		return D3D_OK;
	}
	if (*size < requiredSize) {
		*size = requiredSize;
		return D3DERR_MOREDATA;
	}
	std::memcpy(data, found->second.function.data(), requiredSize);
	*size = requiredSize;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::SetStreamSource(UINT stream, IDirect3DVertexBuffer8 *buffer, UINT stride)
{
	if (stream) return D3DERR_INVALIDCALL;
	if (buffer) buffer->AddRef();
	if (m_vertexBuffer) m_vertexBuffer->Release();
	m_vertexBuffer = static_cast<WebGPUVertexBuffer *>(buffer);
	m_vertexStride = stride;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetStreamSource(UINT stream, IDirect3DVertexBuffer8 **buffer, UINT *stride)
{
	if (stream || !buffer || !stride) return D3DERR_INVALIDCALL;
	*buffer = m_vertexBuffer;
	if (*buffer) (*buffer)->AddRef();
	*stride = m_vertexStride;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::SetIndices(IDirect3DIndexBuffer8 *indices, UINT baseVertexIndex)
{
	if (indices) indices->AddRef();
	if (m_indexBuffer) m_indexBuffer->Release();
	m_indexBuffer = static_cast<WebGPUIndexBuffer *>(indices);
	m_baseVertex = baseVertexIndex;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::GetIndices(IDirect3DIndexBuffer8 **indices, UINT *baseVertexIndex)
{
	if (!indices || !baseVertexIndex) return D3DERR_INVALIDCALL;
	*indices = m_indexBuffer;
	if (*indices) (*indices)->AddRef();
	*baseVertexIndex = m_baseVertex;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::CreatePixelShader(const DWORD *function, DWORD *handle)
{
	if (!function || !handle || function[0] != D3DPS_VERSION(1, 1)) {
		return D3DERR_INVALIDCALL;
	}
	*handle = 0;
	std::vector<DWORD> textureRegisters;
	std::vector<PixelShaderInstruction> instructions;
	UINT offset = 1;
	bool ended = false;
	for (UINT instructionIndex = 0; instructionIndex < 64; ++instructionIndex) {
		const DWORD token = function[offset++];
		const DWORD opcode = token & D3DSI_OPCODE_MASK;
		if (opcode == D3DSIO_END) {
			ended = true;
			break;
		}
		if (opcode == D3DSIO_COMMENT) {
			offset += (token & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT;
			continue;
		}
		if (opcode == D3DSIO_TEX) {
			if (!instructions.empty()) {
				return D3DERR_INVALIDCALL;
			}
			const DWORD destination = function[offset++];
			if ((destination & D3DSP_REGTYPE_MASK) != D3DSPR_TEXTURE ||
				(destination & D3DSP_WRITEMASK_ALL) != D3DSP_WRITEMASK_ALL) {
				return D3DERR_INVALIDCALL;
			}
			const DWORD textureRegister = destination & 0x7ff;
			if (textureRegister > 3) {
				return D3DERR_INVALIDCALL;
			}
			textureRegisters.push_back(textureRegister);
			continue;
		}
		UINT parameterCount = 0;
		switch (opcode) {
			case D3DSIO_MOV: parameterCount = 2; break;
			case D3DSIO_MUL: parameterCount = 3; break;
			case D3DSIO_LRP: parameterCount = 4; break;
			default: return D3DERR_INVALIDCALL;
		}
		PixelShaderInstruction instruction = { opcode, {}, parameterCount };
		for (UINT parameter = 0; parameter < parameterCount; ++parameter) {
			instruction.parameters[parameter] = function[offset++];
		}
		instructions.push_back(instruction);
	}
	if (!ended || textureRegisters.empty()) {
		return D3DERR_INVALIDCALL;
	}

	constexpr DWORD r0Destination = 0x800f0000;
	constexpr DWORD r0 = 0x80e40000;
	constexpr DWORD diffuse = 0x90e40000;
	constexpr DWORD diffuseAlpha = 0x90ff0000;
	auto texture = [](DWORD stage) { return 0xb0e40000 | stage; };
	auto matches = [](const PixelShaderInstruction &instruction, DWORD opcode,
		const std::array<DWORD, 4> &parameters, UINT parameterCount) {
		if (instruction.opcode != opcode || instruction.parameterCount != parameterCount) {
			return false;
		}
		return std::equal(parameters.begin(), parameters.begin() + parameterCount, instruction.parameters.begin());
	};
	auto hasSequentialTextures = [&]() {
		if (textureRegisters.size() < 2 || textureRegisters.size() > 4) {
			return false;
		}
		for (UINT stage = 0; stage < textureRegisters.size(); ++stage) {
			if (textureRegisters[stage] != stage) {
				return false;
			}
		}
		return true;
	};
	auto hasMultiplicationTail = [&](UINT firstTextureStage) {
		for (UINT stage = firstTextureStage; stage < textureRegisters.size(); ++stage) {
			if (!matches(instructions[stage], D3DSIO_MUL, { r0Destination, r0, texture(stage), 0 }, 3)) {
				return false;
			}
		}
		return true;
	};

	PixelShaderProgram program = {};
	if (hasSequentialTextures() && instructions.size() == textureRegisters.size() &&
		matches(instructions[0], D3DSIO_LRP, { r0Destination, diffuseAlpha, texture(1), texture(0) }, 4) &&
		matches(instructions[1], D3DSIO_MUL, { r0Destination, r0, diffuse, 0 }, 3) &&
		hasMultiplicationTail(2)) {
		program = { 1, static_cast<DWORD>(textureRegisters.size()) };
	} else if (hasSequentialTextures() && instructions.size() == textureRegisters.size() &&
		matches(instructions[0], D3DSIO_MUL, { r0Destination, texture(1), texture(0), 0 }, 3) &&
		matches(instructions[1], D3DSIO_MUL, { r0Destination, r0, diffuse, 0 }, 3) &&
		hasMultiplicationTail(2)) {
		program = { 2, static_cast<DWORD>(textureRegisters.size()) };
	} else if (textureRegisters == std::vector<DWORD>{ 1 } && instructions.size() == 2 &&
		matches(instructions[0], D3DSIO_MOV, { r0Destination, texture(1), 0, 0 }, 2) &&
		matches(instructions[1], D3DSIO_MUL, { r0Destination, r0, diffuse, 0 }, 3)) {
		program = { 3, 2 };
	} else if (textureRegisters == std::vector<DWORD>({ 0, 1, 2 }) && instructions.size() == 3 &&
		matches(instructions[0], D3DSIO_MUL, { r0Destination, texture(0), texture(1), 0 }, 3) &&
		matches(instructions[1], D3DSIO_MUL, { r0Destination, r0, texture(2), 0 }, 3) &&
		matches(instructions[2], D3DSIO_MUL, { r0Destination, r0, diffuse, 0 }, 3)) {
		program = { 4, 3 };
	} else {
		return D3DERR_INVALIDCALL;
	}

	while (!m_nextPixelShader || m_pixelShaders.contains(m_nextPixelShader)) {
		++m_nextPixelShader;
	}
	*handle = m_nextPixelShader++;
	m_pixelShaders.emplace(*handle, program);
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::SetPixelShader(DWORD handle)
{
	if (handle && !m_pixelShaders.contains(handle)) {
		return D3DERR_INVALIDCALL;
	}
	m_pixelShader = handle;
	return D3D_OK;
}

HRESULT WebGPUD3DDevice::DeletePixelShader(DWORD handle)
{
	if (!handle || !m_pixelShaders.erase(handle)) {
		return D3DERR_INVALIDCALL;
	}
	if (m_pixelShader == handle) {
		m_pixelShader = 0;
	}
	return D3D_OK;
}

} // namespace

namespace
{
WebGPUVertexBuffer::WebGPUVertexBuffer(WebGPUD3DDevice *device, UINT length, DWORD usage, DWORD fvf, D3DPOOL pool) :
	m_device(device),
	m_data(length),
	m_usage(usage),
	m_fvf(fvf),
	m_pool(pool)
{
	m_device->AddRef();
}

WebGPUVertexBuffer::~WebGPUVertexBuffer()
{
	m_device->Release();
}

HRESULT WebGPUVertexBuffer::QueryInterface(REFIID, void **output)
{
	if (!output) {
		return E_POINTER;
	}
	*output = this;
	AddRef();
	return S_OK;
}

ULONG WebGPUVertexBuffer::AddRef()
{
	return ++m_refs;
}

ULONG WebGPUVertexBuffer::Release()
{
	const ULONG refs = --m_refs;
	if (!refs) {
		delete this;
	}
	return refs;
}

HRESULT WebGPUVertexBuffer::GetDevice(IDirect3DDevice8 **device)
{
	return ReturnObject<IDirect3DDevice8>(m_device, device);
}

HRESULT WebGPUVertexBuffer::Lock(UINT offset, UINT size, BYTE **data, DWORD)
{
	if (!data || offset > m_data.size() || (size && static_cast<size_t>(offset) + size > m_data.size())) {
		return D3DERR_INVALIDCALL;
	}
	*data = m_data.data() + offset;
	return D3D_OK;
}

HRESULT WebGPUVertexBuffer::GetDesc(D3DVERTEXBUFFER_DESC *desc)
{
	if (!desc) {
		return D3DERR_INVALIDCALL;
	}
	*desc = {};
	desc->Format = D3DFMT_VERTEXDATA;
	desc->Type = D3DRTYPE_VERTEXBUFFER;
	desc->Usage = m_usage;
	desc->Pool = m_pool;
	desc->Size = static_cast<UINT>(m_data.size());
	desc->FVF = m_fvf;
	return D3D_OK;
}

WebGPUIndexBuffer::WebGPUIndexBuffer(WebGPUD3DDevice *device, UINT length, DWORD usage, D3DFORMAT format, D3DPOOL pool) :
	m_device(device),
	m_data(length),
	m_usage(usage),
	m_format(format),
	m_pool(pool)
{
	m_device->AddRef();
}

WebGPUIndexBuffer::~WebGPUIndexBuffer()
{
	m_device->Release();
}

HRESULT WebGPUIndexBuffer::QueryInterface(REFIID, void **output)
{
	if (!output) {
		return E_POINTER;
	}
	*output = this;
	AddRef();
	return S_OK;
}

ULONG WebGPUIndexBuffer::AddRef()
{
	return ++m_refs;
}

ULONG WebGPUIndexBuffer::Release()
{
	const ULONG refs = --m_refs;
	if (!refs) {
		delete this;
	}
	return refs;
}

HRESULT WebGPUIndexBuffer::GetDevice(IDirect3DDevice8 **device)
{
	return ReturnObject<IDirect3DDevice8>(m_device, device);
}

HRESULT WebGPUIndexBuffer::Lock(UINT offset, UINT size, BYTE **data, DWORD)
{
	if (!data || offset > m_data.size() || (size && static_cast<size_t>(offset) + size > m_data.size())) {
		return D3DERR_INVALIDCALL;
	}
	*data = m_data.data() + offset;
	return D3D_OK;
}

HRESULT WebGPUIndexBuffer::GetDesc(D3DINDEXBUFFER_DESC *desc)
{
	if (!desc) {
		return D3DERR_INVALIDCALL;
	}
	*desc = {};
	desc->Format = m_format;
	desc->Type = D3DRTYPE_INDEXBUFFER;
	desc->Usage = m_usage;
	desc->Pool = m_pool;
	desc->Size = static_cast<UINT>(m_data.size());
	return D3D_OK;
}

WebGPUTexture::WebGPUTexture(WebGPUD3DDevice *device, UINT width, UINT height, UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool) :
	m_device(device),
	m_usage(usage),
	m_format(format),
	m_pool(pool)
{
	m_device->AddRef();
	if (!levels) {
		levels = 1;
		for (UINT extent = std::max(width, height); extent > 1; extent >>= 1) {
			++levels;
		}
	}
	m_levels.reserve(levels);
	for (UINT level = 0; level < levels; ++level) {
		TextureLevel storage;
		storage.width = std::max(1U, width >> level);
		storage.height = std::max(1U, height >> level);
		storage.pitch = FormatRowPitch(format, storage.width);
		storage.data.resize(static_cast<size_t>(storage.pitch) * FormatRowCount(format, storage.height));
		m_levels.push_back(std::move(storage));
	}
}

WebGPUTexture::~WebGPUTexture()
{
	if (m_view) {
		wgpuTextureViewRelease(m_view);
	}
	if (m_texture) {
		wgpuTextureRelease(m_texture);
	}
	m_device->Release();
}

HRESULT WebGPUTexture::QueryInterface(REFIID, void **output)
{
	if (!output) {
		return E_POINTER;
	}
	*output = this;
	AddRef();
	return S_OK;
}

ULONG WebGPUTexture::AddRef()
{
	return ++m_refs;
}

ULONG WebGPUTexture::Release()
{
	const ULONG refs = --m_refs;
	if (!refs) {
		delete this;
	}
	return refs;
}

HRESULT WebGPUTexture::GetDevice(IDirect3DDevice8 **device)
{
	return ReturnObject<IDirect3DDevice8>(m_device, device);
}

void WebGPUTexture::PreLoad()
{
	upload();
}

HRESULT WebGPUTexture::GetLevelDesc(UINT levelIndex, D3DSURFACE_DESC *desc)
{
	if (!desc || levelIndex >= m_levels.size()) {
		return D3DERR_INVALIDCALL;
	}
	const TextureLevel &storage = m_levels[levelIndex];
	*desc = {};
	desc->Format = m_format;
	desc->Type = D3DRTYPE_SURFACE;
	desc->Usage = m_usage;
	desc->Pool = m_pool;
	desc->Size = static_cast<UINT>(storage.data.size());
	desc->Width = storage.width;
	desc->Height = storage.height;
	return D3D_OK;
}

HRESULT WebGPUTexture::GetSurfaceLevel(UINT levelIndex, IDirect3DSurface8 **surface)
{
	if (!surface || levelIndex >= m_levels.size()) {
		return D3DERR_INVALIDCALL;
	}
	*surface = new WebGPUSurface(this, levelIndex);
	return D3D_OK;
}

HRESULT WebGPUTexture::LockRect(UINT levelIndex, D3DLOCKED_RECT *lockedRect, const RECT *rect, DWORD)
{
	if (!lockedRect || levelIndex >= m_levels.size()) {
		return D3DERR_INVALIDCALL;
	}
	TextureLevel &storage = m_levels[levelIndex];
	if (rect && (rect->left < 0 || rect->top < 0 || rect->right > static_cast<LONG>(storage.width) || rect->bottom > static_cast<LONG>(storage.height))) {
		return D3DERR_INVALIDCALL;
	}
	const UINT x = rect ? static_cast<UINT>(rect->left) : 0;
	const UINT y = rect ? static_cast<UINT>(rect->top) : 0;
	const UINT bytesPerPixel = storage.width ? storage.pitch / storage.width : 0;
	lockedRect->Pitch = storage.pitch;
	lockedRect->pBits = storage.data.data() + static_cast<size_t>(y) * storage.pitch + x * bytesPerPixel;
	m_dirty = true;
	return D3D_OK;
}

HRESULT WebGPUTexture::UnlockRect(UINT levelIndex)
{
	if (levelIndex >= m_levels.size()) {
		return D3DERR_INVALIDCALL;
	}
	m_dirty = true;
	return D3D_OK;
}

bool WebGPUTexture::upload()
{
	if (!m_dirty && m_view) {
		return true;
	}
	// GeneralsX @bugfix Meeseeks 16/11/2026 Create D3DUSAGE_DEPTHSTENCIL textures (e.g. water reflection Z-textures)
	// as Depth24PlusStencil8 rather than RGBA8Unorm so WebGPU render passes accept them as depthStencilAttachment.
	if (isDepthStencil()) {
		if (!m_texture) {
			WGPUTextureDescriptor descriptor = WGPU_TEXTURE_DESCRIPTOR_INIT;
			descriptor.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding;
			descriptor.dimension = WGPUTextureDimension_2D;
			descriptor.size = { m_levels[0].width, m_levels[0].height, 1 };
			descriptor.format = WGPUTextureFormat_Depth24PlusStencil8;
			descriptor.mipLevelCount = static_cast<uint32_t>(m_levels.size());
			descriptor.sampleCount = 1;
			m_texture = wgpuDeviceCreateTexture(m_device->webGPUDevice(), &descriptor);
			if (!m_texture) {
				return false;
			}
			WGPUTextureViewDescriptor viewDesc = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
			viewDesc.format = WGPUTextureFormat_Depth24PlusStencil8;
			viewDesc.dimension = WGPUTextureViewDimension_2D;
			viewDesc.baseMipLevel = 0;
			viewDesc.mipLevelCount = static_cast<uint32_t>(m_levels.size());
			viewDesc.baseArrayLayer = 0;
			viewDesc.arrayLayerCount = 1;
			viewDesc.aspect = WGPUTextureAspect_DepthOnly;
			m_view = wgpuTextureCreateView(m_texture, &viewDesc);
		}
		m_dirty = false;
		return m_view != nullptr;
	}
	const WGPUTextureFormat compressedFormat = wgpuDeviceHasFeature(m_device->webGPUDevice(), WGPUFeatureName_TextureCompressionBC)
		? CompressedTextureFormat(m_format) : WGPUTextureFormat_Undefined;
	const bool compressed = compressedFormat != WGPUTextureFormat_Undefined;
	if (!m_texture) {
		WGPUTextureDescriptor descriptor = WGPU_TEXTURE_DESCRIPTOR_INIT;
		descriptor.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
		if (compressedFormat == WGPUTextureFormat_Undefined) {
			descriptor.usage |= WGPUTextureUsage_RenderAttachment;
		}
		descriptor.dimension = WGPUTextureDimension_2D;
		descriptor.size = { m_levels[0].width, m_levels[0].height, 1 };
		descriptor.format = compressedFormat == WGPUTextureFormat_Undefined
			? WGPUTextureFormat_RGBA8Unorm : compressedFormat;
		descriptor.mipLevelCount = static_cast<uint32_t>(m_levels.size());
		descriptor.sampleCount = 1;
		m_texture = wgpuDeviceCreateTexture(m_device->webGPUDevice(), &descriptor);
		if (!m_texture) {
			return false;
		}
		m_view = wgpuTextureCreateView(m_texture, nullptr);
	}
	for (UINT levelIndex = 0; levelIndex < m_levels.size(); ++levelIndex) {
		const TextureLevel &storage = m_levels[levelIndex];
		std::vector<uint8_t> rgba;
		const uint8_t *source = storage.data.data();
		UINT rowBytes = storage.pitch;
		UINT rowCount = FormatRowCount(m_format, storage.height);
		if (!compressed) {
			ConvertToRGBA(source, storage.width, storage.height, m_format, rgba);
			source = rgba.data();
			rowBytes = storage.width * 4;
			rowCount = storage.height;
		}
		const UINT paddedRowBytes = (rowBytes + 255) & ~255U;
		std::vector<uint8_t> padded(static_cast<size_t>(paddedRowBytes) * rowCount);
		for (UINT y = 0; y < rowCount; ++y) {
			std::memcpy(padded.data() + static_cast<size_t>(y) * paddedRowBytes, source + static_cast<size_t>(y) * rowBytes, rowBytes);
		}

		WGPUTexelCopyTextureInfo destination = WGPU_TEXEL_COPY_TEXTURE_INFO_INIT;
		destination.texture = m_texture;
		destination.mipLevel = levelIndex;
		destination.aspect = WGPUTextureAspect_All;
		WGPUTexelCopyBufferLayout layout = WGPU_TEXEL_COPY_BUFFER_LAYOUT_INIT;
		layout.bytesPerRow = paddedRowBytes;
		layout.rowsPerImage = rowCount;
		WGPUExtent3D extent = { storage.width, storage.height, 1 };
		wgpuQueueWriteTexture(m_device->webGPUQueue(), &destination, padded.data(), padded.size(), &layout, &extent);
	}
	m_dirty = false;
	return true;
}

WGPUTextureView WebGPUTexture::view()
{
	return upload() ? m_view : nullptr;
}

// GeneralsX @port Codex 04/08/2026 Expose one texture mip as a WebGPU render attachment.
WGPUTextureView WebGPUTexture::createRenderView(UINT levelIndex)
{
	if (levelIndex >= m_levels.size() || !upload()) {
		return nullptr;
	}
	WGPUTextureViewDescriptor descriptor = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
	descriptor.format = isDepthStencil() ? WGPUTextureFormat_Depth24PlusStencil8 : WGPUTextureFormat_RGBA8Unorm;
	descriptor.dimension = WGPUTextureViewDimension_2D;
	descriptor.baseMipLevel = levelIndex;
	descriptor.mipLevelCount = 1;
	descriptor.baseArrayLayer = 0;
	descriptor.arrayLayerCount = 1;
	descriptor.aspect = WGPUTextureAspect_All;
	return wgpuTextureCreateView(m_texture, &descriptor);
}

void WebGPUTexture::copyFrom(const WebGPUTexture &source)
{
	const size_t count = std::min(m_levels.size(), source.m_levels.size());
	for (size_t index = 0; index < count; ++index) {
		m_levels[index].data = source.m_levels[index].data;
	}
	m_dirty = true;
}

WebGPUSurface::WebGPUSurface(WebGPUD3DDevice *device, UINT width, UINT height, D3DFORMAT format, DWORD usage, D3DPOOL pool, bool retainDevice) :
	m_device(device),
	m_format(format),
	m_usage(usage),
	m_pool(pool),
	m_retainsDevice(retainDevice)
{
	if (m_retainsDevice) {
		m_device->AddRef();
	}
	m_storage.width = width;
	m_storage.height = height;
	m_storage.pitch = FormatRowPitch(format, width);
	m_storage.data.resize(static_cast<size_t>(m_storage.pitch) * FormatRowCount(format, height));
}

WebGPUSurface::WebGPUSurface(WebGPUTexture *texture, UINT levelIndex) :
	m_texture(texture),
	m_level(levelIndex),
	m_format(texture->format()),
	m_usage(0),
	m_pool(D3DPOOL_DEFAULT)
{
	m_texture->AddRef();
	IDirect3DDevice8 *device = nullptr;
	m_texture->GetDevice(&device);
	m_device = static_cast<WebGPUD3DDevice *>(device);
}

WebGPUSurface::~WebGPUSurface()
{
	if (m_gpuView) {
		wgpuTextureViewRelease(m_gpuView);
	}
	if (m_gpuTexture) {
		wgpuTextureRelease(m_gpuTexture);
	}
	if (m_texture) {
		m_texture->Release();
	}
	if (m_device && m_retainsDevice) {
		m_device->Release();
	}
}

HRESULT WebGPUSurface::QueryInterface(REFIID, void **output)
{
	if (!output) {
		return E_POINTER;
	}
	*output = this;
	AddRef();
	return S_OK;
}

ULONG WebGPUSurface::AddRef()
{
	return ++m_refs;
}

ULONG WebGPUSurface::Release()
{
	const ULONG refs = --m_refs;
	if (!refs) {
		delete this;
	}
	return refs;
}

HRESULT WebGPUSurface::GetDevice(IDirect3DDevice8 **device)
{
	return ReturnObject<IDirect3DDevice8>(m_device, device);
}

HRESULT WebGPUSurface::GetContainer(REFIID, void **container)
{
	if (!container || !m_texture) {
		return E_NOINTERFACE;
	}
	*container = m_texture;
	m_texture->AddRef();
	return S_OK;
}

HRESULT WebGPUSurface::GetDesc(D3DSURFACE_DESC *desc)
{
	if (!desc) {
		return D3DERR_INVALIDCALL;
	}
	const TextureLevel &storage = level();
	*desc = {};
	desc->Format = m_format;
	desc->Type = D3DRTYPE_SURFACE;
	desc->Usage = m_usage;
	desc->Pool = m_pool;
	desc->Size = static_cast<UINT>(storage.data.size());
	desc->Width = storage.width;
	desc->Height = storage.height;
	return D3D_OK;
}

HRESULT WebGPUSurface::LockRect(D3DLOCKED_RECT *lockedRect, const RECT *rect, DWORD flags)
{
	if (m_texture) {
		return m_texture->LockRect(m_level, lockedRect, rect, flags);
	}
	if (!lockedRect) {
		return D3DERR_INVALIDCALL;
	}
	const UINT x = rect ? static_cast<UINT>(rect->left) : 0;
	const UINT y = rect ? static_cast<UINT>(rect->top) : 0;
	const UINT bytesPerPixel = m_storage.width ? m_storage.pitch / m_storage.width : 0;
	lockedRect->Pitch = m_storage.pitch;
	lockedRect->pBits = m_storage.data.data() + static_cast<size_t>(y) * m_storage.pitch + x * bytesPerPixel;
	return D3D_OK;
}

HRESULT WebGPUSurface::UnlockRect()
{
	return m_texture ? m_texture->UnlockRect(m_level) : D3D_OK;
}

TextureLevel &WebGPUSurface::level()
{
	return m_texture ? m_texture->level(m_level) : m_storage;
}

const TextureLevel &WebGPUSurface::level() const
{
	return m_texture ? m_texture->level(m_level) : m_storage;
}

void WebGPUSurface::markDirty()
{
	if (m_texture) {
		m_texture->markDirty();
	}
}

// GeneralsX @port Codex 04/08/2026 Back Direct3D render and depth surfaces with WebGPU attachments.
WGPUTextureView WebGPUSurface::renderView()
{
	if (m_gpuView) {
		return m_gpuView;
	}
	if (m_texture) {
		m_gpuView = m_texture->createRenderView(m_level);
		return m_gpuView;
	}
	if (!(m_usage & (D3DUSAGE_RENDERTARGET | D3DUSAGE_DEPTHSTENCIL))) {
		return nullptr;
	}

	const bool depth = (m_usage & D3DUSAGE_DEPTHSTENCIL) != 0;
	WGPUTextureDescriptor descriptor = WGPU_TEXTURE_DESCRIPTOR_INIT;
	descriptor.usage = WGPUTextureUsage_RenderAttachment;
	descriptor.dimension = WGPUTextureDimension_2D;
	descriptor.size = { m_storage.width, m_storage.height, 1 };
	descriptor.format = depth ? WGPUTextureFormat_Depth24PlusStencil8 : WGPUTextureFormat_RGBA8Unorm;
	descriptor.mipLevelCount = 1;
	descriptor.sampleCount = 1;
	m_gpuTexture = wgpuDeviceCreateTexture(m_device->webGPUDevice(), &descriptor);
	m_gpuView = m_gpuTexture ? wgpuTextureCreateView(m_gpuTexture, nullptr) : nullptr;
	return m_gpuView;
}

bool WebGPUDirect3D8::initialize()
{
	if (!m_context.initialize("#canvas", kDefaultWidth, kDefaultHeight, nullptr, nullptr)) {
		return false;
	}
	while (!m_context.isReady() && !m_context.hasFailed()) {
		emscripten_sleep(1);
	}
	return m_context.isReady();
}

HRESULT WebGPUDirect3D8::QueryInterface(REFIID, void **output)
{
	if (!output) {
		return E_POINTER;
	}
	*output = this;
	AddRef();
	return S_OK;
}

ULONG WebGPUDirect3D8::Release()
{
	const ULONG refs = --m_refs;
	if (!refs) {
		delete this;
	}
	return refs;
}

HRESULT WebGPUDirect3D8::GetAdapterIdentifier(UINT adapter, DWORD, D3DADAPTER_IDENTIFIER8 *identifier)
{
	if (adapter || !identifier) {
		return D3DERR_INVALIDCALL;
	}
	*identifier = {};
	std::strncpy(identifier->Driver, "WebGPU", sizeof(identifier->Driver) - 1);
	std::strncpy(identifier->Description, "Chrome WebGPU", sizeof(identifier->Description) - 1);
	identifier->VendorId = 0;
	identifier->DeviceId = 0;
	identifier->WHQLLevel = 1;
	return D3D_OK;
}

HRESULT WebGPUDirect3D8::EnumAdapterModes(UINT adapter, UINT modeIndex, D3DDISPLAYMODE *displayMode)
{
	static constexpr std::array<std::pair<UINT, UINT>, 6> modes = {
		std::pair { 800U, 600U },
		std::pair { 1024U, 768U },
		std::pair { 1280U, 720U },
		std::pair { 1280U, 800U },
		std::pair { 1600U, 900U },
		std::pair { 1920U, 1080U },
	};
	if (adapter || modeIndex >= modes.size() || !displayMode) {
		return D3DERR_INVALIDCALL;
	}
	displayMode->Width = modes[modeIndex].first;
	displayMode->Height = modes[modeIndex].second;
	displayMode->RefreshRate = 60;
	displayMode->Format = D3DFMT_X8R8G8B8;
	return D3D_OK;
}

HRESULT WebGPUDirect3D8::GetAdapterDisplayMode(UINT adapter, D3DDISPLAYMODE *displayMode)
{
	if (adapter || !displayMode) {
		return D3DERR_INVALIDCALL;
	}
	displayMode->Width = kDefaultWidth;
	displayMode->Height = kDefaultHeight;
	displayMode->RefreshRate = 60;
	displayMode->Format = D3DFMT_X8R8G8B8;
	return D3D_OK;
}

HRESULT WebGPUDirect3D8::CheckDeviceFormat(UINT adapter, D3DDEVTYPE, D3DFORMAT, DWORD usage, D3DRESOURCETYPE type, D3DFORMAT format)
{
	if (adapter || (type != D3DRTYPE_TEXTURE && type != D3DRTYPE_SURFACE)) {
		return D3DERR_NOTAVAILABLE;
	}
	if (usage & D3DUSAGE_DEPTHSTENCIL) {
		return (format == D3DFMT_D16 || format == D3DFMT_D24S8 || format == D3DFMT_D24X8 || format == D3DFMT_D32) ? D3D_OK : D3DERR_NOTAVAILABLE;
	}
	switch (format) {
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
		case D3DFMT_R5G6B5:
		case D3DFMT_A4R4G4B4:
		case D3DFMT_A1R5G5B5:
		case D3DFMT_L8:
		case D3DFMT_A8:
		case D3DFMT_A8L8:
		case D3DFMT_DXT1:
		case D3DFMT_DXT2:
		case D3DFMT_DXT3:
		case D3DFMT_DXT4:
		case D3DFMT_DXT5:
			return D3D_OK;
		default:
			return D3DERR_NOTAVAILABLE;
	}
}

HRESULT WebGPUDirect3D8::CheckDepthStencilMatch(UINT adapter, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, D3DFORMAT depthFormat)
{
	if (adapter) {
		return D3DERR_INVALIDCALL;
	}
	return (depthFormat == D3DFMT_D16 || depthFormat == D3DFMT_D24S8 || depthFormat == D3DFMT_D24X8 || depthFormat == D3DFMT_D32) ? D3D_OK : D3DERR_NOTAVAILABLE;
}

HRESULT WebGPUDirect3D8::GetDeviceCaps(UINT adapter, D3DDEVTYPE type, D3DCAPS8 *caps)
{
	if (adapter || !caps) {
		return D3DERR_INVALIDCALL;
	}
	*caps = {};
	caps->DeviceType = type;
	caps->AdapterOrdinal = adapter;
	caps->Caps = D3DCAPS_READ_SCANLINE;
	caps->DevCaps = D3DDEVCAPS_HWRASTERIZATION | D3DDEVCAPS_DRAWPRIMTLVERTEX | D3DDEVCAPS_DRAWPRIMITIVES2 | D3DDEVCAPS_DRAWPRIMITIVES2EX;
	caps->PrimitiveMiscCaps = D3DPMISCCAPS_CULLNONE | D3DPMISCCAPS_CULLCW | D3DPMISCCAPS_CULLCCW | D3DPMISCCAPS_COLORWRITEENABLE;
	caps->RasterCaps = D3DPRASTERCAPS_DITHER | D3DPRASTERCAPS_FOGVERTEX;
	caps->ZCmpCaps = D3DPCMPCAPS_ALWAYS | D3DPCMPCAPS_LESSEQUAL | D3DPCMPCAPS_LESS | D3DPCMPCAPS_EQUAL | D3DPCMPCAPS_GREATER | D3DPCMPCAPS_GREATEREQUAL;
	caps->SrcBlendCaps = D3DPBLENDCAPS_ZERO | D3DPBLENDCAPS_ONE | D3DPBLENDCAPS_SRCCOLOR | D3DPBLENDCAPS_INVSRCCOLOR |
		D3DPBLENDCAPS_SRCALPHA | D3DPBLENDCAPS_INVSRCALPHA | D3DPBLENDCAPS_DESTALPHA |
		D3DPBLENDCAPS_INVDESTALPHA | D3DPBLENDCAPS_DESTCOLOR | D3DPBLENDCAPS_INVDESTCOLOR;
	caps->DestBlendCaps = caps->SrcBlendCaps;
	caps->AlphaCmpCaps = caps->ZCmpCaps;
	caps->StencilCaps = D3DSTENCILCAPS_KEEP | D3DSTENCILCAPS_ZERO | D3DSTENCILCAPS_REPLACE |
		D3DSTENCILCAPS_INCRSAT | D3DSTENCILCAPS_DECRSAT | D3DSTENCILCAPS_INVERT |
		D3DSTENCILCAPS_INCR | D3DSTENCILCAPS_DECR;
	caps->ShadeCaps = D3DPSHADECAPS_COLORGOURAUDRGB | D3DPSHADECAPS_ALPHAGOURAUDBLEND;
	caps->TextureCaps = D3DPTEXTURECAPS_ALPHA | D3DPTEXTURECAPS_MIPMAP | D3DPTEXTURECAPS_PERSPECTIVE;
	caps->TextureFilterCaps = D3DPTFILTERCAPS_MINFPOINT | D3DPTFILTERCAPS_MINFLINEAR | D3DPTFILTERCAPS_MAGFPOINT | D3DPTFILTERCAPS_MAGFLINEAR | D3DPTFILTERCAPS_MIPFPOINT | D3DPTFILTERCAPS_MIPFLINEAR;
	caps->TextureAddressCaps = D3DPTADDRESSCAPS_WRAP | D3DPTADDRESSCAPS_CLAMP | D3DPTADDRESSCAPS_MIRROR;
	caps->TextureOpCaps = D3DTEXOPCAPS_DISABLE | D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_SELECTARG2 |
		D3DTEXOPCAPS_MODULATE | D3DTEXOPCAPS_MODULATE2X | D3DTEXOPCAPS_MODULATE4X | D3DTEXOPCAPS_ADD |
		D3DTEXOPCAPS_ADDSIGNED | D3DTEXOPCAPS_ADDSIGNED2X | D3DTEXOPCAPS_SUBTRACT | D3DTEXOPCAPS_ADDSMOOTH |
		D3DTEXOPCAPS_BLENDDIFFUSEALPHA | D3DTEXOPCAPS_BLENDTEXTUREALPHA | D3DTEXOPCAPS_BLENDFACTORALPHA |
		D3DTEXOPCAPS_BLENDTEXTUREALPHAPM | D3DTEXOPCAPS_BLENDCURRENTALPHA |
		D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR | D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA |
		D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR | D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA |
		D3DTEXOPCAPS_MULTIPLYADD | D3DTEXOPCAPS_LERP;
	caps->MaxTextureWidth = 8192;
	caps->MaxTextureHeight = 8192;
	caps->MaxTextureRepeat = 8192;
	caps->MaxTextureAspectRatio = 8192;
	caps->MaxAnisotropy = 16;
	caps->MaxVertexW = 1.0e10f;
	caps->GuardBandLeft = -1.0e10f;
	caps->GuardBandTop = -1.0e10f;
	caps->GuardBandRight = 1.0e10f;
	caps->GuardBandBottom = 1.0e10f;
	caps->MaxActiveLights = 8;
	caps->MaxUserClipPlanes = 0;
	caps->MaxVertexBlendMatrices = 1;
	caps->MaxPrimitiveCount = 0x00ffffff;
	caps->MaxVertexIndex = 0x00ffffff;
	caps->MaxStreams = 1;
	caps->MaxStreamStride = 255;
	caps->MaxSimultaneousTextures = 4;
	caps->MaxTextureBlendStages = 4;
	caps->PixelShaderVersion = D3DPS_VERSION(1, 1);
	caps->MaxPixelShaderValue = 1.0f;
	caps->MaxPointSize = 1.0f;
	return D3D_OK;
}

HRESULT WebGPUDirect3D8::CreateDevice(UINT adapter, D3DDEVTYPE type, HWND window, DWORD behavior, D3DPRESENT_PARAMETERS *parameters, IDirect3DDevice8 **device)
{
	if (adapter || !parameters || !device) {
		return D3DERR_INVALIDCALL;
	}
	if (parameters->BackBufferWidth > 0 && parameters->BackBufferHeight > 0 &&
		!m_context.resize(parameters->BackBufferWidth, parameters->BackBufferHeight)) {
		return D3DERR_NOTAVAILABLE;
	}
	auto *created = new WebGPUD3DDevice(this, &m_context, *parameters);
	created->m_creation.AdapterOrdinal = adapter;
	created->m_creation.DeviceType = type;
	created->m_creation.hFocusWindow = window;
	created->m_creation.BehaviorFlags = behavior;
	if (!created->initialize()) {
		created->Release();
		return D3DERR_NOTAVAILABLE;
	}
	*device = created;
	return D3D_OK;
}

} // namespace
