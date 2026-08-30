import { describe, expect, it } from "vitest";
import {
  BROADCAST_IP,
  UdpBridgeError,
  UNASSIGNED_IP,
  WebRtcUdpBridge,
  decodeUdpFrame,
  encodeUdpFrame,
  ipForSlot,
  ipToDisplayString,
  slotForIp,
  type JoinIssue,
  type WebRtcUdpBridgeOptions,
} from "../../src/net/webrtc-udp-bridge.js";
import { FakePeerConnection, FakeSignalingClient, flush } from "./webrtc-fakes.js";

function makeBridge(overrides: Partial<WebRtcUdpBridgeOptions> = {}) {
  const signaling = new FakeSignalingClient();
  const peerConnections: FakePeerConnection[] = [];
  const bridge = new WebRtcUdpBridge({
    signaling: signaling.asSignalingClientLike(),
    createPeerConnection: (config) => {
      const pc = new FakePeerConnection(config);
      peerConnections.push(pc);
      return pc as unknown as RTCPeerConnection;
    },
    ...overrides,
  });
  return { bridge, signaling, peerConnections };
}

/** The server issues a room admission token with every welcome; it is opaque
 * to the client and is only ever forwarded to `/turn-credentials`. */
const ADMISSION_TOKEN = "gxa1.test-admission-token";

function welcome(
  slot: number,
  roster: { slot: number; name: string; isHost: boolean }[],
  capacity = 4,
  /** `null` models a server that issued no admission token at all. */
  admission: string | null = ADMISSION_TOKEN,
) {
  return {
    type: "welcome" as const,
    roomId: "ABCD",
    slot,
    capacity,
    roster,
    ...(admission !== null ? { admission } : {}),
  };
}

const ROSTER_HOST_ONLY = [{ slot: 0, name: "host", isHost: true }];
const ROSTER_TWO_PLAYERS = [
  { slot: 0, name: "host", isHost: true },
  { slot: 1, name: "guest", isHost: false },
];
const ROSTER_THREE_PLAYERS = [...ROSTER_TWO_PLAYERS, { slot: 2, name: "third", isHost: false }];

describe("encodeUdpFrame / decodeUdpFrame (framing)", () => {
  it("round-trips source port, destination port, and payload", () => {
    const payload = new Uint8Array([1, 2, 3, 4, 5]);
    const frame = encodeUdpFrame(1234, 5678, payload);
    const decoded = decodeUdpFrame(frame.buffer);
    expect(decoded).not.toBeNull();
    expect(decoded?.srcPort).toBe(1234);
    expect(decoded?.destPort).toBe(5678);
    expect(decoded?.payload).toEqual(payload);
  });

  it("uses the exact 4-byte wire header: srcPort u16 LE at 0, destPort u16 LE at 2, payload at 4", () => {
    const payload = new Uint8Array([0xaa, 0xbb]);
    const frame = encodeUdpFrame(0x1234, 0x5678, payload);
    expect(frame.byteLength).toBe(4 + payload.byteLength);
    const view = new DataView(frame.buffer);
    expect(view.getUint16(0, true)).toBe(0x1234);
    expect(view.getUint16(2, true)).toBe(0x5678);
    expect(frame.slice(4)).toEqual(payload);
  });

  it("supports an empty payload", () => {
    const frame = encodeUdpFrame(1, 2, new Uint8Array());
    expect(frame.byteLength).toBe(4);
    expect(decodeUdpFrame(frame.buffer)?.payload).toEqual(new Uint8Array());
  });

  it("rejects out-of-range or non-integer ports", () => {
    expect(() => encodeUdpFrame(-1, 100, new Uint8Array())).toThrow(UdpBridgeError);
    expect(() => encodeUdpFrame(100, 70000, new Uint8Array())).toThrow(UdpBridgeError);
    expect(() => encodeUdpFrame(1.5, 100, new Uint8Array())).toThrow(UdpBridgeError);
  });

  it("rejects an oversized payload on encode", () => {
    expect(() => encodeUdpFrame(1, 2, new Uint8Array(64 * 1024))).toThrow(UdpBridgeError);
  });

  it("returns null (never throws) for frames shorter than the 4-byte header", () => {
    expect(decodeUdpFrame(new Uint8Array([1, 2, 3]).buffer)).toBeNull();
    expect(decodeUdpFrame(new Uint8Array(0).buffer)).toBeNull();
  });

  it("returns null (drops) for an oversized incoming payload instead of throwing", () => {
    const oversized = new Uint8Array(4 + 64 * 1024);
    expect(decodeUdpFrame(oversized.buffer)).toBeNull();
  });
});

