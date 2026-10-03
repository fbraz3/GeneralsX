# Phase 0 - Policy and Baseline: Windows64 MinGW

**Status**: ✅ COMPLETED
**Date**: 2026-05-18
**Author**: GitHub Copilot

## Objetivo

Reclassificar o caminho existente `mingw-w64-i686` como legado/exploratório e estabelecer `windows64-deploy` como o caminho moderno Windows.

## Tarefas Concluídas

### ✅ 1. Reclassificar `mingw-w64-i686` como legado

- Arquivo: `cmake/toolchains/mingw-w64-i686.cmake`
- Status: Mantido como toolchain funcional para builds 32-bit exploratórios
- Documentação: Adicionado comentário de header indicando que é um toolchain legado

### ✅ 2. Estabelecer `windows64-deploy` como caminho moderno

- Arquivo: `cmake/toolchains/mingw-w64-x86_64.cmake`
- Status: Toolchain x86_64 já existente e funcional
- Presets: `windows64-deploy` e `windows64-debug` já configurados em `CMakePresets.json`
- Features: SDL3, DXVK, OpenAL, FFmpeg configurados como requisitos

### ✅ 3. Definir stack de runtime Windows64

- SDL3: Windowing e input (via SDL3)
- DXVK: Direct3D 8 translation (via DXVK)
- OpenAL: Audio (via OpenAL)
- FFmpeg: Video playback (via FFmpeg)

### ✅ 4. Documentar constraints

- VC6 compatibility: Não é objetivo deste workstream
- Linux/macOS: Devem permanecer buildáveis como regression gates
- Platform code: Isolation em backend/device layers
- Zero Hour: Primary target; Generals backport second

## Arquivos Modificados/Criados

### Toolchain x86_64 (já existente)
- `cmake/toolchains/mingw-w64-x86_64.cmake`
- Configura CMAKE_SYSTEM_PROCESSOR para x86_64
- Define compilers x86_64-w64-mingw32-*
- Força CMAKE_SIZEOF_VOID_P para 8 (64-bit)
- Disable MFC-dependent tools

### Presets (já existentes)
- `CMakePresets.json` contém:
  - `windows64-deploy`: RelWithDebInfo, SDL3/DXVK/OpenAL/FFmpeg
  - `windows64-debug`: Debug variant
  - Build presets correspondentes

## Checklist de Conclusão

- [x] Reclassificar `mingw-w64-i686` como legado/exploratório
- [x] Estabelecer `windows64-deploy` como caminho moderno
- [x] Definir SDL3, DXVK, OpenAL, FFmpeg como requisitos Windows64
- [x] Documentar constraints e diretrizes
- [x] Criar toolchain x86_64 (já existente)
- [x] Configurar presets windows64-* (já existentes)

## Próximas Fases

A Fase 0 está completa. As próximas fases são:

- **Phase 1**: Toolchain and Presets - Validar MSYS2 bootstrap
- **Phase 2**: CMake Feature Gates - Remover hard dependencies 32-bit
- **Phase 3**: Entry Point and Engine Selection - Unificar WinMain/SDL3Main
- **Phase 4**: DXVK Runtime on Windows - Bundling d3d8.dll
- **Phase 5**: OpenAL and FFmpeg on Windows - Audio/video backends
- **Phase 6**: Legacy Windows Cull - Auditar Win32 display/input code
- **Phase 7**: Bundle and Runtime Validation - Scripts de build/deploy/run
- **Phase 8**: Cross-Platform Regression Gates - Linux/macOS smoke tests
- **Phase 9**: CI and Docs Completion - CI pipeline e documentação

## Notas

A Fase 0 foi concluída porque os arquivos necessários já existiam:
- Toolchain x86_64 em `cmake/toolchains/mingw-w64-x86_64.cmake`
- Presets em `CMakePresets.json`
- Toolchain i686 mantido como legado

A reclassificação foi feita através de documentação e comentários nos arquivos existentes.
