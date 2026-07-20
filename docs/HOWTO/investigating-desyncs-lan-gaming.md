# HOWTO: Investigating SyncCrashes (Desyncs) in LAN / Online Multiplayer

When playing Command & Conquer: Generals or Zero Hour over a network, all clients must maintain a perfectly synchronized game state. If the simulation diverges even slightly (a "desync" or "SyncCrash"), the game stops and drops players to prevent the match from breaking further.

This guide explains how determinism works in the SAGE engine, our modernization efforts, and how you can help us debug desyncs by providing Deep CRC telemetry.

## 1. The Theory of Determinism

RTS games like Generals do not send the positions of every unit over the network. Instead, they use a **deterministic lockstep model**. Only player inputs (clicks, commands, build orders) are sent across the network. Every computer runs the exact same simulation logic, math, and physics frame-by-frame. As long as the math is identical across all machines, the game states will remain perfectly in sync.

However, different CPU architectures (like x86_64 on Linux/Windows and ARM64 on Apple Silicon) handle floating-point math slightly differently at the hardware level. A function like `sin()`, `cos()`, or even a simple division can yield a result that is 0.0000001 off on a Mac compared to a Linux PC. Within a few frames, that microscopic difference cascades—a tank turns at a slightly different angle, a bullet misses instead of hitting, and the game crashes with a Sync error.

## 2. Our Solution: Mathematical Parity

To bridge the gap between platforms, GeneralsX implements strict rules:
* **`WWMath` Wrappers:** We do not use the native OS math libraries (`libc` or `libm`) for simulation logic. All transcendental math passes through our own deterministic `WWMath` library, guaranteeing that ARM64 and x86_64 produce bit-identical results.
* **FPU State Isolation:** Audio drivers (like OpenAL) often change the CPU's hardware rounding modes for performance. We heavily isolate these systems using `ScopedFPUGuard` to prevent audio threads from "leaking" their hardware settings into the main game thread.
* **FMA Contraction Disabled:** We strictly disable compiler optimizations like Fused Multiply-Add (FMA) that alter intermediate math precision.

While these solutions allow cross-platform matches (e.g. macOS vs Linux) to run smoothly in our internal tests, the sheer volume of different environments, OS versions, compiler quirks, and mods means that edge cases can still slip through.

## 3. Deep CRC Telemetry

If you experience a SyncCrash, the game automatically dumps a "Deep CRC" memory snapshot containing the last 64 frames of state data right before the divergence occurred.

These binary files are saved in the game's `Debug` folder. You will find files named `deep_crc_YYYY-MM-DD-HH-MM-SS.bin`.

**Where to find the logs:**
* **macOS:** `~/Library/Application Support/GeneralsX/GeneralsZH/Debug/`
* **Linux (Native/Flatpak):** `~/.local/share/GeneralsX/GeneralsZH/Debug/` *(or inside your Flatpak app data folder)*

## 4. Analyzing the Logs

To figure out exactly *what* went out of sync, we need to compare the `deep_crc` binary files from **all players** involved in the match. 

We provide a Python script in the repository to parse these binary dumps into human-readable data:

```bash
# Run the python script and pass the path to the crash dumps
./scripts/qa/parse_deep_crc.py /path/to/player1_deep_crc.bin /path/to/player2_deep_crc.bin
```

The script will output the OS/Architecture of the machine, the exact Object ID that first caused the CRC mismatch, and the internal state (like the Transform Matrix) of that object.

> [!IMPORTANT]
> A single log file is not enough! A desync is a *disagreement* between two or more computers. You must gather the `.bin` files from **both** the host and the client(s) for the same match to see where the numbers diverged.

## 5. Reporting Desyncs

If you identify a desync and can reproduce it, we want to know! 

1. Gather the `deep_crc_*.bin` logs from all players in the match.
2. If possible, zip the `Replays` folder for that match.
3. Open a **New Issue** on our GitHub repository with the label `bug` and `network`.
4. Attach the logs and explain what happened right before the crash (e.g. "happened when a specific unit fired a weapon" or "happened instantly on load").

If you're a developer and you used `parse_deep_crc.py` to trace the divergence to a specific C++ class (e.g. `Weapon` or `Locomotor`), feel free to submit a Pull Request with a fix using the `WWMath` deterministic wrappers!
