---
name: sync-thesuperhackers-upstream
description: Sync the repository with TheSuperHackers upstream by creating a dated sync branch, merging thesuperhackers/main, resolving conflicts carefully, then committing and pushing the result
argument-hint: Optional extra sync focus areas or constraints (for example: "prioritize audio-related conflicts" or "document every build-system conflict")
agent: Bender
---

# Sync TheSuperHackers Upstream

Perform a full upstream sync from the `thesuperhackers` remote into this repository while preserving the working cross-platform nature of `GeneralsX`.

## Briefing

`TheSuperHackers` and `GeneralsX` have different goals and this must drive every merge decision:

- `TheSuperHackers` focuses on bug fixes, stability, optimizations, and merging the `Generals` and `GeneralsMD` codebases while preserving compatibility with the original Windows binaries.
- `GeneralsX` focuses on making the game truly cross-platform with a modern stack based on SDL3 + DXVK + OpenAL + FFmpeg, without prioritizing original binary compatibility.

The repository is significantly behind upstream `TheSuperHackers`. The purpose of this sync is to import useful upstream improvements without regressing or dismantling the already functional cross-platform architecture in `GeneralsX`.

## Required Workflow

1. Determine the current local date and create a branch named exactly `thesuperhackers-sync-MM-DD-YYYY`.
2. Verify whether the remote `thesuperhackers` exists.
3. If the remote does not exist, create it using `git@github.com:TheSuperHackers/GeneralsGameCode.git`.
4. Fetch `thesuperhackers`.
5. Merge `thesuperhackers/main` into the new branch.
6. Resolve all conflicts by following the merge instructions below exactly.
7. Ensure the repository remains buildable and configurable after conflict resolution.
8. Commit the final merge result.
9. Push the branch `thesuperhackers-sync-MM-DD-YYYY` to origin.

## Critical Merge Instructions

Expect many conflicts because the projects intentionally diverged. Every conflict must be analyzed individually and in detail.

- Expect many files moved from `Generals/` and `GeneralsMD/` into a unified `Core/` directory. Do not assume that all conflicts in those areas should be resolved by keeping the `TheSuperHackers` version.
- Do not use blanket conflict strategies for large areas of the tree.
- Do not sacrifice the cross-platform architecture of `GeneralsX` just to make the merge easy.
- Do not blindly keep either side. Reconcile changes so that useful `TheSuperHackers` bug fixes, stability work, and optimizations are preserved whenever they do not break the `GeneralsX` platform strategy.
- Preserve the functional cross-platform stack already established in `GeneralsX`: SDL3 for platform/windowing/input, DXVK for graphics, OpenAL for audio, and FFmpeg where applicable.
- Preserve platform isolation. Do not allow platform-specific code to leak into gameplay logic.
- Keep legacy compatibility paths only where they are still intentionally maintained by this repository, but do not let original-binary compatibility override the `GeneralsX` cross-platform objective.
- Review conflicts with extra care in these areas:
  - build system and presets
  - SDL3, DXVK, OpenAL, FFmpeg, and platform abstraction layers
  - shared engine code under `Core/`
  - `Generals/` and `GeneralsMD/` code that may have been unified or refactored upstream
  - launch paths, renderer setup, audio wiring, and asset/runtime integration
- If a conflict involves a removal or downgrade of an existing cross-platform capability, treat that as a high-risk decision and justify it explicitly before accepting it.
- Prefer root-cause conflict resolution over temporary hacks, disabled code paths, or quick stubs.

## Validation Requirements

It is imperative that configure and build workflows still work after the merge. At minimum, validate the relevant project tasks and report the outcome clearly.

Prioritize verifying:

- macOS configure/build flow
- Linux configure/build flow
- any touched deployment or run scripts
- core runtime paths affected by the merge

If a full validation cannot be completed, state exactly what was not run and why.

## Deliverables

After the sync is complete, provide:

1. A concise summary of what was merged from `TheSuperHackers`.
2. A conflict-resolution report describing the most important merge decisions and why they were made.
3. A list of files or subsystems that remain risky and should receive extra review.
4. The final branch name, commit hash, and push status.
5. A user checklist of what should be tested next.

## Required User Checklist

The final response must include a checklist covering at least:

- configure on all supported platforms
- build on all supported platforms
- game launch
- main menu
- skirmish gameplay
- campaign flow
- audio playback
- video playback
- renderer stability
- input handling
- mod loading if affected
- any feature areas touched by resolved conflicts

## Execution Notes

- Work carefully and deliberately. This is not a mechanical merge.
- Read relevant repository instructions and reference material before resolving difficult conflicts.
- Keep the final result aligned with the `GeneralsX` mission: a functional, modern, cross-platform codebase.
