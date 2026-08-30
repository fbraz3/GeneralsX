// @vitest-environment happy-dom
import { describe, expect, it, vi } from "vitest";
import {
  createSettingsPanel,
  DEFAULT_SETTINGS,
  generateBrowserPlayerName,
  normalizePlayerName,
} from "../../src/ui/settings.js";

describe("browser player names", () => {
  it("generates a LAN-safe per-page default", () => {
    expect(generateBrowserPlayerName(() => "ab12-cdef")).toBe("Player-AB12");
  });

  it("removes protocol delimiters and enforces the LAN packet limit", () => {
    expect(normalizePlayerName("  Véry!/Long:Name;Here  ")).toBe("VryLongNameH");
  });
});

describe("createSettingsPanel", () => {
  it("mounts hidden and pre-fills inputs from the initial settings", () => {
    const container = document.createElement("div");
    const panel = createSettingsPanel(container, vi.fn(), { ...DEFAULT_SETTINGS, playerName: "Aeneas" });

    expect(container.contains(panel.element)).toBe(true);
    expect(panel.element.hidden).toBe(true);
    const nameInput = panel.element.querySelector('input[type="text"]') as HTMLInputElement;
    expect(nameInput.value).toBe("Aeneas");
  });

  it("getSettings reflects the constructor defaults before any edits", () => {
    const panel = createSettingsPanel(document.createElement("div"), vi.fn());
    expect(panel.getSettings()).toEqual(DEFAULT_SETTINGS);
  });

  it("toggle() flips the hidden attribute", () => {
    const panel = createSettingsPanel(document.createElement("div"), vi.fn());

    panel.toggle();
    expect(panel.element.hidden).toBe(false);
    panel.toggle();
    expect(panel.element.hidden).toBe(true);
  });

  it("editing the player name input emits onChange and updates getSettings", () => {
    const onChange = vi.fn();
    const panel = createSettingsPanel(document.createElement("div"), onChange);
    const nameInput = panel.element.querySelector('input[type="text"]') as HTMLInputElement;

    nameInput.value = "Camilla,Prime";
    nameInput.dispatchEvent(new Event("change"));

    expect(onChange).toHaveBeenCalledWith(expect.objectContaining({ playerName: "CamillaPrime" }));
    expect(panel.getSettings().playerName).toBe("CamillaPrime");
    expect(nameInput.value).toBe("CamillaPrime");
  });

  it("changing the graphics quality select updates getSettings", () => {
    const panel = createSettingsPanel(document.createElement("div"), vi.fn());
    const select = panel.element.querySelector("select") as HTMLSelectElement;

    select.value = "high";
    select.dispatchEvent(new Event("change"));

    expect(panel.getSettings().graphicsQuality).toBe("high");
  });
});
