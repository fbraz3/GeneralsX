# Unified Engine Stage 02: Shared Platform Headers and Remaining License-Identical Device Source

Date: 2026-10-23
Scope: consolidate the 27 byte-identical CompatLib headers onto the Zero Hour tree,
and share the remaining license-title-only `W3DGameLogic.cpp` with Generals.

## Design

Gate 2 of PLAN-023 asks for shared platform implementation before profile or
rules work. CompatLib already builds once under `GeneralsMD/Code/CompatLib`;
Generals only kept a second copy of the Include headers. Those 27 headers were
byte-identical. Retarget Generals and the UNIX global include path to the
Zero Hour CompatLib Include directory, then delete the Generals copies.

This does not move CompatLib into Core. Zero Hour remains the canonical location
that already owns the CompatLib static library and d3dx8 sources. Generals keeps
its own compile definitions (`RTS_GENERALS`) and game headers; only the platform
shim include root is shared.

`W3DGameLogic.cpp` was the leftover license-title-only device source from the
Stage 01 candidate list. Generals now compiles the Zero Hour file against
Generals headers, matching the three W3D Client sources consolidated earlier.
The matching `W3DGameLogic.h` pair remains duplicated for now: include search
still uses each game's `GameEngineDevice/Include` tree.

An accidental junk file `GeneralsMD/Code/CompatLib/Includecho` (truncated text,
not a header) was removed.

## Provenance

`cmake/unified-engine-provenance.json` gains 28 entries (1 device source + 27
CompatLib headers). Baseline revisions are unchanged (`source_revision`
afb09f144, `upstream_revision` 0af9db488). The structural checker requires
deleted Generals paths to remain absent and to have existed at the baseline.

## Commands

```bash
python3 scripts/tooling/cpp/maintenance/check_unified_engine_provenance.py
python3 -m unittest discover -s scripts/qa -p test_unified_engine_provenance.py
```

## Local validation

Reused the Stage 01 `ubuntu-dev` Ubuntu 24.04 container and
`build/linux64-unified-baseline` tree. After retargeting CompatLib includes,
both `g_generals` and `z_generals` reconfigured and rebuilt successfully
(2424 compile/link steps). Headless init/shutdown with
`-replay __unified_smoke_no_matches_*.rep` exited 0 for both executables
against the prepared Steam asset roots. Logs: `logs/unified-stage2-*`.

Provenance structural check reports 32 consolidated entries. The seven
isolated Git fixture tests still pass.

## Intentionally deferred

- Moving CompatLib into `Core/` (larger structural divergence; rehearse only if
  upstream layout or packaging requires it).
- Sharing `W3DGameLogic.h` or other GameEngineDevice headers that still live in
  both include trees.
- `NameKeyGenerator.cpp` (license-identical source, incompatible header layout).
- Profile foundation, shared compiled engine objects, and the single executable.
- Full upstream merge rehearsal beyond the existing mapped-range detection.
