/*! coi-serviceworker — Enables SharedArrayBuffer on static hosts without custom COOP/COEP headers (e.g. GitHub Pages). */
if (typeof window === "undefined") {
  self.addEventListener("install", () => self.skipWaiting());
  self.addEventListener("activate", (event) => event.waitUntil(self.clients.claim()));

  self.addEventListener("fetch", (event) => {
    const request = event.request;
    if (request.cache === "only-if-cached" && request.mode !== "same-origin") {
      return;
    }

    event.respondWith(
      fetch(request).then((response) => {
        if (response.status === 0) {
          return response;
        }
        const headers = new Headers(response.headers);
        headers.set("Cross-Origin-Embedder-Policy", "require-corp");
        headers.set("Cross-Origin-Opener-Policy", "same-origin");
        headers.set("Cross-Origin-Resource-Policy", "cross-origin");

        return new Response(response.body, {
          status: response.status,
          statusText: response.statusText,
          headers,
        });
      }).catch((err) => {
        console.error("[coi-serviceworker] fetch failed:", err);
        throw err;
      })
    );
  });
} else if (!window.crossOriginIsolated && window.isSecureContext && "serviceWorker" in navigator) {
  const reloadedKey = "gx.coiReloaded";
  navigator.serviceWorker.register("./coi-serviceworker.js").then((registration) => {
    registration.addEventListener("updatefound", () => {
      const installing = registration.installing;
      if (!installing) return;
      installing.addEventListener("statechange", () => {
        if (installing.state === "activated" && !sessionStorage.getItem(reloadedKey)) {
          sessionStorage.setItem(reloadedKey, "1");
          window.location.reload();
        }
      });
    });

    if (registration.active && !navigator.serviceWorker.controller && !sessionStorage.getItem(reloadedKey)) {
      sessionStorage.setItem(reloadedKey, "1");
      window.location.reload();
    }
  }).catch((err) => {
    console.warn("[coi-serviceworker] registration failed:", err);
  });
}
