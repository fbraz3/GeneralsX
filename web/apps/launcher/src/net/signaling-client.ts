/**
 * Thin WebSocket client for the room signaling protocol defined in
 * `@generalsx-web/shared/protocol`. Emits typed events via a small internal
 * listener map rather than pulling in a full event-emitter dependency.
 */
import {
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

  constructor(private readonly workerBaseUrl: string) {}

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

  /** Opens the WebSocket and sends the initial `join` request. */
  connect(roomId: string, options: { name?: string; capacity?: number } = {}): void {
    const url = new URL("/room", this.workerBaseUrl);
    url.protocol = url.protocol === "https:" ? "wss:" : "ws:";
    url.searchParams.set("roomId", roomId);
    if (options.capacity !== undefined) url.searchParams.set("capacity", String(options.capacity));

    const socket = new WebSocket(url.toString());
    this.socket = socket;

    socket.addEventListener("open", () => {
      this.sendMessage({
        type: "join",
        roomId,
        ...(options.name !== undefined ? { name: options.name } : {}),
        ...(options.capacity !== undefined ? { capacity: options.capacity } : {}),
      });
    });
    socket.addEventListener("message", (event: MessageEvent) => {
      this.handleServerMessage(typeof event.data === "string" ? event.data : "");
    });
    socket.addEventListener("close", (event: CloseEvent) => {
      this.emit("close", event);
    });
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

  leave(): void {
    if (this.socket && this.socket.readyState === WebSocket.OPEN) {
      this.sendMessage({ type: "leave" });
    }
  }

  close(): void {
    this.socket?.close();
    this.socket = null;
  }
}
