/**
 * Browser WebRTC UDP bridge.
 *
 * Emulates a small UDP-like transport over WebRTC unordered DataChannels so
 * the existing GeneralsX engine networking code (written for real UDP
 * sockets) can run unmodified in the browser. The public surface matches
 * exactly what the engine expects at `window.GeneralsXUdp` — see
 * `GeneralsXUdpApi` and the development-harness reference implementation at
 * `wasm/webrtc_udp.js` in the engine repository, which this bridge mirrors
 * bit-for-bit at the wire/ABI level so either transport can talk to the
 * native engine module unmodified: all addresses and the destination
 * argument to `send()` are host-order `uint32` values (0 means
 * "unassigned"), and `bind()`/`send()` return numbers, not booleans.
 *
 * Addressing: every room slot (0..capacity-1, see
 * `@generalsx-web/shared/protocol` and the worker's `room-logic.ts`) is
 * mapped to a stable synthetic IPv4 address `10.0.0.(slot+1)`, encoded as a
 * host-order `uint32` (see {@link ipForSlot}), for the lifetime of that
 * peer's room membership. `0xffffffff` (`BROADCAST_IP`) is a reserved
 * broadcast address that fans a `send()` out to every currently connected
 * peer with an open channel, mirroring a LAN broadcast; `send()` returns
 * the number of peers the datagram was actually handed to a DataChannel
 * for (0 for an unreachable/unknown/congested target).
 *
 * Framing: every DataChannel message is a UDP-alike datagram: a fixed
 * 4-byte header (2-byte little-endian source port, 2-byte little-endian
 * destination port) followed by the raw payload bytes. Anything shorter
 * than the header, or with an oversized payload, is treated as malformed
 * and dropped.
 *
 * Negotiation: uses the "perfect negotiation" pattern (symmetric offer/
 * answer with glare handled by a deterministic polite/impolite role per
 * peer pair) so either side may (re)negotiate without a signaling-layer
 * lock. For any pair of peers the lower room slot always creates the
 * DataChannel (labeled `generalsx-udp`) and is "impolite" (wins glare, its
 * offer is never ignored); the higher slot listens via `ondatachannel` and
 * is "polite" (yields to the peer's incoming offer on collision). Exactly
 * one DataChannel exists per peer pair.
 *
 * Reliability: DataChannels are created `{ ordered: false, maxRetransmits:
 * 5 }` — unordered so a stalled retransmit never head-of-line-blocks newer
 * datagrams, with a small bounded retransmit count (rather than unlimited)
 * so a lossy link cannot grow the send buffer without bound. GeneralsX's
 * own engine netcode already implements application-level reliability
 * (acks/resends) on top of raw UDP, so this stays close to fire-and-forget
 * UDP semantics instead of adding a second, redundant retry/ordering layer.
 */
import type { RosterEntry, ServerMessage, SlotId } from "@generalsx-web/shared/protocol";
import type { SignalingClient } from "./signaling-client.js";
import { fetchIceServers as fetchIceServersDefault, type TurnCredentialsResponse } from "./turn-client.js";

/** The subset of `SignalingClient`'s public API the bridge depends on.
 * Declared via `Pick` so it can never drift from the real class, while
 * still letting tests inject a lightweight fake instead of a real
 * WebSocket-backed instance. */
export type SignalingClientLike = Pick<
  SignalingClient,
  "on" | "off" | "connect" | "sendOffer" | "sendAnswer" | "sendIceCandidate" | "leave" | "close"
>;

/** Base of the bridge's synthetic `10.0.0.0/24` subnet, as a host-order `uint32`. */
const SUBNET_BASE = (10 << 24) >>> 0;
/** Reserved broadcast address for the bridge's synthetic subnet: a
 * host-order `uint32`, matching the native/C++ `INADDR_BROADCAST` value
 * (`0xffffffff`) rather than a string, per the engine ABI. */
export const BROADCAST_IP = 0xffffffff;
/** Sentinel address returned when no slot (local or host) is known yet. */
export const UNASSIGNED_IP = 0;

const FRAME_HEADER_BYTES = 4;
/** Comfortably under typical WebRTC DataChannel message-size limits. */
const MAX_PAYLOAD_BYTES = 16 * 1024;
const DEFAULT_MAX_INBOX_PACKETS_PER_PORT = 256;
const DEFAULT_MAX_BUFFERED_AMOUNT_BYTES = 64 * 1024;
/** Exact label the engine's DataChannel transport expects; see
 * `wasm/webrtc_udp.js` in the engine repository. */
