import { describe, expect, it } from "vitest";
import { UNIDENTIFIED_CLIENT_KEY } from "../../src/turn/client-identity.js";
import {
  consumeTurnEdgeQuota,
  createTurnEdgeQuota,
  describeTurnEdgeScope,
  pruneTurnEdgeClients,
  TURN_EDGE_CLIENT_SPEC,
  TURN_EDGE_GLOBAL_SPEC,
  TURN_EDGE_MAX_TRACKED_CLIENTS,
  TURN_EDGE_UNIDENTIFIED_SPEC,
  type TurnEdgeQuotaState,
} from "../../src/turn/edge-quota.js";

const CLIENT = { key: "203.0.113.7", identified: true };

function drain(state: TurnEdgeQuotaState, client: { key: string; identified: boolean }, times: number, nowMs = 0) {
  for (let i = 0; i < times; i += 1) consumeTurnEdgeQuota(state, client, nowMs);
}

describe("consumeTurnEdgeQuota: per-client ceiling", () => {
  it("allows a burst and then refuses", () => {
    const state = createTurnEdgeQuota(0);
    for (let i = 0; i < TURN_EDGE_CLIENT_SPEC.burst; i += 1) {
      expect(consumeTurnEdgeQuota(state, CLIENT, 0).allowed, `grant ${i}`).toBe(true);
    }
    const denied = consumeTurnEdgeQuota(state, CLIENT, 0);
    expect(denied.allowed).toBe(false);
    expect(denied.scope).toBe("client");
    expect(denied.retryAfterSeconds).toBeGreaterThanOrEqual(1);
  });

  it("is not reset by rotating rooms, because it is not keyed by room", () => {
    // The whole point: the caller supplies no room id here, so however many
    // rooms an attacker creates, they land in the same bucket.
    const state = createTurnEdgeQuota(0);
    drain(state, CLIENT, TURN_EDGE_CLIENT_SPEC.burst);
    expect(consumeTurnEdgeQuota(state, CLIENT, 0).allowed).toBe(false);
  });

  it("refills over time", () => {
    const state = createTurnEdgeQuota(0);
    drain(state, CLIENT, TURN_EDGE_CLIENT_SPEC.burst);
    expect(consumeTurnEdgeQuota(state, CLIENT, TURN_EDGE_CLIENT_SPEC.refillMs).allowed).toBe(true);
  });

  it("keeps distinct addresses independent", () => {
    const state = createTurnEdgeQuota(0);
    drain(state, CLIENT, TURN_EDGE_CLIENT_SPEC.burst);
    expect(consumeTurnEdgeQuota(state, { key: "198.51.100.4", identified: true }, 0).allowed).toBe(true);
  });

  it("does not charge the global ceiling for a request it refuses", () => {
    const state = createTurnEdgeQuota(0);
    drain(state, CLIENT, TURN_EDGE_CLIENT_SPEC.burst);
    const globalBefore = state.global.tokens;
    consumeTurnEdgeQuota(state, CLIENT, 0);
    // Otherwise one throttled client could drain the deployment-wide ceiling
    // and deny service to everyone else.
    expect(state.global.tokens).toBe(globalBefore);
  });

  it("gives unidentified callers a smaller shared allowance", () => {
    const state = createTurnEdgeQuota(0);
    const anon = { key: UNIDENTIFIED_CLIENT_KEY, identified: false };
    expect(TURN_EDGE_UNIDENTIFIED_SPEC.burst).toBeLessThan(TURN_EDGE_CLIENT_SPEC.burst);
    drain(state, anon, TURN_EDGE_UNIDENTIFIED_SPEC.burst);
    expect(consumeTurnEdgeQuota(state, anon, 0).allowed).toBe(false);
  });
});

