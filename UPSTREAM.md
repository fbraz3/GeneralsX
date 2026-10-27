# Echelon: Fork Identity and Upstream Maintenance

## Product identity

The owner selected **Echelon** on 2026-10-23 as the shared name of the unified
engine and its integrated launcher. The intended executable name is `echelon`.
Generals and Zero Hour are game profiles, not separate Echelon products.

The current migration still builds the inherited `g_generals` and `z_generals`
targets and their `GeneralsX` / `GeneralsXZH` executables. Branding approval
does not mean the single executable or launcher is already implemented.
Executable and packaging changes belong to the single-executable migration
gate in [PLAN-023](docs/WORKDIR/planning/PLAN-023_UNIFIED_ENGINE.md).

The earlier Echelon prototype is a separate project. Preserve its files and
history as references for later work; do not overwrite it or merge its code
merely to apply the name.

## Source lineage

| Source | Role |
| --- | --- |
| [fbraz3/GeneralsX](https://github.com/fbraz3/GeneralsX) | Cross-platform engine foundation and platform updates |
| [TheSuperHackers/GeneralsGameCode](https://github.com/TheSuperHackers/GeneralsGameCode) | Original game-code lineage and gameplay fixes |
| [generalsx-project/dxvk](https://github.com/generalsx-project/dxvk) | Separate graphics dependency and fork history |

The Echelon work started from GeneralsX commit
`afb09f14449bb499d0b60a9e2fd814eb6016ef5b`.
The existing source consolidation map records the original game paths,
canonical destinations and an upstream ancestor. These references must remain
resolvable through Git history.

At branding adoption, `origin` identifies fbraz3/GeneralsX and `upstream`
identifies TheSuperHackers/GeneralsGameCode. Treat these as incoming sources.
Echelon changes belong in the owner's fork; do not publish Echelon branding or
consolidation work to an upstream repository without an explicit request.
Remote names alone are not proof of write ownership: inspect their URLs.

## Updating without losing the fork

- Preserve Git ancestry, original copyright/license notices and contributor
  attributions. Existing `GeneralsX` source annotations describe their origin.
- Keep `Generals/`, `GeneralsMD/` and `Core/` paths where practical. Do not
  perform a global brand substitution in legacy symbols, dependency URLs,
  game identities, serialized data or compatibility paths.
- Use Zero Hour as the canonical engine foundation. Extract Generals-specific
  rules explicitly and retain applicable fixes from both original trees.
- Track moved and consolidated sources in
  `cmake/unified-engine-provenance.json`. Review incoming changes from both
  original paths; the checker reports required review and does not merge code.
- Keep mechanical consolidation, behavior changes and branding in separate
  commits. Integrate upstream on an isolated branch and validate both games.
- Follow the integration procedure and compatibility gates in PLAN-023.
  Update the recorded integration reference only after the incoming work has
  been reviewed and validated. A clean textual merge is insufficient.

Existing release workflows, Flatpak identifiers, asset environment variables
and user-data locations still carry inherited names during migration. Change
them deliberately with packaging and profile compatibility work, including
user-data migration where needed.