const DATA_CHANNEL_LABEL = "generalsx-udp";
/** Unordered + a small bounded retransmit count: see the module doc
 * comment's "Reliability" section above. */
const DATA_CHANNEL_MAX_RETRANSMITS = 5;

/** Maps a stable room slot to its synthetic IPv4 address, as a host-order
 * `uint32` (e.g. slot 0 -> `0x0A000001`, i.e. `10.0.0.1`). */
export function ipForSlot(slot: SlotId): number {
  return (SUBNET_BASE | ((slot + 1) & 0xff)) >>> 0;
}

/** Inverse of {@link ipForSlot}. Returns `null` for anything that is not a
 * well-formed address inside the bridge's `10.0.0.1`-`10.0.0.254` range
 * (this deliberately excludes `BROADCAST_IP`, which callers must check for
 * explicitly since it addresses every peer rather than exactly one). */
export function slotForIp(ip: number): SlotId | null {
  const value = ip >>> 0;
  if (((value & 0xffffff00) >>> 0) !== SUBNET_BASE) return null;
  const lastOctet = value & 0xff;
  if (lastOctet < 1 || lastOctet > 254) return null;
  return lastOctet - 1;
}

/** Formats a host-order `uint32` address as dotted-quad, for display in
 * {@link UdpBridgeStatus} only — the engine-facing API always uses the
 * numeric form. */
export function ipToDisplayString(ip: number): string {
  const value = ip >>> 0;
  return [(value >>> 24) & 0xff, (value >>> 16) & 0xff, (value >>> 8) & 0xff, value & 0xff].join(".");
}

export type UdpBridgeErrorCode =
  | "INVALID_PORT"
  | "INVALID_ADDRESS"
  | "INVALID_PAYLOAD"
  | "PAYLOAD_TOO_LARGE"
  | "PORT_NOT_BOUND";

/** Explicit, typed error for programming mistakes (bad ports/addresses,
 * wrong payload type, using an unbound port) as opposed to ordinary,
 * silently-dropped network conditions (unknown peer, congested channel),
 * which instead surface as a `false` return value or an empty `recv()`. */
export class UdpBridgeError extends Error {
  constructor(
    message: string,
    readonly code: UdpBridgeErrorCode,
  ) {
    super(message);
    this.name = "UdpBridgeError";
  }
}

function assertValidPort(port: number, label: string): void {
  if (!Number.isInteger(port) || port < 0 || port > 0xffff) {
    throw new UdpBridgeError(`${label} must be an integer between 0 and 65535, got ${String(port)}`, "INVALID_PORT");
  }
}

export interface DecodedUdpFrame {
  readonly srcPort: number;
  readonly destPort: number;
  readonly payload: Uint8Array<ArrayBuffer>;
}

/** Encodes a UDP-alike frame: 2-byte LE source port, 2-byte LE destination
 * port, then the raw payload. Throws {@link UdpBridgeError} for out-of-range
 * ports or an oversized payload — both are programming errors, never a
 * network condition. */
export function encodeUdpFrame(srcPort: number, destPort: number, payload: Uint8Array<ArrayBuffer>): Uint8Array<ArrayBuffer> {
  assertValidPort(srcPort, "srcPort");
  assertValidPort(destPort, "destPort");
  if (payload.byteLength > MAX_PAYLOAD_BYTES) {
    throw new UdpBridgeError(`payload of ${payload.byteLength} bytes exceeds the ${MAX_PAYLOAD_BYTES}-byte limit`, "PAYLOAD_TOO_LARGE");
  }
  const frame = new Uint8Array(FRAME_HEADER_BYTES + payload.byteLength);
  const view = new DataView(frame.buffer);
  view.setUint16(0, srcPort, true);
  view.setUint16(2, destPort, true);
  frame.set(payload, FRAME_HEADER_BYTES);
  return frame;
}

/** Decodes a UDP-alike frame. Returns `null` (rather than throwing) for any
 * buffer shorter than the fixed 4-byte header, or whose payload exceeds
 * {@link MAX_PAYLOAD_BYTES}: malformed or hostile-oversized frames arrive
 * from the network — a buggy or malicious peer — not from a local
 * programming mistake, so callers should simply drop them. */
