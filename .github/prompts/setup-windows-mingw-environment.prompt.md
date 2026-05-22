---
name: setup-windows-mingw-environment
description: Set up or validate the local Windows MinGW/MSYS2 environment for GeneralsX using the repository bootstrap scripts and confirm the toolchain is ready for Windows64 MinGW work
argument-hint: Optional setup constraints or scope (for example: "install MSYS2 if missing", "validate only", or "persist environment root and open a prepared shell")
agent: Bender
---

# Setup Windows MinGW Environment

Prepare a Windows development environment for the modern GeneralsX MinGW workflow.

The goal is to make the local machine ready for `windows64-deploy` style work without relying on Visual Studio.

## Primary References

- Use [scripts/env/setup-mingw-windows.ps1](../../scripts/env/setup-mingw-windows.ps1) to configure the current PowerShell session for MSYS2 MinGW64.
- Use [scripts/env/enter-mingw-windows.cmd](../../scripts/env/enter-mingw-windows.cmd) to open a new PowerShell window with the MinGW/MSYS2 environment already loaded.
- Treat [cmake/toolchains/mingw-w64-x86_64.cmake](../../cmake/toolchains/mingw-w64-x86_64.cmake) as the current repository example for MSYS2-aware MinGW toolchain wiring.

## Required Workflow

1. Detect whether MSYS2 is already installed in `C:\msys64` or in the path stored in `GENERALSX_MSYS2_ROOT`.
2. If MSYS2 is missing and the user did not explicitly request validation-only mode, install MSYS2 in a short ASCII-only path.
3. Update the MSYS2 base system with `pacman -Syuu` until it is fully current.
4. Install the packages required for the repository bootstrap and MinGW workstream.
   - At minimum: `base-devel`, `mingw-w64-x86_64-toolchain`, `mingw-w64-x86_64-ninja`, and `mingw-w64-x86_64-cmake`.
5. Run or dot-source [scripts/env/setup-mingw-windows.ps1](../../scripts/env/setup-mingw-windows.ps1) with `-ShowSummary`.
6. If requested, run the script with `-PersistUserRoot` so `GENERALSX_MSYS2_ROOT` is stored for future sessions.
7. Validate the session by checking:
   - `x86_64-w64-mingw32-gcc`
   - `make`
   - `cmake`
   - `MSYSTEM`
   - `MSYSTEM_PREFIX`
8. If requested, launch [scripts/env/enter-mingw-windows.cmd](../../scripts/env/enter-mingw-windows.cmd) so the user gets a ready-to-use shell.
9. Report exactly what was installed, what was already present, and any remaining blockers for `windows64-deploy` work.

## Expected Environment State

After successful setup:

- `GENERALSX_MSYS2_ROOT` should point to the active MSYS2 root.
- `MSYSTEM` should be set to `MINGW64` for the `windows64-deploy` bootstrap path.
- `MSYSTEM_PREFIX` should point at the active MinGW prefix.
- The session `PATH` should include the active MinGW `bin` directory and `usr/bin` from MSYS2.
- `cmake` should remain the repository-preferred Windows installation if already present.

## Constraints

- Do not assume Visual Studio is available or required.
- Prefer the repository bootstrap scripts over ad-hoc PATH editing.
- Keep the setup reproducible and explicit; do not silently mutate system-wide PATH entries unless the user asks for persistence.
- If the machine already has a working MSYS2 installation, reuse it instead of reinstalling it.

## Deliverables

Provide a short final report with:

1. Installation status.
2. Validation status.
3. Active MSYS2 root and MinGW prefix.
4. Any missing packages or next steps for `windows64-deploy` work.