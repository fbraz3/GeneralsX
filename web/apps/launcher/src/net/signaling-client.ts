/**
 * Thin WebSocket client for the room signaling protocol defined in
 * `@generalsx-web/shared/protocol`. Emits typed events via a small internal
 * listener map rather than pulling in a full event-emitter dependency.
 */
import {
  CURRENT_COMPATIBILITY,
  validateClientMessage,
  type ClientMessage,
  type RosterEntry,
  type ServerErrorMessage,
  type ServerMessage,
  type SlotId,
} from "@generalsx-web/shared/protocol";

export interface SignalingClientEvents {
  welcome: (message: Extract<ServerMessage, { type: "welcome" }>) => void;
  roster: (roster: readonly RosterEntry[]) => void;
  signal: (from: SlotId, type: "offer" | "answer" | "ice", payload: unknown) => void;
  peerLeft: (slot: SlotId) => void;
  error: (error: ServerErrorMessage) => void;
  close: (event: CloseEvent) => void;
}

type EventName = keyof SignalingClientEvents;

export class SignalingClient {
  private socket: WebSocket | null = null;
  private readonly listeners: { [K in EventName]: Set<SignalingClientEvents[K]> } = {
    welcome: new Set(),
    roster: new Set(),
    signal: new Set(),
    peerLeft: new Set(),
    error: new Set(),
    close: new Set(),
  };

  constructor(
    private readonly workerBaseUrl: string,
    /** Injectable for tests; defaults to the real `WebSocket` constructor. */
    private readonly createSocket: (url: string) => WebSocket = (url) => new WebSocket(url),
  ) {}

  on<K extends EventName>(event: K, handler: SignalingClientEvents[K]): void {
    this.listeners[event].add(handler);
  }

  off<K extends EventName>(event: K, handler: SignalingClientEvents[K]): void {
    this.listeners[event].delete(handler);
  }

  private emit<K extends EventName>(event: K, ...args: Parameters<SignalingClientEvents[K]>): void {
    for (const handler of this.listeners[event]) {
      (handler as (...a: Parameters<SignalingClientEvents[K]>) => void)(...args);
    }
  }

  /**
   * Opens the WebSocket and sends the initial `join` request.
   *
   * Always supersedes any socket already owned by this client first: a
   * caller invoking `connect()` again (a rejoin, or a switch to a
   * different room) while a previous connection is still open or
   * connecting must never end up with two live sockets — and therefore
   * two live `join`ed room memberships — for the same client. The
   * superseded socket's listeners are detached before it is closed, so
   * its (possibly asynchronous) close event can never fire a stale
   * `close` emission after this newer connection has already started
   * emitting its own events.
   */
  connect(roomId: string, options: { name?: string; capacity?: number } = {}): void {
    this.disconnectSocket();

    const url = new URL("/room", this.workerBaseUrl);
    url.protocol = url.protocol === "https:" ? "wss:" : "ws:";
    url.searchParams.set("roomId", roomId);
    if (options.capacity !== undefined) url.searchParams.set("capacity", String(options.capacity));

    const socket = this.createSocket(url.toString());
    this.socket = socket;

    socket.onopen = () => {
      this.sendMessage({
        type: "join",
        roomId,
        ...(options.name !== undefined ? { name: options.name } : {}),
        ...(options.capacity !== undefined ? { capacity: options.capacity } : {}),
        compatibility: CURRENT_COMPATIBILITY,
      });
    };
    socket.onmessage = (event: MessageEvent) => {
      this.handleServerMessage(typeof event.data === "string" ? event.data : "");
    };
    socket.onclose = (event: CloseEvent) => {
      // This is always the *current* socket's own close (superseding
      // connections detach this handler before closing a stale socket —
      // see `disconnectSocket()`), so it is safe to clear `this.socket`
      // and emit unconditionally.
      this.socket = null;
      this.emit("close", event);
    };
  }

  private handleServerMessage(raw: string): void {
    let parsed: unknown;
    try {
      parsed = JSON.parse(raw);
    } catch {
      return;
    }
    if (typeof parsed !== "object" || parsed === null) return;
    const message = parsed as ServerMessage;

    switch (message.type) {
      case "welcome":
        this.emit("welcome", message);
        this.emit("roster", message.roster);
        break;
      case "roster":
        this.emit("roster", message.roster);
        break;
      case "offer":
      case "answer":
      case "ice":
        this.emit("signal", message.from, message.type, message.payload);
        break;
      case "peer-left":
        this.emit("peerLeft", message.slot);
        break;
      case "error":
        this.emit("error", message);
        break;
    }
  }

  private sendMessage(message: ClientMessage): void {
    // Validate our own outgoing message with the same schema the server
    // enforces, catching protocol drift during development.
    const result = validateClientMessage(message);
    if (!result.valid) {
      throw new Error(`refusing to send an invalid message: ${result.error}`);
    }
    this.socket?.send(JSON.stringify(message));
  }

  sendOffer(to: SlotId, payload: unknown): void {
    this.sendMessage({ type: "offer", to, payload });
  }

  sendAnswer(to: SlotId, payload: unknown): void {
    this.sendMessage({ type: "answer", to, payload });
  }

  sendIceCandidate(to: SlotId, payload: unknown): void {
    this.sendMessage({ type: "ice", to, payload });
  }

  /** Sends a `leave` request (if the socket is open) and then always
   * closes the socket locally, so this client instance can safely be
   * reused for a subsequent `connect()` (rejoin) without any leftover
   * socket/listener state from the room just left. */
  leave(): void {
    if (this.socket && this.socket.readyState === WebSocket.OPEN) {
      this.sendMessage({ type: "leave" });
    }
    this.disconnectSocket();
  }

  close(): void {
    this.disconnectSocket();
  }

  /** Detaches the current socket's listeners and closes it, without
   * emitting a `close` event — used both when a new `connect()`
   * supersedes a still-open prior connection and when `leave()`/`close()`
   * intentionally tear the connection down, so callers never observe a
   * spurious `close` for a socket they themselves discarded. */
  private disconnectSocket(): void {
    const socket = this.socket;
    if (!socket) return;
    this.socket = null;
    socket.onopen = null;
    socket.onmessage = null;
    socket.onclose = null;
    if (socket.readyState === WebSocket.OPEN || socket.readyState === WebSocket.CONNECTING) {
      socket.close();
    }
  }
}