export function decodeUdpFrame(data: ArrayBuffer): DecodedUdpFrame | null {
  if (data.byteLength < FRAME_HEADER_BYTES) return null;
  if (data.byteLength - FRAME_HEADER_BYTES > MAX_PAYLOAD_BYTES) return null;
  const view = new DataView(data);
  const srcPort = view.getUint16(0, true);
  const destPort = view.getUint16(2, true);
  const payload = new Uint8Array(data.slice(FRAME_HEADER_BYTES));
  return { srcPort, destPort, payload };
}

/** A single received datagram, mirroring what a real `recvfrom()` returns.
 * Field names/types match the engine ABI exactly: the Emscripten glue reads
 * `pkt.ip` / `pkt.port` / `pkt.data` directly off this object. */
export interface UdpDatagram {
  readonly ip: number;
  readonly port: number;
  readonly data: Uint8Array<ArrayBuffer>;
}

export interface UdpPeerStatus {
  readonly slot: SlotId;
  /** Dotted-quad, for display only; see {@link ipToDisplayString}. */
  readonly ip: string;
  readonly connectionState: RTCPeerConnectionState;
  readonly channelState: RTCDataChannelState | "none";
  /** Whether this bridge yields to this peer's offer on negotiation glare. */
  readonly polite: boolean;
}

export interface UdpBridgeStatus {
  readonly roomId: string | null;
  readonly localSlot: SlotId | null;
  /** Dotted-quad, for display only; see {@link ipToDisplayString}. */
  readonly localIP: string;
  /** Dotted-quad, for display only; `null` when no host is known. */
  readonly hostIP: string | null;
  readonly boundPorts: readonly number[];
  readonly peers: readonly UdpPeerStatus[];
}

/** The exact surface the engine expects at `window.GeneralsXUdp`. Every
 * address (`destIP` and the two return values) is a host-order `uint32`,
 * matching `wasm/webrtc_udp.js` and the native transport plan — never a
 * string. */
export interface GeneralsXUdpApi {
  /** Registers `port` for `recv()` and returns the caller's own numeric
   * local IP (0 if not yet assigned a room slot). */
  bind(port: number): number;
  /** Returns the number of peers the datagram was actually handed to an
   * open DataChannel for (0 for broadcast-with-no-peers, an unknown
   * target, or a congested/closed channel). */
  send(destIP: number, destPort: number, srcPort: number, data: Uint8Array<ArrayBuffer>): number;
  recv(port: number): UdpDatagram | null;
  close(port: number): void;
  localIP(): number;
  hostIP(): number;
  joinRoom(code: string, options?: { name?: string; capacity?: number }): void;
  status(): UdpBridgeStatus;
}

/** A join-lifecycle problem the launcher UI should surface to the player.
 * `"join-failed"` is a hard failure (the room was never joined; the
 * launcher should show its blocking error overlay). `"turn-unavailable"`
 * is non-fatal — the bridge is proceeding with a direct/STUN-only ICE
 * fallback — but must still be shown as a visible warning rather than
 * silently degrading connectivity. */
export interface JoinIssue {
  readonly kind: "join-failed" | "turn-unavailable" | "signaling-unavailable";
  readonly message: string;
}

export interface WebRtcUdpBridgeOptions {
  readonly signaling: SignalingClientLike;
  /** Injectable for tests; defaults to the real `RTCPeerConnection`. */
  readonly createPeerConnection?: (config: RTCConfiguration) => RTCPeerConnection;
  /** Bounds each bound port's inbox so a flooding or fast peer can never
   * grow memory unboundedly; oldest queued datagrams are evicted first. */
  readonly maxInboxPacketsPerPort?: number;
  /** Outgoing backpressure threshold, in bytes of `RTCDataChannel.bufferedAmount`. */
  readonly maxBufferedAmountBytes?: number;
  readonly playerName?: string;
  /**
   * Base URL of the signaling Worker exposing `/turn-credentials`. Short-
   * lived TURN credentials are fetched fresh on *every* `joinRoom()` call
   * (never once at launcher startup, where the ~10-minute credential TTL
   * could easily expire before a match actually starts), immediately after
   * the room join is accepted and before any peer connection is created for
   * that room. Omit this option entirely for a bridge instance that must
   * never call TURN at all (for example a single-player/offline boot path
   * that never joins a room) — the bridge then always uses
   * {@link WebRtcUdpBridgeOptions.fallbackIceServers}.
   */
  readonly turnWorkerBaseUrl?: string;
  /** ICE servers used until the first TURN fetch resolves, and whenever
   * TURN credentials are unavailable or disabled (direct/STUN-only).
   * Defaults to `[]`. */
  readonly fallbackIceServers?: readonly RTCIceServer[];
  /** Injectable for tests; defaults to the real `fetchIceServers` from
   * `./turn-client.js`. */
  readonly fetchIceServers?: (
    workerBaseUrl: string,
    admissionToken: string,
  ) => Promise<TurnCredentialsResponse>;
  /** Notified of join-lifecycle problems the launcher UI should surface;
   * see {@link JoinIssue}. */
  readonly onJoinIssue?: (issue: JoinIssue) => void;
}