describe("consumeTurnEdgeQuota: global ceiling", () => {
  it("refuses once the deployment-wide bucket is empty, even for a fresh address", () => {
    const state = createTurnEdgeQuota(0);
    state.global.tokens = 0;
    const decision = consumeTurnEdgeQuota(state, { key: "198.51.100.1", identified: true }, 0);
    expect(decision.allowed).toBe(false);
    expect(decision.scope).toBe("global");
  });

  it("caps aggregate issuance across many distinct addresses", () => {
    const state = createTurnEdgeQuota(0);
    let allowed = 0;
    for (let i = 0; i < TURN_EDGE_GLOBAL_SPEC.burst + 50; i += 1) {
      if (consumeTurnEdgeQuota(state, { key: `198.51.100.${i}`, identified: true }, 0).allowed) allowed += 1;
    }
    expect(allowed).toBe(TURN_EDGE_GLOBAL_SPEC.burst);
  });
});

describe("consumeTurnEdgeQuota: tracked-client capacity", () => {
  it("refuses unknown clients rather than growing without bound", () => {
    const state = createTurnEdgeQuota(0);
    // Fill the map with buckets that carry debt, so pruning cannot reclaim
    // them: this is the state an address-rotating attacker creates.
    for (let i = 0; i < TURN_EDGE_MAX_TRACKED_CLIENTS; i += 1) {
      state.perClient.set(`fill-${i}`, { tokens: 0, updatedAtMs: 0 });
    }
    const decision = consumeTurnEdgeQuota(state, { key: "new-client", identified: true }, 0);
    expect(decision.allowed).toBe(false);
    expect(decision.scope).toBe("capacity");
    expect(state.perClient.size).toBe(TURN_EDGE_MAX_TRACKED_CLIENTS);
  });

  it("reclaims fully-refilled buckets before refusing", () => {
    const state = createTurnEdgeQuota(0);
    for (let i = 0; i < TURN_EDGE_MAX_TRACKED_CLIENTS; i += 1) {
      state.perClient.set(`fill-${i}`, { tokens: TURN_EDGE_CLIENT_SPEC.burst, updatedAtMs: 0 });
    }
    expect(consumeTurnEdgeQuota(state, { key: "new-client", identified: true }, 0).allowed).toBe(true);
  });
});

describe("pruneTurnEdgeClients", () => {
  it("drops only buckets that have fully refilled", () => {
    const state = createTurnEdgeQuota(0);
    consumeTurnEdgeQuota(state, CLIENT, 0);
    pruneTurnEdgeClients(state, 0);
    expect(state.perClient.has(CLIENT.key)).toBe(true);

    pruneTurnEdgeClients(state, TURN_EDGE_CLIENT_SPEC.refillMs * TURN_EDGE_CLIENT_SPEC.burst);
    expect(state.perClient.has(CLIENT.key)).toBe(false);
  });

  it("never forgets a throttled client, which would hand back its allowance", () => {
    const state = createTurnEdgeQuota(0);
    drain(state, CLIENT, TURN_EDGE_CLIENT_SPEC.burst);
    pruneTurnEdgeClients(state, 0);
    expect(state.perClient.has(CLIENT.key)).toBe(true);
    expect(consumeTurnEdgeQuota(state, CLIENT, 0).allowed).toBe(false);
  });

  it("keeps the shared unidentified bucket", () => {
    const state = createTurnEdgeQuota(0);
    consumeTurnEdgeQuota(state, { key: UNIDENTIFIED_CLIENT_KEY, identified: false }, 0);
    pruneTurnEdgeClients(state, 10_000_000);
    expect(state.perClient.has(UNIDENTIFIED_CLIENT_KEY)).toBe(true);
  });
});

describe("describeTurnEdgeScope", () => {
  it("names the ceiling that refused the request", () => {
    expect(describeTurnEdgeScope("client")).toContain("this address");
    expect(describeTurnEdgeScope("global")).toContain("deployment-wide");
    expect(describeTurnEdgeScope("capacity")).toContain("capacity");
    expect(describeTurnEdgeScope(null)).toContain("refused");
  });
});
