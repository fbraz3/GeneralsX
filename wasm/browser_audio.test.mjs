import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import vm from 'node:vm';

const source = await readFile(new URL('./browser_audio.js', import.meta.url), 'utf8');

function makeEventTarget() {
  const handlers = new Map();
  return {
    handlers,
    addEventListener(type, handler) {
      handlers.set(type, handler);
    },
    removeEventListener(type) {
      handlers.delete(type);
    },
  };
}

const contextEvents = makeEventTarget();
const audioContext = {
  state: 'suspended',
  resumeCount: 0,
  async resume() {
    this.resumeCount += 1;
    this.state = 'running';
  },
  ...contextEvents,
};
const windowEvents = makeEventTarget();
const documentEvents = makeEventTarget();
const writes = [];
let resumeCalls = 0;
let recoveryCalls = 0;

const sandbox = {
  Module: {
    _generalsx_audio_resume_device() {
      resumeCalls += 1;
      return 1;
    },
    _generalsx_audio_recover_device() {
      recoveryCalls += 1;
      return 1;
    },
    _generalsx_audio_device_state() {
      return 2;
    },
    _generalsx_mount_audio() {
      return 1;
    },
  },
  window: {
    AudioContext: function AudioContext() {},
    miniaudio: { devices: [{ webaudio: audioContext }] },
    ...windowEvents,
  },
  document: {
    visibilityState: 'visible',
    ...documentEvents,
  },
  navigator: {},
  FS: {
    mkdirTree() {},
    writeFile(path, bytes, options) {
      writes.push({ path, bytes: [...bytes], options });
    },
  },
  Blob,
  WeakSet,
  Set,
  Promise,
  Array,
  Uint8Array,
  TypeError,
  Error,
  console,
  setTimeout,
};

vm.runInNewContext(source, sandbox, { filename: 'browser_audio.js' });

const unlocked = await sandbox.Module.generalsxAudio.unlock();
assert.equal(resumeCalls, 1);
assert.equal(audioContext.resumeCount, 1);
assert.equal(unlocked.running, true);

const button = makeEventTarget();
audioContext.state = 'suspended';
const unbind = sandbox.Module.generalsxAudio.bindUserGesture(button);
button.handlers.get('pointerdown')({ type: 'pointerdown' });
await new Promise((resolve) => setTimeout(resolve, 0));
assert.equal(audioContext.resumeCount, 2);
unbind();

audioContext.state = 'closed';
const replacementEvents = makeEventTarget();
const replacementContext = {
  state: 'suspended',
  resumeCount: 0,
  async resume() {
    this.resumeCount += 1;
    this.state = 'running';
  },
  ...replacementEvents,
};
sandbox.Module._generalsx_audio_recover_device = () => {
  recoveryCalls += 1;
  sandbox.window.miniaudio.devices = [{ webaudio: replacementContext }];
  return 1;
};
audioContext.handlers.get('statechange')();
await new Promise((resolve) => setTimeout(resolve, 35));
assert.equal(recoveryCalls, 1);
assert.equal(replacementContext.resumeCount, 1);

await sandbox.Module.generalsxAudio.mountArchives({
  zeroHour: [{ name: 'AudioZH.big', data: new Blob([new Uint8Array([1, 2, 3])]) }],
});
assert.equal(writes.length, 1);
assert.equal(writes[0].path, '/game-audio/AudioZH.big');
assert.deepEqual(writes[0].bytes, [1, 2, 3]);
assert.equal(writes[0].options.canOwn, true);

console.log('browser_audio bridge probes passed');
