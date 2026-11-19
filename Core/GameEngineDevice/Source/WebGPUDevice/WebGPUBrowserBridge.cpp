// GeneralsX @refactor Meeseeks 17/11/2026 Isolate Emscripten browser LAN relay and runtime lifecycle hooks in Core/GameEngineDevice.

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>

extern "C" {

EM_JS(unsigned int, GeneralsXWebLanVirtualIP, (), {
	if (!globalThis.generalsXLanVirtualIP) {
		if (typeof globalThis.generalsXLanHostIP === "number" && globalThis.generalsXLanHostIP !== 0) {
			globalThis.generalsXLanVirtualIP = globalThis.generalsXLanHostIP >>> 0;
		} else if (!globalThis.location || !globalThis.sessionStorage) {
			globalThis.generalsXLanVirtualIP = 0x0A000001 >>> 0;
		} else {
			const queryValue = new URLSearchParams(globalThis.location.search).get("lanClient");
			let clientId = /^(?:[1-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-4])$/.test(queryValue || "")
				? Number(queryValue)
				: 0;
			if (!clientId) {
				const storageKey = "generalsX.lanClient";
				clientId = Number(globalThis.sessionStorage.getItem(storageKey)) || 0;
				if (!clientId) {
					const random = new Uint8Array(1);
					globalThis.crypto.getRandomValues(random);
					clientId = (random[0] % 254) + 1;
					globalThis.sessionStorage.setItem(storageKey, String(clientId));
				}
			}
			globalThis.generalsXLanVirtualIP = (0x0A000000 | clientId) >>> 0;
		}
	}
	return globalThis.generalsXLanVirtualIP;
});

EM_JS(int, GeneralsXWebLanBind, (unsigned int ip, unsigned short port), {
	const virtualIP = ip || _GeneralsXWebLanVirtualIP();
	const sockets = globalThis.generalsXLanSockets ||= new Map();
	const existing = sockets.get(port);
	if (existing) {
		existing.socket.close();
		sockets.delete(port);
	}

	const relayUrl = (globalThis.location.protocol === "https:" ? "wss://" : "ws://")
		+ globalThis.location.host + "/GeneralsXLan";
	const socket = new WebSocket(relayUrl);
	socket.binaryType = "arraybuffer";
	const state = { socket, virtualIP, port, incoming: [], outgoing: [], incomingBytes: 0 };
	sockets.set(port, state);

	const encodeRegistration = () => {
		const message = new ArrayBuffer(7);
		const view = new DataView(message);
		view.setUint8(0, 1);
		view.setUint32(1, virtualIP, false);
		view.setUint16(5, port, false);
		return message;
	};

	socket.addEventListener("open", () => {
		socket.send(encodeRegistration());
		for (const message of state.outgoing) socket.send(message);
		state.outgoing.length = 0;
		globalThis.generalsXLanStatus = { connected: true, ip: virtualIP, port };
	});
	const onRelayMessage = (event) => {
		const bytes = new Uint8Array(event.data);
		if (bytes.length === 5 && bytes[0] === 3) {
			const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
			const assignedIP = view.getUint32(1, false) >>> 0;
			if (assignedIP !== 0) {
				globalThis.generalsXLanVirtualIP = assignedIP;
				state.virtualIP = assignedIP;
			}
			return;
		}
		if (bytes.length < 7 || bytes[0] !== 2) return;
		const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
		const payload = bytes.slice(7);
		state.incoming.push({
			ip: view.getUint32(1, false),
			port: view.getUint16(5, false),
			payload
		});
		state.incomingBytes += payload.byteLength;
		while (state.incoming.length > 16384 || state.incomingBytes > 32 * 1024 * 1024) {
			state.incomingBytes -= state.incoming.shift().payload.byteLength;
		}
	};
	socket.addEventListener("message", onRelayMessage);
	socket.addEventListener("close", () => {
		if (sockets.get(port) !== state) return;
		globalThis.generalsXLanStatus = { connected: false, ip: state.virtualIP, port };
	});
	return 0;
});

EM_JS(void, GeneralsXWebLanClose, (unsigned short port), {
	const sockets = globalThis.generalsXLanSockets;
	const state = sockets?.get(port);
	if (!state) return;
	sockets.delete(port);
	state.socket.close();
});

EM_JS(int, GeneralsXWebLanWrite,
	(const unsigned char *message, unsigned int length, unsigned int ip, unsigned short port, unsigned short localPort), {
	const state = globalThis.generalsXLanSockets?.get(localPort);
	if (!state) return -1;
	const frame = new Uint8Array(7 + length);
	const view = new DataView(frame.buffer);
	view.setUint8(0, 2);
	view.setUint32(1, ip, false);
	view.setUint16(5, port, false);
	frame.set(HEAPU8.subarray(message, message + length), 7);
	if (state.socket.readyState === WebSocket.OPEN) {
		state.socket.send(frame);
	} else if (state.outgoing.length < 256) {
		state.outgoing.push(frame);
	} else {
		return -1;
	}
	return length;
});

EM_JS(int, GeneralsXWebLanRead,
	(unsigned char *message, unsigned int capacity, unsigned short localPort, unsigned int *sourceIP, unsigned short *sourcePort), {
	const state = globalThis.generalsXLanSockets?.get(localPort);
	if (!state || state.incoming.length === 0) return 0;
	const packet = state.incoming.shift();
	state.incomingBytes -= packet.payload.byteLength;
	const length = Math.min(capacity, packet.payload.byteLength);
	HEAPU8.set(packet.payload.subarray(0, length), message);
	HEAPU32[sourceIP >>> 2] = packet.ip;
	HEAPU16[sourcePort >>> 1] = packet.port;
	return length;
});

void GeneralsXBrowserYield(unsigned int ms)
{
	const int isBrowser = EM_ASM_INT({ return typeof window !== "undefined" ? 1 : 0; });
	if (isBrowser) {
		emscripten_sleep(ms);
	}
}

void GeneralsXBrowserSuspendAudio(void)
{
	EM_ASM({
		if (typeof window !== "undefined" && window.miniaudio) {
			window.miniaudio.devices.forEach(function(d) {
				if (d && d.webaudio && d.webaudio.state === "running") d.webaudio.suspend();
			});
		}
	});
}

void GeneralsXBrowserResumeAudio(void)
{
	EM_ASM({
		if (typeof window !== "undefined" && window.miniaudio) {
			window.miniaudio.devices.forEach(function(d) {
				if (d && d.webaudio) d.webaudio.resume();
			});
		}
	});
}

int GeneralsXBrowserShouldHoldMatch(void)
{
	return EM_ASM_INT({ return globalThis.__gxHoldMatch ? 1 : 0; });
}

} // extern "C"
#endif
