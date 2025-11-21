# Phase 38.5 - Dependências do Sistema para Testes

**Data**: October 30, 2025  
**Propósito**: Documentar o que está built-in vs o que precisa ser instalado

## TL;DR - Resposta Rápida

Para rodar **Phase 38.5 (teste com Phase 27-37 Metal/OpenGL)**: ✅ **Tudo está built-in**

Para rodar **Phase 39+ (DXVK/Vulkan)**: ⚠️ **Precisa instalar DXVK + MoltenVK**

---

## Phase 38.5 - Teste com Metal/OpenGL (AGORA)

### ✅ O Que Está Built-in (Nenhuma Instalação Necessária)

**Frameworks macOS** (Built-in desde o Xcode):
- ✅ **Metal** - Framework gráfico nativo (phase 27-37)
- ✅ **MetalKit** - Utilitários Metal (phase 27-37)
- ✅ **OpenGL** - Legacy graphics (phase 27)
- ✅ **Cocoa** - Interface gráfica
- ✅ **QuartzCore** - Renderização

**Bibliotecas Compiladas**:
- ✅ **SDL2** - Interface de janela (link em vcpkg.json)
- ✅ **OpenAL** - Áudio (link em cmake/openal.cmake)
- ✅ **ZLIB** - Compressão (link em vcpkg.json)
- ✅ **FFmpeg** - Vídeo (link em vcpkg.json)

**Status Compilação**:
- ✅ Executável de 14MB com tudo incluído
- ✅ Todas as dependências resolvidas via CMake
- ✅ Pronto para rodar imediatamente

### 🚀 Como Rodar Phase 38.5

```bash
# Build (se não tiver já)
cd /Users/felipebraz/PhpstormProjects/pessoal/GeneralsGameCode
cmake --preset macos-arm64
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

# Deploy executável
cp build/macos-arm64/GeneralsMD/GeneralsXZH $HOME/GeneralsX/GeneralsMD/

# Copiar assets (se não tiver)
# - Coloque Textures.big, INI.big, etc em $HOME/GeneralsX/GeneralsMD/Data/

# Rodar teste
cd $HOME/GeneralsX/GeneralsMD
USE_METAL=1 ./GeneralsXZH 2>&1 | tee logs/phase38_test.log

# Deixar rodar 30+ minutos
# Depois: grep -i error logs/phase38_test.log
```

**Resultado Esperado**: Jogo rodando com Metal backend, comportamento idêntico a Phase 27-37

---

## Phase 39+ - DXVK/Vulkan (FUTURO)

### ⚠️ O Que Precisará Ser Instalado

**Para usar DXVK Backend** (Phase 39):
1. **DXVK** - Tradutor DirectX 8 → Vulkan
   - Opção A: Usar DXVK pré-compilado (mais fácil)
   - Opção B: Build from source (mais controle)

2. **MoltenVK** (macOS específico) - Tradutor Vulkan → Metal
   - Necessário para usar Vulkan em macOS
   - Mantido pela Khronos (confiável)

3. **Vulkan SDK** (opcional, para debugging)
   - VulkanSDK ferramentas (validation layers, etc)

### 📦 Opções de Instalação para Phase 39

#### Opção 1: Usar DXVK via Homebrew (RECOMENDADO)

```bash
# DXVK via Homebrew
brew install dxvk

# MoltenVK via Homebrew  
brew install molten-vk
```

**Vantagens**:
- ✅ Instalação em 2 minutos
- ✅ Automático updates
- ✅ Sem build complexo

**Desvantagens**:
- Menos controle sobre versão

#### Opção 2: Build DXVK from Source

```bash
# Clone DXVK
git clone https://github.com/doitsujin/dxvk.git
cd dxvk

# Build (macOS)
./package-release.sh master

# Instalar
cp -r dxvk-builds/dxvk-master /opt/dxvk
export DXVK_HUD=fps  # Ativar stats
```

**Vantagens**:
- ✅ Máximo controle
- ✅ Patches locais possível
- ✅ Debugging facilitado

**Desvantagens**:
- ⚠️ Build leva 30+ minutos
- ⚠️ Requer manutenção

#### Opção 3: Proton Container (Para Windows Apps)

```bash
# Não aplicável para GeneralsX
# (Proton é para aplicações Windows, não para código compilado nativo)
```

### 🔧 Phase 39+ Build com DXVK

Esperado no Phase 39 (não relevante agora):

```bash
# Build com DXVK habilitado
cmake --preset macos-arm64 -DUSE_DXVK=ON
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

# Rodar com DXVK backend
export DXVK_HUD=fps         # Mostrar FPS
export VK_LAYER_PATH=$DXVK_LAYER_PATH
./GeneralsXZH

# Alternativamente, OpenGL legacy
export USE_OPENGL=1
./GeneralsXZH
```

---

