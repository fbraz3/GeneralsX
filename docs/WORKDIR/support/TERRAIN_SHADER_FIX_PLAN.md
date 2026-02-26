# Terrain Shader Metal Compilation Fix — Session 67

**Status**: Em Progresso  
**Priority**: CRÍTICO — Bloqueando renderização de terrenos  
**Root Cause**: DXVK Patch 13 precisa ser reaplicado ao DXVK macOS

---

## O Problema

### Log Error
```log
[mvk-error] VK_ERROR_INITIALIZATION_FAILED: Shader library compile failed (Error code 3):
program_source:393:122: error: use of undeclared identifier 's0_2d_shadowSmplr'
```

### Por  Que Ocorre
1. DXVK processa D3D8 pixel shaders (PS1.x) com SEMPRE "implicit mode"
2. PS1.x força `majorVersion() < 2` + `forceSamplerTypeSpecConstants` → emite TODOS os 5 tipos de sampler no SPIR-V:
   - `s0_2d`, `s0_2d_shadow`
   - `s0_3d`
   - `s0_cube`, `s0_cube_shadow`
3. SPIRV-Cross gera entrada de shader Metal que passa TODOS como parâmetros:
   ```metal
   ps_main(v, ..., s0_2d, s0_2dSmplr, s0_2d_shadow, s0_2d_shadowSmplr, s0_3d, s0_3dSmplr, s0_cube, s0_cubeSmplr, ...)
   ```
4. MoltenVK MSL wrapper só inicializa descritores **realmente usados** (só 2D para terreno):
   ```metal
   s0_2d = spvDescriptorSet0.s0_2d         // OK
   s0_2dSmplr = spvDescriptorSet0.s0_2dSmplr   // OK
   // s0_2d_shadowSmplr, s0_3d, etc. NÃO EXISTEM
   ```
5. Metal compiler vê `ps_main(..., s0_2d_shadow, s0_2d_shadowSmplr, ...)` como referências não declaradas → ERRO

---

## A Solução: DXVK Patch 13

**Local**: `cmake/dxvk-macos-patches.py` (linha 596+)

**Alvo**: `build/macos-vulkan/_deps/dxvk-src/src/dxso/dxso_compiler.cpp`

### Mudança 1: emitDclSampler() ~line 754
**Antes**:
```cpp
const bool implicit = m_programInfo.majorVersion() < 2 || m_moduleInfo.options.forceSamplerTypeSpecConstants;
```

**Depois**:
```cpp
// GeneralsX Patch 13: Remove PS1.x auto-trigger
const bool implicit = m_moduleInfo.options.forceSamplerTypeSpecConstants;
```

**Efeito**: Com `forceSamplerTypeSpecConstants=False` (default), PS1.x shaders agora usam o tipo detectado (2D), não todos os 5 tipos.

### Mudança 2: emitTextureSample() ~line 3015
**Antes**:
```cpp
if (m_programInfo.majorVersion() >= 2 && !m_moduleInfo.options.forceSamplerTypeSpecConstants) {
```

**Depois**:
```cpp
if (!m_moduleInfo.options.forceSamplerTypeSpecConstants) {
```

**Efeito**: Para PS1.x, não emite `OpSwitch` de spec constants; emite diretamente o tipo detectado.

---

## Passos de Implementação

### 1. Reconfigurar CMake
```bash
cd /Users/felipebraz/PhpstormProjects/pessoal/GeneralsX
rm -rf build/macos-vulkan
cmake --preset macos-vulkan
```
- CMake executa `FetchContent_MakeAvailable(dxvk)` → download DXVK pristino
- Depois chama `add_custom_command` que roda `cmake/dxvk-macos-patches.py`
- Script aplica Patch 13 + outros patches

### 2. Recompilar DXVK
```bash
cmake --build build/macos-vulkan --target dxvk_compiler --parallel
```

### 3. Recompilar GeneralsXZH
```bash
cmake --build build/macos-vulkan --target GeneralsXZH --parallel
```

### 4. Deploy e Test
```bash
./scripts/deploy-macos-zh.sh
./scripts/run-macos-zh.sh -win
```

---

## Diagnostic Checks

### Verificar Patch Aplicado
```bash
grep -A5 "GeneralsX Patch 13" build/macos-vulkan/_deps/dxvk-src/src/dxso/dxso_compiler.cpp
# Deve mostrar:
# const bool implicit = m_moduleInfo.options.forceSamplerTypeSpecConstants;
```

### Verificar Metal Shader Compilation
```bash
# No xcode build output, buscar por "Metal shader compiled"
# Terreno deve renderizar SEM erros "undeclared identifier"
```

---

## Resultado Esperado

- ✅ Terreno renderiza com texturas visíveis
- ✅ Menu principal carrega sem shader errors
- ✅ Nenhum erro `VK_ERROR_INITIALIZATION_FAILED` em shaders

---

## Documentação Relacionada
- `docs/WORKDIR/lessons/2026-02-LESSONS.md` — Lesson: "DXVK PS1.x Sampler Spec Constants"
- `docs/DEV_BLOG/2026-02-DIARY.md` — Session 66 entry
