import { describe, expect, it } from "vitest";
import {
  BROADCAST_IP,
  UdpBridgeError,
  UNASSIGNED_IP,
  WebRtcUdpBridge,
  decodeUdpFrame,
  encodeUdpFrame,
  ipForSlot,
  slotForIp,
  type WebRtcUdpBridgeOptions,
} from "../../src/net/webrtc-udp-bridge.js";
import { FakePeerConnection, FakeSignalingClient, flush } from "./webrtc-fakes.js";

function makeBridge(overrides: Partial<WebRtcUdpBridgeOptions> = {}) {
  const signaling = new FakeSignalingClient();
  const peerConnections: FakePeerConnection[] = [];
  const bridge = new WebRtcUdpBridge({
    signaling: signaling.asSignalingClientLike(),
    iceServers: [],
    createPeerConnection: (config) => {
      const pc = new FakePeerConnection(config);
      peerConnections.push(pc);
      return pc as unknown as RTCPeerConnection;
    },
    ...overrides,
  });
  return { bridge, signaling, peerConnections };
}

function welcome(slot: number, roster: { slot: number; name: string; isHost: boolean }[], capacity = 4) {
  return { type: "welcome" as const, roomId: "ABCD", slot, capacity, roster };
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

  it("supports an empty payload", () => {
    const frame = encodeUdpFrame(1, 2, new Uint8Array());
    expect(frame.byteLength).toBe(8);
    expect(decodeUdpFrame(frame.buffer)?.payload).toEqual(new Uint8Array());
  });

  it("rejects out-of-range or non-integer ports", () => {
    expect(() => encodeUdpFrame(-1, 100, new Uint8Array())).toThrow(UdpBridgeError);
    expect(() => encodeUdpFrame(100, 70000, new Uint8Array())).toThrow(UdpBridgeError);
    expect(() => encodeUdpFrame(1.5, 100, new Uint8Array())).toThrow(UdpBridgeError);
  });

  it("rejects an oversized payload", () => {
    expect(() => encodeUdpFrame(1, 2, new Uint8Array(64 * 1024))).toThrow(UdpBridgeError);
  });

  it("returns null (never throws) for frames shorter than the 8-byte header", () => {
    expect(decodeUdpFrame(new Uint8Array([1, 2, 3]).buffer)).toBeNull();
    expect(decodeUdpFrame(new Uint8Array(0).buffer)).toBeNull();
  });
});

