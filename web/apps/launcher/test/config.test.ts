import { describe, expect, it } from "vitest";
import { LAUNCHER_CONFIG } from "../src/config.js";

describe("LAUNCHER_CONFIG", () => {
  it("is frozen so it cannot be mutated at runtime", () => {
    expect(Object.isFrozen(LAUNCHER_CONFIG)).toBe(true);
  });

  it("only points at https:// origins", () => {
    expect(LAUNCHER_CONFIG.manifestUrl.startsWith("https://")).toBe(true);
    expect(LAUNCHER_CONFIG.signalingWorkerUrl.startsWith("https://")).toBe(true);
  });
});