interface PeerRecord {
  readonly slot: SlotId;
  readonly polite: boolean;
  readonly pc: RTCPeerConnection;
  channel: RTCDataChannel | null;
  makingOffer: boolean;
  ignoreOffer: boolean;
}

export class WebRtcUdpBridge implements GeneralsXUdpApi {
  private readonly signaling: SignalingClientLike;
  /** Current ICE servers used for *new* peer connections; refreshed by
   * {@link refreshIceServers} on every `joinRoom()`, never mutated
   * mid-connection for existing peers. */
  private iceServers: readonly RTCIceServer[];
  private readonly fallbackIceServers: readonly RTCIceServer[];
  private readonly turnWorkerBaseUrl: string | undefined;
  private readonly fetchIceServersImpl: (
    workerBaseUrl: string,
    admissionToken: string,
  ) => Promise<TurnCredentialsResponse>;
  private readonly onJoinIssue: ((issue: JoinIssue) => void) | undefined;
  private readonly createPeerConnectionImpl: (config: RTCConfiguration) => RTCPeerConnection;
  private readonly maxInboxPacketsPerPort: number;
  private readonly maxBufferedAmountBytes: number;
  private readonly playerName: string | undefined;

  private readonly peers = new Map<SlotId, PeerRecord>();
  private readonly inboxes = new Map<number, UdpDatagram[]>();

  private roomId: string | null = null;
  private localSlot: SlotId | null = null;
  private roster: readonly RosterEntry[] = [];

  /** Bumped on every `joinRoom()`/`leaveRoom()` call. Any async work
   * (the TURN fetch, a queued inbound signal) started by an earlier call
   * checks this before acting, so a slow, superseded join can never
   * resurrect stale state or race ahead of a newer one. */
  private connectionGeneration = 0;
  /** True from the moment `signaling.connect()` is called for the
   * current generation until either a `welcome` arrives or the socket
   * closes; used to tell a genuine join failure (closed before welcome)
   * apart from an ordinary disconnect after a room was already joined. */
  private joinPending = false;
  /** Resolves once the current join's TURN fetch has settled (successfully
   * or not) and {@link iceServers} is final for this room. Peer creation and
   * inbound signal handling wait on it, so no peer connection is ever built
   * with placeholder ICE servers that a moment later would have been
   * relay-capable. */
  private iceServersReady: Promise<void> = Promise.resolve();
  private releaseIceServersReady: () => void = () => {};
  /** Roster received before the TURN fetch settled, replayed once it has. */
  private pendingRoster: readonly RosterEntry[] | null = null;
  /** Serializes inbound signal handling behind {@link iceServersReady} while
   * preserving arrival order (an `offer` must still be applied before the
   * ICE candidates that follow it). */
  private signalChain: Promise<void> = Promise.resolve();

  constructor(options: WebRtcUdpBridgeOptions) {
    this.signaling = options.signaling;
    this.fallbackIceServers = options.fallbackIceServers ?? [];
    this.iceServers = this.fallbackIceServers;
    this.turnWorkerBaseUrl = options.turnWorkerBaseUrl;
    this.fetchIceServersImpl = options.fetchIceServers ?? fetchIceServersDefault;
    this.onJoinIssue = options.onJoinIssue;
    this.createPeerConnectionImpl = options.createPeerConnection ?? ((config) => new RTCPeerConnection(config));
    this.maxInboxPacketsPerPort = options.maxInboxPacketsPerPort ?? DEFAULT_MAX_INBOX_PACKETS_PER_PORT;
    this.maxBufferedAmountBytes = options.maxBufferedAmountBytes ?? DEFAULT_MAX_BUFFERED_AMOUNT_BYTES;
    this.playerName = options.playerName;

    this.signaling.on("welcome", this.handleWelcome);
    this.signaling.on("roster", this.handleRoster);
    this.signaling.on("signal", this.handleSignal);
    this.signaling.on("peerLeft", this.handlePeerLeft);
    this.signaling.on("close", this.handleSignalingClose);
  }

