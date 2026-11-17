#include "WebGPUDevice/WebGPUContext.h"

#include <cstdio>

namespace
{
WGPUStringView MakeStringView(const char *value)
{
	return { value, WGPU_STRLEN };
}

void OnUncapturedError(WGPUDevice const *, WGPUErrorType type, WGPUStringView message, void *, void *)
{
	fprintf(stderr, "WebGPU validation error type=%d: %.*s\n", static_cast<int>(type), static_cast<int>(message.length), message.data ? message.data : "");
}
}

// GeneralsX @feature Codex 04/08/2026 Implement browser WebGPU initialization and presentation.
WebGPUContext::~WebGPUContext()
{
	if (m_surface) {
		wgpuSurfaceUnconfigure(m_surface);
	}
	if (m_queue) {
		wgpuQueueRelease(m_queue);
	}
	if (m_device) {
		wgpuDeviceRelease(m_device);
	}
	if (m_adapter) {
		wgpuAdapterRelease(m_adapter);
	}
	if (m_surface) {
		wgpuSurfaceRelease(m_surface);
	}
	if (m_instance) {
		wgpuInstanceRelease(m_instance);
	}
}

bool WebGPUContext::initialize(const char *canvasSelector, uint32_t width, uint32_t height, ReadyCallback callback, void *userData)
{
	if (!canvasSelector || width == 0 || height == 0 || m_instance) {
		return false;
	}

	m_width = width;
	m_height = height;
	m_callback = callback;
	m_userData = userData;
	m_instance = wgpuCreateInstance(nullptr);
	if (!m_instance) {
		return false;
	}

	WGPUEmscriptenSurfaceSourceCanvasHTMLSelector canvasSource = WGPU_EMSCRIPTEN_SURFACE_SOURCE_CANVAS_HTML_SELECTOR_INIT;
	canvasSource.selector = MakeStringView(canvasSelector);
	WGPUSurfaceDescriptor surfaceDescriptor = WGPU_SURFACE_DESCRIPTOR_INIT;
	surfaceDescriptor.nextInChain = &canvasSource.chain;
	m_surface = wgpuInstanceCreateSurface(m_instance, &surfaceDescriptor);
	if (!m_surface) {
		return false;
	}

	WGPURequestAdapterOptions options = WGPU_REQUEST_ADAPTER_OPTIONS_INIT;
	options.compatibleSurface = m_surface;
	wgpuInstanceRequestAdapter(m_instance, &options, WGPURequestAdapterCallbackInfo {
		.mode = WGPUCallbackMode_AllowSpontaneous,
		.callback = onAdapterRequest,
		.userdata1 = this,
	});
	return true;
}

bool WebGPUContext::renderClear(double red, double green, double blue, double alpha)
{
	if (!isReady()) {
		return false;
	}

	WGPUSurfaceTexture surfaceTexture = WGPU_SURFACE_TEXTURE_INIT;
	wgpuSurfaceGetCurrentTexture(m_surface, &surfaceTexture);
	if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
		surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
		return false;
	}

	WGPUTextureView view = wgpuTextureCreateView(surfaceTexture.texture, nullptr);
	WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_device, nullptr);
	WGPURenderPassColorAttachment colorAttachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
	colorAttachment.view = view;
	colorAttachment.loadOp = WGPULoadOp_Clear;
	colorAttachment.storeOp = WGPUStoreOp_Store;
	colorAttachment.clearValue = { red, green, blue, alpha };
	WGPURenderPassDescriptor passDescriptor = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
	passDescriptor.colorAttachmentCount = 1;
	passDescriptor.colorAttachments = &colorAttachment;
	WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &passDescriptor);
	wgpuRenderPassEncoderEnd(pass);

	WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, nullptr);
	wgpuQueueSubmit(m_queue, 1, &commands);

	wgpuCommandBufferRelease(commands);
	wgpuRenderPassEncoderRelease(pass);
	wgpuCommandEncoderRelease(encoder);
	wgpuTextureViewRelease(view);
	wgpuTextureRelease(surfaceTexture.texture);
	return true;
}