describe("ipForSlot / slotForIp (addressing)", () => {
  it("maps slot 0 to 10.0.0.1", () => {
    expect(ipForSlot(0)).toBe("10.0.0.1");
    expect(slotForIp("10.0.0.1")).toBe(0);
  });

  it("round-trips across the full room capacity range", () => {
    for (let slot = 0; slot < 8; slot += 1) {
      expect(slotForIp(ipForSlot(slot))).toBe(slot);
    }
  });

  it("rejects malformed addresses, addresses outside the subnet, and the broadcast address", () => {
    expect(slotForIp("not-an-ip")).toBeNull();
    expect(slotForIp("192.168.0.1")).toBeNull();
    expect(slotForIp("10.0.1.1")).toBeNull();
    expect(slotForIp(BROADCAST_IP)).toBeNull();
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

  it("only the lower slot of a pair eagerly creates the DataChannel", async () => {
    const lower = makeBridge();
    lower.signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    lower.signaling.emit("roster", ROSTER_TWO_PLAYERS);
    await flush();
    expect(lower.peerConnections[0]?.createdChannels).toHaveLength(1);

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

  it("resets all peers and room state when the signaling socket closes", () => {
    const { bridge, signaling, peerConnections } = makeBridge();
    signaling.emit("welcome", welcome(0, ROSTER_HOST_ONLY));
    signaling.emit("roster", ROSTER_THREE_PLAYERS);
    expect(bridge.status().peers).toHaveLength(2);

    signaling.emit("close", new CloseEvent("close"));

    expect(peerConnections.every((pc) => pc.connectionState === "closed")).toBe(true);
    expect(bridge.status()).toMatchObject({ roomId: null, localSlot: null, peers: [] });
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

  it("delivers a unicast datagram to the destination peer's channel only", async () => {
    const { bridge, peerConnections } = connectTwoPeers();
    await flush();
    peerConnections[0]!.createdChannels[0]!.open();

    bridge.bind(7000);
    const sent = bridge.send(ipForSlot(1), 7000, 6000, new Uint8Array([9, 9, 9]));

    expect(sent).toBe(true);
    expect(peerConnections[0]?.createdChannels[0]?.sent).toHaveLength(1);
    expect(peerConnections[1]?.createdChannels[0]?.sent ?? []).toHaveLength(0);
  });

  it("delivers a broadcast datagram to every peer with an open channel", () => {
    const { bridge, peerConnections } = connectTwoPeers();
    peerConnections[0]!.createdChannels[0]!.open();
    peerConnections[1]!.createdChannels[0]!.open();

    const sent = bridge.send(BROADCAST_IP, 7000, 6000, new Uint8Array([1]));

    expect(sent).toBe(true);
    expect(peerConnections[0]?.createdChannels[0]?.sent).toHaveLength(1);
    expect(peerConnections[1]?.createdChannels[0]?.sent).toHaveLength(1);
  });

  it("returns false (does not throw) when sending to a peer with no open channel", () => {
    const { bridge, peerConnections } = connectTwoPeers();
    // Both channels are left in "connecting" state (never opened).
    expect(bridge.send(ipForSlot(1), 7000, 6000, new Uint8Array([1]))).toBe(false);
    expect(peerConnections[0]?.createdChannels[0]?.sent).toHaveLength(0);
  });

  it("returns false when sending to a slot that is not currently connected", () => {
    const { bridge } = connectTwoPeers();
    expect(bridge.send(ipForSlot(5), 7000, 6000, new Uint8Array([1]))).toBe(false);
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

    expect(sent).toBe(false);
    expect(channel.sent).toHaveLength(0);
  });

  it("throws explicit errors for invalid ports, addresses, and payload types", () => {
    const { bridge } = makeBridge();
    expect(() => bridge.send("10.0.0.2", -1, 100, new Uint8Array())).toThrow(UdpBridgeError);
    expect(() => bridge.send("10.0.0.2", 100, 70000, new Uint8Array())).toThrow(UdpBridgeError);
    expect(() => bridge.send("not-an-ip", 100, 100, new Uint8Array())).toThrow(UdpBridgeError);
    expect(() => bridge.send("10.0.0.2", 100, 100, "nope" as unknown as Uint8Array<ArrayBuffer>)).toThrow(UdpBridgeError);
    expect(() => bridge.bind(70000)).toThrow(UdpBridgeError);
  });

  it("throws when recv() is called on a port that was never bound", () => {
    const { bridge } = makeBridge();
    expect(() => bridge.recv(1234)).toThrow(UdpBridgeError);
  });

  it("queues an incoming datagram for recv(), addressed by the sender's synthetic IP", async () => {
    const { bridge, peerConnections } = connectTwoPeers();
    await flush();
    bridge.bind(9000);
    const channel = peerConnections[0]!.createdChannels[0]!;

    channel.simulateMessage(encodeUdpFrame(1111, 9000, new Uint8Array([7, 8])).buffer);

    const datagram = bridge.recv(9000);
    expect(datagram).toEqual({ data: new Uint8Array([7, 8]), srcIP: ipForSlot(1), srcPort: 1111 });
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

    expect(bridge.recv(9000)?.srcPort).toBe(3);
    expect(bridge.recv(9000)?.srcPort).toBe(4);
    expect(bridge.recv(9000)?.srcPort).toBe(5);
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

  it("reports the assigned local and host addresses after joining", () => {
    const { bridge, signaling } = makeBridge();
    signaling.emit("welcome", welcome(2, [...ROSTER_TWO_PLAYERS, { slot: 2, name: "me", isHost: false }]));

    expect(bridge.localIP()).toBe("10.0.0.3");
    expect(bridge.hostIP()).toBe("10.0.0.1");
    expect(bridge.status().roomId).toBe("ABCD");
    expect(bridge.status().localSlot).toBe(2);
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
  it("joinRoom() forwards the room code and options to the signaling client", () => {
    const { bridge, signaling } = makeBridge();
    bridge.joinRoom("R7K2QX", { capacity: 6 });
    expect(signaling.connectCalls).toEqual([{ roomId: "R7K2QX", options: { capacity: 6 } }]);
  });

  it("joinRoom() falls back to the configured player name when none is given", () => {
    const { bridge, signaling } = makeBridge({ playerName: "Ada" });
    bridge.joinRoom("R7K2QX");
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
});
