/**
 * Ambient global augmentation for the engine-facing UDP bridge. The
 * WebAssembly engine (not yet integrated into this scaffold) reads
 * `window.GeneralsXUdp` at startup to route its socket calls over WebRTC
 * instead of real UDP; see `net/webrtc-udp-bridge.ts`.
 */
import type { GeneralsXUdpApi } from "./net/webrtc-udp-bridge.js";

declare global {
  interface Window {
    GeneralsXUdp?: GeneralsXUdpApi;
  }
}

export {};
