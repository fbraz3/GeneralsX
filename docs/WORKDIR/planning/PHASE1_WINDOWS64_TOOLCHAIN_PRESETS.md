# Phase 1 - Toolchain and Presets: Windows64 MinGW Setup

**Status**: IN PROGRESS
**Date**: 2026-05-18
**Author**: GitHub Copilot

## Objetivo

Validar que MSYS2 está instalado e testar o configure com `cmake --preset windows64-deploy` em ambiente MSYS2.

## Tarefas

### 1. Instalar MSYS2 (se necessário)

```powershell
# Instalar MSYS2 via winget
winget install MSYS2.MSYS2

# Ou via chocolatey
choco install msys2

# Ou baixar diretamente do site: https://www.msys2.org/
```

### 2. Instalar compiladores MinGW-w64 x86_64

```powershell
# Instalar toolchain x86_64
pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-make mingw-w64-x86_64-ninja

# Instalar dependências do CMake
pacman -S --needed cmake ninja
```

### 3. Validar toolchain

```powershell
# Verificar compiladores
x86_64-w64-mingw32-gcc --version
x86_64-w64-mingw32-g++ --version

# Verificar Ninja
ninja --version
```

### 4. Testar configure

```powershell
# Configurar com preset windows64-deploy
cmake --preset windows64-deploy

# Verificar saída
# Deve mostrar:
# "Configuring MinGW-w64 build settings"
# "MinGW-w64 64-bit (x86_64) detected"
```

### 5. Testar build

```powershell
# Build do target z_generals
cmake --build build/windows64-deploy --target z_generals

# Verificar executável
test -f build/windows64-deploy/GeneralsMD/GeneralsXZH
```

## Arquivos Criados

- `scripts/env/setup-mingw-windows.ps1` - Script de setup MSYS2
- `docs/WORKDIR/planning/PHASE1_WINDOWS64_TOOLCHAIN_PRESETS.md` - Documentação da Fase 1

## Checklist

- [ ] Instalar MSYS2
- [ ] Instalar MinGW-w64 x86_64 toolchain
- [ ] Instalar CMake e Ninja
- [ ] Validar compiladores
- [ ] Testar configure com `cmake --preset windows64-deploy`
- [ ] Testar build
- [ ] Documentar resultados

## Notas

Se MSYS2 não estiver instalado, o usuário precisa instalá-lo primeiro. O script de setup automatizará a instalação das dependências necessárias.
