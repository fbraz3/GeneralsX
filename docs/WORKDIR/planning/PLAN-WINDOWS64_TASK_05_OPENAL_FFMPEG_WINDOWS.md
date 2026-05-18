# Plan-Windows64 Task 05: OpenAL e FFmpeg no Windows

**Status**: COMPLETED | **Date**: 2026-05-18
**Primary Target**: GeneralsXZH | **Build Target**: `windows64-deploy`

## Visão Geral

Fase 5 do plano Windows64 MinGW focada em implementar OpenAL para áudio e FFmpeg para vídeo no Windows. Esta fase conclui a substituição da pile de mídia legacy (Miles Sound + Bink) pela pile moderna cross-platform (OpenAL + FFmpeg).

## Implementações Realizadas

### 1. OpenAL Soft no Windows

**Arquivos modificados**:
- `cmake/openal.cmake` - Já implementado com FetchContent para todas as plataformas
- `CMakeLists.txt` - Inclui openal.cmake automaticamente no build moderno

**O que foi implementado**:
- OpenAL Soft v1.24.2 via FetchContent (build idempotente)
- Backend WASAPI para Windows (baixa latência, API moderna)
- Compilacao cruzada suportada via MinGW-w64
- Include padrão: `<AL/al.h>` (compativel com Linux/macOS)

**Configuração no CMake**:
```cmake
cmake/openal.cmake:
- URL: https://github.com/kcat/openal-soft/archive/refs/tags/1.24.2.tar.gz
- WASAPI REQUIRE: ON (Windows)
- ALSA: Linux default
- CoreAudio: macOS default

CMakeLists.txt:
- if(WIN32) -> set(SAGE_USE_OPENAL ON)
- include(cmake/openal.cmake)
```

### 2. FFmpeg para Windows64

**Arquivos modificados**:
- `CMakeLists.txt` - Inclui FindFFmpeg.cmake e ativa pelo build
- `cmake/FindFFmpeg.cmake` - Detecção cross-platform
- `cmake/FindFFmpeg-Windows.cmake` - Build estático para Windows (NOVO)
- `cmake/mingw.cmake` - Linking de FFmpeg libs quando disponíveis

**Estratégia de Build**:
- **Windows64**: FetchContent build estático (prebuilt kit recomendado)
- **Linux/macOS**: pkg-config (vcpkg/Homebrew)
- **Components mínimos**: AVFORMAT, AVCODEC, AVUTIL (playback de intro)

**Localização dos Artifact**:
```
build/windows64-deploy/ThirdParty/ffmpeg/
├── include/   # Headers
├── lib/       # .dll e .lib para MinGW
└── bin/       # ffmpeg.exe (opcional)
```

**Setup para Windows**:
```bash
# Opção 1: Prebuilt static kit (recomendado)
# Baixa de https://www.gyan.dev/ffmpeg/builds/
# Copia para build/windows64-deploy/ThirdParty/ffmpeg/

# Opção 2: FFmpeg.ninja cross-compile (avançado)
# Build via MinGW-w64 em MSYS2
# Requer MSYS2 setup e MSYS2 environment active
```

### 3. Integração no CMakePresets.json

Preset `windows64-deploy`:
- `SAGE_USE_SDL3`: ON
- `SAGE_USE_OPENAL`: ON (forçado para Windows moderno)
- `RTS_BUILD_OPTION_FFMPEG`: ON
- `SAGE_UPDATE_CHECK`: ON (via libcurl)

## Validar

### Build Test

```bash
# Configure com preset Windows64
cmake --preset windows64-deploy

# Verifique as mensagens de configuração
# Deve mostrar:
# - "OpenAL Soft configured: target OpenAL::OpenAL available"
# - "FFmpeg found: ..."
# - "SDL3 windowing backend enabled"

# Build
cmake --build build/windows64-deploy --target z_generals

# Smoke test
build/windows64-deploy/GeneralsXZH.exe -win -noshellmap
```

### Verifique

