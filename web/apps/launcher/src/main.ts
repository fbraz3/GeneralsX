/**
 * Launcher entry point: wires the canvas, loading/error overlays, settings
 * panel, room create/join UI, asset pipeline, and Emscripten engine together.
 */
import { LAUNCHER_CONFIG } from "./config.js";
import { loadEngineManifest } from "./engine-manifest.js";
import { AssetManager } from "./assets/asset-manager.js";
import { openAssetStorage, requestPersistentStorage } from "./assets/storage.js";
import { AssetStorageQuotaError, AssetIntegrityError, AssetCancelledError } from "./assets/errors.js";
import type { AssetVfs } from "./assets/vfs.js";
import { launchEmscriptenEngine } from "./engine/emscripten-loader.js";
import { createGameCanvas } from "./ui/canvas.js";
import { createLoadingOverlay } from "./ui/loading.js";
import { createErrorOverlay } from "./ui/error.js";
import { createSettingsPanel } from "./ui/settings.js";
import { createRoomPanel, generateRoomId, type RoomPanel } from "./ui/room.js";
import { SignalingClient } from "./net/signaling-client.js";
import { WebRtcUdpBridge, type JoinIssue } from "./net/webrtc-udp-bridge.js";
import "./style.css";

function requireElement(id: string): HTMLElement {
  const element = document.getElementById(id);
  if (!element) throw new Error(`missing required #${id} element in index.html`);
  return element;
}

/** Turns a typed asset failure into an actionable message for the player. */
function describeAssetFailure(error: unknown): string {
  if (error instanceof AssetStorageQuotaError) {
    return "Not enough browser storage to install the game data. Free up disk space or clear site data, then retry.";
  }
  if (error instanceof AssetIntegrityError) {
    return `Game data failed verification (${error.path}). The download was discarded; retry to fetch a clean copy.`;
  }
  if (error instanceof AssetCancelledError) {
    return "Loading was cancelled.";
  }
  return error instanceof Error ? error.message : "asset download failed";
}

let activeBoot: AbortController | undefined;
let mountedAssets: AssetVfs | undefined;
let activeUdpBridge: WebRtcUdpBridge | undefined;

/** Verified, mounted asset tree used by the Emscripten module at startup. */
export function mountedAssetVfs(): AssetVfs | undefined {
  return mountedAssets;
}

async function startEngineBoot(): Promise<void> {
  // A retry must cancel the downloads the previous attempt left in flight;
  // partial files stay on disk and the next attempt resumes them.
  activeBoot?.abort();
  activeUdpBridge?.dispose();
  activeUdpBridge = undefined;
  const bootAbort = new AbortController();
  activeBoot = bootAbort;

  const app = requireElement("app");
  // Retry (see the error overlay's onRetry callback below) re-invokes this
  // whole function; without clearing first, each retry would append a
  // second full set of canvas/overlay/panel elements on top of the first.
  app.replaceChildren();
  const canvas = createGameCanvas(app);
  const loading = createLoadingOverlay(app);
  const error = createErrorOverlay(app);
  const settings = createSettingsPanel(app, () => {
    /* Settings changes are applied to the engine once it is embedded; this
     * scaffold only persists them in memory via the panel's own state. */
  });

  // `roomRef.value` is assigned after the manifest-backed bridge exists;
  // `onJoinIssue` only fires later in response to a room-panel action.
  const roomRef: { value: RoomPanel | null } = { value: null };
  function handleJoinIssue(issue: JoinIssue): void {
    if (issue.kind !== "join-failed") {
      // Non-fatal: TURN relay is unavailable, but direct/STUN-only ICE
      // may still work. Never fall back silently — always show a visible
      // warning instead of just logging.
      roomRef.value?.setWarning(`Warning: ${issue.message}`);
    } else {
      // Recoverable without a full relaunch: dismiss the blocking overlay
      // so the player can retry from the room panel.
      error.show(issue.message, () => error.hide());
    }
  }

  loading.show();
  loading.setStatus("Fetching engine manifest…");
  const manifestResult = await loadEngineManifest(LAUNCHER_CONFIG.manifestUrl);
  if (!manifestResult.ok) {
    loading.hide();
    error.show(manifestResult.reason, () => void startEngineBoot());
    return;
  }

  const signaling = new SignalingClient(
    LAUNCHER_CONFIG.signalingWorkerUrl,
    manifestResult.manifest.compatibility,
  );
  // TURN credentials are fetched by the bridge itself, fresh on every
  // `joinRoom()` call — never once here at launcher startup, where the
  // short (~10 minute) credential TTL could expire long before a match
  // actually starts. A launcher session that never creates/joins a room
  // (e.g. a future single-player/offline path) therefore never calls
  // TURN at all.
  const udpBridge = new WebRtcUdpBridge({
    signaling,
    turnWorkerBaseUrl: LAUNCHER_CONFIG.signalingWorkerUrl,
    onJoinIssue: handleJoinIssue,
  });
  activeUdpBridge = udpBridge;
  // Published before any (future) engine module instantiation, per the
  // integration contract documented in `net/webrtc-udp-bridge.ts`.
  window.GeneralsXUdp = udpBridge;

  const room = createRoomPanel(
    app,
    {
      onCreateRoom(capacity) {
        room.setWarning(null);
        udpBridge.joinRoom(generateRoomId(), { capacity });
      },
      onJoinRoom(roomId) {
        room.setWarning(null);
        udpBridge.joinRoom(roomId);
      },
      onLeaveRoom() {
        udpBridge.leaveRoom();
        room.setWarning(null);
        room.showLobbyState();
      },
    },
    LAUNCHER_CONFIG.defaultRoomCapacity,
  );
  roomRef.value = room;
  signaling.on("welcome", (welcome) => {
    room.showJoinedState(welcome.roomId);
    room.setStatus(`Connected as slot ${welcome.slot} of ${welcome.capacity}`);
  });
  signaling.on("roster", (roster) => room.setRoster(roster));
  signaling.on("error", (err) => error.show(err.message));

  const assetManager = new AssetManager(manifestResult.manifest, {
    storage: await openAssetStorage(),
  });
  // Persistent storage keeps multi-gigabyte archives from being evicted
  // between sessions; a refusal is not fatal, it only means slower reboots.
  await requestPersistentStorage();

  try {
    loading.setStatus("Verifying game data…");
    const vfs = await assetManager.ensureAssets({
      signal: bootAbort.signal,
      onProgress(progress) {
        loading.setStatus(
          progress.source === "cache"
            ? `Verified ${progress.path}`
            : `Downloading ${progress.path}…`,
        );
        loading.setProgress(progress.overallLoadedBytes / Math.max(1, progress.overallTotalBytes));
      },
    });
    if (activeBoot !== bootAbort) {
      loading.hide();
      return;
    }
    mountedAssets = vfs;
  } catch (err) {
    loading.hide();
    // A superseded attempt must not paint over the overlay the newer one owns.
    if (activeBoot !== bootAbort) return;
    error.show(describeAssetFailure(err), () => void startEngineBoot());
    return;
  }

  try {
    const engine = await launchEmscriptenEngine({
      assets: mountedAssets,
      canvas,
      settings: settings.getSettings(),
      onStatus: (status) => loading.setStatus(status),
      onLog: (text) => globalThis.console.log(`[GeneralsX] ${text}`),
    });
    engine.generalsxAudio?.bindUserGesture(canvas);
  } catch (err) {
    loading.hide();
    error.show(err instanceof Error ? err.message : "engine startup failed", () => void startEngineBoot());
    return;
  }

  loading.hide();
}

void startEngineBoot();
