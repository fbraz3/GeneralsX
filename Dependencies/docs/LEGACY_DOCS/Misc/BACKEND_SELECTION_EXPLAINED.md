# Backend Selection: Compile-Time vs Runtime

## A Confusão Explicada

Existe uma **dupla decisão** sobre qual backend usar:

### 1. Tempo de Compilação (CMake) 🔨

Decidido em **`cmake/config-build.cmake` linha 11**:
```cmake
option(USE_DXVK "Enable DXVK/Vulkan graphics backend (Phase 40+)" ON)
```

**Resultado**:
- `USE_DXVK=ON` (padrão) → Compila `DXVKGraphicsBackend` (Vulkan via MoltenVK)
- `USE_DXVK=OFF` → Compila `LegacyGraphicsBackend` (Metal ou OpenGL)

**Arquivo responsável**: `Core/Libraries/Source/WWVegas/WW3D2/graphics_backend_init.cpp` linhas 39-68

```cpp
#ifdef USE_DXVK
    g_graphicsBackend = new DXVKGraphicsBackend();
#else
    g_graphicsBackend = new LegacyGraphicsBackend();
#endif
```

### 2. Tempo de Execução (WinMain.cpp) ⚙️

Decidido em **`GeneralsMD/Code/Main/WinMain.cpp` linhas 886-915**:

```cpp
extern bool g_useMetalBackend;  // Global flag

#ifdef __APPLE__
    // macOS: Metal is the default
    const char* use_opengl = getenv("USE_OPENGL");
    g_useMetalBackend = (use_opengl == nullptr);  // Metal unless USE_OPENGL=1
#endif
```

**Resultado**:
- `g_useMetalBackend = true` → Metal
- `g_useMetalBackend = false` → OpenGL

**MAS**: Este flag **só tem efeito se o backend compilado for LegacyGraphicsBackend**!

## O Fluxo Verdadeiro

```
Compilação (CMake)
├── USE_DXVK=ON (padrão)
│   └─ DXVKGraphicsBackend compilado ✅
│      └─ Vulkan/MoltenVK é o ÚNICO backend disponível
│
└── USE_DXVK=OFF
    └─ LegacyGraphicsBackend compilado ✅
       └─ Tempo de execução (WinMain.cpp):
          ├─ g_useMetalBackend = true → Metal
          └─ g_useMetalBackend = false → OpenGL (USE_OPENGL=1)
```

## O Que Você Viu nos Logs

No arquivo `/tmp/manual_run.log`:

```
===== WinMain: macOS detected, backend = METAL (default) =====
...
Graphics Backend: Creating DXVK/Vulkan backend (USE_DXVK enabled)
```

**Explicação**:
1. WinMain.cpp calcula `g_useMetalBackend = true` (Metal by default on macOS)
2. MAS o binário foi compilado com `USE_DXVK=ON`
3. Então `CreateGraphicsBackend()` **ignora** `g_useMetalBackend` 
4. E cria `DXVKGraphicsBackend` (Vulkan) em vez disso
5. A mensagem "backend = METAL" é apenas da decisão de tempo de execução, não tem efeito

## Resumo: Qual Backend Realmente Está Sendo Usado?

| Compilação | Execução | Resultado |
|-----------|----------|-----------|
| `USE_DXVK=ON` | N/A | **Vulkan/MoltenVK** (g_useMetalBackend ignorado) |
| `USE_DXVK=OFF` | Metal env | **Metal** |
| `USE_DXVK=OFF` | OpenGL env | **OpenGL** |

## Como Confirmar Qual Backend Está Rodando

**Procure neste texto do log**:

```cpp
// COMPILE-TIME seleção (arquivo: graphics_backend_init.cpp)
if(USE_DXVK)
    printf("Graphics Backend: Creating DXVK/Vulkan backend (USE_DXVK enabled)");
else
    printf("Graphics Backend: Creating Legacy Metal/OpenGL backend (USE_DXVK disabled)");

// RUNTIME decisão (arquivo: WinMain.cpp) - SÓ EFEITO SE LEGACY
printf("===== WinMain: macOS detected, backend = %s =====", 
    g_useMetalBackend ? "METAL (default)" : "OPENGL (via USE_OPENGL=1)");
```

**Ordem no log**:
1. Primeira: Mensagem de `WinMain.cpp` (runtime decision)
2. Segunda: Mensagem de `graphics_backend_init.cpp` (compile-time decision)
3. **Segunda mensagem sobrescreve a primeira**

## Por Que Existe Essa Confusão?

Histórico do código:
- **Fase 27-38**: Apenas `LegacyGraphicsBackend` (Metal/OpenGL), decisão em runtime
- **Fase 39+**: `DXVKGraphicsBackend` (Vulkan) adicionado, decisão em compile-time
- Código de decisão runtime não foi removido (compatibilidade para futuros fallbacks)

## Solução: Integrar Backend Fallback

No futuro, poderia haver fallback:
```
cmake --preset macos-arm64 -DUSE_DXVK=ON (default)
├─ Tenta carregar: DXVKGraphicsBackend (Vulkan)
├─ Se falhar, fallback para: LegacyGraphicsBackend (Metal/OpenGL)
└─ WinMain.cpp g_useMetalBackend escolhe qual usar
```

Mas isso **não está implementado atualmente** - é tudo ou nada em compile-time.

## Conclusão

✅ **Seu backend artesanal Metal ainda existe e funciona!**
- Compilado quando `USE_DXVK=OFF`
- Padrão: `USE_DXVK=ON` (Vulkan)
- Para forçar Metal: `cmake --preset macos-arm64 -DUSE_DXVK=OFF`

A mensagem confusa "backend = METAL" é apenas uma decisão de tempo de execução que é **overridden** pela decisão de compile-time quando `USE_DXVK=ON`.

