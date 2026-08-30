export interface LauncherSettings {
  readonly volume: number; // 0..1
  readonly graphicsQuality: "low" | "medium" | "high";
  readonly playerName: string;
}

export const DEFAULT_SETTINGS: LauncherSettings = {
  volume: 0.8,
  graphicsQuality: "medium",
  playerName: "",
};

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
      playerName: nameInput.value,
      volume: Number(volumeInput.value),
      graphicsQuality: qualitySelect.value as LauncherSettings["graphicsQuality"],
    };
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