describe("ipForSlot / slotForIp (addressing)", () => {
  it("maps slot 0 to the host-order uint32 for 10.0.0.1", () => {
    expect(ipForSlot(0)).toBe(0x0a000001);
    expect(slotForIp(0x0a000001)).toBe(0);
  });

  it("formats a numeric address as dotted-quad for display only", () => {
    expect(ipToDisplayString(ipForSlot(0))).toBe("10.0.0.1");
    expect(ipToDisplayString(BROADCAST_IP)).toBe("255.255.255.255");
  });

  it("round-trips across the full room capacity range", () => {
    for (let slot = 0; slot < 8; slot += 1) {
      expect(slotForIp(ipForSlot(slot))).toBe(slot);
    }
  });

  it("rejects addresses outside the subnet, the network address, and the broadcast address", () => {
    expect(slotForIp(0xc0a80001)).toBeNull(); // 192.168.0.1
    expect(slotForIp(0x0a000101)).toBeNull(); // 10.0.1.1 (different third octet)
    expect(slotForIp(0x0a000000)).toBeNull(); // 10.0.0.0 (network address, last octet 0)
    expect(slotForIp(BROADCAST_IP)).toBeNull();
    expect(slotForIp(UNASSIGNED_IP)).toBeNull();
  });
});

describe("WebRtcUdpBridge peer lifecycle", () => {
  it("creates a peer connection for a newly-seen roster slot", () => {
    const { bridge, signaling, peerConnections } = makeBridge();
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);

    expect(peerConnections).toHaveLength(1);
    expect(bridge.status().peers.map((p) => p.slot)).toEqual([1]);
  });

  it("only the lower slot of a pair eagerly creates the DataChannel, with the exact engine-compatible label and options", async () => {
    const lower = makeBridge();
    lower.signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    lower.signaling.emit("roster", ROSTER_TWO_PLAYERS);
    await flush();
    expect(lower.peerConnections[0]?.createdChannels).toHaveLength(1);
    const channel = lower.peerConnections[0]!.createdChannels[0]!;
    expect(channel.label).toBe("generalsx-udp");
    expect(channel.options).toEqual({ ordered: false, maxRetransmits: 5 });

    const higher = makeBridge();
    higher.signaling.emit("welcome", welcome(1, ROSTER_TWO_PLAYERS));
    await flush();
    expect(higher.peerConnections[0]?.createdChannels).toHaveLength(0);
  });

  it("tears down a peer connection when it leaves the roster", () => {
    const { bridge, signaling, peerConnections } = makeBridge();
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);
    expect(bridge.status().peers).toHaveLength(1);

    signaling.emit("roster", ROSTER_HOST_ONLY);

    expect(peerConnections[0]?.connectionState).toBe("closed");
    expect(bridge.status().peers).toHaveLength(0);
  });

  it("tears down a peer connection on an explicit peer-left event", () => {
    const { bridge, signaling, peerConnections } = makeBridge();
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);

    signaling.emit("peerLeft", 1);

    expect(peerConnections[0]?.connectionState).toBe("closed");
    expect(bridge.status().peers).toHaveLength(0);
  });

  it("is safe to receive peer-left twice, or for an unknown slot", () => {
    const { bridge, signaling } = makeBridge();
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);

    expect(() => {
      signaling.emit("peerLeft", 1);
      signaling.emit("peerLeft", 1);
      signaling.emit("peerLeft", 99);
    }).not.toThrow();
    expect(bridge.status().peers).toHaveLength(0);
  });

  it("keeps established peer channels alive when the signaling socket closes", () => {
    const issues: JoinIssue[] = [];
    const { bridge, signaling, peerConnections } = makeBridge({
      onJoinIssue: (issue) => issues.push(issue),
    });
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_THREE_PLAYERS);
    expect(bridge.status().peers).toHaveLength(2);

    signaling.emit("close", new CloseEvent("close"));

    expect(peerConnections.every((pc) => pc.connectionState !== "closed")).toBe(true);
    expect(bridge.status()).toMatchObject({ roomId: "ABCD", localSlot: 0 });
    expect(issues).toEqual([
      { kind: "signaling-unavailable", message: expect.stringContaining("current peer links remain active") },
    ]);
  });
});

