/**
 * Signaling protocol shared between the launcher's WebSocket client and the
 * Cloudflare Worker / Durable Object room backend. Every message is plain
 * JSON so it can be validated without trusting the sender.
 */

/** Maximum number of stable player slots any room may be configured with. */
export const MAX_ROOM_CAPACITY = 8;
/** Minimum number of stable player slots a room may be configured with. */
export const MIN_ROOM_CAPACITY = 2;

/** Room ids are short, URL-safe, human-shareable codes (e.g. "R7K2QX"). */
export const ROOM_ID_RE = /^[A-Z0-9]{4,10}$/;
/** Display names are limited to a small, printable-safe character set. */
export const PLAYER_NAME_RE = /^[\w .-]{1,24}$/;

/** Hard cap on serialized message size accepted from a client, in bytes. */
export const MAX_MESSAGE_BYTES = 16 * 1024;
/** Hard cap on SDP payload length, generous for real-world offers/answers. */
export const MAX_SDP_LENGTH = 12 * 1024;

export type SlotId = number;

/** Lockstep compatibility identity. Keep this synchronized with the native
 * protocol constants and the browser wasm bridge. Bump the individual
 * integer whenever that compatibility surface changes; peers must match all
 * three values before the Worker assigns a room slot. */
export interface CompatibilityVersion {
  readonly engine: number;
  readonly protocol: number;
  readonly determinism: number;
}

export const CURRENT_COMPATIBILITY: CompatibilityVersion = Object.freeze({
  engine: 1,
  protocol: 1,
  determinism: 1,
});

export interface ClientJoinMessage {
  readonly type: "join";
  readonly roomId: string;
  readonly name?: string;
  /** Only honored when the room does not exist yet. */
  readonly capacity?: number;
  readonly compatibility: CompatibilityVersion;
}

export interface ClientSignalMessage {
  readonly type: "offer" | "answer" | "ice";
  readonly to: SlotId;
  readonly payload: unknown;
}

export interface ClientLeaveMessage {
  readonly type: "leave";
}

export type ClientMessage = ClientJoinMessage | ClientSignalMessage | ClientLeaveMessage;

export interface RosterEntry {
  readonly slot: SlotId;
  readonly name: string;
  readonly isHost: boolean;
}

export interface ServerWelcomeMessage {
  readonly type: "welcome";
  readonly roomId: string;
  readonly slot: SlotId;
  readonly capacity: number;
  readonly roster: readonly RosterEntry[];
}

export interface ServerRosterMessage {
  readonly type: "roster";
  readonly roster: readonly RosterEntry[];
}

export interface ServerSignalMessage {
  readonly type: "offer" | "answer" | "ice";
  readonly from: SlotId;
  readonly payload: unknown;
}

export interface ServerPeerLeftMessage {
  readonly type: "peer-left";
  readonly slot: SlotId;
}

export type ServerErrorCode =
  | "INVALID_MESSAGE"
  | "ROOM_FULL"
  | "ROOM_NOT_FOUND"
  | "UNKNOWN_TARGET_SLOT"
  | "NOT_JOINED"
  | "ALREADY_JOINED"
  | "INCOMPATIBLE_CLIENT"
  | "RATE_LIMITED";

export interface ServerErrorMessage {
  readonly type: "error";
  readonly code: ServerErrorCode;
  readonly message: string;
}

export type ServerMessage =
  | ServerWelcomeMessage
  | ServerRosterMessage
  | ServerSignalMessage
  | ServerPeerLeftMessage
  | ServerErrorMessage;

export interface MessageValidationResult<T> {
  readonly valid: boolean;
  readonly message?: T;
  readonly error?: string;
}

function isPlainObject(value: unknown): value is Record<string, unknown> {
  return typeof value === "object" && value !== null && !Array.isArray(value);
}

export function isCompatibilityVersion(value: unknown): value is CompatibilityVersion {
  if (!isPlainObject(value)) return false;
  return (
    typeof value.engine === "number" &&
    Number.isInteger(value.engine) &&
    value.engine > 0 &&
    typeof value.protocol === "number" &&
    Number.isInteger(value.protocol) &&
    value.protocol > 0 &&
    typeof value.determinism === "number" &&
    Number.isInteger(value.determinism) &&
    value.determinism > 0
  );
}

export function compatibilityMatches(left: CompatibilityVersion, right: CompatibilityVersion): boolean {
  return left.engine === right.engine && left.protocol === right.protocol && left.determinism === right.determinism;
}

/**
 * Parses and validates a raw client-provided string as a {@link ClientMessage}.
 * Never throws; all failure paths return `valid: false` with a human-readable
 * reason so the caller can send back a {@link ServerErrorMessage}.
 */
export function parseClientMessage(raw: string): MessageValidationResult<ClientMessage> {
  if (typeof raw !== "string") {
    return { valid: false, error: "message must be a string" };
  }
  if (raw.length > MAX_MESSAGE_BYTES) {
    return { valid: false, error: `message exceeds ${MAX_MESSAGE_BYTES} bytes` };
  }

  let parsed: unknown;
  try {
    parsed = JSON.parse(raw);
  } catch {
    return { valid: false, error: "message is not valid JSON" };
  }

  return validateClientMessage(parsed);
}

/** Validates an already-parsed value as a {@link ClientMessage}. */
export function validateClientMessage(input: unknown): MessageValidationResult<ClientMessage> {
  if (!isPlainObject(input)) {
    return { valid: false, error: "message must be a JSON object" };
  }

  switch (input.type) {
    case "join": {
      if (typeof input.roomId !== "string" || !ROOM_ID_RE.test(input.roomId)) {
        return { valid: false, error: "join.roomId is missing or malformed" };
      }
      if (input.name !== undefined && (typeof input.name !== "string" || !PLAYER_NAME_RE.test(input.name))) {
        return { valid: false, error: "join.name is malformed" };
      }
      if (
        input.capacity !== undefined &&
        (typeof input.capacity !== "number" ||
          !Number.isInteger(input.capacity) ||
          input.capacity < MIN_ROOM_CAPACITY ||
          input.capacity > MAX_ROOM_CAPACITY)
      ) {
        return {
          valid: false,
          error: `join.capacity must be an integer between ${MIN_ROOM_CAPACITY} and ${MAX_ROOM_CAPACITY}`,
        };
      }
      if (!isCompatibilityVersion(input.compatibility)) {
        return { valid: false, error: "join.compatibility is missing or malformed" };
      }
      const message: ClientJoinMessage = {
        type: "join",
        roomId: input.roomId,
        ...(input.name !== undefined ? { name: input.name as string } : {}),
        ...(input.capacity !== undefined ? { capacity: input.capacity as number } : {}),
        compatibility: input.compatibility,
      };
      return { valid: true, message };
    }
    case "offer":
    case "answer":
    case "ice": {
      if (typeof input.to !== "number" || !Number.isInteger(input.to) || input.to < 0) {
        return { valid: false, error: `${input.type}.to must be a non-negative integer slot id` };
      }
      if (input.payload === undefined || input.payload === null) {
        return { valid: false, error: `${input.type}.payload is required` };
      }
      if (typeof input.payload === "string" && input.payload.length > MAX_SDP_LENGTH) {
        return { valid: false, error: `${input.type}.payload exceeds ${MAX_SDP_LENGTH} characters` };
      }
      const message: ClientSignalMessage = {
        type: input.type,
        to: input.to,
        payload: input.payload,
      };
      return { valid: true, message };
    }
    case "leave":
      return { valid: true, message: { type: "leave" } };
    default:
      return { valid: false, error: `unknown message type: ${String(input.type)}` };
  }
}
