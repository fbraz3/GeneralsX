/**
 * Cloudflare Durable Object that owns one signaling room's live WebSocket
 * connections. All slot/roster/capacity decisions live in `room-logic.ts`
 * (framework-agnostic and unit tested); this class only adapts that logic to
 * the Cloudflare Workers WebSocket API.
 *
 * It is also the authorization boundary for TURN credential issuance. A
 * Durable Object is the single-threaded, single-instance authority for its
 * room, so it is the only component that can answer "is this client actually
 * sitting in this seat right now?" and enforce a rate limit that cannot be
 * bypassed by spreading requests across Worker isolates or colos. See
 * `../room/admission.ts` for the token design.
 */
/// <reference types="@cloudflare/workers-types" />
import {
  MAX_MESSAGE_BYTES,
  parseClientMessage,
  type ServerErrorMessage,
  type ServerMessage,
} from "@generalsx-web/shared/protocol";
import {
  ADMISSION_TOKEN_TTL_SECONDS,
  createAdmissionId,
  createRoomKeyBytes,
  importRoomKey,
  signAdmissionToken,
  verifyAdmissionToken,
} from "../room/admission.js";
import { TURN_GRANT_PATH, type TurnGrantResult } from "../room/turn-grant.js";
import {
  consumeTurnGrant,
  createTurnGrantLimiter,
  pruneTurnGrantSlots,
  type TurnGrantLimiterState,
} from "../room/turn-rate-limit.js";
import {
  buildRoster,
  createRoomState,
  isSlotHeldBy,
  joinRoom,
  leaveRoom,
  resolveSlot,
  type RoomState,
} from "./room-logic.js";

interface Connection {
  readonly socket: WebSocket;
  slot: number | null;
}

/** Durable Object storage key holding this room's raw HMAC key bytes. */
const ROOM_KEY_STORAGE_KEY = "turn-admission-key-v1";

export interface RoomDurableObjectEnv {
  readonly ALLOWED_ORIGINS?: string;
}

export class RoomDurableObject implements DurableObject {
  private room: RoomState | null = null;
  private readonly connections = new Set<Connection>();
  private nextConnectionId = 0;
  /** Per-room HMAC key for admission tokens. Loaded once in the constructor,
   * imported non-extractable, and never logged or sent anywhere. */
  private admissionKey: CryptoKey | null = null;
  private turnLimiter: TurnGrantLimiterState | null = null;

  constructor(
    private readonly state: DurableObjectState,
    private readonly env: RoomDurableObjectEnv,
  ) {
    // Load (or create exactly once) this room's admission signing key before
    // any request is dispatched, so a token can never be minted or verified
    // against a half-initialized key.
    void this.state.blockConcurrencyWhile(async () => {
      const stored = await this.state.storage.get<ArrayBuffer | Uint8Array>(ROOM_KEY_STORAGE_KEY);
      let keyBytes: Uint8Array;
      if (stored) {
        keyBytes = stored instanceof Uint8Array ? stored : new Uint8Array(stored);
      } else {
        keyBytes = createRoomKeyBytes();
        await this.state.storage.put(ROOM_KEY_STORAGE_KEY, keyBytes);
      }
      this.admissionKey = await importRoomKey(keyBytes);
    });
  }

  async fetch(request: Request): Promise<Response> {
    const url = new URL(request.url);
    if (url.pathname === TURN_GRANT_PATH) {
      return this.handleTurnGrant(request);
    }
    return this.handleWebSocketUpgrade(request, url);
  }

