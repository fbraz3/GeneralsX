import { PLAYER_NAME_RE } from "@generalsx-web/shared/protocol";

export interface LauncherSettings {
  readonly volume: number; // 0..1
  readonly graphicsQuality: "low" | "medium" | "high";
  readonly playerName: string;
}

const MAX_LAN_PLAYER_NAME_LENGTH = 12;

export const DEFAULT_SETTINGS: LauncherSettings = {
  volume: 0.8,
  graphicsQuality: "medium",
  playerName: "",
};

export function normalizePlayerName(value: string): string {
  const validCharacters = [...value].filter((character) => PLAYER_NAME_RE.test(character));
  return validCharacters.join("").trim().slice(0, MAX_LAN_PLAYER_NAME_LENGTH);
}

export function generateBrowserPlayerName(
  randomId: () => string = () => crypto.randomUUID(),
): string {
  const suffix = randomId().replace(/[^A-Fa-f0-9]/g, "").slice(0, 4).toUpperCase().padEnd(4, "0");
  return `Player-${suffix}`;
}

export interface SettingsPanel {
  readonly element: HTMLElement;
  getSettings(): LauncherSettings;
  toggle(): void;
}

export function createSettingsPanel(
  container: HTMLElement,
  onChange: (settings: LauncherSettings) => void,
  initial: LauncherSettings = DEFAULT_SETTINGS,
): SettingsPanel {
  let settings = { ...initial };

  const element = document.createElement("section");
  element.className = "gx-panel gx-settings-panel";
  element.hidden = true;

  const heading = document.createElement("h2");
  heading.textContent = "Settings";

  const nameLabel = document.createElement("label");
  nameLabel.textContent = "Player name";
  const nameInput = document.createElement("input");
  nameInput.type = "text";
  nameInput.maxLength = 24;
  nameInput.value = settings.playerName;
  nameLabel.appendChild(nameInput);

  const volumeLabel = document.createElement("label");
  volumeLabel.textContent = "Volume";
  const volumeInput = document.createElement("input");
  volumeInput.type = "range";
  volumeInput.min = "0";
  volumeInput.max = "1";
  volumeInput.step = "0.01";
  volumeInput.value = String(settings.volume);
  volumeLabel.appendChild(volumeInput);

  const qualityLabel = document.createElement("label");
  qualityLabel.textContent = "Graphics quality";
  const qualitySelect = document.createElement("select");
  for (const option of ["low", "medium", "high"] as const) {
    const opt = document.createElement("option");
    opt.value = option;
    opt.textContent = option;
    qualitySelect.appendChild(opt);
  }
  qualitySelect.value = settings.graphicsQuality;
  qualityLabel.appendChild(qualitySelect);

  const emit = (): void => {
    settings = {
      playerName: normalizePlayerName(nameInput.value),
      volume: Number(volumeInput.value),
      graphicsQuality: qualitySelect.value as LauncherSettings["graphicsQuality"],
    };
    nameInput.value = settings.playerName;
    onChange(settings);
  };

  nameInput.addEventListener("change", emit);
  volumeInput.addEventListener("input", emit);
  qualitySelect.addEventListener("change", emit);

  element.append(heading, nameLabel, volumeLabel, qualityLabel);
  container.appendChild(element);

  return {
    element,
    getSettings: () => settings,
    toggle: () => {
      element.hidden = !element.hidden;
    },
  };
}