  // ---- GeneralsXUdpApi -----------------------------------------------

  bind(port: number): number {
    assertValidPort(port, "port");
    if (!this.inboxes.has(port)) this.inboxes.set(port, []);
    return this.localIP();
  }

  close(port: number): void {
    this.inboxes.delete(port);
  }

  recv(port: number): UdpDatagram | null {
    const inbox = this.inboxes.get(port);
    if (!inbox) {
      throw new UdpBridgeError(`port ${port} is not bound; call bind(${port}) before recv()`, "PORT_NOT_BOUND");
    }
    return inbox.shift() ?? null;
  }

  send(destIP: number, destPort: number, srcPort: number, data: Uint8Array<ArrayBuffer>): number {
    assertValidPort(destPort, "destPort");
    assertValidPort(srcPort, "srcPort");
    if (!(data instanceof Uint8Array)) {
      throw new UdpBridgeError("data must be a Uint8Array", "INVALID_PAYLOAD");
    }
    const frame = encodeUdpFrame(srcPort, destPort, data);
    const normalizedDestIP = destIP >>> 0;

    if (normalizedDestIP === BROADCAST_IP) {
      let sentCount = 0;
      for (const peer of this.peers.values()) {
        if (this.sendFrameToPeer(peer, frame)) sentCount += 1;
      }
      return sentCount;
    }

    const targetSlot = slotForIp(normalizedDestIP);
    if (targetSlot === null) {
      throw new UdpBridgeError(`0x${normalizedDestIP.toString(16)} is not a valid GeneralsX room address`, "INVALID_ADDRESS");
    }
    const peer = this.peers.get(targetSlot);
    // No route to that peer (never connected, or already left the room):
    // a real UDP socket has no way to signal this either, so it is a
    // silent drop (0 peers reached) rather than a thrown error.
    if (!peer) return 0;
    return this.sendFrameToPeer(peer, frame) ? 1 : 0;
  }

  localIP(): number {
    return this.localSlot === null ? UNASSIGNED_IP : ipForSlot(this.localSlot);
  }

  hostIP(): number {
    return this.resolveHostIP();
  }

  /**
   * Joins (or switches to) room `code`. Always starts from a clean slate:
   * any peers/room state from a previous room are torn down immediately,
   * and the signaling connection is opened right away.
   *
   * Ordering: the room join happens **first**, and TURN credentials are
   * fetched only once the server has admitted this client and issued a room
   * admission token (see `handleWelcome`). `/turn-credentials` is authorized
   * by that token, so there is nothing to fetch before joining — and this
   * also means a player who never joins a room never costs a TURN
   * allocation. Peer connections are held back until the fetch settles, so
   * no peer is ever built with placeholder ICE servers.
   *
   * A generation counter guards every step so a slow, superseded join (an
   * earlier `joinRoom()`/`leaveRoom()` call) can never race ahead of a newer
   * one and leave a duplicate room membership or resurrect torn-down peers.
   */
  joinRoom(code: string, options: { name?: string; capacity?: number } = {}): void {
    this.connectionGeneration += 1;
    this.signaling.close();
    this.resetState();

    this.signalChain = Promise.resolve();
    this.iceServersReady = new Promise<void>((resolve) => {
      this.releaseIceServersReady = resolve;
    });

    const name = options.name ?? this.playerName;
    const connectOptions = {
      ...(name !== undefined ? { name } : {}),
      ...(options.capacity !== undefined ? { capacity: options.capacity } : {}),
    };

    this.joinPending = true;
    this.signaling.connect(code, connectOptions);
  }

  status(): UdpBridgeStatus {
    const hostIP = this.resolveHostIP();
    return {
      roomId: this.roomId,
      localSlot: this.localSlot,
      localIP: ipToDisplayString(this.localIP()),
      hostIP: hostIP === UNASSIGNED_IP ? null : ipToDisplayString(hostIP),
      boundPorts: [...this.inboxes.keys()],
      peers: [...this.peers.values()].map((peer) => ({
        slot: peer.slot,
        ip: ipToDisplayString(ipForSlot(peer.slot)),
        connectionState: peer.pc.connectionState,
        channelState: peer.channel?.readyState ?? "none",
        polite: peer.polite,
      })),
    };
  }

  // ---- Room lifecycle, driven by the launcher UI (not part of the
  // engine-facing GeneralsXUdpApi surface, but used by src/main.ts). -----

