// Development-harness WebRTC transport for the engine's UDP abstraction.
// Production uses the equivalent typed bridge in web/apps/launcher.
//
// Wire frame (little-endian): srcPort:u16, destPort:u16, payload:bytes.
// Synthetic addresses are stable for a room session: slot 0 is 10.0.0.1.
// GeneralsX keeps its existing packet framing, ACK/resend, and lockstep logic.

(function () {
  "use strict";

  var BROADCAST = 0xffffffff;
  var MAX_INBOX_PACKETS = 1024;
  var MAX_BUFFERED_BYTES = 1024 * 1024;
  var COMPATIBILITY = Object.freeze({ engine: 1, protocol: 1, determinism: 1 });

  function slotIP(slot) {
    return ((10 << 24) | ((slot + 1) & 255)) >>> 0;
  }

  function ipString(ip) {
    return [(ip >>> 24) & 255, (ip >>> 16) & 255, (ip >>> 8) & 255, ip & 255].join(".");
  }

  function GeneralsXUdp() {
    this.base = "";
    this.room = "";
    this.name = "";
    this.slot = null;
    this.capacity = 0;
    this.roster = [];
    this.iceServers = [];
    this.peers = new Map();
    this.ipToSlot = new Map();
    this.inboxes = new Map();
    this.connected = false;
    this.connectionGeneration = 0;
    this.log = function (message) {
      if (window.__udplog) window.__udplog("[udp] " + message);
    };
  }

  GeneralsXUdp.prototype._resetPeers = function () {
    this.peers.forEach(function (peer) { peer.close(); });
    this.peers.clear();
    this.ipToSlot.clear();
    this.roster = [];
    this.slot = null;
    this.connected = false;
  };

  GeneralsXUdp.prototype._fetchIceServers = function (base) {
    return fetch(new URL("/turn-credentials", base).toString(), {
      credentials: "omit",
      mode: "cors",
    }).then(function (response) {
      if (!response.ok) throw new Error("TURN HTTP " + response.status);
      return response.json();
    }).then(function (body) {
      if (!body || !Array.isArray(body.iceServers)) {
        throw new Error("invalid TURN response");
      }
      return body.iceServers;
    }).catch(function (error) {
      // Direct ICE still works on permissive networks. The production launcher
      // surfaces TURN failures before starting an internet match.
      if (window.__udplog) window.__udplog("[udp] TURN unavailable; using direct ICE: " + error.message);
      return [];
    });
  };

  GeneralsXUdp.prototype.connect = function (base, room, name, capacity) {
    var self = this;
    var generation = ++this.connectionGeneration;
    this.base = String(base || "").replace(/\/+$/, "");
    this.room = String(room || "").trim().toUpperCase();
    this.name = String(name || "engine").slice(0, 24);
    this.capacity = capacity || 4;
    this._resetPeers();
    if (this.ws) {
      try { this.ws.close(); } catch (_) {}
    }

    return this._fetchIceServers(this.base).then(function (iceServers) {
      if (generation !== self.connectionGeneration) throw new Error("connection superseded");
      self.iceServers = iceServers;
      var url = new URL("/room", self.base);
      url.protocol = url.protocol === "https:" ? "wss:" : "ws:";
      url.searchParams.set("roomId", self.room);
      url.searchParams.set("capacity", String(self.capacity));

      return new Promise(function (resolve, reject) {
        var welcomed = false;
        var ws = new WebSocket(url.toString());
        self.ws = ws;
        ws.onopen = function () {
          self._send({
            type: "join",
            roomId: self.room,
            name: self.name,
            capacity: self.capacity,
            compatibility: COMPATIBILITY,
          });
        };
        ws.onmessage = function (event) {
          var message;
          try {
            message = JSON.parse(typeof event.data === "string" ? event.data : "");
          } catch (_) {
            self.log("ignored invalid signaling JSON");
            return;
          }
          self._onMessage(message);
          if (!welcomed && message.type === "welcome") {
            welcomed = true;
            resolve(message);
          } else if (!welcomed && message.type === "error") {
            reject(new Error(message.message || message.code || "signaling rejected"));
          }
        };
        ws.onerror = function () {
          self.log("signaling connection failed");
        };
        ws.onclose = function () {
          if (generation !== self.connectionGeneration) return;
          self.connected = false;
          if (!welcomed) reject(new Error("signaling closed before welcome"));
          self.log("signaling closed");
        };
      });
    });
  };

  GeneralsXUdp.prototype._send = function (message) {
    if (!this.ws || this.ws.readyState !== WebSocket.OPEN) {
      throw new Error("signaling socket is not open");
    }
    this.ws.send(JSON.stringify(message));
  };

  GeneralsXUdp.prototype._signalDescription = function (to, description) {
    this._send({ type: description.type, to: to, payload: description });
  };

  GeneralsXUdp.prototype._signalCandidate = function (to, candidate) {
    this._send({ type: "ice", to: to, payload: candidate });
  };

  GeneralsXUdp.prototype._onMessage = function (message) {
    if (!message || typeof message.type !== "string") return;
    switch (message.type) {
      case "welcome":
        this.slot = message.slot;
        this.capacity = message.capacity;
        this.connected = true;
        this._updateRoster(message.roster || []);
        this.log("joined " + message.roomId + " as slot " + this.slot);
        break;
      case "roster":
        this._updateRoster(message.roster || []);
        break;
      case "offer":
      case "answer":
        this._peer(message.from).onDescription(message.payload);
        break;
      case "ice":
        this._peer(message.from).onCandidate(message.payload);
        break;
      case "peer-left":
        this._removePeer(message.slot);
        break;
      case "error":
        this.log("signaling error " + (message.code || "") + ": " + (message.message || ""));
        break;
    }
  };

  GeneralsXUdp.prototype._updateRoster = function (roster) {
    var self = this;
    this.roster = roster.filter(function (entry) {
      return Number.isInteger(entry.slot) && entry.slot >= 0 && entry.slot < 255;
    });
    this.ipToSlot.clear();
    this.roster.forEach(function (entry) {
      self.ipToSlot.set(slotIP(entry.slot), entry.slot);
    });

    var live = new Set(this.roster.map(function (entry) { return entry.slot; }));
    this.roster.forEach(function (entry) {
      if (entry.slot !== self.slot && !self.peers.has(entry.slot)) self._peer(entry.slot);
    });
    Array.from(this.peers.keys()).forEach(function (slot) {
      if (!live.has(slot)) self._removePeer(slot);
    });
  };

  GeneralsXUdp.prototype._peer = function (slot) {
    var peer = this.peers.get(slot);
    if (!peer) {
      if (this.slot === null) throw new Error("peer signal arrived before welcome");
      peer = new PeerLink(this, slot);
      this.peers.set(slot, peer);
    }
    return peer;
  };

  GeneralsXUdp.prototype._removePeer = function (slot) {
    var peer = this.peers.get(slot);
    if (peer) peer.close();
    this.peers.delete(slot);
    this.ipToSlot.delete(slotIP(slot));
    this.roster = this.roster.filter(function (entry) { return entry.slot !== slot; });
  };

  GeneralsXUdp.prototype.bind = function (port) {
    if (!this.inboxes.has(port)) this.inboxes.set(port, []);
    return this.localIP();
  };

  GeneralsXUdp.prototype.send = function (destIP, destPort, srcPort, payload) {
    var frame = new Uint8Array(4 + payload.length);
    var view = new DataView(frame.buffer);
    view.setUint16(0, srcPort, true);
    view.setUint16(2, destPort, true);
    frame.set(payload, 4);

    if ((destIP >>> 0) === BROADCAST) {
      var sent = 0;
      this.peers.forEach(function (peer) {
        if (peer.send(frame)) sent += 1;
      });
      return sent;
    }

    var slot = this.ipToSlot.get(destIP >>> 0);
    var peer = slot === undefined ? null : this.peers.get(slot);
    return peer && peer.send(frame) ? 1 : 0;
  };

  GeneralsXUdp.prototype.recv = function (port) {
    var inbox = this.inboxes.get(port);
    return inbox && inbox.length ? inbox.shift() : null;
  };

  GeneralsXUdp.prototype.close = function (port) {
    this.inboxes.delete(port);
  };

  GeneralsXUdp.prototype.localIP = function () {
    return this.slot === null ? 0 : slotIP(this.slot);
  };

  GeneralsXUdp.prototype.hostIP = function () {
    var self = this;
    var host = this.roster.find(function (entry) {
      return entry.isHost && entry.slot !== self.slot;
    });
    if (!host) {
      var others = this.roster.filter(function (entry) { return entry.slot !== self.slot; });
      if (others.length === 1) host = others[0];
    }
    return host ? slotIP(host.slot) : 0;
  };

  GeneralsXUdp.prototype.joinRoom = function (code) {
    code = String(code || "").trim().toUpperCase();
    if (!/^[A-Z0-9]{4,10}$/.test(code) || code === this.room) return;
    return this.connect(this.base, code, this.name, this.capacity);
  };

  GeneralsXUdp.prototype.status = function () {
    var self = this;
    var host = this.roster.find(function (entry) { return entry.isHost; });
    var hostPeer = host && host.slot !== this.slot ? this.peers.get(host.slot) : null;
    return {
      room: this.room || "?",
      connected: this.connected,
      myIP: ipString(this.localIP()),
      myName: this.name || "you",
      peers: this.roster.length,
      isHost: !!(host && host.slot === this.slot),
      hostAlive: !!(host && (host.slot === this.slot || (hostPeer && hostPeer.isOpen()))),
      hostIP: ipString(this.hostIP()),
      ping: hostPeer && hostPeer.rtt !== null ? hostPeer.rtt : null,
      players: this.roster.map(function (entry) {
        var peer = self.peers.get(entry.slot);
        return {
          name: entry.name || "player",
          host: !!entry.isHost,
          isMe: entry.slot === self.slot,
          ping: entry.slot === self.slot ? 0 : (peer ? peer.rtt : null),
        };
      }),
    };
  };

  GeneralsXUdp.prototype._deliver = function (slot, data) {
    if (!(data instanceof ArrayBuffer) || data.byteLength < 4) return;
    var view = new DataView(data);
    var srcPort = view.getUint16(0, true);
    var destPort = view.getUint16(2, true);
    var inbox = this.inboxes.get(destPort);
    if (!inbox || inbox.length >= MAX_INBOX_PACKETS) return;
    inbox.push({
      ip: slotIP(slot),
      port: srcPort,
      data: new Uint8Array(data.slice(4)),
    });
  };

  function PeerLink(udp, slot) {
    var self = this;
    this.udp = udp;
    this.slot = slot;
    this.channel = null;
    this.rtt = null;
    this.makingOffer = false;
    this.ignoreOffer = false;
    this.polite = udp.slot > slot;
    this.pc = new RTCPeerConnection({
      iceServers: udp.iceServers,
      iceTransportPolicy: window.GENERALSX_ICE_TRANSPORT_POLICY === "relay" ? "relay" : "all",
    });

    this.pc.onicecandidate = function (event) {
      if (event.candidate) udp._signalCandidate(slot, event.candidate);
    };
    var negotiate = function () {
      if (self.makingOffer || self.pc.signalingState !== "stable") return;
      self.makingOffer = true;
      self.pc.createOffer().then(function(offer) {
        return self.pc.setLocalDescription(offer);
      }).then(function () {
        udp._signalDescription(slot, self.pc.localDescription);
      }).catch(function (error) {
        udp.log("negotiation failed: " + error);
      }).finally(function () {
        self.makingOffer = false;
      });
    };
    this.pc.onnegotiationneeded = negotiate;
    this.pc.ondatachannel = function (event) { self._bind(event.channel); };
    this.pc.onconnectionstatechange = function () {
      if (self.pc.connectionState === "connected") udp.log("peer " + slot + " connected");
    };
    if (udp.slot < slot) {
      this._bind(this.pc.createDataChannel("generalsx-udp", {
        ordered: false,
        maxRetransmits: 5,
      }));
      // Safari/WebKit can omit negotiationneeded for a newly-created channel.
      // The guard above keeps this explicit fallback idempotent elsewhere.
      Promise.resolve().then(negotiate);
    }
  }

  PeerLink.prototype._bind = function (channel) {
    var self = this;
    this.channel = channel;
    channel.binaryType = "arraybuffer";
    channel.bufferedAmountLowThreshold = MAX_BUFFERED_BYTES / 2;
    channel.onmessage = function (event) { self.udp._deliver(self.slot, event.data); };
  };

  PeerLink.prototype.isOpen = function () {
    return !!this.channel && this.channel.readyState === "open";
  };

  PeerLink.prototype.send = function (frame) {
    if (!this.isOpen() || this.channel.bufferedAmount > MAX_BUFFERED_BYTES) return false;
    this.channel.send(frame);
    return true;
  };

  PeerLink.prototype.onDescription = function (description) {
    var self = this;
    if (!description || (description.type !== "offer" && description.type !== "answer")) return;
    var collision = description.type === "offer" &&
      (this.makingOffer || this.pc.signalingState !== "stable");
    this.ignoreOffer = !this.polite && collision;
    if (this.ignoreOffer) return;
    this.pc.setRemoteDescription(description).then(function () {
      if (description.type === "offer") {
        return self.pc.setLocalDescription().then(function () {
          self.udp._signalDescription(self.slot, self.pc.localDescription);
        });
      }
    }).catch(function (error) {
      self.udp.log("description failed: " + error);
    });
  };

  PeerLink.prototype.onCandidate = function (candidate) {
    var self = this;
    if (!candidate) return;
    this.pc.addIceCandidate(candidate).catch(function (error) {
      if (!self.ignoreOffer) self.udp.log("ICE candidate failed: " + error);
    });
  };

  PeerLink.prototype.close = function () {
    try { if (this.channel) this.channel.close(); } catch (_) {}
    try { this.pc.close(); } catch (_) {}
  };

  var udp = new GeneralsXUdp();
  window.GeneralsXUdp = udp;
  if (window.GENERALSX_WEBRTC_ENABLED) {
    window.GENERALSX_UDP_READY = udp.connect(
      window.GENERALSX_SIGNALING_URL || "https://signaling.generalsx.org",
      window.GENERALSX_ROOM || "LAN1",
      window.GENERALSX_PLAYER_NAME || ("engine-" + Math.floor(Math.random() * 10000)),
      4
    ).then(function () {
      udp.log("ready");
    }).catch(function (error) {
      udp.log("connection failed: " + error.message);
      throw error;
    });
  }
})();