## Resumo: O Que Instalar Quando

### ✅ Phase 38.5 (AGORA - Nada a Instalar)

**Status**: Tudo built-in no executável

**Comando**:
```bash
./GeneralsXZH  # Pronto para rodar
```

### ⏳ Phase 39 (Próxima Semana)

**Instale quando começar Phase 39**:
```bash
brew install dxvk molten-vk
# ou
brew install vulkan-tools  # Para debugging (opcional)
```

### 📋 Checklist Pré-Phase-39

- [ ] SDL2 installed (já está via vcpkg)
- [ ] OpenAL installed (já está via cmake)
- [ ] Metal frameworks (built-in no macOS)
- [ ] OpenGL frameworks (built-in no macOS)
- [ ] Quando iniciar Phase 39:
  - [ ] Install DXVK (via brew ou source)
  - [ ] Install MoltenVK (via brew ou source)
  - [ ] Vulkan SDK (opcional, para validation)

---

## Dependências Detalhadas por Layer

### Layer 1: Sistema Operacional (Built-in no macOS)

| Componente | Status | Localização |
|-----------|--------|-------------|
| Metal | ✅ Included | `/System/Library/Frameworks/Metal.framework` |
| OpenGL | ✅ Included | `/System/Library/Frameworks/OpenGL.framework` |
| Cocoa | ✅ Included | `/System/Library/Frameworks/Cocoa.framework` |
| MetalKit | ✅ Included | `/System/Library/Frameworks/MetalKit.framework` |
| QuartzCore | ✅ Included | `/System/Library/Frameworks/QuartzCore.framework` |

### Layer 2: Build Dependencies (Via CMake/vcpkg)

| Componente | Status | Gerenciador |
|-----------|--------|-------------|
| SDL2 | ✅ Compiled | vcpkg.json dependency |
| OpenAL | ✅ Available | cmake/openal.cmake |
| ZLIB | ✅ Compiled | vcpkg.json dependency |
| FFmpeg | ✅ Compiled | vcpkg.json dependency |
| GLAD | ✅ Included | Submodule ou FetchContent |

### Layer 3: Graphics Backends

**Phase 38.5 (Metal/OpenGL)**:
- ✅ Metal - Built-in
- ✅ OpenGL - Built-in
- ❌ DXVK - Não necessário (ainda em Phase 27-37)

**Phase 39+ (DXVK)**:
- ⚠️ DXVK - Instalar quando Phase 39 começar
- ⚠️ MoltenVK - Instalar quando Phase 39 começar
- ✅ Vulkan - Opcional (para validation/debugging)

---

## Troubleshooting: Se Algo Faltar

### Erro: "SDL2 not found"

```bash
# Install SDL2
brew install sdl2

# Rebuild
cmake --preset macos-arm64 -DCMAKE_FIND_DEBUG_MODE=ON
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

### Erro: "OpenAL not found"

```bash
# Install OpenAL
brew install openal-soft

# Rebuild
cmake --preset macos-arm64
```

### Erro: "Metal framework not found"

```bash
# Metal está built-in, problema é no CMake
# Solução: Reinstalar Xcode Command Line Tools
xcode-select --install
```

### Erro ao Rodar: "Metal device creation failed"

```bash
# Provavelmente issue com Metal en macOS
# Verifique GPU:
system_profiler SPDisplaysDataType | grep Chipset

# Se tiver AMD GPU antigo:
export USE_OPENGL=1  # Use OpenGL em vez de Metal
./GeneralsXZH
```

---

## Verificar Dependências Instaladas

```bash
# Verificar SDL2
pkg-config --list-all | grep sdl2
brew list sdl2

# Verificar OpenAL
pkg-config --list-all | grep openal
brew list openal-soft

# Verificar Metal (macOS)
xcrun -find metal

# Verificar Xcode version
xcode-select --version
```

---

## Próximas Etapas

### Phase 38.5 (AGORA)
✅ Nenhuma instalação necessária - executável pronto

### Phase 39 (Quando começar DXVK)
```bash
brew install dxvk molten-vk
# Build com DXVK habilitado
# Testar backend Vulkan
```

### Phase 40+ (Otimizações)
- Instalar Vulkan SDK se quiser validation layers
- Opcional: build DXVK from source para patches locais

---

## Resumo Final

**Para Phase 38.5 (agora)**:
- ✅ **Tudo built-in** - execute e teste
- ✅ Sem instalações adicionais necessárias
- ✅ Executável de 14MB com tudo incluído

**Para Phase 39+ (futuro)**:
- ⚠️ **Instale DXVK + MoltenVK** quando começar Phase 39
- ✅ Simples: `brew install dxvk molten-vk`
- ✅ ~5 minutos de instalação

---

**Conclusão**: Você está 100% pronto para Phase 38.5 agora. Nenhuma ação necessária até Phase 39!