  /** Leaves the current room (if any) and tears down every peer connection. */
  leaveRoom(): void {
    // Invalidate any in-flight TURN fetch / connect() from a join that
    // was still settling when the player chose to leave.
    this.connectionGeneration += 1;
    this.signaling.leave();
    this.resetState();
  }

  /** Releases signaling listeners and every transport resource. */
  dispose(): void {
    this.connectionGeneration += 1;
    this.signaling.close();
    this.resetState();
    this.signaling.off("welcome", this.handleWelcome);
    this.signaling.off("roster", this.handleRoster);
    this.signaling.off("signal", this.handleSignal);
    this.signaling.off("peerLeft", this.handlePeerLeft);
    this.signaling.off("close", this.handleSignalingClose);
  }

  private resetState(): void {
    for (const peer of this.peers.values()) {
      this.teardownPeer(peer);
    }
    this.peers.clear();
    this.roomId = null;
    this.localSlot = null;
    this.roster = [];
    this.pendingRoster = null;
    this.joinPending = false;
    // Never leave a queued signal or a caller awaiting a gate that will now
    // never open; generation checks keep the released work inert.
    this.releaseIceServersReady();
  }

  /**
   * Fetches this join's TURN credentials, always resetting to
   * `fallbackIceServers` first so a previous join's credentials can never
   * linger into a new one.
   *
   * Never calls TURN at all when `turnWorkerBaseUrl` was not configured (a
   * bridge used only for single-player/offline boot) or when the server
   * issued no admission token. A failure is non-fatal: it falls back to
   * direct/STUN-only ICE and reports a `turn-unavailable` {@link JoinIssue}
   * so the launcher shows a visible warning instead of silently degrading
   * connectivity.
   */
  private async refreshIceServers(generation: number, admissionToken: string | undefined): Promise<void> {
    this.iceServers = this.fallbackIceServers;
    if (!this.turnWorkerBaseUrl) return;
    if (!admissionToken) {
      this.onJoinIssue?.({
        kind: "turn-unavailable",
        message:
          "the room issued no admission token, so TURN relay could not be requested; continuing with direct/STUN-only ICE",
      });
      return;
    }
    try {
      const credentials = await this.fetchIceServersImpl(this.turnWorkerBaseUrl, admissionToken);
      if (generation !== this.connectionGeneration) return; // superseded
      this.iceServers = credentials.iceServers;
    } catch (err) {
      if (generation !== this.connectionGeneration) return; // superseded
      this.onJoinIssue?.({
        kind: "turn-unavailable",
        message: `TURN credentials unavailable; continuing with direct/STUN-only ICE (${
          err instanceof Error ? err.message : String(err)
        })`,
      });
    }
  }

  // ---- Signaling event handlers ---------------------------------------

  /**
   * The room accepted this client. Records the seat, then uses the room
   * admission token that came with it to fetch TURN credentials before any
   * peer connection is created. The roster is deliberately not applied until
   * that fetch settles — applying it immediately would create peers pinned to
   * the placeholder ICE servers.
   */
  private readonly handleWelcome = (welcome: Extract<ServerMessage, { type: "welcome" }>): void => {
    this.joinPending = false;
    this.roomId = welcome.roomId;
    this.localSlot = welcome.slot;

    if (!this.turnWorkerBaseUrl) {
      // This bridge never requests TURN, so the ICE configuration is already
      // final and there is nothing to wait for.
      this.iceServers = this.fallbackIceServers;
      this.releaseIceServersReady();
      this.syncRoster(welcome.roster);
      return;
    }

    this.pendingRoster = welcome.roster;
    const generation = this.connectionGeneration;
    void this.refreshIceServers(generation, welcome.admission).then(() => {
      if (generation !== this.connectionGeneration) return; // superseded
      this.releaseIceServersReady();
      const roster = this.pendingRoster;
      this.pendingRoster = null;
      if (roster) this.syncRoster(roster);
    });
  };

  private readonly handleRoster = (roster: readonly RosterEntry[]): void => {
    if (this.localSlot === null) return;
    // Still waiting on this join's TURN credentials: remember the newest
    // roster and apply it once the ICE configuration is final.
    if (this.pendingRoster !== null) {
      this.pendingRoster = roster;
      return;
    }
    this.syncRoster(roster);
  };

