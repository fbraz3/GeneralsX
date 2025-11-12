# Multithreading Modernization Plan

This document outlines a staged strategy to modernize Command & Conquer: Generals (GeneralsX) for multi-core CPUs while preserving stability and deterministic gameplay.

## Scope and Principles

- Preserve single-threaded correctness first; threading is introduced incrementally.
- Keep multiplayer deterministic: avoid non-deterministic ordering and floating-point divergence.
- Prefer coarse-grained jobs with clear ownership to minimize locking.
- Use existing primitives (ThreadClass, CriticalSectionClass, Mutex) and extend with a small task system.

## Current Architecture (Baseline)

- Single-threaded main loop: Logic → AI → Physics → Rendering.
- Limited background threads: networking, audio, I/O.
- Threading infrastructure exists but is minimally used.

## Opportunities by Phase

### Phase 02: Low-Risk Parallelism (Foundations)
- Parallel asset/INI loading during initialization.
- Background audio processing and streaming I/O.
- Texture/model preprocessing in worker threads.

Expected impact: Faster startup (40–60%), lower main-thread stalls during loads.

### Phase 03: Moderate Complexity
- AI batch updates (per-squad/per-platoon) with immutable-frame views.
- Parallel object updates for independent entities using spatial partitioning to avoid write conflicts.
- Background resource management (eviction, decompression, streaming).

Expected impact: 20–50% logic-side speedup depending on map and unit counts.

### Phase 04: Advanced Architecture
- Producer–consumer game loop: game logic produces frame state; render thread consumes previous frame.
- Task-based scheduler with dependencies (job graph) for physics/particles/pathing.
- NUMA-aware thread affinity and work-stealing for scalability.

Expected impact: 25–40% FPS improvement on 4–8 cores; diminishing returns beyond.

## Technical Challenges and Mitigations

- Determinism: lock step networking requires fixed update order; use deterministic queues and stable iteration.
- Shared state: adopt “write-owner per system” or double-buffered data; avoid cross-thread writes.
- Order dependencies: define explicit barriers between subsystems; document contracts.
- Floating point: use consistent FP modes; avoid denormals; pin math libraries per platform.

## Minimal Task System (Proposal)

- Thread pool with N = cores-1 workers.
- Job API: submit(function, range) with optional dependencies; work-stealing deque per worker.
- Barriers for frame phases (AI, physics, draw-prepare).

## Implementation Strategy and Timeline

1) Foundation (2–3 weeks)
- Implement thread pool + job API.
- Make asset/INI loading multi-threaded with safe queues and progress reporting.

2) Safe Enhancements (4–6 weeks)
- Move audio processing and streaming I/O fully off the main thread.
- Add background resource preparation (texture upload staging, model processing).

3) Core Loop Modernization (8–12 weeks)
- Introduce render/logic decoupling (one-frame latency).
- Parallelize AI/object updates using partitioning and immutable-frame snapshots.

## Testing and Validation

- Single-player benchmarks: frame time variance, FPS, CPU utilization.
- Multiplayer lockstep: checksum or state hash each tick; detect desyncs.
- Determinism tests across platforms (macOS/Linux/Windows) with fixed seeds.

## Expected Results

- 4-core systems: 40–60% improvement typical.
- 8-core systems: near-linear improvements up to contention and memory bandwidth limits.
- No gameplay regressions; identical simulation results with and without threading.

## Why After Cross-Platform Stability

Threading can hide platform bugs and complicate debugging. The plan assumes:
1) Stable single-threaded execution on all platforms.
2) Incremental feature flags to enable/disable threaded paths.
3) Extensive logging/telemetry to isolate regressions quickly.

---

See README Roadmap for when this initiative becomes active. This document will evolve with concrete APIs and code references once Phase 02 begins.
