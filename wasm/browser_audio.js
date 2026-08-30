/*
 * GeneralsX browser audio and archive bridge.
 *
 * Derived from the WebAssembly audio work in meerzulee/GeneralsXWeb commits
 * d10226a87, a34e2b088, 19ff0d706, 0f50bbad0, ea3434263, b4d487cca,
 * 2b2f3b636, and 4e5e16abf.
 */
(function() {
  'use strict';

  var unlocked = false;
  var recoveryInstalled = false;
  var recoveryPromise = null;
  var monitoredContexts = new WeakSet();
  var listeners = new Set();
  var workerMounts = new Set();

  function callExport(name) {
    var fn = Module[name];
    return typeof fn === 'function' ? fn() : 0;
  }

  function devices() {
    if (typeof window === 'undefined' || !window.miniaudio || !window.miniaudio.devices) {
      return [];
    }
    return window.miniaudio.devices.filter(function(device) {
      return device && device.webaudio;
    });
  }

  function snapshot() {
    var contexts = devices().map(function(device) {
      return device.webaudio.state || 'unknown';
    });
    return {
      supported: typeof window !== 'undefined' &&
        typeof (window.AudioContext || window.webkitAudioContext) === 'function',
      unlocked: unlocked,
      engineState: callExport('_generalsx_audio_device_state'),
      contextStates: contexts,
      running: contexts.length > 0 && contexts.every(function(state) {
        return state === 'running';
      })
    };
  }

  function publish() {
    var state = snapshot();
    listeners.forEach(function(listener) {
      listener(state);
    });
    return state;
  }

  function monitor(device) {
    var context = device.webaudio;
    if (!context || monitoredContexts.has(context)) return;
    monitoredContexts.add(context);
    if (typeof context.addEventListener === 'function') {
      context.addEventListener('statechange', function() {
        publish();
        if (!unlocked) return;
        if (context.state === 'closed') {
          void recover();
        } else if (context.state === 'suspended' &&
                   typeof document !== 'undefined' &&
                   document.visibilityState === 'visible') {
          void resume();
        }
      });
    }
  }

  async function resume() {
    callExport('_generalsx_audio_resume_device');
    var currentDevices = devices();
    currentDevices.forEach(monitor);

    var closed = false;
    await Promise.all(currentDevices.map(function(device) {
      var context = device.webaudio;
      if (context.state === 'closed') {
        closed = true;
        return Promise.resolve();
      }
      if (context.state === 'running') {
        return Promise.resolve();
      }
      return context.resume().catch(function(error) {
        console.warn('GeneralsX audio resume failed:', error);
      });
    }));

    if (closed) {
      void recover();
    }
    return publish();
  }

  function recover() {
    if (recoveryPromise) return recoveryPromise;
    var previousContexts = devices().map(function(device) { return device.webaudio; });
    callExport('_generalsx_audio_recover_device');
    recoveryPromise = (async function() {
      for (var attempt = 0; attempt < 200; ++attempt) {
        await new Promise(function(resolve) { setTimeout(resolve, 25); });
        var replacement = devices().some(function(device) {
          return previousContexts.indexOf(device.webaudio) === -1 &&
            device.webaudio.state !== 'closed';
        });
        if (replacement) return resume();
      }
      return publish();
    })().finally(function() {
      recoveryPromise = null;
    });
    return recoveryPromise;
  }

  function installRecovery() {
    if (recoveryInstalled || typeof window === 'undefined') return;
    recoveryInstalled = true;

    var resumeWhenVisible = function() {
      if (!unlocked) return;
      if (typeof document === 'undefined' || document.visibilityState === 'visible') {
        void resume();
      }
    };
    window.addEventListener('focus', resumeWhenVisible);
    window.addEventListener('pageshow', resumeWhenVisible);
    if (typeof document !== 'undefined') {
      document.addEventListener('visibilitychange', resumeWhenVisible);
    }
    if (typeof navigator !== 'undefined' && navigator.mediaDevices &&
        typeof navigator.mediaDevices.addEventListener === 'function') {
      navigator.mediaDevices.addEventListener('devicechange', function() {
        if (!unlocked) return;
        void recover();
      });
    }
  }

  async function unlock() {
    unlocked = true;
    installRecovery();
    return resume();
  }

  function bindUserGesture(element) {
    if (!element || typeof element.addEventListener !== 'function') {
      throw new TypeError('A launcher element is required to unlock audio');
    }

    var active = true;
    var handler = function(event) {
      if (!active) return;
      if (event.type === 'keydown' && event.key !== 'Enter' && event.key !== ' ') return;
      void unlock().then(function(state) {
        if (state.running) unbind();
      });
    };
    function unbind() {
      if (!active) return;
      active = false;
      element.removeEventListener('pointerdown', handler);
      element.removeEventListener('keydown', handler);
    }

    element.addEventListener('pointerdown', handler);
    element.addEventListener('keydown', handler);
    return unbind;
  }

  function isBlob(value) {
    return value && typeof value.arrayBuffer === 'function' &&
      typeof value.slice === 'function' && typeof value.size === 'number';
  }

  async function normalizeArchive(entry) {
    if (typeof entry === 'string') {
      var response = await fetch(entry);
      if (!response.ok) throw new Error(entry + ': HTTP ' + response.status);
      return {
        name: decodeURIComponent(entry.split('/').pop().split('?')[0]),
        data: await response.blob()
      };
    }
    if (isBlob(entry) && entry.name) {
      return { name: entry.name, data: entry };
    }
    if (entry && entry.name && isBlob(entry.data)) {
      return { name: entry.name, data: entry.data };
    }
    if (entry && entry.name && entry.url) {
      var archiveResponse = await fetch(entry.url);
      if (!archiveResponse.ok) throw new Error(entry.url + ': HTTP ' + archiveResponse.status);
      return { name: entry.name, data: await archiveResponse.blob() };
    }
    throw new TypeError('Audio archives must be File objects, URLs, or {name, data/url} entries');
  }

  async function mountArchiveSet(mountPoint, archives) {
    if (!archives || archives.length === 0) return;
    var entries = await Promise.all(Array.from(archives, normalizeArchive));
    FS.mkdirTree(mountPoint);

    var canUseWorkerFS = typeof WORKERFS !== 'undefined' &&
      typeof FileReaderSync !== 'undefined';
    if (canUseWorkerFS) {
      if (workerMounts.has(mountPoint)) FS.unmount(mountPoint);
      FS.mount(WORKERFS, { blobs: entries }, mountPoint);
      workerMounts.add(mountPoint);
      return;
    }

    for (var i = 0; i < entries.length; ++i) {
      var bytes = new Uint8Array(await entries[i].data.arrayBuffer());
      FS.writeFile(mountPoint + '/' + entries[i].name, bytes, { canOwn: true });
    }
  }

  async function mountArchives(options) {
    options = options || {};
    await Promise.all([
      mountArchiveSet('/game-audio', options.zeroHour || []),
      mountArchiveSet('/game-base-audio', options.generals || [])
    ]);

    for (var attempt = 0; attempt < 200; ++attempt) {
      if (callExport('_generalsx_mount_audio')) {
        return true;
      }
      await new Promise(function(resolve) { setTimeout(resolve, 25); });
    }
    throw new Error('The GeneralsX archive filesystem did not become ready');
  }

  Module.generalsxAudio = {
    unlock: unlock,
    resume: resume,
    recover: recover,
    bindUserGesture: bindUserGesture,
    mountArchives: mountArchives,
    getState: snapshot,
    subscribe: function(listener) {
      listeners.add(listener);
      listener(snapshot());
      return function() { listeners.delete(listener); };
    }
  };
})();