  private handleWebSocketUpgrade(request: Request, url: URL): Response {
    if (request.headers.get("Upgrade") !== "websocket") {
      return new Response("expected a WebSocket upgrade", { status: 426 });
    }

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
      void this.handleMessage(connection, connectionId, event);
    });
    server.addEventListener("close", () => {
      this.handleDisconnect(connection);
    });
    server.addEventListener("error", () => {
      this.handleDisconnect(connection);
    });

    return new Response(null, { status: 101, webSocket: client });
  }

  /**
   * Authorizes exactly one TURN credential grant.
   *
   * Order matters: the signature is verified first (it is what makes every
   * later claim trustworthy), then live seat occupancy, and only then is
   * rate-limit quota consumed — a request that was going to be rejected
   * anyway must not spend the room's allowance.
   *
   * Denials are returned as HTTP 200 with `ok: false` so the Worker can tell
   * a *decision* apart from a Durable Object transport failure and translate
   * each into the right client-facing status.
   */
  private async handleTurnGrant(request: Request): Promise<Response> {
    const decided = (result: TurnGrantResult): Response =>
      new Response(JSON.stringify(result), { status: 200, headers: { "Content-Type": "application/json" } });

    if (request.method !== "POST") {
      return decided({ ok: false, code: "UNAUTHORIZED", detail: "turn grant requires POST" });
    }
    const key = this.admissionKey;
    if (!key) {
      return decided({ ok: false, code: "UNAUTHORIZED", detail: "room admission key is unavailable" });
    }

    let token = "";
    try {
      const body = (await request.json()) as { token?: unknown };
      if (typeof body.token === "string") token = body.token;
    } catch {
      return decided({ ok: false, code: "UNAUTHORIZED", detail: "malformed grant request" });
    }
    if (!token) {
      return decided({ ok: false, code: "UNAUTHORIZED", detail: "missing room admission token" });
    }

    const room = this.room;
    if (!room) {
      return decided({
        ok: false,
        code: "UNAUTHORIZED",
        detail: "room has no active members; join the room before requesting TURN credentials",
      });
    }

    const verification = await verifyAdmissionToken(token, key, { expectedRoomId: room.roomId });
    if (!verification.ok) {
      return decided({ ok: false, code: "UNAUTHORIZED", detail: verification.detail });
    }
    const { slot, admissionId } = verification.claims;
    if (!isSlotHeldBy(room, slot, admissionId)) {
      return decided({
        ok: false,
        code: "UNAUTHORIZED",
        detail: "this admission no longer holds a slot in the room",
      });
    }

    const nowMs = Date.now();
    this.turnLimiter ??= createTurnGrantLimiter(nowMs);
    const decision = consumeTurnGrant(this.turnLimiter, slot, nowMs);
    if (!decision.allowed) {
      return decided({
        ok: false,
        code: "RATE_LIMITED",
        detail: `too many TURN credential requests for this ${String(decision.scope)}`,
        retryAfterSeconds: decision.retryAfterSeconds,
      });
    }
    pruneTurnGrantSlots(this.turnLimiter, nowMs);

    return decided({ ok: true, roomId: room.roomId, slot });
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

  /**
   * Mints this seat's TURN admission token.
   *
   * A minting failure is deliberately non-fatal: the player is already in the
   * room, and the launcher degrades to direct/STUN-only ICE with a visible
   * warning rather than being denied the match entirely.
   */
  private async mintAdmission(roomId: string, slot: number, admissionId: string): Promise<string | null> {
    const key = this.admissionKey;
    if (!key) return null;
    try {
      return await signAdmissionToken(key, {
        roomId,
        slot,
        admissionId,
        exp: Math.floor(Date.now() / 1000) + ADMISSION_TOKEN_TTL_SECONDS,
      });
    } catch {
      return null;
    }
  }

  private async handleMessage(connection: Connection, connectionId: string, event: MessageEvent): Promise<void> {
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
      const admissionId = createAdmissionId();
      const joinResult = joinRoom(this.room, {
        name: message.name ?? "Player",
        connectionId,
        compatibility: message.compatibility,
        admissionId,
      });
      if (!joinResult.ok) {
        const incompatible = joinResult.error === "INCOMPATIBLE_CLIENT";
        this.sendError(connection.socket, {
          type: "error",
          code: joinResult.error,
          message: incompatible
            ? "engine, network protocol, or determinism version does not match this room"
            : "room is full",
        });
        connection.socket.close(incompatible ? 1008 : 1013, incompatible ? "incompatible client" : "room full");
        return;
      }
      connection.slot = joinResult.value;
      const admission = await this.mintAdmission(this.room.roomId, joinResult.value, admissionId);
      // The connection may have dropped while the token was being signed;
      // `handleDisconnect` has then already freed the slot and broadcast the
      // roster, so there is nothing left to send.
      if (!this.connections.has(connection)) return;
      this.send(connection.socket, {
        type: "welcome",
        roomId: this.room.roomId,
        slot: joinResult.value,
        capacity: this.room.capacity,
        roster: buildRoster(this.room),
        ...(admission ? { admission } : {}),
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
