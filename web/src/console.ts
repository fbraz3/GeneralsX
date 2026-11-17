/** Backtick developer console, ported from the pre-Vite shell: ` toggles it, Esc closes it,
    Tab completes, arrows walk the history. Commands run against the live module exports. */
import { el } from "./ui";
import type { EmscriptenModule } from "./types";

export interface ConsoleDeps {
  module: () => EmscriptenModule | undefined;
  /** Toggle the page mute and return the line to print. */
  mute: () => string;
  /** One status line for the `status` command. */
  status: () => string;
  /** Guest sync state for the `sync` command. */
  sync: () => string;
}

const COMMANDS = [
  "capture", "clear", "close", "cursor", "fullscreen", "help", "lan", "mute",
  "pause", "reload", "replay", "status", "sync",
];

const HELP =
  "capture | clear | close | cursor free | fullscreen | help | "
  + "lan open|identity <1-254>|host|join|map <2-8>|ai <slot> easy|medium|hard|accept|start|surrender|status | "
  + "mute | pause on|off | reload | replay [path] [speed] | status | sync";

export function initConsole(deps: ConsoleDeps): void {
  const panel = el("dev-console");
  const output = el<HTMLPreElement>("dev-console-output");
  const form = el<HTMLFormElement>("dev-console-form");
  const input = el<HTMLInputElement>("dev-console-input");
  const canvas = el<HTMLCanvasElement>("canvas");
  const history: string[] = [];
  let historyIndex = 0;

  function write(value: string): void {
    output.textContent += `${value}\n`;
    output.scrollTop = output.scrollHeight;
  }

  function setOpen(open: boolean): void {
    panel.hidden = !open;
    if (open) {
      if (!output.textContent) write("GeneralsX console — type help. ` closes.");
      document.exitPointerLock?.();
      input.focus();
    } else {
      input.blur();
      canvas.blur();
    }
  }

  /** Call an engine export by name, 0 when the runtime or the export is missing. */
  function call(name: string, ...args: number[]): number {
    const exports = deps.module() as unknown as Record<string, ((...a: number[]) => number) | undefined>;
    return exports?.[`_${name}`]?.(...args) ?? 0;
  }

  function lan(args: string[]): string {
    const action = args[0];
    if (action === "status") {
      return JSON.stringify({
        state: call("GeneralsXLanState"),
        logicFrame: call("GeneralsXLogicFrame"),
        endFrame: call("GeneralsXLanEndFrame"),
      });
    }
    if (action === "open") {
      if (!call("GeneralsXOpenLanMenu")) return "LAN menu is not ready.";
      setOpen(false);
      return "Opening LAN lobby.";
    }
    if (action === "identity") {
      const client = Number(args[1]);
      return Number.isInteger(client) && client >= 1 && client <= 254
        && call("GeneralsXLanSetIdentity", client)
        ? `LAN identity set to Browser${client}.` : "Usage: lan identity <1-254>";
    }
    if (action === "map") {
      const players = Number(args[1] || 2);
      return Number.isInteger(players) && players >= 2 && players <= 8
        && call("GeneralsXLanSetMapMinPlayers", players)
        ? `Map set for ${players}+ players.` : "Usage: lan map <2-8>";
    }
    if (action === "ai") {
      const slot = Number(args[1]);
      const difficulty = { easy: 0, medium: 1, hard: 2 }[args[2] || "hard"];
      return Number.isInteger(slot) && difficulty !== undefined
        && call("GeneralsXLanSetSlotAI", slot, difficulty)
        ? `Slot ${slot} set to ${args[2] || "hard"} AI.` : "Usage: lan ai <slot> easy|medium|hard";
    }
    const requests: Record<string, string> = {
      host: "GeneralsXLanHost",
      join: "GeneralsXLanJoinFirst",
      accept: "GeneralsXLanAccept",
      start: "GeneralsXLanStart",
      surrender: "GeneralsXLanSurrender",
    };
    const exported = requests[action ?? ""];
    if (exported) return call(exported) ? `LAN ${action} requested.` : `LAN ${action} is not ready.`;
    return "Usage: lan open|identity <1-254>|host|join|map <2-8>|ai <slot> easy|medium|hard|accept|start|surrender|status";
  }

  function execute(commandLine: string): string {
    const [command, ...args] = commandLine.split(/\s+/);
    switch ((command ?? "").toLowerCase()) {
      case "help": return HELP;
      case "status": return deps.status();
      case "sync": return deps.sync();
      case "mute": return deps.mute();
      case "pause": {
        if (args[0] !== "on" && args[0] !== "off") return "Usage: pause on|off";
        return call("GeneralsXSetPaused", args[0] === "on" ? 1 : 0)
          ? `Pause ${args[0]}.` : "Not in a pausable match.";
      }
      case "cursor":
        if (args[0] === "free") {
          document.exitPointerLock?.();
          canvas.blur();
          return "Cursor released.";
        }
        return "Usage: cursor free";
      case "lan": return lan(args);
      case "capture": {
        const capture = { width: canvas.width, height: canvas.height, dataUrl: canvas.toDataURL("image/png") };
        (window as unknown as { generalsXReferenceCapture?: object }).generalsXReferenceCapture = capture;
        return `Captured ${capture.width}x${capture.height}.`;
      }
      case "replay": {
        const path = args[0] || "/GeneralsReplays/visual-reference.rep";
        const speed = Number(args[1] || 1);
        if (!/^\/GeneralsReplays\/[A-Za-z0-9_.-]+\.rep$/.test(path)) {
          return "Replay path must be a local /GeneralsReplays/*.rep file.";
        }
        if (!Number.isInteger(speed) || speed < 1 || speed > 10) {
          return "Replay speed must be an integer from 1 to 10.";
        }
        const query = new URLSearchParams(location.search);
        query.set("replay", path);
        query.set("replaySpeed", String(speed));
        location.search = query.toString();
        return `Loading ${path} at ${speed}x.`;
      }
      case "fullscreen":
        void el("frame").requestFullscreen().catch(() => {});
        return "Fullscreen requested.";
      case "reload":
        location.reload();
        return "Reloading.";
      case "clear":
        output.textContent = "";
        return "";
      case "close":
        setOpen(false);
        return "";
      default:
        return `Unknown command: ${command}. Type help.`;
    }
  }

  form.addEventListener("submit", (event) => {
    event.preventDefault();
    const line = input.value.trim();
    input.value = "";
    if (!line) return;
    history.push(line);
    historyIndex = history.length;
    write(`> ${line}`);
    const result = execute(line);
    if (result) write(result);
  });

  input.addEventListener("keydown", (event) => {
    if (event.key === "ArrowUp" && historyIndex > 0) {
      historyIndex -= 1;
      input.value = history[historyIndex] ?? "";
      event.preventDefault();
    } else if (event.key === "ArrowDown") {
      historyIndex = Math.min(historyIndex + 1, history.length);
      input.value = history[historyIndex] ?? "";
      event.preventDefault();
    } else if (event.key === "Tab") {
      event.preventDefault();
      const parts = input.value.trimStart().split(/\s+/);
      const matches = COMMANDS.filter((name) => name.startsWith(parts[0] ?? ""));
      if (parts.length === 1 && matches.length === 1) input.value = `${matches[0]} `;
      else if (matches.length > 1) write(matches.join("  "));
    }
  });

  // Capture phase, ahead of the engine's own listeners: while the console is open the game must
  // not see any keys, and the backtick itself must never reach it.
  addEventListener("keydown", (event) => {
    if (event.code === "Backquote" && !event.repeat) {
      event.preventDefault();
      event.stopImmediatePropagation();
      setOpen(panel.hidden);
      return;
    }
    if (!panel.hidden) {
      if (event.key === "Escape") {
        event.preventDefault();
        setOpen(false);
      }
      event.stopImmediatePropagation();
    }
  }, { capture: true });
}
