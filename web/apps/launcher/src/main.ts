/**
 * Launcher entry point: wires the canvas, loading/error overlays, settings
 * panel, and room create/join UI together. This module intentionally stops
 * short of instantiating the actual Emscripten engine module (not yet part
 * of this scaffold) — `startEngineBoot` is the seam where that integration
 * will land.
 */
import { LAUNCHER_CONFIG } from "./config.js";
import { loadEngineManifest } from "./engine-manifest.js";
import { AssetManager, CacheStorageAssetStore } from "./assets/asset-manager.js";
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

async function startEngineBoot(): Promise<void> {
  const app = requireElement("app");
  createGameCanvas(app);
  const loading = createLoadingOverlay(app);
  const error = createErrorOverlay(app);
  const settings = createSettingsPanel(app, () => {
    /* Settings changes are applied to the engine once it is embedded; this
     * scaffold only persists them in memory via the panel's own state. */
  });
  const signaling = new SignalingClient(LAUNCHER_CONFIG.signalingWorkerUrl);

  // `roomRef.value` is assigned just below, once the bridge (which the
  // room panel's callbacks need) exists; `onJoinIssue` only ever fires
  // later, in response to a `joinRoom()` triggered by a room-panel
  // button, by which time it is always already assigned.
  const roomRef: { value: RoomPanel | null } = { value: null };
  function handleJoinIssue(issue: JoinIssue): void {
    if (issue.kind === "turn-unavailable") {
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

  loading.show();
  loading.setStatus("Fetching engine manifest…");
  const manifestResult = await loadEngineManifest(LAUNCHER_CONFIG.manifestUrl);
  if (!manifestResult.ok) {
    loading.hide();
    error.show(manifestResult.reason, () => void startEngineBoot());
    return;
  }

  const assetManager = new AssetManager(manifestResult.manifest, {
    cache: new CacheStorageAssetStore(`generalsx-assets-${manifestResult.manifest.engineVersion}`),
  });

  try {
    await assetManager.downloadAll((progress) => {
      loading.setStatus(`Downloading ${progress.path}…`);
      loading.setProgress(progress.loadedBytes / Math.max(1, progress.totalBytes));
    });
  } catch (err) {
    loading.hide();
    error.show(err instanceof Error ? err.message : "asset download failed", () => void startEngineBoot());
    return;
  }

  loading.hide();
  // NOTE: Instantiating the Emscripten module and starting the render loop
  // is intentionally out of scope for this infrastructure scaffold.
  void settings.getSettings();
}

void startEngineBoot();