describe("WebRtcUdpBridge signaling (perfect negotiation)", () => {
  it("sends an offer once the eagerly-created DataChannel triggers negotiationneeded", async () => {
    const { signaling, peerConnections } = makeBridge();
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);
    await flush();

    expect(signaling.sentOffers).toHaveLength(1);
    expect(signaling.sentOffers[0]?.to).toBe(1);
    expect(peerConnections[0]?.localDescription?.type).toBe("offer");
  });

  it("answers an incoming offer and relays ICE candidates", async () => {
    const { signaling, peerConnections } = makeBridge();
    signaling.emit("welcome", welcome(1, ROSTER_TWO_PLAYERS));

    signaling.emit("signal", 0, "offer", { type: "offer", sdp: "remote-offer-sdp" });
    await flush();

    expect(peerConnections[0]?.remoteDescription).toEqual({ type: "offer", sdp: "remote-offer-sdp" });
    expect(signaling.sentAnswers).toHaveLength(1);
    expect(signaling.sentAnswers[0]?.to).toBe(0);

    signaling.emit("signal", 0, "ice", { candidate: "fake-candidate" });
    await flush();
    expect(peerConnections[0]?.addedIceCandidates).toEqual([{ candidate: "fake-candidate" }]);
  });

  it("applies a remote answer without re-answering", async () => {
    const { signaling, peerConnections } = makeBridge();
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);
    await flush();

    signaling.emit("signal", 1, "answer", { type: "answer", sdp: "remote-answer-sdp" });
    await flush();

    expect(peerConnections[0]?.remoteDescription).toEqual({ type: "answer", sdp: "remote-answer-sdp" });
    expect(signaling.sentAnswers).toHaveLength(0);
  });

  it("impolite peer (lower slot) ignores a colliding incoming offer (glare)", async () => {
    const { signaling, peerConnections } = makeBridge();
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);
    await flush();
    // Slot 0 is impolite towards slot 1 (the lower slot always wins glare).
    // Force it into "already have a local offer" to simulate a collision.
    peerConnections[0]!.signalingState = "have-local-offer";

    signaling.emit("signal", 1, "offer", { type: "offer", sdp: "colliding-offer" });
    await flush();

    expect(signaling.sentAnswers).toHaveLength(0);
    expect(peerConnections[0]?.remoteDescription).toBeNull();
  });

  it("polite peer (higher slot) accepts a colliding incoming offer (glare)", async () => {
    const { signaling, peerConnections } = makeBridge();
    signaling.emit("welcome", welcome(1, ROSTER_TWO_PLAYERS));
    signaling.emit("signal", 0, "ice", { candidate: "warm-up" });
    await flush();
    // Force the polite side into a pretend local-offer state to simulate collision.
    peerConnections[0]!.signalingState = "have-local-offer";

    signaling.emit("signal", 0, "offer", { type: "offer", sdp: "colliding-offer" });
    await flush();

    expect(peerConnections[0]?.remoteDescription).toEqual({ type: "offer", sdp: "colliding-offer" });
    expect(signaling.sentAnswers).toHaveLength(1);
  });
});

