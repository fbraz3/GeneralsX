/** Page chrome: GeneralsX tactical header, framed WebGPU stage, and first-run ownership gate. */

export const STEAM_HELP = `
  <p>If Steam is installed in the default location on drive C:</p>
  <p><strong>Command &amp; Conquer: Generals</strong><br>
     <code class="text-hud-accent break-all">C:\\Program Files (x86)\\Steam\\steamapps\\common\\Command and Conquer Generals\\</code></p>
  <p><strong>Generals: Zero Hour</strong><br>
     <code class="text-hud-accent break-all">C:\\Program Files (x86)\\Steam\\steamapps\\common\\Command &amp; Conquer Generals - Zero Hour\\</code></p>
  <p>The main executable is <code class="text-hud-accent">Generals.exe</code>. To open the folder directly:
     Steam → Library → right-click the game → Manage → Browse local files.</p>
  <p>On macOS or Linux, pick the folder holding the game's <code class="text-hud-accent">.big</code> archives
     (<code class="text-hud-accent">INIZH.big</code>, <code class="text-hud-accent">TexturesZH.big</code>,
     <code class="text-hud-accent">AudioZH.big</code>…). Select the <strong>Zero Hour</strong> folder first;
     you will then be asked for the base <strong>Generals</strong> folder.</p>`;

