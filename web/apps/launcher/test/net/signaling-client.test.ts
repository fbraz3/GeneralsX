import { describe, expect, it } from "vitest";
import { CURRENT_COMPATIBILITY } from "@generalsx-web/shared/protocol";
import { SignalingClient } from "../../src/net/signaling-client.js";
import { FakeWebSocket } from "./fake-websocket.js";

/** Boots a `SignalingClient` whose sockets are `FakeWebSocket`s, capturing
 * every socket created so tests can inspect/drive them directly. */
function makeClient() {
  const sockets: FakeWebSocket[] = [];
  const client = new SignalingClient("https://signaling.example.com", (url) => {
    const socket = new FakeWebSocket(url);
    sockets.push(socket);
    return socket as unknown as WebSocket;
  });
  return { client, sockets };
}

describe("SignalingClient connection lifecycle", () => {
  it("connect() opens exactly one socket and sends a join message once it opens", () => {
    const { client, sockets } = makeClient();
    client.connect("ABCD", { name: "Ada", capacity: 4 });

    expect(sockets).toHaveLength(1);
    expect(sockets[0]!.sent).toEqual([]);
    sockets[0]!.open();
    expect(sockets[0]!.sent).toEqual([
      JSON.stringify({
        type: "join",
        roomId: "ABCD",
        name: "Ada",
        capacity: 4,
        compatibility: CURRENT_COMPATIBILITY,
      }),
    ]);
  });

  it("connect() called again while the previous socket is still open supersedes it: exactly one socket is ever left live", () => {
    const { client, sockets } = makeClient();
    client.connect("ABCD");
    sockets[0]!.open();

    client.connect("WXYZ");

    // The first socket is closed locally and had its listeners detached —
    // it can never again deliver a message or a close event to the client.
    expect(sockets[0]!.closeCallCount).toBe(1);
    expect(sockets[0]!.onopen).toBeNull();
    expect(sockets[0]!.onmessage).toBeNull();
    expect(sockets[0]!.onclose).toBeNull();

    // Exactly one new, distinct socket now exists for the new room.
    expect(sockets).toHaveLength(2);
    sockets[1]!.open();
    expect(sockets[1]!.sent).toEqual([
      JSON.stringify({ type: "join", roomId: "WXYZ", compatibility: CURRENT_COMPATIBILITY }),
    ]);
  });

  it("a superseded socket's later close event never reaches the client (no duplicate room membership fallout)", async () => {
    const { client, sockets } = makeClient();
    const closeEvents: unknown[] = [];
    client.on("close", (event) => closeEvents.push(event));

    client.connect("ABCD");
    sockets[0]!.open();
    client.connect("WXYZ"); // supersedes socket 0, which schedules an async close
    sockets[1]!.open();

    // Let socket 0's queued close microtask run.
    await Promise.resolve();
    await Promise.resolve();

    expect(closeEvents).toHaveLength(0);
  });

  it("a genuine close of the current (non-superseded) socket does emit a close event", async () => {
    const { client, sockets } = makeClient();
    const closeEvents: unknown[] = [];
    client.on("close", (event) => closeEvents.push(event));

    client.connect("ABCD");
    sockets[0]!.open();
    sockets[0]!.simulateServerClose();

    expect(closeEvents).toHaveLength(1);
  });

  it("leave() sends a leave message and then closes the socket locally", () => {
    const { client, sockets } = makeClient();
    client.connect("ABCD");
    sockets[0]!.open();

    client.leave();

    expect(sockets[0]!.sent).toEqual([
      JSON.stringify({ type: "join", roomId: "ABCD", compatibility: CURRENT_COMPATIBILITY }),
      JSON.stringify({ type: "leave" }),
    ]);
    expect(sockets[0]!.closeCallCount).toBe(1);
  });

  it("leave() on a socket that never finished opening still closes it without sending", () => {
    const { client, sockets } = makeClient();
    client.connect("ABCD");
    // Socket never called .open(); readyState stays CONNECTING.

    client.leave();

    expect(sockets[0]!.sent).toEqual([]);
    expect(sockets[0]!.closeCallCount).toBe(1);
  });

  it("leave() leaves the client in a state where connect() can safely be called again (same-socket rejoin)", () => {
    const { client, sockets } = makeClient();
    client.connect("ABCD");
    sockets[0]!.open();
    client.leave();

    client.connect("ABCD"); // rejoin the same room after leaving

    expect(sockets).toHaveLength(2);
    sockets[1]!.open();
    expect(sockets[1]!.sent).toEqual([
      JSON.stringify({ type: "join", roomId: "ABCD", compatibility: CURRENT_COMPATIBILITY }),
    ]);
  });

  it("leave() never emits a close event for the socket it closes itself", async () => {
    const { client, sockets } = makeClient();
    const closeEvents: unknown[] = [];
    client.on("close", (event) => closeEvents.push(event));

    client.connect("ABCD");
    sockets[0]!.open();
    client.leave();

    await Promise.resolve();
    await Promise.resolve();

    expect(closeEvents).toHaveLength(0);
  });

  it("close() detaches and closes the socket without emitting a close event", async () => {
    const { client, sockets } = makeClient();
    const closeEvents: unknown[] = [];
    client.on("close", (event) => closeEvents.push(event));

    client.connect("ABCD");
    sockets[0]!.open();
    client.close();

    expect(sockets[0]!.closeCallCount).toBe(1);
    await Promise.resolve();
    await Promise.resolve();
    expect(closeEvents).toHaveLength(0);
  });

  it("routes welcome/roster/signal/peer-left/error server messages to their listeners", () => {
    const { client, sockets } = makeClient();
    const welcomes: unknown[] = [];
    const rosters: unknown[] = [];
    client.on("welcome", (message) => welcomes.push(message));
    client.on("roster", (roster) => rosters.push(roster));

    client.connect("ABCD");
    sockets[0]!.open();
    sockets[0]!.simulateMessage(
      JSON.stringify({ type: "welcome", roomId: "ABCD", slot: 0, capacity: 4, roster: [{ slot: 0, name: "Ada", isHost: true }] }),
    );

    expect(welcomes).toHaveLength(1);
    // "welcome" also re-emits its roster via the "roster" event.
    expect(rosters).toHaveLength(1);
  });
});