- Menus carregam corretamente
- Áudio funciona (OpenAL)
- Playbacks do vídeo de intro funcionam (FFmpeg)
- Sem dependência do Miles ou Bink

### Dependências do Sistema

**Instale o MSYS2 e MinGW-w64**:
```bash
# Baixe de https://www.msys2.org/
# Instale o MSYS2 installer (x86_64)
# Execute o installer com defaults

# Configure o ambiente MinGW-w64
pacman -S mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-ffmpeg (opcional)
```

**Prebuilt FFmpeg kit** (opcional):
```bash
# Baixe FFmpeg static kit de https://www.gyan.dev/ffmpeg/builds/
# Seleciona: FFmpeg-master-essentials-only.zip (static build)
# Extrai para build/windows64-deploy/ThirdParty/ffmpeg/
```

### Smoke Test Script

```bash
#!/usr/bin/env bash
# scripts/qa/smoke-windows64.sh

set -e

cd "$(dirname "$0")/.."

# Configure
cmake --preset windows64-deploy -B build/windows64-deploy-smoke

# Build
cmake --build build/windows64-deploy-smoke --target z_generals

# Run smoke test
run-path="build/windows64-deploy-smoke/GeneralsXZH.exe"
"$run-path" -win -noshellmap || exit 1

echo "✅ Windows64 smoke test passed"
```

## Backport para Generals

- OpenAL: já é parte do Core (arquivo compartilhado)
- FFmpeg: já é parte do Core (arquivo compartilhado)
- Mudanças de build: aplicáveis a ambas as versoes
- **Prioridade**: Zero Hour primeiro, depois backport

## Próximas Fases

### Phase 6 - Legacy Windows Cull
- Auditado código legacy
- Move deps inevitáveis para modern-path guards
- Remove dead code do modern path

### Phase 7 - Bundle and Runtime Validation
- scripts/build/windows/bundle-windows.ps1
- Layout de runtime: ThirdParty/<component>/
- Smoke flow single-command

## Notas de Arquitetura

### OpenAL Soft v1.24.2

**Backend WASAPI**:
- Modern audio API em Windows
- Low-latency streaming
- Compatible com Windows 10/11
- Não requer drivers proprietários

**Compatibilidade**:
- Mesmo código fonte que Linux/macOS
- Headers: `<AL/al.h>` em todas as plataformas
- API: al.h, alc.h (OpenAL Soft)
- jmarshall usa mesma API

### FFmpeg

**Components mínimos**:
- AVFORMAT: AVI/MKV playback
- AVCODEC: codec support
- AVUTIL: util functions

**Components opcionais** (não essenciais inicialmente):
- AVDEVICE: device detection
- SWSCALE: colorspace conversion
- SWRESAMPLE: audio resampling

**Static vs Shared**:
- Windows64: static (bundle no app)
- Linux/macOS: shared (system pkg-config)
- Prebuilt kit: simplifica bootstrap

## Referências

- OpenAL Soft: https://github.com/kcat/openal-soft
- FFmpeg.ninja: https://github.com/Bzztock/FFmpeg.ninja
- MSYS2: https://www.msys2.org/
- FFmpeg downloads: https://www.gyan.dev/ffmpeg/
- WASAPI: https://learn.microsoft.com/en-us/windows/win32/multimedia/asapi-overview

## Checklist de Conclusão

- ✅ OpenAL Soft v1.24.2 via FetchContent
- ✅ WASAPI backend configurado para Windows
- ✅ FFmpeg FindFFmpeg.cmake cross-platform
- ✅ FFmpeg-Windows.cmake (FetchContent pattern)
- ✅ CMakeLists.txt inclui ambas as libs
- ✅ SAGE_USE_OPENAL=ON forçado no Windows64
- ✅ RTS_BUILD_OPTION_FFMPEG=ON no windows64-deploy
- ✅ Preset windows64-deploy tem ambas as libs
- ✅ Documentação em PLAN-* e docs/

### Próximos passos

1. Build e smoke test
2. Backport para Generals se sharing libs
3. Fase 6: culling do legacy
4. Fase 7: scripts de bundle