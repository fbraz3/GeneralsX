import { chromium } from "playwright";

const [
  mode = "direct",
  room = "TEST1",
  staticOrigin = "http://127.0.0.1:8765",
  signalingOrigin = "http://127.0.0.1:8787",
  engine = "",
  protocol = "",
  determinism = "",
  contentMismatchEngine = "",
  determinismMismatchVersion = "",
] = process.argv.slice(2);
if (mode !== "direct" && mode !== "turn") throw new Error(`unsupported mode: ${mode}`);
const compatibility = {
  engine: Number(engine),
  protocol: Number(protocol),
  determinism: Number(determinism),
};
if (Object.values(compatibility).some((value) => !Number.isInteger(value) || value <= 0)) {
  throw new Error("CMake-generated engine, protocol, and determinism compatibility values are required");
}
if (
  !Number.isInteger(Number(contentMismatchEngine)) ||
  Number(contentMismatchEngine) <= 0 ||
  Number(contentMismatchEngine) === compatibility.engine ||
  !Number.isInteger(Number(determinismMismatchVersion)) ||
  Number(determinismMismatchVersion) <= 0 ||
  Number(determinismMismatchVersion) === compatibility.determinism
) {
  throw new Error("distinct content and determinism mismatch versions are required");
}

const browser = await chromium.launch({ headless: true });
const page = await browser.newPage();
const diagnostics = [];
page.on("console", (message) => diagnostics.push(`${message.type()}: ${message.text()}`));
page.on("pageerror", (error) => diagnostics.push(`pageerror: ${error.stack ?? error}`));

try {
  const url = new URL("/wasm/native_browser_interop.html", staticOrigin);
  url.searchParams.set("mode", mode);
  url.searchParams.set("room", room);
  url.searchParams.set("signaling", signalingOrigin);
  url.searchParams.set("engine", String(compatibility.engine));
  url.searchParams.set("protocol", String(compatibility.protocol));
  url.searchParams.set("determinism", String(compatibility.determinism));
  await page.goto(url.toString());
  await page.waitForFunction(
    () => window.__interop?.status === "passed" || window.__interop?.status === "failed",
    undefined,
    { timeout: 55_000 },
  );

  const result = await page.evaluate(() => ({ interop: window.__interop, candidates: window.__interop.candidates ?? null }));

  if (result.interop.status !== "passed") {
    throw new Error(result.interop.error ?? "browser interoperability probe failed");
  }
  const selectedCandidateTypes = [result.candidates?.local, result.candidates?.remote].filter(Boolean);
  if (selectedCandidateTypes.length === 0) throw new Error("selected ICE candidate stats were unavailable");
  const usesRelay = selectedCandidateTypes.includes("relay");
  if (mode === "turn" && !usesRelay) throw new Error(`TURN mode selected non-relay candidates: ${JSON.stringify(result.candidates)}`);
  if (mode === "direct" && usesRelay) throw new Error(`direct mode unexpectedly selected TURN: ${JSON.stringify(result.candidates)}`);

  const rejectProfile = (mismatchedCompatibility) => page.evaluate(
    ({ signalingOrigin, room, mismatchedCompatibility }) =>
      new Promise((resolve, reject) => {
        const url = new URL("/room", signalingOrigin);
        url.protocol = "ws:";
        url.searchParams.set("roomId", room);
        url.searchParams.set("capacity", "2");
        const socket = new WebSocket(url);
        const timeout = setTimeout(() => reject(new Error("timed out waiting for compatibility rejection")), 5000);
        socket.onopen = () => {
          socket.send(
            JSON.stringify({
              type: "join",
              roomId: room,
              name: "mismatched-browser",
              capacity: 2,
              compatibility: mismatchedCompatibility,
            }),
          );
        };
        socket.onmessage = (event) => {
          const message = JSON.parse(event.data);
          if (message.type !== "error") return;
          clearTimeout(timeout);
          socket.close();
          resolve(message.code);
        };
        socket.onerror = () => {
          clearTimeout(timeout);
          reject(new Error("mismatch WebSocket failed before returning an error"));
        };
      }),
    { signalingOrigin, room, mismatchedCompatibility },
  );
  const contentMismatchCode = await rejectProfile({
    ...compatibility,
    engine: Number(contentMismatchEngine),
  });
  const determinismMismatchCode = await rejectProfile({
    ...compatibility,
    determinism: Number(determinismMismatchVersion),
  });
  if (contentMismatchCode !== "INCOMPATIBLE_CLIENT" || determinismMismatchCode !== "INCOMPATIBLE_CLIENT") {
    throw new Error(
      `expected content/math INCOMPATIBLE_CLIENT, received ${String(contentMismatchCode)}/${String(determinismMismatchCode)}`,
    );
  }

  process.stdout.write(
    `${JSON.stringify({
      mode,
      status: "passed",
      candidates: result.candidates,
      compatibility,
      contentMismatchCode,
      determinismMismatchCode,
      browserToNativeHeader: result.interop.browserToNativeHeader,
      nativeToBrowserHeader: result.interop.nativeToBrowserHeader,
    })}\n`,
  );
} catch (error) {
  const state = await page.evaluate(() => window.__interop ?? null).catch(() => null);
  throw new Error(`${error.stack ?? error}\nstate=${JSON.stringify(state)}\n${diagnostics.join("\n")}`, { cause: error });
} finally {
  await browser.close();
}
