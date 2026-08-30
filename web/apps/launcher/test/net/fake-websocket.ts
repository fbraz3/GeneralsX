/**
 * Lightweight `WebSocket` test double for `signaling-client.test.ts`. Only
 * implements what `SignalingClient` actually uses (`readyState`, `send`,
 * `close`, and the `onopen`/`onmessage`/`onclose` properties); the real
 * `WebSocket.OPEN`/`CONNECTING`/`CLOSED` numeric constants are standardized
 * (0/1/2/3) across every implementation, including Node's built-in global
 * used by these tests, so this fake's `readyState` values compare correctly
 * against them without needing to mock the global at all.
 */
export class FakeWebSocket {
  static readonly CONNECTING = 0;
  static readonly OPEN = 1;
  static readonly CLOSING = 2;
  static readonly CLOSED = 3;

  readyState: number = FakeWebSocket.CONNECTING;
  onopen: (() => void) | null = null;
  onmessage: ((event: { data: unknown }) => void) | null = null;
  onclose: ((event: { code?: number; reason?: string }) => void) | null = null;
  readonly sent: string[] = [];
  closeCallCount = 0;

  constructor(readonly url: string) {}

  /** Simulates the underlying transport finishing its handshake. */
  open(): void {
    this.readyState = FakeWebSocket.OPEN;
    this.onopen?.();
  }

  send(data: string): void {
    if (this.readyState !== FakeWebSocket.OPEN) {
      throw new Error(`cannot send on a socket in state ${this.readyState}`);
    }
    this.sent.push(data);
  }

  /** Simulates the server sending a message. */
  simulateMessage(data: unknown): void {
    this.onmessage?.({ data });
  }

  /** Simulates the server (or the network) closing the connection, without
   * this having been requested via `close()` — i.e. a genuine, observable
   * close the client did not initiate. */
  simulateServerClose(): void {
    this.readyState = FakeWebSocket.CLOSED;
    this.onclose?.({});
  }

  /** Mirrors a real `WebSocket.close()`: transitions state immediately (as
   * real browsers do) but only fires the close event on a later microtask,
   * so tests can assert that a superseding `connect()` has already
   * detached this socket's listeners *before* that event would fire. */
  close(): void {
    this.closeCallCount += 1;
    this.readyState = FakeWebSocket.CLOSED;
    queueMicrotask(() => this.onclose?.({}));
  }
}
