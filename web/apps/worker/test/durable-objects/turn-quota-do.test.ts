import { describe, expect, it } from "vitest";
import { TurnQuotaDurableObject } from "../../src/durable-objects/turn-quota-do.js";
import { UNIDENTIFIED_CLIENT_KEY } from "../../src/turn/client-identity.js";
import {
  TURN_EDGE_CLIENT_SPEC,
  TURN_EDGE_QUOTA_URL,
  TURN_EDGE_UNIDENTIFIED_SPEC,
  type TurnEdgeDecision,
} from "../../src/turn/edge-quota.js";

function makeQuotaDo(): TurnQuotaDurableObject {
  return new TurnQuotaDurableObject();
}

async function ask(quota: TurnQuotaDurableObject, body: unknown, method = "POST"): Promise<Response> {
  return quota.fetch(
    new Request(TURN_EDGE_QUOTA_URL, {
      method,
      ...(method === "POST" ? { body: JSON.stringify(body), headers: { "Content-Type": "application/json" } } : {}),
    }),
  );
}

async function decisionFor(quota: TurnQuotaDurableObject, key: string, identified = true): Promise<TurnEdgeDecision> {
  const response = await ask(quota, { client: { key, identified } });
  return (await response.json()) as TurnEdgeDecision;
}

describe("TurnQuotaDurableObject", () => {
  it("allows a burst from one address and then refuses it", async () => {
    const quota = makeQuotaDo();
    for (let i = 0; i < TURN_EDGE_CLIENT_SPEC.burst; i += 1) {
      expect((await decisionFor(quota, "203.0.113.7")).allowed, `grant ${i}`).toBe(true);
    }
    const denied = await decisionFor(quota, "203.0.113.7");
    expect(denied.allowed).toBe(false);
    expect(denied.scope).toBe("client");
  });

  it("holds one shared counter, so rotating rooms cannot reset it", async () => {
    // The request body carries no room id at all: there is nothing an
    // attacker can vary to be counted as somebody new.
    const quota = makeQuotaDo();
    for (let i = 0; i < TURN_EDGE_CLIENT_SPEC.burst; i += 1) await decisionFor(quota, "203.0.113.7");
    expect((await decisionFor(quota, "203.0.113.7")).allowed).toBe(false);
    expect((await decisionFor(quota, "198.51.100.4")).allowed).toBe(true);
  });

  it("rejects a non-POST request", async () => {
    const response = await ask(makeQuotaDo(), null, "GET");
    expect(response.status).toBe(404);
  });

  it("rejects an unknown path", async () => {
    const quota = makeQuotaDo();
    const response = await quota.fetch(new Request("https://turn-quota.invalid/nope", { method: "POST" }));
    expect(response.status).toBe(404);
  });

  it("charges an unparseable body to the shared unidentified bucket", async () => {
    const quota = makeQuotaDo();
    const results: TurnEdgeDecision[] = [];
    for (let i = 0; i < TURN_EDGE_UNIDENTIFIED_SPEC.burst + 1; i += 1) {
      const response = await quota.fetch(
        new Request(TURN_EDGE_QUOTA_URL, { method: "POST", body: "not json" }),
      );
      results.push((await response.json()) as TurnEdgeDecision);
    }
    // A malformed internal request must never be a free pass around the limit.
    expect(results.at(-1)?.allowed).toBe(false);
  });

  it("does not let a caller claim to be identified as the shared bucket", async () => {
    const quota = makeQuotaDo();
    for (let i = 0; i < TURN_EDGE_UNIDENTIFIED_SPEC.burst; i += 1) {
      expect((await decisionFor(quota, UNIDENTIFIED_CLIENT_KEY, true)).allowed).toBe(true);
    }
    expect((await decisionFor(quota, UNIDENTIFIED_CLIENT_KEY, true)).allowed).toBe(false);
  });

  it("persists nothing: it has no storage at all", () => {
    // Constructed without a DurableObjectState, which only type-checks and
    // runs because the limiter is deliberately in-memory only.
    expect(() => makeQuotaDo()).not.toThrow();
  });
});
