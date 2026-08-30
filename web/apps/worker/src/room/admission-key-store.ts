/**
 * Storage lifecycle for a room's TURN admission signing key.
 *
 * Kept separate from the Durable Object class so the *when* of key creation is
 * unit testable, because the timing is the security property:
 *
 *  - A Durable Object id can be derived from any string, so `idFromName()` on
 *    an attacker-supplied room id will happily instantiate an object. If the
 *    constructor generated and persisted a key, every such id would leave a
 *    durable, billable storage record behind — an unauthenticated write
 *    amplification triggered by nothing more than a forged token.
 *  - So a key is only ever *loaded* on construction, and only ever *created*
 *    on the path where a join has already been accepted into a live room.
 *  - When the last member leaves, the key is destroyed along with the rest of
 *    the room's state. The room is gone; keeping its signing key would only
 *    preserve the ability to validate tokens for seats nobody holds.
 */
import { createRoomKeyBytes, importRoomKey } from "./admission.js";

/** Durable Object storage key holding this room's raw HMAC key bytes. */
export const ROOM_KEY_STORAGE_KEY = "turn-admission-key-v1";

/** The slice of `DurableObjectStorage` this module needs, so tests can supply
 * a plain map instead of a Cloudflare runtime. */
export interface AdmissionKeyStorage {
  get<T>(key: string): Promise<T | undefined>;
  put<T>(key: string, value: T): Promise<void>;
  deleteAll(): Promise<void>;
}

function toBytes(stored: ArrayBuffer | Uint8Array): Uint8Array {
  return stored instanceof Uint8Array ? stored : new Uint8Array(stored);
}

/**
 * Imports this room's existing key, or returns null when it has none.
 *
 * Never writes. Constructing a Durable Object for an unknown id must be
 * completely free of side effects.
 */
export async function loadRoomAdmissionKey(storage: AdmissionKeyStorage): Promise<CryptoKey | null> {
  const stored = await storage.get<ArrayBuffer | Uint8Array>(ROOM_KEY_STORAGE_KEY);
  if (!stored) return null;
  return importRoomKey(toBytes(stored));
}

/**
 * Returns this room's key, generating and persisting one on first use.
 *
 * Call only after a join has been accepted: that is what makes the write
 * attributable to a real room member rather than to an arbitrary id.
 */
export async function ensureRoomAdmissionKey(storage: AdmissionKeyStorage): Promise<CryptoKey> {
  const existing = await loadRoomAdmissionKey(storage);
  if (existing) return existing;
  const bytes = createRoomKeyBytes();
  await storage.put(ROOM_KEY_STORAGE_KEY, bytes);
  return importRoomKey(bytes);
}

/**
 * Erases every trace of the room once it is empty.
 *
 * `deleteAll` rather than a targeted delete: the key is the only thing stored
 * today, and clearing the whole object is what makes it eligible for full
 * cleanup rather than lingering as an empty-but-present record.
 */
export async function destroyRoomAdmissionKey(storage: AdmissionKeyStorage): Promise<void> {
  await storage.deleteAll();
}
