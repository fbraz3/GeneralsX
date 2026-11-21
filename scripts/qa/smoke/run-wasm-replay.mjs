#!/usr/bin/env node
/**
 * Run a GeneralsX Zero Hour replay (.rep) headlessly in Node.js using the WebAssembly build.
 *
 * Usage:
 *   node scripts/qa/smoke/run-wasm-replay.mjs <replay.rep> [--runtime path/to/GeneralsXZH.js]
 *        [--zh-dir /path/to/GeneralsZH] [--gen-dir /path/to/Generals] [--user-data /path/to/UserData]
 */

import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { pathToFileURL } from "node:url";

function parseArgs(argv) {
  const args = {
    replay: "",
    runtime: "",
    zhDir: process.env.CNC_GENERALS_ZH_PATH || "",
    genDir: process.env.CNC_GENERALS_PATH || "",
    userData: "",
    ignoreSyncErrors: false,
    expectCrc: "",
  };
  for (let i = 2; i < argv.length; i++) {
    const arg = argv[i];
    if (arg === "--runtime" && argv[i + 1]) args.runtime = argv[++i];
    else if (arg === "--zh-dir" && argv[i + 1]) args.zhDir = argv[++i];
    else if (arg === "--gen-dir" && argv[i + 1]) args.genDir = argv[++i];
    else if (arg === "--user-data" && argv[i + 1]) args.userData = argv[++i];
    else if (arg === "--ignore-sync-errors") args.ignoreSyncErrors = true;
    else if (arg === "--expect-crc" && argv[i + 1]) args.expectCrc = argv[++i].replace(/^0x/i, "").toUpperCase();
    else if (!arg.startsWith("-") && !args.replay) args.replay = arg;
  }
  return args;
}

function hasRetailBig(dir) {
  try {
    return (
      fs.existsSync(dir) &&
      fs.statSync(dir).isDirectory() &&
      fs.readdirSync(dir).some((f) => f.toLowerCase().endsWith(".big") && f.toLowerCase() !== "generalsx.big")
    );
  } catch {
    return false;
  }
}

function firstValidGameDir(candidates) {
  for (const dir of candidates) {
    if (dir && hasRetailBig(dir)) {
      return path.resolve(dir);
    }
  }
  return "";
}

function firstExistingDir(candidates) {
  for (const dir of candidates) {
    if (dir && fs.existsSync(dir) && fs.statSync(dir).isDirectory()) {
      return path.resolve(dir);
    }
  }
  return "";
}

const opts = parseArgs(process.argv);
if (!opts.replay || !fs.existsSync(opts.replay)) {
  console.error(`Usage: node run-wasm-replay.mjs <replay.rep> [--runtime GeneralsXZH.js]`);
  process.exit(2);
}

const home = os.homedir();
const zhDir = firstValidGameDir([
  opts.zhDir,
  path.join(home, "GeneralsX", "GeneralsZH"),
  path.join(home, "GeneralsX", "data", "GeneralsZH"),
]);
const genDir = firstValidGameDir([
  opts.genDir,
  path.join(home, "GeneralsX", "Generals"),
  path.join(home, "GeneralsX", "data", "Generals"),
]);
if (!zhDir) {
  console.error("ERROR: Could not locate Zero Hour game data directory (set CNC_GENERALS_ZH_PATH or --zh-dir).");
  process.exit(2);
}

const runtimePath = path.resolve(
  opts.runtime ||
    (fs.existsSync("build/webgpu/GeneralsMD/GeneralsXZH.js")
      ? "build/webgpu/GeneralsMD/GeneralsXZH.js"
      : "GeneralsXZH.js")
);
const runtimeDir = path.dirname(runtimePath);
const generalsxBigCandidates = [
  path.join(runtimeDir, "generalsx.big"),
  path.join(runtimeDir, "..", "generalsx.big"),
  path.resolve("build/webgpu/generalsx.big"),
];
const generalsxBigPath = generalsxBigCandidates.find((p) => fs.existsSync(p)) || "";

const userDataHost = firstExistingDir([
  opts.userData,
  path.join(home, "Library", "Application Support", "GeneralsX", "GeneralsZH"),
  path.join(home, ".local", "share", "GeneralsX", "GeneralsZH"),
]);

const replayAbs = path.resolve(opts.replay);
const replayName = path.basename(replayAbs);
const vfsUserData = "/home/web_user/.local/share/GeneralsX/GeneralsZH";
const vfsReplayPath = `${vfsUserData}/Replays/${replayName}`;

const cleanup = () => {};
process.on("exit", cleanup);
process.on("uncaughtException", (err) => {
  cleanup();
  console.error("Uncaught WASM replay exception:", err?.message || err);
  if (err?.stack) console.error(err.stack.split("\n").slice(0, 8).join("\n"));
  process.exit(1);
});

