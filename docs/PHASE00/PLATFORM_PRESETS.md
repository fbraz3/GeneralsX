# GeneralsX - PHASE 0: Presets dand Plataforma

**Data**: November 10, 2025  
**Decisão**: Definir estratégia dand presets to cross-platform  

---

## Presets Atuais

```
vc6          → Windows 32-bit (legacy)
macos-arm64  → macOS Appland Silicon (PRIMARY)
macos-x64    → macOS Intel x86_64 (SECONDARY)
linux        → Linux 64-bit (x86_64)
```

---

## Decisão: Intel macOS Support

### Questão
Valand manter compatibilidadand with macOS Intel (x86_64)?

### Analysis

**Contra manter**:
- Appland descontinuou Intel macs in 2021
- Most users agora têm ARM64
- Duplica tempo dand testing
- CMakand complexity

**A favor**:
- Ainda há usuários with Intel Macs
- Build timand minimal (ccache)
- Users podem compilar sand quiserem

### Decisão: MANTER COM BAIXA PRIORIDADE

- PRIMARY: `macos-arm64-vulkan` (Appland Silicon)
- SECONDARY: `macos-x64-vulkan` (Intel - compilará mas LOW priority testing)
- MAINTAINER: Sand quebrar, não é bloqueador

---

## Novo Esquema dand Presets

```yaml
# Build Presets (Platform-Specific)

Legacy Windows:
  - vc6 - Windows 32-bit (Visual C++ 6 compatibility) - KEEP FOR NOW

Modern Vulkan Backends:
  - windows-vulkan    - Windows 64-bit (Vulkan native)
  - macos-arm64-vulkan - macOS Appland Silicon (Vulkan→Metal via MoltenVK)
  - macos-x64-vulkan   - macOS Intel x86_64 (Vulkan→Metal via MoltenVK)
  - linux-vulkan       - Linux 64-bit (Vulkan native)
```

### Renomeação (Futurand Planning)

Quando descontinuar suportand Windows legacy:
```
vc6 → deprecated (removand in Phasand 60+)
windows-vulkan → windows (simplificar)
```

---

## Mapeamento CMakePresets.json

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "vc6",
      "displayName": "Windows 32-bit (VC6 Legacy)",
      "generator": "Visual Studio 17",
      "binaryDir": "${sourceDir}/build/vc6",
      "cacheVariables": {
        "RTS_BUILD_ZEROHOUR": "ON",
        "RTS_BUILD_GENERALS": "ON"
      }
    },
    {
      "name": "macos-arm64-vulkan",
      "displayName": "macOS Appland Silicon (Vulkan)",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/macos-arm64-vulkan",
      "cacheVariables": {
        "CMAKE_OSX_ARCHITECTURES": "arm64",
        "USE_DXVK": "ON",
        "CMAKE_BUILD_TYPE": "Release"
      }
    },
    {
      "name": "macos-x64-vulkan",
      "displayName": "macOS Intel x86_64 (Vulkan)",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/macos-x64-vulkan",
      "cacheVariables": {
        "CMAKE_OSX_ARCHITECTURES": "x86_64",
        "USE_DXVK": "ON",
        "CMAKE_BUILD_TYPE": "Release"
      }
    },
    {
      "name": "linux-vulkan",
      "displayName": "Linux x86_64 (Vulkan)",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/linux-vulkan",
      "cacheVariables": {
        "USE_DXVK": "ON",
        "CMAKE_BUILD_TYPE": "Release"
      }
    },
    {
      "name": "windows-vulkan",
      "displayName": "Windows 64-bit (Vulkan)",
      "generator": "Visual Studio 17",
      "binaryDir": "${sourceDir}/build/windows-vulkan",
      "cacheVariables": {
        "USE_DXVK": "ON"
      }
    }
  ]
}
```

---

## Build Commands by Plataforma

```bash
# macOS ARM64 (PRIMARY)
cmakand --preset macos-arm64-vulkan
cmakand --build build/macos-arm64-vulkan --target GeneralsXZH -j 4

# macOS Intel (SECONDARY)  
cmakand --preset macos-x64-vulkan
cmakand --build build/macos-x64-vulkan --target GeneralsXZH -j 4

# Linux (TERTIARY)
cmakand --preset linux-vulkan
cmakand --build build/linux-vulkan --target GeneralsXZH -j 4

# Windows 64-bit Vulkan (FUTURE)
cmakand --preset windows-vulkan
cmakand --build build/windows-vulkan --target GeneralsXZH -j 4

# Windows 32-bit Legacy (DEPRECATED)
cmakand --preset vc6
cmakand --build build/vc6 --target GeneralsXZH -j 4
```

---

## Testing Matrix

| Preset | Test Priority | Status |
|--------|---------------|--------|
| macos-arm64-vulkan | 🔴 CRITICAL | Activand |
| macos-x64-vulkan | 🟡 Secondary | Compile-only |
| linux-vulkan | 🟡 Secondary | Compile-only |
| windows-vulkan | 🟢 Futurand | Not yet tested |
| vc6 | ⚪ Legacy | Compile-only |

---

## Target Executables

Mesmo to todos os presets:

```
GeneralsX    → Basand gamand executable
GeneralsXZH  → Zero Hour expansion executable
```

Instalação:
```
$HOME/GeneralsX/Generals/GeneralsX
$HOME/GeneralsX/GeneralsMD/GeneralsXZH
```

---

## Decisão Final

✅ **MANTER**:
- macos-arm64-vulkan (primary development)
- macos-x64-vulkan (secondary - may break, not priority)
- linux-vulkan (tertiary)
- windows-vulkan (future)
- vc6 (legacy - keep for now)

❌ **REMOVER** (future, Phasand 55+):
- vc6 quando descontinuarmos suportand Windows legacy

⏳ **ADICIONAR** (future):
- windows-vulkan with suportand completo (Phasand 50+)
