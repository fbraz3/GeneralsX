/**
 * Pure, runtime-agnostic room state machine used by the `RoomDurableObject`.
 * Kept free of any Cloudflare-specific types (WebSocket, DurableObjectState)
 * so the slot assignment, roster, capacity, and disconnect logic can be unit
 * tested without a Workers runtime.
 */
import {
  MAX_ROOM_CAPACITY,
  MIN_ROOM_CAPACITY,
  type RosterEntry,
  type ServerErrorCode,
  type SlotId,
} from "@generalsx-web/shared/protocol";

export interface SlotInfo {
  readonly name: string;
  readonly isHost: boolean;
  /** Opaque id correlating this slot with its transport-level connection
   * (e.g. a WebSocket instance identity), never exposed to clients. */
  readonly connectionId: string;
}

export interface RoomState {
  readonly roomId: string;
  readonly capacity: number;
  readonly slots: Map<SlotId, SlotInfo>;
}

export type RoomResult<T> = { readonly ok: true; readonly value: T } | { readonly ok: false; readonly error: ServerErrorCode };

const DEFAULT_CAPACITY = 4;

/** Clamps a requested capacity into the supported room-size range. */
export function normalizeCapacity(requested: number | undefined): number {
  if (requested === undefined || !Number.isInteger(requested)) return DEFAULT_CAPACITY;
  return Math.min(MAX_ROOM_CAPACITY, Math.max(MIN_ROOM_CAPACITY, requested));
}

export function createRoomState(roomId: string, capacity: number | undefined): RoomState {
  return { roomId, capacity: normalizeCapacity(capacity), slots: new Map() };
}

/** Returns the lowest-numbered free slot, or `null` when the room is full.
 * Slots stay stable for the lifetime of a connection: a departed player's
 * slot is only reused once fully vacated, and lower slots are always
 * preferred so reconnecting players see a predictable seat. */
export function findFreeSlot(state: RoomState): SlotId | null {
  for (let slot = 0; slot < state.capacity; slot += 1) {
    if (!state.slots.has(slot)) return slot;
  }
  return null;
}

export interface JoinRequest {
  readonly name: string;
  readonly connectionId: string;
}

/** Assigns a stable slot to a new connection, or returns `ROOM_FULL`. */
export function joinRoom(state: RoomState, request: JoinRequest): RoomResult<SlotId> {
  const slot = findFreeSlot(state);
  if (slot === null) {
    return { ok: false, error: "ROOM_FULL" };
  }
  const isHost = state.slots.size === 0;
  state.slots.set(slot, { name: request.name, isHost, connectionId: request.connectionId });
  return { ok: true, value: slot };
}

/** Removes a slot's occupant, freeing it for a future joiner. No-op if the
 * slot was already empty (idempotent, safe to call from multiple disconnect
 * signal paths). */
export function leaveRoom(state: RoomState, slot: SlotId): void {
  state.slots.delete(slot);
}

export function isRoomEmpty(state: RoomState): boolean {
  return state.slots.size === 0;
}

export function buildRoster(state: RoomState): RosterEntry[] {
  return [...state.slots.entries()]
    .sort(([a], [b]) => a - b)
    .map(([slot, info]) => ({ slot, name: info.name, isHost: info.isHost }));
}

/** Looks up the occupant of a target slot for message routing. Returns
 * `undefined` when the slot is empty or out of range, which the caller
 * should surface to the sender as `UNKNOWN_TARGET_SLOT`. */
export function resolveSlot(state: RoomState, slot: SlotId): SlotInfo | undefined {
  return state.slots.get(slot);
}
