/**
 * Cloudflare Durable Object that owns one signaling room's live WebSocket
 * connections. All slot/roster/capacity decisions live in `room-logic.ts`
 * (framework-agnostic and unit tested); this class only adapts that logic to
 * the Cloudflare Workers WebSocket API.
 */
/// <reference types="@cloudflare/workers-types" />
import {
  MAX_MESSAGE_BYTES,
  parseClientMessage,
  type ServerErrorMessage,
  type ServerMessage,
} from "@generalsx-web/shared/protocol";
import {
  buildRoster,
  createRoomState,
  joinRoom,
  leaveRoom,
  resolveSlot,
  type RoomState,
} from "./room-logic.js";

interface Connection {
  readonly socket: WebSocket;
  slot: number | null;
}

export interface RoomDurableObjectEnv {
  readonly ALLOWED_ORIGINS?: string;
}

export class RoomDurableObject implements DurableObject {
  private room: RoomState | null = null;
  private readonly connections = new Set<Connection>();
  private nextConnectionId = 0;

  constructor(
    private readonly state: DurableObjectState,
    private readonly env: RoomDurableObjectEnv,
  ) {}

  async fetch(request: Request): Promise<Response> {
    if (request.headers.get("Upgrade") !== "websocket") {
      return new Response("expected a WebSocket upgrade", { status: 426 });
    }

    const url = new URL(request.url);
    const roomId = url.searchParams.get("roomId") ?? "";
    const requestedCapacity = Number(url.searchParams.get("capacity") ?? "");

    const pair = new WebSocketPair();
    const [client, server] = Object.values(pair) as [WebSocket, WebSocket];
    server.accept();

    if (!this.room) {
      this.room = createRoomState(roomId, Number.isFinite(requestedCapacity) ? requestedCapacity : undefined);
    }

    const connection: Connection = { socket: server, slot: null };
    this.connections.add(connection);
    const connectionId = String(this.nextConnectionId++);

    server.addEventListener("message", (event: MessageEvent) => {
      this.handleMessage(connection, connectionId, event);
    });
    server.addEventListener("close", () => {
      this.handleDisconnect(connection);
    });
    server.addEventListener("error", () => {
      this.handleDisconnect(connection);
    });

    return new Response(null, { status: 101, webSocket: client });
  }

  private send(socket: WebSocket, message: ServerMessage): void {
    socket.send(JSON.stringify(message));
  }

  private sendError(socket: WebSocket, error: ServerErrorMessage): void {
    this.send(socket, error);
  }

  private broadcastRoster(exclude?: Connection): void {
    if (!this.room) return;
    const roster = buildRoster(this.room);
    for (const conn of this.connections) {
      if (conn === exclude || conn.slot === null) continue;
      this.send(conn.socket, { type: "roster", roster });
    }
  }

  private handleMessage(connection: Connection, connectionId: string, event: MessageEvent): void {
    if (!this.room) return;
    const raw = typeof event.data === "string" ? event.data : "";
    if (raw.length === 0 || raw.length > MAX_MESSAGE_BYTES) {
      this.sendError(connection.socket, {
        type: "error",
        code: "INVALID_MESSAGE",
        message: "message must be a non-empty JSON string within the size limit",
      });
      return;
    }

    const result = parseClientMessage(raw);
    if (!result.valid || !result.message) {
      this.sendError(connection.socket, {
        type: "error",
        code: "INVALID_MESSAGE",
        message: result.error ?? "invalid message",
      });
      return;
    }

    const message = result.message;
    if (message.type === "join") {
      if (connection.slot !== null) {
        this.sendError(connection.socket, {
          type: "error",
          code: "ALREADY_JOINED",
          message: "this connection already holds a slot",
        });
        return;
      }
      const joinResult = joinRoom(this.room, { name: message.name ?? "Player", connectionId });
      if (!joinResult.ok) {
        this.sendError(connection.socket, {
          type: "error",
          code: joinResult.error,
          message: "room is full",
        });
        connection.socket.close(1013, "room full");
        return;
      }
      connection.slot = joinResult.value;
      this.send(connection.socket, {
        type: "welcome",
        roomId: this.room.roomId,
        slot: joinResult.value,
        capacity: this.room.capacity,
        roster: buildRoster(this.room),
      });
      this.broadcastRoster(connection);
      return;
    }

    if (connection.slot === null) {
      this.sendError(connection.socket, {
        type: "error",
        code: "NOT_JOINED",
        message: "send a join message before signaling",
      });
      return;
    }

    if (message.type === "leave") {
      leaveRoom(this.room, connection.slot);
      connection.slot = null;
      this.broadcastRoster();
      connection.socket.close(1000, "left room");
      return;
    }

    // offer / answer / ice: relay only to the addressed slot.
    const target = resolveSlot(this.room, message.to);
    if (!target) {
      this.sendError(connection.socket, {
        type: "error",
        code: "UNKNOWN_TARGET_SLOT",
        message: `no peer occupies slot ${message.to}`,
      });
      return;
    }
    const targetConnection = [...this.connections].find((c) => c.slot === message.to);
    if (targetConnection) {
      this.send(targetConnection.socket, {
        type: message.type,
        from: connection.slot,
        payload: message.payload,
      });
    }
  }

  private handleDisconnect(connection: Connection): void {
    this.connections.delete(connection);
    if (this.room && connection.slot !== null) {
      const slot = connection.slot;
      leaveRoom(this.room, slot);
      for (const conn of this.connections) {
        if (conn.slot === null) continue;
        this.send(conn.socket, { type: "peer-left", slot });
      }
      this.broadcastRoster();
    }
  }
}
