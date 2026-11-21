#pragma once

struct IDirect3D8;

// GeneralsX @feature Codex 04/08/2026 Expose the browser-native Direct3D 8 compatibility boundary.
IDirect3D8 *CreateWebGPUDirect3D8();
bool WebGPUDeviceCanSubmitFrame();
