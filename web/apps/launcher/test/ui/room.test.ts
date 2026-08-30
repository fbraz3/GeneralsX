// @vitest-environment happy-dom
import { describe, expect, it, vi } from "vitest";
import { createRoomPanel, generateRoomId } from "../../src/ui/room.js";
import { ROOM_ID_RE } from "@generalsx-web/shared/protocol";

describe("generateRoomId", () => {
  it("always produces a code matching the shared protocol pattern", () => {
    for (let i = 0; i < 50; i += 1) {
      expect(generateRoomId()).toMatch(ROOM_ID_RE);
    }
  });
});

describe("createRoomPanel", () => {
  function makePanel(defaultCapacity = 4) {
    const callbacks = { onCreateRoom: vi.fn(), onJoinRoom: vi.fn(), onLeaveRoom: vi.fn() };
    const panel = createRoomPanel(document.createElement("div"), callbacks, defaultCapacity);
    return { panel, callbacks };
  }

  it("starts in the lobby state with the active panel hidden", () => {
    const { panel } = makePanel();
    const lobby = panel.element.querySelector(".gx-room-lobby") as HTMLElement;
    const active = panel.element.querySelector(".gx-room-active") as HTMLElement;
    expect(lobby.hidden).toBe(false);
    expect(active.hidden).toBe(true);
  });

  it("clicking create room invokes onCreateRoom with the selected capacity", () => {
    const { panel, callbacks } = makePanel();
    const capacitySelect = panel.element.querySelector("select") as HTMLSelectElement;
    const createButton = panel.element.querySelector(".gx-button-primary") as HTMLButtonElement;

    // Set explicitly rather than relying on the pre-selected default option
    // (a real click always reads whatever the select's current value is).
    capacitySelect.value = "6";
    createButton.click();

    expect(callbacks.onCreateRoom).toHaveBeenCalledWith(6);
  });

  it("rejects an invalid room code without calling onJoinRoom", () => {
    const { panel, callbacks } = makePanel();
    const joinInput = panel.element.querySelector(".gx-room-code-input") as HTMLInputElement;
    const joinButton = panel.element.querySelector(".gx-room-lobby .gx-button:not(.gx-button-primary)") as HTMLButtonElement;

    joinInput.value = "!!";
    joinButton.click();

    expect(callbacks.onJoinRoom).not.toHaveBeenCalled();
    const joinError = panel.element.querySelector(".gx-field-error") as HTMLElement;
    expect(joinError.hidden).toBe(false);
  });

  it("accepts a valid room code and calls onJoinRoom with it upper-cased", () => {
    const { panel, callbacks } = makePanel();
    const joinInput = panel.element.querySelector(".gx-room-code-input") as HTMLInputElement;
    const joinButton = panel.element.querySelector(".gx-room-lobby .gx-button:not(.gx-button-primary)") as HTMLButtonElement;

    joinInput.value = "abcd12";
    joinButton.click();

    expect(callbacks.onJoinRoom).toHaveBeenCalledWith("ABCD12");
  });

  it("showJoinedState reveals the active panel and displays the room code", () => {
    const { panel } = makePanel();
    panel.showJoinedState("ABCD12");

    const lobby = panel.element.querySelector(".gx-room-lobby") as HTMLElement;
    const active = panel.element.querySelector(".gx-room-active") as HTMLElement;
    expect(lobby.hidden).toBe(true);
    expect(active.hidden).toBe(false);
    expect(panel.element.querySelector(".gx-room-code-display")?.textContent).toBe("Room code: ABCD12");
  });

  it("setRoster renders one list item per entry, marking the host", () => {
    const { panel } = makePanel();
    panel.setRoster([
      { slot: 0, name: "Host", isHost: true },
      { slot: 1, name: "Guest", isHost: false },
    ]);

    const items = [...panel.element.querySelectorAll(".gx-roster-list li")].map((li) => li.textContent);
    expect(items).toEqual(["Host (host)", "Guest"]);
  });

  it("showLobbyState returns to the lobby and clears the roster", () => {
    const { panel } = makePanel();
    panel.setRoster([{ slot: 0, name: "Host", isHost: true }]);
    panel.showJoinedState("ABCD12");
    panel.showLobbyState();

    const lobby = panel.element.querySelector(".gx-room-lobby") as HTMLElement;
    const active = panel.element.querySelector(".gx-room-active") as HTMLElement;
    expect(lobby.hidden).toBe(false);
    expect(active.hidden).toBe(true);
    expect(panel.element.querySelectorAll(".gx-roster-list li")).toHaveLength(0);
  });

  it("leaving the room invokes onLeaveRoom", () => {
    const { panel, callbacks } = makePanel();
    panel.showJoinedState("ABCD12");
    const leaveButton = panel.element.querySelector(".gx-room-active .gx-button") as HTMLButtonElement;
    leaveButton.click();
    expect(callbacks.onLeaveRoom).toHaveBeenCalledTimes(1);
  });
});