  private readonly handlePeerLeft = (slot: SlotId): void => {
    if (this.localSlot === null) return;
    // Redundant with the roster diff in `syncRoster` (a `peer-left` message
    // and the next `roster` broadcast both arrive from the same departure),
    // but `teardownPeer`/`Map.delete` are idempotent, so handling both event
    // orders is always safe.
    const peer = this.peers.get(slot);
    if (!peer) return;
    this.teardownPeer(peer);
    this.peers.delete(slot);
  };

  private readonly handleSignalingClose = (): void => {
    // Only a socket that closes *before* ever reaching `welcome` for the
    // current join is a join failure worth surfacing; an ordinary
    // disconnect after a room was already joined resets state silently,
    // matching the previous behavior.
    const failedJoin = this.joinPending;
    if (failedJoin) {
      this.resetState();
      this.onJoinIssue?.({
        kind: "join-failed",
        message: "signaling connection closed before the room join completed",
      });
    } else if (this.localSlot !== null) {
      this.onJoinIssue?.({
        kind: "signaling-unavailable",
        message: "signaling disconnected; current peer links remain active, but new players cannot join",
      });
    }
  };

  /**
   * Queues an inbound signal behind this join's TURN fetch.
   *
   * A peer that is already in the room sees our arrival and can offer before
   * our own credentials have arrived; answering immediately would build that
   * peer connection with placeholder ICE servers. Signals are chained rather
   * than dispatched concurrently so arrival order is preserved — an `offer`
   * must still be applied before the ICE candidates that follow it.
   */
  private readonly handleSignal = (from: SlotId, type: "offer" | "answer" | "ice", payload: unknown): void => {
    if (this.localSlot === null) return;
    const generation = this.connectionGeneration;
    this.signalChain = this.signalChain
      .then(() => this.iceServersReady)
      .then(() => {
        if (generation !== this.connectionGeneration) return undefined; // superseded
        return this.processSignal(from, type, payload);
      })
      .catch((err: unknown) => {
        console.warn(`[GeneralsXUdp] failed to process ${type} from slot ${from}:`, err);
      });
  };

  private syncRoster(roster: readonly RosterEntry[]): void {
    this.roster = roster;

    const rosterSlots = new Set(roster.map((entry) => entry.slot));
    for (const [slot, peer] of this.peers) {
      if (slot !== this.localSlot && !rosterSlots.has(slot)) {
        this.teardownPeer(peer);
        this.peers.delete(slot);
      }
    }
    for (const entry of roster) {
      if (entry.slot === this.localSlot) continue;
      if (!this.peers.has(entry.slot)) {
        this.createPeer(entry.slot);
      }
    }
  }

  /** Resolves the host's numeric address, excluding the local slot even if
   * the local player is the host (there is never a peer connection to
   * one's own client, so that case is reported as unassigned). Falls back
   * to treating the sole other roster entry as the host in a two-player
   * room even if its `isHost` flag has not propagated yet, matching
   * `wasm/webrtc_udp.js`. */
  private resolveHostIP(): number {
    if (this.localSlot === null) return UNASSIGNED_IP;
    let host = this.roster.find((entry) => entry.isHost && entry.slot !== this.localSlot);
    if (!host) {
      const others = this.roster.filter((entry) => entry.slot !== this.localSlot);
      if (others.length === 1) host = others[0];
    }
    return host ? ipForSlot(host.slot) : UNASSIGNED_IP;
  }

  private createPeer(slot: SlotId): PeerRecord {
    const polite = this.localSlot !== null && this.localSlot > slot;
    const pc = this.createPeerConnectionImpl({ iceServers: [...this.iceServers] });
    const peer: PeerRecord = { slot, polite, pc, channel: null, makingOffer: false, ignoreOffer: false };
    this.peers.set(slot, peer);

    pc.onicecandidate = (event) => {
      if (event.candidate) this.signaling.sendIceCandidate(slot, event.candidate.toJSON());
    };

    pc.onnegotiationneeded = () => {
      this.negotiate(peer).catch((err: unknown) => {
        console.warn(`[GeneralsXUdp] negotiation failed for slot ${slot}:`, err);
      });
    };

    pc.ondatachannel = (event) => {
      this.attachDataChannel(peer, event.channel);
    };

    if (!polite) {
      // The lower-numbered slot always owns channel creation, so exactly
      // one DataChannel is ever created per peer pair.
      const channel = pc.createDataChannel(DATA_CHANNEL_LABEL, {
        ordered: false,
        maxRetransmits: DATA_CHANNEL_MAX_RETRANSMITS,
      });
      this.attachDataChannel(peer, channel);
      // Safari/WebKit can omit `negotiationneeded` for a newly-created
      // DataChannel. Queue an explicit, idempotent offer as a fallback.
      queueMicrotask(() => {
        void this.negotiate(peer).catch((err: unknown) => {
          console.warn(`[GeneralsXUdp] negotiation failed for slot ${slot}:`, err);
        });
      });
    }

    return peer;
  }

