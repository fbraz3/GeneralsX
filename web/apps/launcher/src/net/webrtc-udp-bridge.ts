/**
 * Browser WebRTC UDP bridge.
 *
 * Emulates a small UDP-like transport over WebRTC unordered DataChannels so
 * the existing GeneralsX engine networking code (written for real UDP
 * sockets) can run unmodified in the browser. The public surface matches
 * exactly what the engine expects at `window.GeneralsXUdp`: bind/send/recv/
 * close plus localIP/hostIP/joinRoom/status (see `GeneralsXUdpApi`).
 *
 * Addressing: every room slot (0..capacity-1, see
 * `@generalsx-web/shared/protocol` and the worker's `room-logic.ts`) is
 * mapped to a stable synthetic IPv4 address `10.0.0.(slot+1)` for the
 * lifetime of that peer's room membership. `10.0.0.255` (`BROADCAST_IP`) is
 * a reserved broadcast address that fans a `send()` out to every currently
 * connected peer with an open channel, mirroring a LAN broadcast.
 *
 * Framing: every DataChannel message is a UDP-alike datagram: a fixed
 * 8-byte header (4-byte little-endian source port, 4-byte little-endian
 * destination port) followed by the raw payload bytes. Anything shorter
 * than the header is treated as malformed and dropped.
 *
 * Negotiation: uses the "perfect negotiation" pattern (symmetric offer/
 * answer with glare handled by a deterministic polite/impolite role per
 * peer pair) so either side may (re)negotiate without a signaling-layer
 * lock. For any pair of peers the lower room slot always creates the
 * DataChannel and is "impolite" (wins glare, its offer is never ignored);
 * the higher slot listens via `ondatachannel` and is "polite" (yields to
 * the peer's incoming offer on collision). Exactly one DataChannel exists
 * per peer pair.
 *
 * Reliability: DataChannels are created `{ ordered: false, maxRetransmits:
 * 0 }`. GeneralsX's own engine netcode already implements application-level
 * reliability (acks/resends) on top of raw UDP, so the transport here stays
 * as close to fire-and-forget UDP semantics as WebRTC allows instead of
 * adding a second, redundant retry/ordering layer.
 */
import type { RosterEntry, ServerMessage, SlotId } from "@generalsx-web/shared/protocol";
import type { SignalingClient } from "./signaling-client.js";

/** The subset of `SignalingClient`'s public API the bridge depends on.
 * Declared via `Pick` so it can never drift from the real class, while
 * still letting tests inject a lightweight fake instead of a real
 * WebSocket-backed instance. */
export type SignalingClientLike = Pick<
  SignalingClient,
  "on" | "off" | "connect" | "sendOffer" | "sendAnswer" | "sendIceCandidate" | "leave" | "close"
>;

const SUBNET_PREFIX = "10.0.0.";
/** Reserved broadcast address for the bridge's synthetic `10.0.0.0/24` subnet. */
export const BROADCAST_IP = "10.0.0.255";
/** Sentinel address returned when no slot (local or host) is known yet. */
export const UNASSIGNED_IP = "0.0.0.0";

const IPV4_RE = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/;
const FRAME_HEADER_BYTES = 8;
/** Comfortably under typical WebRTC DataChannel message-size limits. */
const MAX_PAYLOAD_BYTES = 16 * 1024;
const DEFAULT_MAX_INBOX_PACKETS_PER_PORT = 256;
const DEFAULT_MAX_BUFFERED_AMOUNT_BYTES = 64 * 1024;
/** Unordered + the smallest possible retransmit bound: see the module doc
 * comment's "Reliability" section above. */
const DATA_CHANNEL_MAX_RETRANSMITS = 0;

/** Maps a stable room slot to its synthetic IPv4 address. */
export function ipForSlot(slot: SlotId): string {
  return `${SUBNET_PREFIX}${slot + 1}`;
}

/** Inverse of {@link ipForSlot}. Returns `null` for anything that is not a
 * well-formed address inside the bridge's `10.0.0.1`-`10.0.0.254` range
 * (this deliberately excludes `BROADCAST_IP`, which callers must check for
 * explicitly since it addresses every peer rather than exactly one). */
