# Test Replays

The GeneralsReplays folder contains replays and the required maps that are tested in CI to ensure that the game is retail compatible.

You can also test with these replays locally:
- Copy the replays into a subfolder in your `%USERPROFILE%/Documents/Command and Conquer Generals Zero Hour Data/Replays` folder.
- Copy the maps into `%USERPROFILE%/Documents/Command and Conquer Generals Zero Hour Data/Maps`
- Start the test with this: (copy into a .bat file next to your executable)
```
START /B /W generalszh.exe -jobs 4 -headless -replay subfolder/*.rep > replay_check.log
echo %errorlevel%
PAUSE
```
It will run the game in the background and check that each replay is compatible. You need to use a VC6 build with optimizations and RTS_BUILD_OPTION_DEBUG = OFF, otherwise the game won't be compatible.

## Native/browser WebRTC interoperability

Configure a native WebRTC-enabled build and compile the integration probe:

```bash
VCPKG_ROOT=/Users/you/vcpkg cmake --preset macos-vulkan \
  -B build/macos-webrtc-determinism \
  -DSAGE_USE_NATIVE_WEBRTC=ON \
  -DSAGE_USE_DETERMINISTIC_MATH=ON
cmake --build build/macos-webrtc-determinism \
  --target core_native_webrtc_integration_probe
cd web && npm ci && cd ..
scripts/qa/smoke/native-browser-webrtc-interop.sh \
  direct build/macos-webrtc-determinism
```

This runs the real native and browser WebRTC implementations through a local
Worker, exchanges the canonical four-byte UDP header in both directions, and
verifies that a mismatched compatibility handshake is rejected before
lockstep. Set `TURN_KEY_ID` and `TURN_KEY_API_TOKEN` and use `turn` instead of
`direct` for relay-only validation.

With an authorized engine-asset harness and local Worker already running, the
two-browser lobby/match probe is:

```bash
node web/apps/launcher/e2e/authorized-browser-lobby.mjs \
  http://127.0.0.1:8765 http://127.0.0.1:18787 ROOM1 60
```

The final argument is a post-match-load wall-clock wait. The harness captures
screenshots and WebRTC DataChannel counters before and after that interval;
the interval alone is not proof that lockstep simulation advanced.