  private async negotiate(peer: PeerRecord): Promise<void> {
    if (peer.makingOffer || peer.pc.signalingState !== "stable") return;
    try {
      peer.makingOffer = true;
      const offer = await peer.pc.createOffer();
      await peer.pc.setLocalDescription(offer);
      if (peer.pc.localDescription) this.signaling.sendOffer(peer.slot, peer.pc.localDescription.toJSON());
    } finally {
      peer.makingOffer = false;
    }
  }

  private attachDataChannel(peer: PeerRecord, channel: RTCDataChannel): void {
    channel.binaryType = "arraybuffer";
    peer.channel = channel;
    channel.onmessage = (event: MessageEvent<unknown>) => {
      this.handleIncomingFrame(peer.slot, event.data);
    };
    channel.onclose = () => {
      if (peer.channel === channel) peer.channel = null;
    };
  }

  private async processSignal(from: SlotId, type: "offer" | "answer" | "ice", payload: unknown): Promise<void> {
    const peer = this.peers.get(from) ?? this.createPeer(from);

    if (type === "ice") {
      try {
        await peer.pc.addIceCandidate(payload as RTCIceCandidateInit);
      } catch (err) {
        if (!peer.ignoreOffer) {
          console.warn(`[GeneralsXUdp] failed to add ICE candidate from slot ${from}:`, err);
        }
      }
      return;
    }

    const description = payload as RTCSessionDescriptionInit;
    const offerCollision = description.type === "offer" && (peer.makingOffer || peer.pc.signalingState !== "stable");
    peer.ignoreOffer = !peer.polite && offerCollision;
    if (peer.ignoreOffer) return;

    await peer.pc.setRemoteDescription(description);
    if (description.type === "offer") {
      const answer = await peer.pc.createAnswer();
      await peer.pc.setLocalDescription(answer);
      if (peer.pc.localDescription) this.signaling.sendAnswer(from, peer.pc.localDescription.toJSON());
    }
  }

  /** `channel.binaryType` is always set to `"arraybuffer"` in
   * {@link attachDataChannel}, so `event.data` should always already be an
   * `ArrayBuffer` — but browsers are not required to honor that for every
   * code path (and a non-conforming or malicious remote peer's payload
   * reaches this handler unauthenticated), so the type is still validated
   * defensively here rather than assumed. */
  private handleIncomingFrame(fromSlot: SlotId, rawData: unknown): void {
    if (!(rawData instanceof ArrayBuffer)) {
      console.warn(`[GeneralsXUdp] dropped non-ArrayBuffer message from slot ${fromSlot} (typeof ${typeof rawData})`);
      return;
    }
    const decoded = decodeUdpFrame(rawData);
    if (!decoded) {
      console.warn(
        `[GeneralsXUdp] dropped malformed or oversized frame from slot ${fromSlot} (${rawData.byteLength} bytes)`,
      );
      return;
    }
    const inbox = this.inboxes.get(decoded.destPort);
    // No socket bound to this destination port: a real UDP stack would
    // silently drop the datagram, so we do too.
    if (!inbox) return;

    if (inbox.length >= this.maxInboxPacketsPerPort) {
      inbox.shift(); // evict the oldest queued datagram to bound memory growth
    }
    inbox.push({ ip: ipForSlot(fromSlot), port: decoded.srcPort, data: decoded.payload });
  }

  private sendFrameToPeer(peer: PeerRecord, frame: Uint8Array<ArrayBuffer>): boolean {
    const channel = peer.channel;
    if (!channel || channel.readyState !== "open") return false;
    if (channel.bufferedAmount > this.maxBufferedAmountBytes) {
      // Backpressure: the channel's send buffer is already saturated. Real
      // UDP sockets drop datagrams under congestion rather than blocking
      // the caller, so we do the same instead of letting `bufferedAmount`
      // grow without bound.
      return false;
    }
    try {
      channel.send(frame);
      return true;
    } catch (err) {
      console.warn("[GeneralsXUdp] dropped outgoing datagram:", err);
      return false;
    }
  }

  private teardownPeer(peer: PeerRecord): void {
    peer.channel?.close();
    peer.pc.close();
  }
}