describe("WebRtcUdpBridge send/recv", () => {
  function connectTwoPeers() {
    const { bridge, signaling, peerConnections } = makeBridge();
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_THREE_PLAYERS);
    return { bridge, signaling, peerConnections };
  }

  it("delivers a unicast datagram to the destination peer's channel only, returning the peer count reached", async () => {
    const { bridge, peerConnections } = connectTwoPeers();
    await flush();
    peerConnections[0]!.createdChannels[0]!.open();

    bridge.bind(7000);
    const sent = bridge.send(ipForSlot(1), 7000, 6000, new Uint8Array([9, 9, 9]));

    expect(sent).toBe(1);
    expect(peerConnections[0]?.createdChannels[0]?.sent).toHaveLength(1);
    expect(peerConnections[1]?.createdChannels[0]?.sent ?? []).toHaveLength(0);
  });

  it("delivers a broadcast datagram (0xffffffff) to every peer with an open channel", () => {
    const { bridge, peerConnections } = connectTwoPeers();
    peerConnections[0]!.createdChannels[0]!.open();
    peerConnections[1]!.createdChannels[0]!.open();

    const sent = bridge.send(BROADCAST_IP, 7000, 6000, new Uint8Array([1]));

    expect(sent).toBe(2);
    expect(peerConnections[0]?.createdChannels[0]?.sent).toHaveLength(1);
    expect(peerConnections[1]?.createdChannels[0]?.sent).toHaveLength(1);
  });

  it("returns 0 (does not throw) when sending to a peer with no open channel", () => {
    const { bridge, peerConnections } = connectTwoPeers();
    // Both channels are left in "connecting" state (never opened).
    expect(bridge.send(ipForSlot(1), 7000, 6000, new Uint8Array([1]))).toBe(0);
    expect(peerConnections[0]?.createdChannels[0]?.sent).toHaveLength(0);
  });

  it("returns 0 when sending to a slot that is not currently connected", () => {
    const { bridge } = connectTwoPeers();
    expect(bridge.send(ipForSlot(5), 7000, 6000, new Uint8Array([1]))).toBe(0);
  });

  it("drops an outgoing datagram under DataChannel send-buffer backpressure", async () => {
    const { bridge, signaling, peerConnections } = makeBridge({ maxBufferedAmountBytes: 1024 });
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);
    await flush();
    const channel = peerConnections[0]!.createdChannels[0]!;
    channel.open();
    channel.bufferedAmount = 5000; // over the 1024-byte threshold

    const sent = bridge.send(ipForSlot(1), 7000, 6000, new Uint8Array([1, 2, 3]));

    expect(sent).toBe(0);
    expect(channel.sent).toHaveLength(0);
  });

  it("throws explicit errors for invalid ports, addresses, and payload types", () => {
    const { bridge } = makeBridge();
    expect(() => bridge.send(ipForSlot(2), -1, 100, new Uint8Array())).toThrow(UdpBridgeError);
    expect(() => bridge.send(ipForSlot(2), 100, 70000, new Uint8Array())).toThrow(UdpBridgeError);
    expect(() => bridge.send(0xc0a80002, 100, 100, new Uint8Array())).toThrow(UdpBridgeError); // 192.168.0.2, wrong subnet
    expect(() => bridge.send(ipForSlot(2), 100, 100, "nope" as unknown as Uint8Array<ArrayBuffer>)).toThrow(UdpBridgeError);
    expect(() => bridge.bind(70000)).toThrow(UdpBridgeError);
  });

  it("bind() returns the caller's numeric local IP (0 before joining a room)", () => {
    const { bridge, signaling } = makeBridge();
    expect(bridge.bind(7000)).toBe(UNASSIGNED_IP);

    signaling.emit("welcome", welcome(1, ROSTER_TWO_PLAYERS));
    expect(bridge.bind(7001)).toBe(ipForSlot(1));
  });

  it("throws when recv() is called on a port that was never bound", () => {
    const { bridge } = makeBridge();
    expect(() => bridge.recv(1234)).toThrow(UdpBridgeError);
  });

  it("queues an incoming datagram for recv(), addressed by the sender's numeric synthetic IP", async () => {
    const { bridge, peerConnections } = connectTwoPeers();
    await flush();
    bridge.bind(9000);
    const channel = peerConnections[0]!.createdChannels[0]!;

    channel.simulateMessage(encodeUdpFrame(1111, 9000, new Uint8Array([7, 8])).buffer);

    const datagram = bridge.recv(9000);
    expect(datagram).toEqual({ data: new Uint8Array([7, 8]), ip: ipForSlot(1), port: 1111 });
    expect(bridge.recv(9000)).toBeNull();
  });

  it("drops malformed (too-short) incoming frames instead of queueing them", async () => {
    const { bridge, peerConnections } = connectTwoPeers();
    await flush();
    bridge.bind(9000);
    const channel = peerConnections[0]!.createdChannels[0]!;

    channel.simulateMessage(new Uint8Array([1, 2, 3]).buffer);

    expect(bridge.recv(9000)).toBeNull();
  });

  it("drops an oversized incoming frame instead of queueing it", async () => {
    const { bridge, peerConnections } = connectTwoPeers();
    await flush();
    bridge.bind(9000);
    const channel = peerConnections[0]!.createdChannels[0]!;

    const oversized = new Uint8Array(4 + 64 * 1024);
    new DataView(oversized.buffer).setUint16(2, 9000, true);
    channel.simulateMessage(oversized.buffer);

    expect(bridge.recv(9000)).toBeNull();
  });

  it("drops a non-ArrayBuffer message (e.g. a Blob or string) instead of throwing", async () => {
    const { bridge, peerConnections } = connectTwoPeers();
    await flush();
    bridge.bind(9000);
    const channel = peerConnections[0]!.createdChannels[0]!;

    expect(() => channel.simulateMessage("not-a-frame" as unknown as ArrayBuffer)).not.toThrow();
    expect(() => channel.simulateMessage({} as unknown as ArrayBuffer)).not.toThrow();
    expect(bridge.recv(9000)).toBeNull();
  });

  it("silently drops datagrams addressed to an unbound port", async () => {
    const { peerConnections } = connectTwoPeers();
    await flush();
    const channel = peerConnections[0]!.createdChannels[0]!;

    expect(() => channel.simulateMessage(encodeUdpFrame(1, 9999, new Uint8Array([1])).buffer)).not.toThrow();
  });

  it("bounds inbox growth by evicting the oldest queued datagram", async () => {
    const { bridge, signaling, peerConnections } = makeBridge({ maxInboxPacketsPerPort: 3 });
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);
    await flush();
    bridge.bind(9000);
    const channel = peerConnections[0]!.createdChannels[0]!;

    for (let srcPort = 1; srcPort <= 5; srcPort += 1) {
      channel.simulateMessage(encodeUdpFrame(srcPort, 9000, new Uint8Array([srcPort])).buffer);
    }

    expect(bridge.recv(9000)?.port).toBe(3);
    expect(bridge.recv(9000)?.port).toBe(4);
    expect(bridge.recv(9000)?.port).toBe(5);
    expect(bridge.recv(9000)).toBeNull();
  });

  it("close() discards the port's inbox", async () => {
    const { bridge, peerConnections } = connectTwoPeers();
    await flush();
    bridge.bind(9000);
    const channel = peerConnections[0]!.createdChannels[0]!;
    channel.simulateMessage(encodeUdpFrame(1, 9000, new Uint8Array([1])).buffer);

    bridge.close(9000);

    expect(() => bridge.recv(9000)).toThrow(UdpBridgeError);
  });
});