// GeneralsX @port Codex 04/08/2026 Reconfigure the browser WebGPU surface after a Direct3D device reset.
bool WebGPUContext::resize(uint32_t width, uint32_t height)
{
	if (!isReady() || width == 0 || height == 0) {
		return false;
	}
	if (m_width == width && m_height == height) {
		return true;
	}

	m_width = width;
	m_height = height;
	return configureSurface();
}

bool WebGPUContext::isReady() const
{
	return m_device && m_queue && m_surfaceFormat != WGPUTextureFormat_Undefined;
}

bool WebGPUContext::hasFailed() const
{
	return m_failed;
}

WGPUDevice WebGPUContext::device() const
{
	return m_device;
}

WGPUQueue WebGPUContext::queue() const
{
	return m_queue;
}

WGPUSurface WebGPUContext::surface() const
{
	return m_surface;
}

WGPUTextureFormat WebGPUContext::surfaceFormat() const
{
	return m_surfaceFormat;
}

uint32_t WebGPUContext::width() const
{
	return m_width;
}

uint32_t WebGPUContext::height() const
{
	return m_height;
}

void WebGPUContext::onAdapterRequest(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void *userData1, void *)
{
	auto *context = static_cast<WebGPUContext *>(userData1);
	if (status != WGPURequestAdapterStatus_Success) {
		context->reportFailure("adapter", message);
		return;
	}

	context->m_adapter = adapter;
	WGPUDeviceDescriptor descriptor = WGPU_DEVICE_DESCRIPTOR_INIT;
	static constexpr WGPUFeatureName compressedTextureFeature = WGPUFeatureName_TextureCompressionBC;
	if (wgpuAdapterHasFeature(adapter, compressedTextureFeature)) {
		descriptor.requiredFeatureCount = 1;
		descriptor.requiredFeatures = &compressedTextureFeature;
	}
	// GeneralsX @port Codex 04/08/2026 Surface browser WebGPU validation failures through native diagnostics.
	descriptor.uncapturedErrorCallbackInfo.callback = OnUncapturedError;
	wgpuAdapterRequestDevice(adapter, &descriptor, WGPURequestDeviceCallbackInfo {
		.mode = WGPUCallbackMode_AllowSpontaneous,
		.callback = onDeviceRequest,
		.userdata1 = context,
	});
}

void WebGPUContext::onDeviceRequest(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void *userData1, void *)
{
	auto *context = static_cast<WebGPUContext *>(userData1);
	if (status != WGPURequestDeviceStatus_Success) {
		context->reportFailure("device", message);
		return;
	}

	context->m_device = device;
	context->m_queue = wgpuDeviceGetQueue(device);
	if (!context->configureSurface()) {
		context->reportFailure("surface", WGPU_STRING_VIEW_INIT);
		return;
	}

	if (context->m_callback) {
		context->m_callback(*context, context->m_userData);
	}
}

bool WebGPUContext::configureSurface()
{
	WGPUSurfaceCapabilities capabilities = WGPU_SURFACE_CAPABILITIES_INIT;
	if (wgpuSurfaceGetCapabilities(m_surface, m_adapter, &capabilities) != WGPUStatus_Success || capabilities.formatCount == 0) {
		wgpuSurfaceCapabilitiesFreeMembers(capabilities);
		return false;
	}

	m_surfaceFormat = capabilities.formats[0];
	WGPUSurfaceConfiguration configuration = WGPU_SURFACE_CONFIGURATION_INIT;
	configuration.device = m_device;
	configuration.format = m_surfaceFormat;
	configuration.width = m_width;
	configuration.height = m_height;
	configuration.presentMode = WGPUPresentMode_Fifo;
	configuration.alphaMode = WGPUCompositeAlphaMode_Opaque;
	wgpuSurfaceConfigure(m_surface, &configuration);
	wgpuSurfaceCapabilitiesFreeMembers(capabilities);
	return true;
}

void WebGPUContext::reportFailure(const char *stage, WGPUStringView message)
{
	m_failed = true;
	fprintf(stderr, "WebGPU %s initialization failed: %.*s\n", stage, static_cast<int>(message.length), message.data ? message.data : "");
}
