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

#pragma once

/**
 * Phase 50+: Metal Graphics Driver (Stub)
 *
 * This is a placeholder for future Metal backend implementation.
 * Currently NOT implemented - Vulkan is the primary backend.
 *
 * Design Notes:
 * - Will provide Metal rendering for macOS/iOS
 * - Modern Metal 3 features
 * - Metal Shading Language (MSL) for shaders
 * - Command buffer and render command encoder model
 * - Complete D3D8→Metal state mapping
 *
 * TODO (Phase 50+):
 * - Metal device and command queue creation
 * - Metal library compilation (MSL)
 * - Render pipeline state management
 * - Texture and buffer management
 * - Command buffer recording
 * - Drawable acquisition for presentation
 */

#include "../IGraphicsDriver.h"

/**
 * Metal graphics backend implementation
 * Provides high-performance rendering for macOS
 */
class MetalGraphicsDriver : public IGraphicsDriver
{
public:
	MetalGraphicsDriver() = default;
	virtual ~MetalGraphicsDriver() override = default;

	// Initialization & Lifecycle
	virtual bool Initialize(void* window_handle, bool debug_mode = false) override;
	virtual void Shutdown() override;
	virtual bool IsInitialized() const override { return false; }
	virtual const char* GetBackendName() const override { return "Metal"; }

	// Frame Management
	virtual void BeginFrame() override;
	virtual void EndFrame() override;
	virtual bool Present() override;
	virtual void Clear(ClearFlags flags, const Color& color, float depth = 1.0f, uint8_t stencil = 0) override;

	// Rendering Operations
	virtual void BeginScene() override;
	virtual void EndScene() override;

	// Draw Operations
	virtual void DrawPrimitive(PrimitiveType primitive_type, uint16_t start_vertex, uint16_t vertex_count) override;
	virtual void DrawIndexedPrimitive(PrimitiveType primitive_type, uint16_t min_vertex_index,
	                                   uint16_t vertex_count, uint16_t start_index, uint16_t polygon_count) override;

	// Render State Management
	virtual void SetRenderState(RenderState state, uint32_t value) override;
	virtual void SetTextureStageState(uint32_t stage, uint32_t state_type, uint32_t value) override;

	// Vertex & Index Buffer Management
	virtual void SetStreamSource(uint32_t stream, VertexBufferHandle buffer, uint32_t stride) override;
	virtual void SetIndexBuffer(IndexBufferHandle buffer, uint32_t stride = 0) override;
	virtual void SetTexture(uint32_t stage, TextureHandle texture) override;

	// Vertex Format
	virtual VertexFormatHandle CreateVertexFormat(const VertexElement* elements, uint32_t element_count) override;
	virtual void DestroyVertexFormat(VertexFormatHandle handle) override;
	virtual void SetVertexFormat(VertexFormatHandle handle) override;

	// Texture Management
	virtual TextureHandle CreateTexture(const TextureDescriptor& descriptor) override;
	virtual void DestroyTexture(TextureHandle handle) override;
	virtual TextureHandle CreateTextureFromMemory(uint32_t width, uint32_t height, TextureFormat format,
	                                                const void* pixel_data, uint32_t data_size) override;

	// Buffer Management
	virtual VertexBufferHandle CreateVertexBuffer(const void* data, uint32_t size) override;
	virtual void DestroyVertexBuffer(VertexBufferHandle handle) override;
	virtual IndexBufferHandle CreateIndexBuffer(const void* data, uint32_t size) override;
	virtual void DestroyIndexBuffer(IndexBufferHandle handle) override;

	// Viewport & Scissor
	virtual void SetViewport(float x, float y, float width, float height, float min_z = 0.0f, float max_z = 1.0f) override;
	virtual void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) override;

	// Transform Matrices
	virtual void SetTransform(TransformType transform_type, const float* matrix) override;

	// Material & Lighting
	virtual void SetMaterial(const Material& material) override;
	virtual void SetLightingEnabled(bool enabled) override;
	virtual void SetAmbientLight(const Color& color) override;
	virtual void SetLight(uint32_t index, const Light& light) override;
	virtual void SetLightEnabled(uint32_t index, bool enabled) override;

	// Texture Stage State
	virtual void SetTextureAddressMode(uint32_t stage, AddressMode mode) override;
	virtual void SetTextureFilterMode(uint32_t stage, FilterMode mode) override;

	// Querying & Capabilities
	virtual void GetDeviceCaps(void* caps_struct, uint32_t caps_size) override;
	virtual void GetDisplayMode(uint32_t* out_width, uint32_t* out_height) override;
	virtual uint32_t GetMaxTextureSize() const override;
	virtual uint32_t GetMaxLights() const override;
	virtual bool IsTextureFormatSupported(TextureFormat format) const override;

	// Debugging & Utilities
	virtual void SetDebugName(const char* name) override;
	virtual bool TakeScreenshot(const char* filename) override;
	virtual float GetFrameTime() const override;
	virtual float GetFramesPerSecond() const override;

private:
	bool m_initialized = false;
};

#endif // METAL_GRAPHICS_DRIVER_H
