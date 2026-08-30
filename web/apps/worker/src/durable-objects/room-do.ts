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
 *
 * Its authority stops at its own room. Because creating a room is free, the
 * room-keyed limits here cannot bound what one *client* costs across many
 * rooms; the room-independent ceiling in `../turn/edge-quota.ts` does that,
 * and both are enforced on every credential request.
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
  signAdmissionToken,
  verifyAdmissionToken,
} from "../room/admission.js";
import {
  destroyRoomAdmissionKey,
  ensureRoomAdmissionKey,
  loadRoomAdmissionKey,
} from "../room/admission-key-store.js";
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
  isRoomEmpty,
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

/** Shared refusal for every "this room holds nobody" case, so a caller cannot
 * distinguish an unknown room from an empty one. */
const NO_ACTIVE_MEMBERS = "room has no active members; join the room before requesting TURN credentials";

export interface RoomDurableObjectEnv {
  readonly ALLOWED_ORIGINS?: string;
}

export class RoomDurableObject implements DurableObject {
  private room: RoomState | null = null;
  private readonly connections = new Set<Connection>();
  private nextConnectionId = 0;
  /** Per-room HMAC key for admission tokens: non-extractable, never logged,
   * never sent anywhere. Null until this room has actually admitted someone
   * (see {@link mintAdmission}) and again once it empties. */
  private admissionKey: CryptoKey | null = null;
  private turnLimiter: TurnGrantLimiterState | null = null;

  constructor(
    private readonly state: DurableObjectState,
    private readonly env: RoomDurableObjectEnv,
  ) {
    // Load — never create — the admission key before any request is
    // dispatched, so a token is never minted or verified against a
    // half-initialized key. Creating one here instead would mean any
    // attacker-chosen room id, including one peeked out of a forged token,
    // could force a durable storage write for a room that does not exist.
    void this.state.blockConcurrencyWhile(async () => {
      this.admissionKey = await loadRoomAdmissionKey(this.state.storage);
    });
  }

  /**
   * Erases this room's persistent footprint once no seat is occupied.
   *
   * Run under `blockConcurrencyWhile` so it cannot interleave with a join
   * that is concurrently creating a key: an empty room must never be left
   * holding a signing key, and a live room must never lose one.
   *
   * Dropping the rate-limiter state with it is safe *because* the room is
   * empty — no admission token can still be valid, so the only way to spend
   * the fresh allowance is to join again, and that path is metered by the
   * room-independent per-address ceiling in `../turn/edge-quota.ts`.
   */
  private collectIfEmpty(): void {
    if (!this.room || !isRoomEmpty(this.room)) return;
    void this.state.blockConcurrencyWhile(async () => {
      if (this.room && !isRoomEmpty(this.room)) return;
      this.admissionKey = null;
      this.turnLimiter = null;
      if (this.connections.size === 0) this.room = null;
      await destroyRoomAdmissionKey(this.state.storage);
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
    // No key means this room has never admitted anyone, or has emptied since.
    // Either way there is no seat to authorize, and — critically — a key is
    // *not* created here: the request is unauthenticated.
    const key = this.admissionKey;
    if (!key) {
      return decided({ ok: false, code: "UNAUTHORIZED", detail: NO_ACTIVE_MEMBERS });
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
    if (!room || isRoomEmpty(room)) {
      return decided({ ok: false, code: "UNAUTHORIZED", detail: NO_ACTIVE_MEMBERS });
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
    try {
      // First accepted join in this room creates the key. Reaching here means
      // the seat is already held, so the write is attributable to a real
      // member rather than to an arbitrary Durable Object id.
      const key = (this.admissionKey ??= await ensureRoomAdmissionKey(this.state.storage));
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
      // roster, so there is nothing left to send. Re-run collection because
      // minting may have re-created a key for a room that emptied meanwhile.
      if (!this.connections.has(connection)) {
        this.collectIfEmpty();
        return;
      }
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
      this.collectIfEmpty();
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
    this.collectIfEmpty();
  }
}
