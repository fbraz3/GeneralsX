import { mkdir, readFile, writeFile } from "node:fs/promises";
import { resolve } from "node:path";
import { chromium } from "playwright";

const [
  harnessOrigin = "http://127.0.0.1:8765",
  signalingOrigin = "http://127.0.0.1:18787",
  room = "LOBBY1",
  runSecondsText = "15",
] =
  process.argv.slice(2);
const runSeconds = Number(runSecondsText);
if (!Number.isFinite(runSeconds) || runSeconds < 0 || runSeconds > 1800) {
  throw new Error("runSeconds must be between 0 and 1800");
}
const root = resolve(import.meta.dirname, "../../../..");
const outputDirectory = resolve(root, "logs/wasm-determinism/browser-lobby");
const bridge = await readFile(resolve(root, "wasm/webrtc_udp.js"), "utf8");
await mkdir(outputDirectory, { recursive: true });

const browser = await chromium.launch({
  headless: true,
  args: [
    "--disable-background-timer-throttling",
    "--disable-backgrounding-occluded-windows",
    "--disable-renderer-backgrounding",
  ],
});
const context = await browser.newContext({ viewport: { width: 1280, height: 900 } });
const pages = await Promise.all([context.newPage(), context.newPage()]);
const diagnostics = [[], []];

async function collectPeerStats(page) {
  return page.evaluate(async () => {
    const totals = { messagesSent: 0, messagesReceived: 0, bytesSent: 0, bytesReceived: 0 };
    const peers = window.GeneralsXUdp?.peers;
    if (!peers) return totals;
    for (const peer of peers.values()) {
      const reports = await peer.pc.getStats();
      reports.forEach((report) => {
        if (report.type !== "data-channel") return;
        totals.messagesSent += report.messagesSent ?? 0;
        totals.messagesReceived += report.messagesReceived ?? 0;
        totals.bytesSent += report.bytesSent ?? 0;
        totals.bytesReceived += report.bytesReceived ?? 0;
      });
    }
    return totals;
  });
}

try {
  for (const [index, page] of pages.entries()) {
    page.on("console", (message) => diagnostics[index].push(`${message.type()}: ${message.text()}`));
    page.on("pageerror", (error) => diagnostics[index].push(`pageerror: ${error.stack ?? error}`));
    await page.addInitScript(() => {
      window.__gxLobbyReached = false;
      window.__gxMatchLoadBegin = null;
      window.__gxMatchLoadEnd = false;
      let module;
      Object.defineProperty(window, "Module", {
        configurable: true,
        get: () => module,
        set: (value) => {
          value.onLobbyReached = () => {
            window.__gxLobbyReached = true;
          };
          value.onMatchLoadBegin = (mapName) => {
            window.__gxMatchLoadBegin = mapName;
          };
          value.onMatchLoadEnd = () => {
            window.__gxMatchLoadEnd = true;
          };
          module = value;
        },
      });
    });
    await page.route("**/webrtc_udp.js*", (route) =>
      route.fulfill({ status: 200, contentType: "text/javascript", body: bridge }),
    );

    const url = new URL("/boot.html", harnessOrigin);
    url.searchParams.set("signaling", signalingOrigin);
    url.searchParams.set("room", room);
    url.searchParams.set("player", index === 0 ? "browser-host" : "browser-guest");
    url.searchParams.set(index === 0 ? "host" : "autojoin", "1");
    url.searchParams.set("args", "-noshellmap,-noaudio");
    await page.goto(url.toString(), { waitUntil: "domcontentloaded" });
  }

  await Promise.all(
    pages.map((page) =>
      page.waitForFunction(() => window.__gxLobbyReached === true, undefined, {
        timeout: 180_000,
      }),
    ),
  );
  async function clickGameOptionButton(page) {
    const canvas = page.locator("#canvas");
    const box = await canvas.boundingBox();
    if (!box) throw new Error("game canvas is not visible");
    await page.mouse.click(box.x + 208, box.y + 684);
  }

  await clickGameOptionButton(pages[1]);
  await pages[0].waitForTimeout(1500);
  await clickGameOptionButton(pages[0]);
  await Promise.all(
    pages.map((page) =>
      page.waitForFunction(() => window.__gxMatchLoadBegin !== null, undefined, {
        timeout: 60_000,
      }),
    ),
  );
  await Promise.all(
    pages.map((page) =>
      page.waitForFunction(() => window.__gxMatchLoadEnd === true, undefined, {
        timeout: 180_000,
      }),
    ),
  );
  const statsAfterLoad = await Promise.all(pages.map(collectPeerStats));
  await Promise.all(
    pages.map((page, index) =>
      page.screenshot({
        path: resolve(outputDirectory, index === 0 ? "host-loaded.png" : "guest-loaded.png"),
        fullPage: true,
      }),
    ),
  );
  await pages[0].waitForTimeout(runSeconds * 1000);
  const statsAfterWait = await Promise.all(pages.map(collectPeerStats));

  const statuses = await Promise.all(
    pages.map(async (page, index) => ({
      ...(await page.evaluate(() => ({
        lobbyReached: window.__gxLobbyReached,
        matchLoadBegin: window.__gxMatchLoadBegin,
        matchLoadEnd: window.__gxMatchLoadEnd,
        udp: window.GeneralsXUdp?.status() ?? null,
      }))),
      dataChannelAfterLoad: statsAfterLoad[index],
      dataChannelAfterWait: statsAfterWait[index],
    })),
  );
  await Promise.all(
    pages.map((page, index) =>
      page.screenshot({ path: resolve(outputDirectory, index === 0 ? "host.png" : "guest.png"), fullPage: true }),
    ),
  );
  await writeFile(resolve(outputDirectory, "browser.log"), diagnostics.map((entries) => entries.join("\n")).join("\n--- peer ---\n"));
  const failedPeer = statuses.find(
    (peer) =>
      !peer.udp?.connected ||
      !peer.udp.hostAlive ||
      (runSeconds > 0 &&
        (peer.dataChannelAfterWait.messagesSent <= peer.dataChannelAfterLoad.messagesSent ||
          peer.dataChannelAfterWait.messagesReceived <= peer.dataChannelAfterLoad.messagesReceived)),
  );
  if (failedPeer) {
    throw new Error(`match transport degraded during the post-load interval: ${JSON.stringify(failedPeer)}`);
  }
  process.stdout.write(
    `${JSON.stringify({ status: "passed", room, postLoadWaitSeconds: runSeconds, peers: statuses })}\n`,
  );
} catch (error) {
  await writeFile(resolve(outputDirectory, "browser.log"), diagnostics.map((entries) => entries.join("\n")).join("\n--- peer ---\n"));
  const states = await Promise.all(
    pages.map((page) =>
      page
        .evaluate(() => ({
          lobbyReached: window.__gxLobbyReached ?? false,
          matchLoadBegin: window.__gxMatchLoadBegin ?? null,
          matchLoadEnd: window.__gxMatchLoadEnd ?? false,
          udp: window.GeneralsXUdp?.status() ?? null,
          log: document.querySelector("#log")?.textContent?.slice(-5000) ?? "",
        }))
        .catch(() => null),
    ),
  );
  throw new Error(`${error.stack ?? error}\nstates=${JSON.stringify(states)}`, { cause: error });
} finally {
  await browser.close();
}
