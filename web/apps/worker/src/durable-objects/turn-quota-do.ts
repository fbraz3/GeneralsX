/**
 * The single Durable Object instance that enforces the deployment-wide TURN
 * credential ceiling.
 *
 * Every `/turn-credentials` request passes through this one object before any
 * room is consulted, so the limit holds no matter how many rooms, isolates,
 * or colos the traffic is spread across. See `../turn/edge-quota.ts` for why
 * a room-keyed limit alone is not enough.
 *
 * State is held in memory and never persisted. That is a deliberate trade:
 * persisting would mean a storage write on every credential request, and the
 * only way to clear an in-memory bucket is for the object to be evicted after
 * a stretch of inactivity — which by definition cannot happen while an
 * attacker is actively sending traffic, i.e. exactly when the limit matters.
 * The cost of eviction is that a long-idle deployment starts with a full
 * burst, which is the intended resting state anyway.
 */
/// <reference types="@cloudflare/workers-types" />
import { UNIDENTIFIED_CLIENT_KEY } from "../turn/client-identity.js";
import {
  consumeTurnEdgeQuota,
  createTurnEdgeQuota,
  pruneTurnEdgeClients,
  TURN_EDGE_QUOTA_PATH,
  type TurnEdgeDecision,
  type TurnEdgeQuotaRequestBody,
  type TurnEdgeQuotaState,
} from "../turn/edge-quota.js";

/** Requests between sweeps of the tracked-client map. Pruning is O(tracked
 * clients), so doing it on every request would make the limiter's own cost
 * grow with the size of the attack it is absorbing. */
const PRUNE_INTERVAL_REQUESTS = 256;

export class TurnQuotaDurableObject implements DurableObject {
  private quota: TurnEdgeQuotaState | null = null;
  private requestsSincePrune = 0;

  async fetch(request: Request): Promise<Response> {
    const decided = (decision: TurnEdgeDecision): Response =>
      new Response(JSON.stringify(decision), { status: 200, headers: { "Content-Type": "application/json" } });

    const url = new URL(request.url);
    if (url.pathname !== TURN_EDGE_QUOTA_PATH || request.method !== "POST") {
      return new Response("not found", { status: 404 });
    }

    let body: TurnEdgeQuotaRequestBody | null;
    try {
      body = (await request.json()) as TurnEdgeQuotaRequestBody;
    } catch {
      body = null;
    }

    const client = body?.client;
    const rawKey = typeof client?.key === "string" ? client.key : "";
    // An unusable identity falls back to the shared unidentified bucket, never
    // to a fresh per-caller allowance: a malformed internal request must not
    // become a way around the limiter.
    const key = rawKey.length > 0 ? rawKey : UNIDENTIFIED_CLIENT_KEY;
    const identified = client?.identified === true && key !== UNIDENTIFIED_CLIENT_KEY;

    const nowMs = Date.now();
    this.quota ??= createTurnEdgeQuota(nowMs);
    const decision = consumeTurnEdgeQuota(this.quota, { key, identified }, nowMs);

    this.requestsSincePrune += 1;
    if (this.requestsSincePrune >= PRUNE_INTERVAL_REQUESTS) {
      this.requestsSincePrune = 0;
      pruneTurnEdgeClients(this.quota, nowMs);
    }
    return decided(decision);
  }
}