describe("WebRtcUdpBridge addressing / status", () => {
  it("reports unassigned addresses before joining a room", () => {
    const { bridge } = makeBridge();
    expect(bridge.localIP()).toBe(UNASSIGNED_IP);
    expect(bridge.hostIP()).toBe(UNASSIGNED_IP);
    expect(bridge.status()).toMatchObject({ roomId: null, localSlot: null, hostIP: null, peers: [] });
  });

  it("reports the assigned local and host addresses after joining, as host-order uint32 values", () => {
    const { bridge, signaling } = makeBridge();
    signaling.emit("welcome", welcome(2, [...ROSTER_TWO_PLAYERS, { slot: 2, name: "me", isHost: false }]));

    expect(bridge.localIP()).toBe(ipForSlot(2));
    expect(bridge.hostIP()).toBe(ipForSlot(0));
    expect(bridge.status().roomId).toBe("ABCD");
    expect(bridge.status().localSlot).toBe(2);
    expect(bridge.status().localIP).toBe("10.0.0.3");
    expect(bridge.status().hostIP).toBe("10.0.0.1");
  });

  it("hostIP() excludes the local slot even when the local player is host, falling back to the sole other peer", () => {
    const { bridge, signaling } = makeBridge();
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY)); // self only, no other peer yet
    expect(bridge.hostIP()).toBe(UNASSIGNED_IP);

    signaling.emit("roster", ROSTER_TWO_PLAYERS); // self (host) + exactly one other peer
    expect(bridge.hostIP()).toBe(ipForSlot(1));

    signaling.emit("roster", ROSTER_THREE_PLAYERS); // self (host) + two other peers: ambiguous
    expect(bridge.hostIP()).toBe(UNASSIGNED_IP);
  });

  it("status() reflects bound ports and per-peer connection/channel state", async () => {
    const { bridge, signaling, peerConnections } = makeBridge();
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);
    await flush();
    bridge.bind(4000);
    peerConnections[0]!.createdChannels[0]!.open();

    const status = bridge.status();
    expect(status.boundPorts).toEqual([4000]);
    expect(status.peers).toEqual([{ slot: 1, ip: "10.0.0.2", connectionState: "new", channelState: "open", polite: false }]);
  });
});