export function render(root: HTMLElement): void {
  root.className = "flex min-h-svh flex-col";
  root.innerHTML = `
    <header class="flex min-h-[60px] flex-wrap items-center justify-between gap-x-6 gap-y-2 border-b border-hud-accent/35 bg-hud-surface/95 px-3 py-2 shadow-[0_4px_24px_rgba(0,0,0,0.45)] backdrop-blur sm:px-8">
      <div class="flex items-center gap-3">
        <img src="./generalsx-zh-icon.png" alt="GeneralsX" width="36" height="36"
             class="h-9 w-9 rounded border border-hud-accent/40 bg-black/40 object-contain shadow-[0_0_12px_hsl(38_92%_50%/.2)]">
        <div class="flex flex-col">
          <div class="flex items-center gap-2.5">
            <h1 class="m-0 text-lg font-bold uppercase tracking-[0.14em] text-hud-fg sm:text-xl">
              Generals<span class="text-hud-accent">X</span>
            </h1>
            <span class="border border-hud-accent/40 bg-hud-accent/10 px-1.5 py-0.5 text-[10px] font-semibold uppercase tracking-[0.12em] text-hud-accent">
              Zero Hour · WebGPU
            </span>
          </div>
          <p class="m-0 hidden text-[11px] tracking-[0.06em] text-hud-muted sm:block">
            Cross-Platform C&amp;C Generals Engine in WebAssembly
          </p>
        </div>
      </div>

      <nav class="flex items-center gap-2" aria-label="GeneralsX links">
        <a href="https://github.com/fbraz3/GeneralsX/releases" target="_blank" rel="noopener"
           class="hud-button inline-flex items-center whitespace-nowrap">Releases</a>
        <a href="https://github.com/fbraz3/GeneralsX/wiki" target="_blank" rel="noopener"
           class="hud-button inline-flex items-center whitespace-nowrap">Wiki</a>
        <a href="https://github.com/fbraz3/GeneralsX" target="_blank" rel="noopener"
           aria-label="GeneralsX on GitHub" title="GeneralsX on GitHub"
           class="hud-button inline-flex items-center gap-1.5 whitespace-nowrap">
          <svg viewBox="0 0 16 16" width="15" height="15" fill="currentColor" aria-hidden="true"><path d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27s1.36.09 2 .27c1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.01 8.01 0 0 0 16 8c0-4.42-3.58-8-8-8Z"/></svg>
          <span class="hidden sm:inline">GitHub</span>
        </a>
      </nav>
    </header>

    <main class="flex min-h-0 w-full flex-1 flex-col gap-2.5 px-2 py-2.5 sm:px-6">
      <section class="hud-cut flex min-h-[52px] flex-wrap items-center justify-between gap-2 px-3.5 py-2" style="--hud-cut-surface: var(--color-hud-surface)">
        <div class="min-w-0 flex-1">
          <div class="flex items-center gap-2.5">
            <span class="h-2 w-2 shrink-0 rounded-full bg-hud-accent shadow-[0_0_8px_hsl(38_92%_50%/.8)]"></span>
            <span id="status" role="status" aria-live="polite" class="text-sm font-bold tracking-wide">Starting…</span>
            <span id="status-detail" class="truncate text-xs text-hud-muted"></span>
          </div>
          <div id="progress-track" hidden class="mt-1.5 h-1.5 w-full border border-hud-border/80 bg-black/60 p-px">
            <div id="progress-bar" class="h-full w-0 bg-hud-accent transition-[width] duration-150"></div>
          </div>
        </div>
        <div class="flex min-w-0 flex-wrap items-center justify-end gap-2">
          <span id="cap-wasm" hidden class="border-l-[3px] border-hud-warm bg-hud-raised px-2 py-1 text-xs text-hud-warm">WASM missing</span>
          <span id="cap-webgpu" hidden class="border-l-[3px] border-hud-warm bg-hud-raised px-2 py-1 text-xs text-hud-warm">WebGPU missing</span>
          <label class="flex items-center gap-1.5 text-xs uppercase tracking-wider text-hud-muted"><span class="hidden lg:inline">Boot</span>
            <select id="boot" class="hud-select">
              <option value="fast">Fast start</option>
              <option value="full">Full start</option>
            </select>
          </label>
          <button id="share" hidden class="hud-button whitespace-nowrap" title="Copy the link for players on your network">Multiplayer</button>
          <button id="sound" class="hud-button whitespace-nowrap">Sound on</button>
          <button id="fullscreen" class="hud-button whitespace-nowrap">Fullscreen</button>
          <button id="reset" class="hud-button whitespace-nowrap" title="Clear saved settings and ownership, then reload">Reset</button>
        </div>
      </section>

      <div id="stage" class="grid min-h-0 w-full min-w-0 flex-1 place-items-center overflow-hidden">
        <section id="frame" class="hud-cut relative grid min-w-0 place-items-center p-2" style="--hud-cut-surface: #000">
          <canvas id="canvas" tabindex="0" class="block border-0 bg-black"></canvas>
          <button id="play" hidden
                  class="absolute inset-0 z-[7] grid place-items-center bg-[#090b0e]/90 text-hud-accent">
            <span class="hud-cut px-10 py-5 text-2xl font-bold uppercase tracking-[0.2em]"
                  style="--hud-cut-surface: var(--color-hud-raised)">Launch Game</span>
          </button>

          <!-- Guest pre-game sync: shown while menu assets stream from host -->
          <div id="holo" hidden class="absolute inset-0 z-[8] grid place-items-center bg-[#090b0e]/95">
            <div class="holo-min">
              <div class="holo-ring-wrap">
                <svg class="holo-ring-svg" viewBox="0 0 120 120" aria-hidden="true">
                  <circle class="holo-ring-track" cx="60" cy="60" r="54"></circle>
                  <circle id="holo-ring-fill" class="holo-ring-fill" cx="60" cy="60" r="54"></circle>
                </svg>
                <div class="holo-center">
                  <div id="holo-percent">0%</div>
                  <div id="holo-mb">contacting host…</div>
                </div>
              </div>
              <div id="holo-file">&nbsp;</div>
            </div>
          </div>

          <!-- Backtick developer console -->
          <div id="dev-console" hidden class="absolute inset-x-0 top-0 z-[9] flex max-h-[55%] flex-col border-b border-hud-accent/40 bg-[#090b0e]/95">
            <pre id="dev-console-output" class="m-0 flex-1 overflow-auto whitespace-pre-wrap p-2 text-xs text-hud-fg"></pre>
            <form id="dev-console-form" class="flex items-center border-t border-hud-border">
              <span class="px-2 py-1 text-xs text-hud-accent">&gt;</span>
              <input id="dev-console-input" autocomplete="off" autocapitalize="off" spellcheck="false"
                     class="min-w-0 flex-1 bg-transparent py-1 pr-2 text-xs text-hud-fg outline-none">
            </form>
          </div>
          <img id="cursor-overlay" alt="" hidden class="pointer-events-none fixed left-0 top-0 z-[5] [image-rendering:pixelated]">

          <div id="firstrun" hidden class="absolute inset-0 z-[6] grid place-items-center bg-[#090b0e]/95 p-4">
            <div class="hud-cut max-h-full max-w-3xl overflow-auto p-5 text-left sm:p-7" style="--hud-cut-surface: var(--color-hud-raised)">
              <div class="mb-4 flex items-center gap-3 border-b border-hud-border pb-3">
                <img src="./generalsx-zh-icon.png" alt="GeneralsX" width="40" height="40"
                     class="h-10 w-10 rounded border border-hud-accent/40 bg-black/50 object-contain">
                <div>
                  <h2 class="m-0 text-base font-bold uppercase tracking-[0.14em] text-hud-accent sm:text-lg">Mission Briefing · Load Game Files</h2>
                  <p class="m-0 text-xs text-hud-muted">Command &amp; Conquer: Generals — Zero Hour</p>
                </div>
              </div>
              <p class="mb-4 text-[13px] leading-relaxed text-hud-muted">
                <strong class="text-hud-fg">GeneralsX</strong> runs directly from your own installed copy of
                <strong>Command &amp; Conquer: Generals — Zero Hour</strong>. No copyrighted game assets are downloaded or redistributed.
              </p>
              <div class="border border-hud-border bg-hud-surface p-4">
                <h3 class="m-0 mb-2 flex items-center gap-2 text-sm font-semibold uppercase tracking-[0.08em] text-hud-accent">
                  Select your local game directory
                  <button id="firstrun-info" aria-label="Where to find the game folder"
                          class="hud-button h-6 min-h-6 w-6 rounded-full px-0 text-xs [clip-path:none]">?</button>
                </h3>
                <p class="text-[13px] text-hud-muted">Point the browser at your installed Zero Hour folder. All files are read locally in memory.</p>
                <button id="firstrun-folder" class="hud-button hud-button-primary mt-3">Select Game Folder</button>
                <p id="firstrun-folder-note" class="mt-2 min-h-4 text-xs text-hud-warm"></p>
              </div>
              <div id="firstrun-info-panel" hidden class="mt-4 space-y-2 border-l-[3px] border-hud-accent bg-hud-surface p-3.5 text-xs text-hud-muted">${STEAM_HELP}</div>
            </div>
          </div>
        </section>
      </div>

      <div class="flex flex-wrap items-center justify-between gap-2">
        <details class="hud-cut min-w-0 flex-1 px-3 py-2 text-sm" style="--hud-cut-surface: var(--color-hud-surface)">
          <summary class="cursor-pointer text-xs uppercase tracking-wider text-hud-muted">Runtime Console Log</summary>
          <textarea id="output" readonly aria-label="Runtime log"
                    class="mt-2 h-44 w-full resize-none bg-black/80 p-2 font-mono text-xs text-hud-muted"></textarea>
        </details>
      </div>

      <footer class="flex flex-wrap items-center justify-between gap-2 px-1 text-[11px] text-hud-muted">
        <span>GeneralsX Project · <a href="https://github.com/fbraz3/GeneralsX" target="_blank" rel="noopener" class="text-hud-accent hover:underline">github.com/fbraz3/GeneralsX</a></span>
        <span>
          Based on <a href="https://github.com/origami-ltd/wasm-generals" target="_blank" rel="noopener" class="text-hud-fg/90 hover:text-hud-accent hover:underline">wasm-generals</a>
          by <a href="https://github.com/ebellumat" target="_blank" rel="noopener" class="text-hud-fg/90 hover:text-hud-accent hover:underline">@ebellumat</a>
          · <a href="https://buymeacoffee.com/ebellumat" target="_blank" rel="noopener" class="text-hud-warm hover:underline">Support original author</a>
        </span>
      </footer>
    </main>`;
}

export const el = <T extends HTMLElement>(id: string): T => document.getElementById(id) as T;