export function slotForIp(ip: string): SlotId | null {
  const match = IPV4_RE.exec(ip);
  if (!match) return null;
  const [, a, b, c, d] = match;
  if (`${a}.${b}.${c}` !== "10.0.0") return null;
  const lastOctet = Number(d);
  if (!Number.isInteger(lastOctet) || lastOctet < 1 || lastOctet > 254) return null;
  return lastOctet - 1;
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

/** Encodes a UDP-alike frame: 4-byte LE source port, 4-byte LE destination
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
  view.setUint32(0, srcPort, true);
  view.setUint32(4, destPort, true);
  frame.set(payload, FRAME_HEADER_BYTES);
  return frame;
}

/** Decodes a UDP-alike frame. Returns `null` (rather than throwing) for any
 * buffer shorter than the fixed 8-byte header: malformed frames arrive from
 * the network — a buggy or hostile peer — not from a local programming
 * mistake, so callers should simply drop them. */
export function decodeUdpFrame(data: ArrayBuffer): DecodedUdpFrame | null {
  if (data.byteLength < FRAME_HEADER_BYTES) return null;
  const view = new DataView(data);
  const srcPort = view.getUint32(0, true);
  const destPort = view.getUint32(4, true);
  const payload = new Uint8Array(data.slice(FRAME_HEADER_BYTES));
  return { srcPort, destPort, payload };
}

/** A single received datagram, mirroring what a real `recvfrom()` returns. */
export interface UdpDatagram {
  readonly data: Uint8Array<ArrayBuffer>;
  readonly srcIP: string;
  readonly srcPort: number;
}

export interface UdpPeerStatus {
  readonly slot: SlotId;
  readonly ip: string;
  readonly connectionState: RTCPeerConnectionState;
  readonly channelState: RTCDataChannelState | "none";
  /** Whether this bridge yields to this peer's offer on negotiation glare. */
  readonly polite: boolean;
}

export interface UdpBridgeStatus {
  readonly roomId: string | null;
  readonly localSlot: SlotId | null;
  readonly localIP: string;
  readonly hostIP: string | null;
  readonly boundPorts: readonly number[];
  readonly peers: readonly UdpPeerStatus[];
}

/** The exact surface the engine expects at `window.GeneralsXUdp`. */
export interface GeneralsXUdpApi {
  bind(port: number): void;
  send(destIP: string, destPort: number, srcPort: number, data: Uint8Array<ArrayBuffer>): boolean;
  recv(port: number): UdpDatagram | null;
  close(port: number): void;
  localIP(): string;
  hostIP(): string;
  joinRoom(code: string, options?: { name?: string; capacity?: number }): void;
  status(): UdpBridgeStatus;
}

export interface WebRtcUdpBridgeOptions {
  readonly signaling: SignalingClientLike;
  readonly iceServers: readonly RTCIceServer[];
  /** Injectable for tests; defaults to the real `RTCPeerConnection`. */
  readonly createPeerConnection?: (config: RTCConfiguration) => RTCPeerConnection;
  /** Bounds each bound port's inbox so a flooding or fast peer can never
   * grow memory unboundedly; oldest queued datagrams are evicted first. */
  readonly maxInboxPacketsPerPort?: number;
  /** Outgoing backpressure threshold, in bytes of `RTCDataChannel.bufferedAmount`. */
  readonly maxBufferedAmountBytes?: number;
  readonly playerName?: string;
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
  private readonly iceServers: readonly RTCIceServer[];
  private readonly createPeerConnectionImpl: (config: RTCConfiguration) => RTCPeerConnection;
  private readonly maxInboxPacketsPerPort: number;
  private readonly maxBufferedAmountBytes: number;
  private readonly playerName: string | undefined;

  private readonly peers = new Map<SlotId, PeerRecord>();
  private readonly inboxes = new Map<number, UdpDatagram[]>();

  private roomId: string | null = null;
  private localSlot: SlotId | null = null;
  private hostSlot: SlotId | null = null;

  constructor(options: WebRtcUdpBridgeOptions) {
    this.signaling = options.signaling;
    this.iceServers = options.iceServers;
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

  bind(port: number): void {
    assertValidPort(port, "port");
    if (!this.inboxes.has(port)) this.inboxes.set(port, []);
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

  send(destIP: string, destPort: number, srcPort: number, data: Uint8Array<ArrayBuffer>): boolean {
    assertValidPort(destPort, "destPort");
    assertValidPort(srcPort, "srcPort");
    if (!(data instanceof Uint8Array)) {
      throw new UdpBridgeError("data must be a Uint8Array", "INVALID_PAYLOAD");
    }
    const frame = encodeUdpFrame(srcPort, destPort, data);

    if (destIP === BROADCAST_IP) {
      let sentToAny = false;
      for (const peer of this.peers.values()) {
        if (this.sendFrameToPeer(peer, frame)) sentToAny = true;
      }
      return sentToAny;
    }

    const targetSlot = slotForIp(destIP);
    if (targetSlot === null) {
      throw new UdpBridgeError(`"${destIP}" is not a valid GeneralsX room address`, "INVALID_ADDRESS");
    }
    const peer = this.peers.get(targetSlot);
    // No route to that peer (never connected, or already left the room):
    // a real UDP socket has no way to signal this either, so it is a
    // silent drop rather than a thrown error.
    if (!peer) return false;
    return this.sendFrameToPeer(peer, frame);
  }

  localIP(): string {
    return this.localSlot === null ? UNASSIGNED_IP : ipForSlot(this.localSlot);
  }

  hostIP(): string {
    return this.hostSlot === null ? UNASSIGNED_IP : ipForSlot(this.hostSlot);
  }

  joinRoom(code: string, options: { name?: string; capacity?: number } = {}): void {
    const name = options.name ?? this.playerName;
    this.signaling.connect(code, {
      ...(name !== undefined ? { name } : {}),
      ...(options.capacity !== undefined ? { capacity: options.capacity } : {}),
    });
  }

  status(): UdpBridgeStatus {
    return {
      roomId: this.roomId,
      localSlot: this.localSlot,
      localIP: this.localIP(),
      hostIP: this.hostSlot === null ? null : ipForSlot(this.hostSlot),
      boundPorts: [...this.inboxes.keys()],
      peers: [...this.peers.values()].map((peer) => ({
        slot: peer.slot,
        ip: ipForSlot(peer.slot),
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
    this.signaling.leave();
    this.resetState();
  }

  private resetState(): void {
    for (const peer of this.peers.values()) {
      this.teardownPeer(peer);
    }
    this.peers.clear();
    this.roomId = null;
    this.localSlot = null;
    this.hostSlot = null;
  }

  // ---- Signaling event handlers ---------------------------------------

  private readonly handleWelcome = (welcome: Extract<ServerMessage, { type: "welcome" }>): void => {
    this.roomId = welcome.roomId;
    this.localSlot = welcome.slot;
    this.syncRoster(welcome.roster);
  };

  private readonly handleRoster = (roster: readonly RosterEntry[]): void => {
    this.syncRoster(roster);
  };

  private readonly handlePeerLeft = (slot: SlotId): void => {
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
    this.resetState();
  };

  private readonly handleSignal = (from: SlotId, type: "offer" | "answer" | "ice", payload: unknown): void => {
    this.processSignal(from, type, payload).catch((err: unknown) => {
      console.warn(`[GeneralsXUdp] failed to process ${type} from slot ${from}:`, err);
    });
  };

  private syncRoster(roster: readonly RosterEntry[]): void {
    const hostEntry = roster.find((entry) => entry.isHost);
    this.hostSlot = hostEntry ? hostEntry.slot : null;

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
      const channel = pc.createDataChannel("udp", {
        ordered: false,
        maxRetransmits: DATA_CHANNEL_MAX_RETRANSMITS,
      });
      this.attachDataChannel(peer, channel);
    }

    return peer;
  }

  private async negotiate(peer: PeerRecord): Promise<void> {
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
    channel.onmessage = (event: MessageEvent) => {
      this.handleIncomingFrame(peer.slot, event.data as ArrayBuffer);
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

  private handleIncomingFrame(fromSlot: SlotId, rawData: ArrayBuffer): void {
    const decoded = decodeUdpFrame(rawData);
    if (!decoded) {
      console.warn(
        `[GeneralsXUdp] dropped malformed frame from slot ${fromSlot} (${rawData.byteLength} bytes, need >= ${FRAME_HEADER_BYTES})`,
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
    inbox.push({ data: decoded.payload, srcIP: ipForSlot(fromSlot), srcPort: decoded.srcPort });
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