const { default: createGeneralsX } = await import(pathToFileURL(runtimePath).href);
globalThis.generalsXLanHostIP = 0x0a000001;
const wasmPath = path.join(runtimeDir, "GeneralsXZH.wasm");
const dataPath = path.join(runtimeDir, "GeneralsXZH.data");

let exitCode = 0;
let sawReplayMismatch = false;
let observedFrame100Crc = "";
const inspectLogLine = (line) => {
  if (typeof line !== "string") return;
  if (line.includes("REPLAY_CRC_MISMATCH")) {
    sawReplayMismatch = true;
    exitCode = 1;
  }
  const checkpoint = line.match(/REPLAY_CRC_CHECKPOINT\s+frame=100\s+inGame=0x([0-9A-Fa-f]{8})/);
  if (checkpoint && !observedFrame100Crc) {
    observedFrame100Crc = checkpoint[1].toUpperCase();
  }
  const f100 = line.match(/CRC Mismatch in Frame 100 \(ignored\) InGame:([0-9A-Fa-f]{8})/);
  if (f100 && !observedFrame100Crc) {
    observedFrame100Crc = f100[1].toUpperCase();
  }
  const m = line.match(/(?:GameMain\(\) returned with code|Exiting with code)\s+(-?\d+)/);
  if (m) {
    const code = Number.parseInt(m[1], 10);
    if (!Number.isNaN(code) && code !== 0) {
      exitCode = code;
    }
  }
};

const engineArgs = ["-headless", "-quickstart", "-noshellmap"];
if (opts.ignoreSyncErrors) {
  engineArgs.push("-ignoreReplaySyncErrors");
}
engineArgs.push("-replay", vfsReplayPath);

const finalizeExitCode = (runtimeCode = 0) => {
  let finalCode = runtimeCode !== 0 ? runtimeCode : (exitCode || (sawReplayMismatch ? 1 : 0));
  if (opts.expectCrc) {
    if (!observedFrame100Crc) {
      console.error(`ERROR: Expected Frame 100 InGame CRC 0x${opts.expectCrc}, but no Frame 100 checkpoint was observed.`);
      finalCode = 1;
    } else if (observedFrame100Crc !== opts.expectCrc) {
      console.error(`ERROR: Frame 100 InGame CRC mismatch: got 0x${observedFrame100Crc}, expected 0x${opts.expectCrc}`);
      finalCode = 1;
    } else {
      console.log(`VERIFIED Frame 100 InGame CRC: 0x${observedFrame100Crc}`);
    }
  }
  return finalCode;
};

await createGeneralsX({
  arguments: engineArgs,
  wasmBinary: fs.existsSync(wasmPath) ? fs.readFileSync(wasmPath) : undefined,
  print(text) {
    inspectLogLine(text);
    console.log(text);
  },
  printErr(text) {
    inspectLogLine(text);
    console.error(text);
  },
  getPreloadedPackage(name) {
    const p = path.join(runtimeDir, path.basename(name));
    const buf = fs.readFileSync(fs.existsSync(p) ? p : dataPath);
    return buf.buffer.slice(buf.byteOffset, buf.byteOffset + buf.byteLength);
  },
  locateFile(file) {
    return path.join(runtimeDir, file);
  },
  preRun: [
    (instance) => {
      const { FS, NODEFS } = instance;
      FS.mkdirTree("/GeneralsZH");
      if (generalsxBigPath && !fs.existsSync(path.join(zhDir, "generalsx.big"))) {
        FS.mkdirTree("/GeneralsZH_host");
        FS.mount(NODEFS, { root: zhDir }, "/GeneralsZH_host");
        for (const entry of fs.readdirSync(zhDir)) {
          FS.symlink(`/GeneralsZH_host/${entry}`, `/GeneralsZH/${entry}`);
        }
        FS.writeFile("/GeneralsZH/generalsx.big", fs.readFileSync(generalsxBigPath));
      } else {
        FS.mount(NODEFS, { root: zhDir }, "/GeneralsZH");
      }

      if (genDir) {
        FS.mkdirTree("/Generals");
        FS.mount(NODEFS, { root: genDir }, "/Generals");
      }

      FS.mkdirTree(`${vfsUserData}/Replays`);
      FS.mkdirTree(`${vfsUserData}/Maps`);
      if (userDataHost && fs.existsSync(path.join(userDataHost, "Maps"))) {
        FS.mount(NODEFS, { root: path.join(userDataHost, "Maps") }, `${vfsUserData}/Maps`);
      }
      FS.writeFile(vfsReplayPath, fs.readFileSync(replayAbs));
    },
  ],
  onExit(code) {
    cleanup();
    process.exit(finalizeExitCode(code));
  },
});

cleanup();
process.exit(finalizeExitCode(0));