describe("WebRtcUdpBridge joinRoom / leaveRoom", () => {
  it("joinRoom() forwards the room code and options to the signaling client", async () => {
    const { bridge, signaling } = makeBridge();
    bridge.joinRoom("R7K2QX", { capacity: 6 });
    await flush();
    expect(signaling.closeCalled).toBe(true);
    expect(signaling.connectCalls).toEqual([{ roomId: "R7K2QX", options: { capacity: 6 } }]);
  });

  it("ignores stale roster and signal events before the new room welcome", async () => {
    const { bridge, signaling, peerConnections } = makeBridge({
      turnWorkerBaseUrl: "https://signaling.example.com",
      fetchIceServers: async () => {
        await new Promise((resolve) => setTimeout(resolve, 5));
        return { iceServers: [], ttlSeconds: 600 };
      },
    });

    bridge.joinRoom("R7K2QX");
    signaling.emit("roster", ROSTER_TWO_PLAYERS);
    signaling.emit("signal", 1, "offer", { type: "offer", sdp: "stale" });
    await flush();

    expect(peerConnections).toHaveLength(0);
  });

  it("joins the room before requesting TURN, since credentials require a room admission token", async () => {
    const order: string[] = [];
    const { bridge, signaling } = makeBridge({
      turnWorkerBaseUrl: "https://signaling.example.com",
      fetchIceServers: async () => {
        order.push("turn");
        return { iceServers: [], ttlSeconds: 600 };
      },
    });
    signaling.onConnect = () => order.push("connect");

    bridge.joinRoom("R7K2QX");
    await flush();
    // The join must not wait on TURN: an authorized credential cannot exist
    // before the room has admitted this client.
    expect(order).toEqual(["connect"]);

    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    await flush();
    expect(order).toEqual(["connect", "turn"]);
  });

  it("dispose closes signaling and unregisters every listener", () => {
    const { bridge, signaling } = makeBridge();
    bridge.dispose();

    expect(signaling.closeCalled).toBe(true);
    expect([...signaling.listeners.values()].every((listeners) => listeners.size === 0)).toBe(true);
  });

  it("joinRoom() falls back to the configured player name when none is given", async () => {
    const { bridge, signaling } = makeBridge({ playerName: "Ada" });
    bridge.joinRoom("R7K2QX");
    await flush();
    expect(signaling.connectCalls[0]?.options).toEqual({ name: "Ada" });
  });

  it("leaveRoom() calls signaling.leave() and tears down peers/state", () => {
    const { bridge, signaling, peerConnections } = makeBridge();
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);

    bridge.leaveRoom();

    expect(signaling.leaveCalled).toBe(true);
    expect(peerConnections[0]?.connectionState).toBe("closed");
    expect(bridge.status()).toMatchObject({ roomId: null, localSlot: null, peers: [] });
  });

  it("never calls fetchIceServers when no turnWorkerBaseUrl is configured (e.g. a single-player boot that never joins a room)", async () => {
    let fetchCallCount = 0;
    const { bridge, signaling } = makeBridge({
      fetchIceServers: async () => {
        fetchCallCount += 1;
        return { iceServers: [], ttlSeconds: 600 };
      },
    });

    bridge.joinRoom("R7K2QX");
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    await flush();

    expect(fetchCallCount).toBe(0);
    expect(signaling.connectCalls).toHaveLength(1);
  });

  it("presents the room admission token from the welcome and uses the issued credentials for new peers", async () => {
    const iceServers = [{ urls: "turn:turn.example.com", username: "u", credential: "c" }];
    const calls: { base: string; token: string }[] = [];
    const { bridge, signaling, peerConnections } = makeBridge({
      turnWorkerBaseUrl: "https://signaling.example.com",
      fetchIceServers: async (base, token) => {
        calls.push({ base, token });
        return { iceServers, ttlSeconds: 600 };
      },
    });

    bridge.joinRoom("R7K2QX");
    await flush();
    expect(calls).toHaveLength(0);
    expect(signaling.connectCalls).toHaveLength(1);

    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);
    await flush();

    expect(calls).toEqual([{ base: "https://signaling.example.com", token: ADMISSION_TOKEN }]);
    expect(peerConnections[0]?.config).toEqual({ iceServers });
  });

  it("fetches fresh credentials for every join rather than reusing an earlier room's", async () => {
    const tokens: string[] = [];
    const { bridge, signaling } = makeBridge({
      turnWorkerBaseUrl: "https://signaling.example.com",
      fetchIceServers: async (_base, token) => {
        tokens.push(token);
        return { iceServers: [], ttlSeconds: 600 };
      },
    });

    bridge.joinRoom("AAAA");
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY, 4, "token-room-a"));
    await flush();
    bridge.joinRoom("BBBB");
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY, 4, "token-room-b"));
    await flush();

    expect(tokens).toEqual(["token-room-a", "token-room-b"]);
  });

  it("creates no peer connection until the TURN fetch settles, so none is pinned to placeholder ICE servers", async () => {
    const iceServers = [{ urls: "turn:turn.example.com", username: "u", credential: "c" }];
    let resolveFetch: ((value: { iceServers: RTCIceServer[]; ttlSeconds: number }) => void) | undefined;
    const { bridge, signaling, peerConnections } = makeBridge({
      turnWorkerBaseUrl: "https://signaling.example.com",
      fetchIceServers: () =>
        new Promise((resolve) => {
          resolveFetch = resolve;
        }),
    });

    bridge.joinRoom("R7K2QX");
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);
    await flush();
    expect(peerConnections).toHaveLength(0);

    resolveFetch?.({ iceServers, ttlSeconds: 600 });
    await flush();
    expect(peerConnections).toHaveLength(1);
    expect(peerConnections[0]?.config).toEqual({ iceServers });
  });

  it("applies only the newest roster once the TURN fetch settles", async () => {
    let resolveFetch: ((value: { iceServers: RTCIceServer[]; ttlSeconds: number }) => void) | undefined;
    const { bridge, signaling, peerConnections } = makeBridge({
      turnWorkerBaseUrl: "https://signaling.example.com",
      fetchIceServers: () =>
        new Promise((resolve) => {
          resolveFetch = resolve;
        }),
    });

    bridge.joinRoom("R7K2QX");
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_THREE_PLAYERS);
    signaling.emit("roster", ROSTER_TWO_PLAYERS); // the third player left again
    resolveFetch?.({ iceServers: [], ttlSeconds: 600 });
    await flush();

    // Only the surviving peer is ever created; the transient third player
    // never gets a connection that would have to be torn down.
    expect(peerConnections).toHaveLength(1);
    expect(bridge.status().peers.map((peer) => peer.slot)).toEqual([1]);
  });

  it("defers an inbound offer that arrives before the TURN fetch settles, preserving signal order", async () => {
    let resolveFetch: ((value: { iceServers: RTCIceServer[]; ttlSeconds: number }) => void) | undefined;
    const { bridge, signaling, peerConnections } = makeBridge({
      turnWorkerBaseUrl: "https://signaling.example.com",
      fetchIceServers: () =>
        new Promise((resolve) => {
          resolveFetch = resolve;
        }),
    });

    bridge.joinRoom("R7K2QX");
    signaling.emit("welcome", welcome(1, ROSTER_TWO_PLAYERS));
    // A peer already in the room offers immediately, before our credentials
    // have arrived.
    signaling.emit("signal", 0, "offer", { type: "offer", sdp: "early-offer" });
    signaling.emit("signal", 0, "ice", { candidate: "early-candidate" });
    await flush();
    expect(peerConnections).toHaveLength(0);

    const iceServers = [{ urls: "turn:turn.example.com", username: "u", credential: "c" }];
    resolveFetch?.({ iceServers, ttlSeconds: 600 });
    await flush();

    expect(peerConnections[0]?.config).toEqual({ iceServers });
    expect(peerConnections[0]?.remoteDescription).toEqual({ type: "offer", sdp: "early-offer" });
    expect(peerConnections[0]?.addedIceCandidates).toEqual([{ candidate: "early-candidate" }]);
    expect(signaling.sentAnswers).toHaveLength(1);
  });

  it("drops a signal buffered before its sender left, so a departed peer is never recreated", async () => {
    // The hazard the buffering introduces: signals wait for TURN, departures
    // do not, so a naive implementation dequeues the offer afterwards and
    // builds a connection to somebody who has already gone.
    let resolveFetch: ((value: { iceServers: RTCIceServer[]; ttlSeconds: number }) => void) | undefined;
    const { bridge, signaling, peerConnections } = makeBridge({
      turnWorkerBaseUrl: "https://signaling.example.com",
      fetchIceServers: () =>
        new Promise((resolve) => {
          resolveFetch = resolve;
        }),
    });

    bridge.joinRoom("R7K2QX");
    signaling.emit("welcome", welcome(1, ROSTER_TWO_PLAYERS));
    signaling.emit("signal", 0, "offer", { type: "offer", sdp: "early-offer" });
    signaling.emit("peerLeft", 0);
    signaling.emit("roster", [{ slot: 1, name: "guest", isHost: true }]);
    await flush();

    resolveFetch?.({ iceServers: [], ttlSeconds: 600 });
    await flush();

    expect(peerConnections).toHaveLength(0);
    expect(bridge.status().peers).toEqual([]);
    expect(signaling.sentAnswers).toHaveLength(0);
  });

  it("drops every buffered signal from a departed peer, including trailing candidates", async () => {
    let resolveFetch: ((value: { iceServers: RTCIceServer[]; ttlSeconds: number }) => void) | undefined;
    const { bridge, signaling, peerConnections } = makeBridge({
      turnWorkerBaseUrl: "https://signaling.example.com",
      fetchIceServers: () =>
        new Promise((resolve) => {
          resolveFetch = resolve;
        }),
    });

    bridge.joinRoom("R7K2QX");
    signaling.emit("welcome", welcome(1, ROSTER_TWO_PLAYERS));
    signaling.emit("signal", 0, "offer", { type: "offer", sdp: "early-offer" });
    signaling.emit("peerLeft", 0);
    signaling.emit("signal", 0, "ice", { candidate: "late-candidate" });
    await flush();

    resolveFetch?.({ iceServers: [], ttlSeconds: 600 });
    await flush();
    expect(peerConnections).toHaveLength(0);
  });

  it("still connects a peer whose seat the roster refills after the departure", async () => {
    // The departure record must not become a permanent ban on the slot: a new
    // player taking seat 0 is a legitimate peer.
    let resolveFetch: ((value: { iceServers: RTCIceServer[]; ttlSeconds: number }) => void) | undefined;
    const { bridge, signaling, peerConnections } = makeBridge({
      turnWorkerBaseUrl: "https://signaling.example.com",
      fetchIceServers: () =>
        new Promise((resolve) => {
          resolveFetch = resolve;
        }),
    });

    bridge.joinRoom("R7K2QX");
    signaling.emit("welcome", welcome(1, ROSTER_TWO_PLAYERS));
    signaling.emit("peerLeft", 0);
    signaling.emit("roster", [
      { slot: 0, name: "replacement", isHost: true },
      { slot: 1, name: "guest", isHost: false },
    ]);
    resolveFetch?.({ iceServers: [], ttlSeconds: 600 });
    await flush();

    signaling.emit("signal", 0, "offer", { type: "offer", sdp: "new-offer" });
    await flush();

    expect(bridge.status().peers.map((peer) => peer.slot)).toEqual([0]);
    expect(peerConnections[0]?.remoteDescription).toEqual({ type: "offer", sdp: "new-offer" });
  });

  it("does not recreate a peer the roster dropped, even without a peer-left message", async () => {
    let resolveFetch: ((value: { iceServers: RTCIceServer[]; ttlSeconds: number }) => void) | undefined;
    const { bridge, signaling, peerConnections } = makeBridge({
      turnWorkerBaseUrl: "https://signaling.example.com",
      fetchIceServers: () =>
        new Promise((resolve) => {
          resolveFetch = resolve;
        }),
    });

    bridge.joinRoom("R7K2QX");
    signaling.emit("welcome", welcome(1, ROSTER_TWO_PLAYERS));
    resolveFetch?.({ iceServers: [], ttlSeconds: 600 });
    await flush();
    expect(peerConnections).toHaveLength(1);

    // The roster is authoritative: slot 0 is gone even though no explicit
    // peer-left arrived.
    signaling.emit("roster", [{ slot: 1, name: "guest", isHost: true }]);
    signaling.emit("signal", 0, "ice", { candidate: "stale" });
    await flush();

    expect(bridge.status().peers).toEqual([]);
    expect(peerConnections).toHaveLength(1);
  });

  it("falls back to direct/STUN-only ICE and reports a turn-unavailable issue when the TURN fetch fails", async () => {
    const issues: JoinIssue[] = [];
    const { bridge, signaling, peerConnections } = makeBridge({
      turnWorkerBaseUrl: "https://signaling.example.com",
      fetchIceServers: async () => {
        throw new Error("HTTP 503");
      },
      onJoinIssue: (issue) => issues.push(issue),
    });

    bridge.joinRoom("R7K2QX");
    await flush();
    // TURN failure is never fatal, and cannot even be observed before the
    // join succeeds: the join always comes first.
    expect(issues).toEqual([]);
    expect(signaling.connectCalls).toHaveLength(1);

    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);
    await flush();

    expect(issues).toEqual([{ kind: "turn-unavailable", message: expect.stringContaining("HTTP 503") }]);
    expect(peerConnections[0]?.config).toEqual({ iceServers: [] });
  });

  it("warns and stays in the room when the server issues no admission token", async () => {
    const issues: JoinIssue[] = [];
    let fetchCallCount = 0;
    const { bridge, signaling, peerConnections } = makeBridge({
      turnWorkerBaseUrl: "https://signaling.example.com",
      fetchIceServers: async () => {
        fetchCallCount += 1;
        return { iceServers: [], ttlSeconds: 600 };
      },
      onJoinIssue: (issue) => issues.push(issue),
    });

    bridge.joinRoom("R7K2QX");
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY, 4, null));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);
    await flush();

    // No token means nothing to authorize with, so TURN is never called at all.
    expect(fetchCallCount).toBe(0);
    expect(issues).toEqual([
      { kind: "turn-unavailable", message: expect.stringContaining("no admission token") },
    ]);
    expect(bridge.status()).toMatchObject({ roomId: "ABCD", localSlot: 0 });
    expect(peerConnections[0]?.config).toEqual({ iceServers: [] });
  });

  it("reports a join-failed issue when the signaling socket closes before the room join completes", async () => {
    const issues: JoinIssue[] = [];
    const { bridge, signaling } = makeBridge({ onJoinIssue: (issue) => issues.push(issue) });

    bridge.joinRoom("R7K2QX");
    await flush();
    expect(signaling.connectCalls).toHaveLength(1);

    signaling.emit("close");

    expect(issues).toEqual([{ kind: "join-failed", message: expect.stringContaining("closed") }]);
  });

  it("reports a non-fatal signaling warning after the room was already joined", async () => {
    const issues: JoinIssue[] = [];
    const { bridge, signaling } = makeBridge({ onJoinIssue: (issue) => issues.push(issue) });

    bridge.joinRoom("R7K2QX");
    await flush();
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));

    signaling.emit("close");

    expect(issues).toEqual([
      { kind: "signaling-unavailable", message: expect.stringContaining("current peer links remain active") },
    ]);
    expect(bridge.status()).toMatchObject({ roomId: "ABCD", localSlot: 0 });
  });

  it("a superseded join's TURN credentials never reach the room that replaced it", async () => {
    const staleIceServers = [{ urls: "turn:stale.example.com" }];
    const freshIceServers = [{ urls: "turn:fresh.example.com" }];
    let resolveFirstFetch: ((value: { iceServers: RTCIceServer[]; ttlSeconds: number }) => void) | undefined;
    let fetchCallCount = 0;
    const { bridge, signaling, peerConnections } = makeBridge({
      turnWorkerBaseUrl: "https://signaling.example.com",
      fetchIceServers: async () => {
        fetchCallCount += 1;
        if (fetchCallCount === 1) {
          return new Promise((resolve) => {
            resolveFirstFetch = resolve;
          });
        }
        return { iceServers: freshIceServers, ttlSeconds: 600 };
      },
    });

    bridge.joinRoom("AAAA");
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    bridge.joinRoom("BBBB");
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    await flush();
    resolveFirstFetch?.({ iceServers: staleIceServers, ttlSeconds: 600 });
    await flush();

    signaling.emit("roster", ROSTER_TWO_PLAYERS);
    await flush();
    expect(signaling.connectCalls.map((call) => call.roomId)).toEqual(["AAAA", "BBBB"]);
    expect(peerConnections[0]?.config).toEqual({ iceServers: freshIceServers });
  });

  it("leaveRoom() invalidates an in-flight TURN fetch so no peer is ever created for it", async () => {
    let resolveFetch: ((value: { iceServers: RTCIceServer[]; ttlSeconds: number }) => void) | undefined;
    const { bridge, signaling, peerConnections } = makeBridge({
      turnWorkerBaseUrl: "https://signaling.example.com",
      fetchIceServers: () =>
        new Promise((resolve) => {
          resolveFetch = resolve;
        }),
    });

    bridge.joinRoom("R7K2QX");
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_TWO_PLAYERS);
    bridge.leaveRoom();
    resolveFetch?.({ iceServers: [], ttlSeconds: 600 });
    await flush();

    expect(signaling.leaveCalled).toBe(true);
    expect(peerConnections).toHaveLength(0);
    expect(bridge.status()).toMatchObject({ roomId: null, localSlot: null, peers: [] });
  });
});
