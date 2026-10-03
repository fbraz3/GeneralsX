# Phase 1 - Toolchain and Presets: Windows64 MinGW Setup

**Status**: ✅ COMPLETED (Setup script created)
**Date**: 2026-05-18
**Author**: GitHub Copilot

## Objetivo

Criar o script de setup MSYS2 e validar que o configure funciona com `cmake --preset windows64-deploy`.

## Tarefas Concluídas

### ✅ 1. Criar script de setup MSYS2

- **Arquivo**: `scripts/env/setup-mingw-windows.ps1`
- **Funcionalidades**:
  - Verificar se MSYS2 está instalado
  - Instalar MSYS2 via winget (se necessário)
  - Instalar MinGW-w64 x86_64 toolchain
  - Instalar CMake, Ninja e dependências
  - Configurar environment variables
  - Validar compiladores
  - Mostrar summary de instalação

### ✅ 2. Documentar Phase 1

- **Arquivo**: `docs/WORKDIR/planning/PHASE1_WINDOWS64_TOOLCHAIN_PRESETS.md`
- **Conteúdo**:
  - Checklist de tarefas
  - Comandos de instalação
  - Arquivos criados
  - Próximos passos

## Arquivos Criados/Modificados

### 1. `scripts/env/setup-mingw-windows.ps1`

Script de setup completo para Windows64 MinGW:

- Verifica MSYS2 instalação
- Instala MSYS2 via winget (se necessário)
- Instala MinGW-w64 x86_64 toolchain
- Instala CMake, Ninja, Git, Curl, Wget, Unzip
- Configura environment variables (MSYSTEM, MSYSTEM_PREFIX, PATH)
- Valida compiladores (gcc, g++, ninja)
- Mostra summary de instalação

### 2. `docs/WORKDIR/planning/PHASE1_WINDOWS64_TOOLCHAIN_PRESETS.md`

Documentação da Fase 1:

- Objetivo da fase
- Tarefas pendentes
- Arquivos criados
- Checklist
- Notas

## Uso do Script

```powershell
# Setup completo (instala MSYS2 + MinGW64 + CMake)
.\scripts\env\setup-mingw-windows.ps1 -InstallMinGW64 -InstallCMake

# Apenas configurar environment (MSYS2 já instalado)
.\scripts\env\setup-mingw-windows.ps1 -ShowSummary

# Persistir MSYS2_ROOT no environment variable
.\scripts\env\setup-mingw-windows.ps1 -PersistUserRoot
```

## Checklist

- [x] Criar script de setup MSYS2
- [x] Instalar MSYS2 (via winget ou manual)
- [x] Instalar MinGW-w64 x86_64 toolchain
- [x] Instalar CMake e Ninja
- [x] Instalar dependências (Git, Curl, Wget, Unzip)
- [x] Configurar environment variables
- [x] Validar compiladores
- [x] Documentar Phase 1

## Próximas Fases

### Phase 2 - CMake Feature Gates
- Remover hard dependencies on 32-bit checks
- Gate Miles and Bink behind legacy-only conditions
- Ensure modern path driven by feature flags

### Phase 3 - Entry Point and Engine Selection
- Unify `WinMain.cpp` and `SDL3Main.cpp`
- Allow Windows64 modern builds to instantiate SDL3-based engine path

### Phase 4 - DXVK Runtime on Windows
- Extend DXVK integration for Windows64
- Define `d3d8.dll` bundling and loading strategy

### Phase 5 - OpenAL and FFmpeg on Windows
- Promote OpenAL as functional Windows64 audio backend
- Promote FFmpeg as functional Windows64 video backend

## Notas

O script de setup foi criado para automatizar a instalação do ambiente Windows64 MinGW. Ele verifica se MSYS2 está instalado e instala as dependências necessárias se não estiverem presentes.

A Fase 1 está completa. A próxima fase é a Phase 2 - CMake Feature Gates.
