import { ROOM_ID_RE, type RosterEntry } from "@generalsx-web/shared/protocol";

export interface RoomPanelCallbacks {
  onCreateRoom(capacity: number): void;
  onJoinRoom(roomId: string): void;
  onLeaveRoom(): void;
}

export interface RoomPanel {
  readonly element: HTMLElement;
  setRoster(roster: readonly RosterEntry[]): void;
  setStatus(text: string): void;
  /** Shows (or, passing `null`, hides) a non-blocking warning banner —
   * e.g. "continuing without TURN relay" — that does not prevent the
   * player from proceeding, unlike the launcher's blocking error overlay. */
  setWarning(text: string | null): void;
  showJoinedState(roomId: string): void;
  showLobbyState(): void;
}

/** Generates a random, human-shareable room code matching `ROOM_ID_RE`. */
export function generateRoomId(): string {
  const alphabet = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789"; // no O/0/I/1 ambiguity
  let id = "";
  for (let i = 0; i < 6; i += 1) {
    id += alphabet[Math.floor(Math.random() * alphabet.length)];
  }
  return id;
}

export function createRoomPanel(container: HTMLElement, callbacks: RoomPanelCallbacks, defaultCapacity: number): RoomPanel {
  const element = document.createElement("section");
  element.className = "gx-panel gx-room-panel";

  const warning = document.createElement("p");
  warning.className = "gx-room-warning";
  warning.setAttribute("role", "status");
  warning.hidden = true;

  const lobby = document.createElement("div");
  lobby.className = "gx-room-lobby";

  const createButton = document.createElement("button");
  createButton.type = "button";
  createButton.className = "gx-button gx-button-primary";
  createButton.textContent = "Create room";

  const capacitySelect = document.createElement("select");
  for (const capacity of [2, 3, 4, 5, 6, 7, 8]) {
    const opt = document.createElement("option");
    opt.value = String(capacity);
    opt.textContent = `${capacity} players`;
    if (capacity === defaultCapacity) opt.selected = true;
    capacitySelect.appendChild(opt);
  }

  const joinInput = document.createElement("input");
  joinInput.type = "text";
  joinInput.placeholder = "Room code";
  joinInput.maxLength = 10;
  joinInput.className = "gx-room-code-input";

  const joinButton = document.createElement("button");
  joinButton.type = "button";
  joinButton.className = "gx-button";
  joinButton.textContent = "Join room";

  const joinError = document.createElement("p");
  joinError.className = "gx-field-error";
  joinError.hidden = true;

  lobby.append(capacitySelect, createButton, joinInput, joinButton, joinError);

  const active = document.createElement("div");
  active.className = "gx-room-active";
  active.hidden = true;

  const roomCodeDisplay = document.createElement("p");
  roomCodeDisplay.className = "gx-room-code-display";

  const rosterList = document.createElement("ul");
  rosterList.className = "gx-roster-list";

  const status = document.createElement("p");
  status.className = "gx-room-status";

  const leaveButton = document.createElement("button");
  leaveButton.type = "button";
  leaveButton.className = "gx-button";
  leaveButton.textContent = "Leave room";

  active.append(roomCodeDisplay, rosterList, status, leaveButton);

  element.append(warning, lobby, active);
  container.appendChild(element);

  createButton.addEventListener("click", () => {
    callbacks.onCreateRoom(Number(capacitySelect.value));
  });

  joinButton.addEventListener("click", () => {
    const roomId = joinInput.value.trim().toUpperCase();
    if (!ROOM_ID_RE.test(roomId)) {
      joinError.textContent = "Enter a valid room code (4-10 letters/numbers).";
      joinError.hidden = false;
      return;
    }
    joinError.hidden = true;
    callbacks.onJoinRoom(roomId);
  });

  leaveButton.addEventListener("click", () => {
    callbacks.onLeaveRoom();
  });

  return {
    element,
    setRoster(roster: readonly RosterEntry[]) {
      rosterList.replaceChildren(
        ...roster.map((entry) => {
          const item = document.createElement("li");
          item.textContent = `${entry.name}${entry.isHost ? " (host)" : ""}`;
          return item;
        }),
      );
    },
    setStatus(text: string) {
      status.textContent = text;
    },
    setWarning(text: string | null) {
      warning.textContent = text ?? "";
      warning.hidden = text === null;
    },
    showJoinedState(roomId: string) {
      lobby.hidden = true;
      active.hidden = false;
      roomCodeDisplay.textContent = `Room code: ${roomId}`;
    },
    showLobbyState() {
      lobby.hidden = false;
      active.hidden = true;
      rosterList.replaceChildren();
    },
  };
}
