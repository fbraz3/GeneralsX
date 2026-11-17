#pragma once

#include <cstdint>
#include <webgpu/webgpu.h>

// GeneralsX @feature Codex 04/08/2026 Own browser WebGPU initialization and presentation.
class WebGPUContext
{
public:
	using ReadyCallback = void (*)(WebGPUContext &, void *);

	WebGPUContext() = default;
	~WebGPUContext();

	WebGPUContext(const WebGPUContext &) = delete;
	WebGPUContext &operator=(const WebGPUContext &) = delete;

	bool initialize(const char *canvasSelector, uint32_t width, uint32_t height, ReadyCallback callback, void *userData);
	bool resize(uint32_t width, uint32_t height);
	bool renderClear(double red, double green, double blue, double alpha);
	bool isReady() const;
	bool hasFailed() const;
	WGPUDevice device() const;
	WGPUQueue queue() const;
	WGPUSurface surface() const;
	WGPUTextureFormat surfaceFormat() const;
	uint32_t width() const;
	uint32_t height() const;

private:
	static void onAdapterRequest(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void *userData1, void *userData2);
	static void onDeviceRequest(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void *userData1, void *userData2);
	bool configureSurface();
	void reportFailure(const char *stage, WGPUStringView message);

	WGPUInstance m_instance = nullptr;
	WGPUSurface m_surface = nullptr;
	WGPUAdapter m_adapter = nullptr;
	WGPUDevice m_device = nullptr;
	WGPUQueue m_queue = nullptr;
	WGPUTextureFormat m_surfaceFormat = WGPUTextureFormat_Undefined;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	ReadyCallback m_callback = nullptr;
	void *m_userData = nullptr;
	bool m_failed = false;
};
