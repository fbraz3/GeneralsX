import { describe, expect, it } from "vitest";
import { fetchTurnCredentials } from "../../src/turn/turn-credentials.js";

/**
 * Live smoke test for the real Cloudflare Realtime TURN credential API.
 *
 * This intentionally makes a real network call and is skipped by default
 * (including in the normal `npm run test` unit gate) unless both
 * TURN_KEY_ID and TURN_KEY_API_TOKEN are present in the environment. It is
 * only ever exercised by the staged, secret-gated `turn-credentials-smoke`
 * job in .github/workflows/build-web.yml, triggered manually via
 * workflow_dispatch against an environment with those secrets configured.
 *
 * This test proves that this repository's TURN credential issuance code
 * still round-trips against the live Cloudflare API. It does NOT exercise
 * browser-to-native WebRTC connectivity, and passing this test must never
 * be interpreted as end-to-end browser-to-native or TURN relay coverage.
 */
const hasLiveCredentials = Boolean(process.env.TURN_KEY_ID && process.env.TURN_KEY_API_TOKEN);

describe("fetchTurnCredentials (live Cloudflare API)", () => {
  it.skipIf(!hasLiveCredentials)("issues real short-lived TURN credentials", async () => {
    const outcome = await fetchTurnCredentials({
      TURN_KEY_ID: process.env.TURN_KEY_ID,
      TURN_KEY_API_TOKEN: process.env.TURN_KEY_API_TOKEN,
    });

    expect(outcome.ok).toBe(true);
    if (outcome.ok) {
      expect(Array.isArray(outcome.iceServers)).toBe(true);
      expect((outcome.iceServers as unknown[]).length).toBeGreaterThan(0);
    }
  });
});
