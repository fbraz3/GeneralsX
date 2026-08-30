/**
 * Immutable launcher configuration. These values are baked in at build time
 * and frozen at module load, so the running launcher can never be tricked
 * (by a compromised dependency, XSS, or a stray runtime mutation) into
 * fetching the engine or its manifest from an unexpected origin.
 *
 * This file describes *where to look* for the engine manifest; it never
 * embeds engine binaries or game assets itself.
 */

export interface LauncherConfig {
  /** Origin + path serving the signed `EngineManifest` JSON document. */
  readonly manifestUrl: string;
  /** Base URL of the Cloudflare Worker providing room signaling + TURN
   * credentials (see apps/worker). */
  readonly signalingWorkerUrl: string;
  /** Default room capacity offered in the "create room" UI. */
  readonly defaultRoomCapacity: number;
}

export const LAUNCHER_CONFIG: LauncherConfig = Object.freeze({
  manifestUrl:
    import.meta.env.VITE_GENERALSX_MANIFEST_URL ?? "https://assets.generalsx.org/manifest.json",
  signalingWorkerUrl:
    import.meta.env.VITE_GENERALSX_SIGNALING_URL ?? "https://signaling.generalsx.org",
  defaultRoomCapacity: 4,
});
