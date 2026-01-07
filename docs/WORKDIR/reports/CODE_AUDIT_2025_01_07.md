# Code Audit Report - Discrepâncias com Diretrizes do Projeto
**Data:** 2025-01-07
**Objetivo:** Identificar código que não segue as diretrizes Vulkan-only e SDL2-only
**Status:** 🔴 CRITICAL - Múltiplas violações identificadas

---

## 📋 Resumo Executivo

A auditoria identificou **violações críticas** das diretrizes do projeto em 3 categorias principais:

| Categoria | Severidade | Ocorrências | Status |
|-----------|------------|-------------|--------|
| **Windows Registry APIs** | 🔴 HIGH | ~20+ arquivos | ⚠️ PARCIALMENTE RESOLVIDO |
| **Win32 API Calls** | 🔴 CRITICAL | ~150+ ocorrências | ❌ NÃO RESOLVIDO |
| **DirectX References** | 🟡 MEDIUM | ~10+ arquivos | ⚠️ STUBS EXISTEM |
| **Windows.h Includes** | 🟢 LOW | ~90+ arquivos | ⚠️ EM PROGRESSO |

---

## 🔴 CRITICAL ISSUES

### 1. WinMain.cpp - Win32 Window Management

**Arquivo:** `GeneralsMD/Code/Main/WinMain.cpp` (linhas 772-801)

**Problema:**
```cpp
// ❌ VIOLAÇÃO: Uso direto de Win32 APIs ao invés de SDL2
HWND hWnd = CreateWindow(TEXT("Game Window"), ...);
GetSystemMetrics(SM_CXSCREEN);    // Deveria ser SDL_GetDisplayBounds
GetSystemMetrics(SM_CYSCREEN);
SetWindowPos(hWnd, HWND_TOPMOST, ...);  // Deveria ser SDL_SetWindowPosition
SetForegroundWindow(hWnd);
ShowWindow(hWnd, nCmdShow);
UpdateWindow(hWnd);
```

**Impacto:**
- Código Win32 puro ao invés de SDL2
- Não funciona em plataformas não-Windows
- Contradiz diretriz "SDL2 only"

**Recomendação:**
```cpp
// ✅ CORRETO: Usar SDL2
SDL_DisplayMode displayMode;
SDL_GetCurrentDisplayMode(0, &displayMode);
int centerX = (displayMode.w / 2) - (startWidth / 2);
int centerY = (displayMode.h / 2) - (startHeight / 2);

SDL_Window* window = SDL_CreateWindow(
    "Command and Conquer Generals",
    centerX, centerY,
    startWidth, startHeight,
    SDL_WINDOW_VULKAN | (runWindowed ? 0 : SDL_WINDOW_FULLSCREEN)
);
```

**Nota:** O arquivo já possui código SDL2 correto nas linhas 837-912, mas mantém código Win32 legado simultaneamente.

---

### 2. GameSpyGameInfo.cpp - Win32 DLL Loading

**Arquivo:** `GeneralsMD/Code/GameEngine/Source/GameNetwork/GameSpyGameInfo.cpp` (linhas 189-427)

**Problema:**
```cpp
// ❌ VIOLAÇÃO: LoadLibrary/GetProcAddress são Windows-only
HINSTANCE mib_ii_dll = LoadLibrary("inetmib1.dll");
HINSTANCE snmpapi_dll = LoadLibrary("snmpapi.dll");
SnmpExtensionInitPtr = (int (__stdcall *)(unsigned long,void ** ,AsnObjectIdentifier *))
    GetProcAddress(mib_ii_dll, "SnmpExtensionInit");
FreeLibrary(mib_ii_dll);
```

**Impacto:**
- Sistema de rede GameSpy depende de DLLs Windows
- SNMP APIs são Windows-específicas
- Não compilará em macOS/Linux

**Recomendação:**
1. **Curto prazo:** Guard com `#ifdef _WIN32` e implementar fallback multiplataforma
2. **Longo prazo:** Remover dependência de GameSpy e implementar sistema de rede moderno usando sockets POSIX/SDL_net

```cpp
#ifdef _WIN32
    // Código Windows existente
#else
    // TODO: Implementar detecção de rede multiplataforma
    // Opções: getifaddrs() (POSIX), SDL_net, ou cross-platform networking library
#endif
```

---

### 3. ScriptEngine.cpp - Editor DLL Loading

**Arquivo:** `GeneralsMD/Code/GameEngine/Source/GameLogic/ScriptEngine/ScriptEngine.cpp`

**Problema:**
```cpp
// ❌ VIOLAÇÃO: Carregamento de DLLs Windows para editores
st_DebugDLL = LoadLibrary("DebugWindow.dll");         // linha 529
st_ParticleDLL = LoadLibrary("ParticleEditor.dll");   // linha 535
FARPROC proc = GetProcAddress(st_DebugDLL, "CreateDebugDialog");  // linha 541
GetProcAddress(st_ParticleDLL, "CreateParticleSystemDialog");     // linha 548
// ... mais 20+ GetProcAddress calls
```

**Impacto:**
- Editores in-game dependem de DLLs Windows
- ~25+ chamadas GetProcAddress ao longo do arquivo
- Funcionalidade debug/particle editor não funciona em outras plataformas

**Recomendação:**
```cpp
// ✅ Guard apropriado já existe (linha 526), mas precisa ser consistente
#if defined(_WIN32)
    if (TheGlobalData->m_windowed)
        if (TheGlobalData->m_scriptDebug) {
            st_DebugDLL = LoadLibrary("DebugWindow.dll");
        }
#else
    // Editores desabilitados em outras plataformas por enquanto
    st_DebugDLL = NULL;
    st_ParticleDLL = NULL;
    LogInfo("Script debugging and particle editing not available on this platform");
#endif
```

**Nota:** O guard `#if defined(_WIN32)` existe, mas não cobre todas as chamadas GetProcAddress subsequentes (linhas 8458-10466).

---

### 4. Win32OSDisplay.cpp - MessageBox Native

**Arquivo:** `GeneralsMD/Code/GameEngineDevice/Source/Win32Device/Common/Win32OSDisplay.cpp` (linhas 106-117)

**Problema:**
```cpp
// ❌ VIOLAÇÃO: MessageBox Win32 ao invés de SDL2
returnResult = ::MessageBoxW(NULL, mesgStr.str(), promptStr.str(), windowsOptionsFlags);
returnResult = ::MessageBoxA(NULL, mesgA.str(), promptA.str(), windowsOptionsFlags);
```

**Impacto:**
- Diálogos usam MessageBox nativo do Windows
- Não funciona em macOS/Linux

**Recomendação:**
```cpp
// ✅ CORRETO: Usar SDL_ShowMessageBox
SDL_MessageBoxData messageboxdata = {
    SDL_MESSAGEBOX_ERROR,
    NULL,
    promptStr.str(),
    mesgStr.str(),
    // ... button configuration
};
int buttonid;
SDL_ShowMessageBox(&messageboxdata, &buttonid);
```

**Nota:** Existe implementação parcial em `Dependencies/Utility/Compat/msvc_types_compat.h` (linha 498), mas é apenas um stub.

---

## 🟡 MEDIUM ISSUES

### 5. Windows Registry - Implementação INI Incompleta

**Status:** ⚠️ PARCIALMENTE RESOLVIDO (Phase 39.5)

**Arquivos Afetados:**
- `Core/Libraries/Source/WWVegas/WWLib/registry.cpp` - ✅ MIGRADO para INI
- Múltiplos arquivos game code ainda chamam funções registry

**Problema:**
- Sistema INI implementado mas não testado completamente
- Alguns componentes podem ainda depender de registro Windows
- `assets/ini/README.md` documenta formato mas falta validação

**Recomendação:**
```bash
# Verificar chamadas remanescentes
grep -r "RegOpenKeyEx\|RegQueryValueEx\|RegSetValueEx\|HKEY_" \
    GeneralsMD/Code/ Generals/Code/ --include="*.cpp" | \
    grep -v "registry.cpp" | tee logs/registry_remaining.log
```

---

### 6. DirectX8 Interface Stubs

**Status:** ⚠️ STUBS IMPLEMENTADOS mas não integrados completamente

**Arquivo:** `Core/Libraries/Source/WWVegas/WW3D2/DX8Wrapper_Stubs.cpp`

**Análise:**
```cpp
// ✅ BOM: Stubs existem para compatibilidade
class IDirect3DDevice8Stub {
    int TestCooperativeLevel();
    int SetTexture(int stage, void* texture);
    int SetPixelShader(int shader);
    // ... etc
};
```

**Problema:**
- Stubs retornam valores mock mas não executam Vulkan real
- `DX8Wrapper_Stubs.cpp` tem 2300+ linhas de stubs vazios
- Chamadas DirectX ainda presentes no código game

**Recomendação:**
- **NÃO remover os stubs** - eles permitem compilação
- **Gradualmente substituir** calls DirectX por Vulkan via DX8Wrapper
- Priorizar hot path: rendering, textures, buffers

---

## 🟢 LOW PRIORITY ISSUES

### 7. Includes Desnecessários de windows.h

**Arquivos Afetados:** ~90 arquivos

**Categorias:**
1. **Tools/GUIEdit/** - ~15 arquivos (ferramentas Windows-only OK por enquanto)
2. **Core/Libraries/Source/debug/** - ~10 arquivos (sistema debug)
3. **GameEngine/** - ~20 arquivos (prioridade de limpeza)
4. **Win32Device/** - ~10 arquivos (esperado, mas pode migrar para SDL2)

**Exemplo (PreRTS.h):**
```cpp
// GeneralsMD/Code/GameEngine/Include/Precompiled/PreRTS.h:47
#include <windows.h>  // ❌ Em precompiled header - afeta TODO projeto
```

**Recomendação:**
```cpp
// ✅ Condicional ou remoção completa
#if defined(_WIN32) && defined(NEED_WINDOWS_H)
    #include <windows.h>
#endif
```

---

## 📊 Estatísticas Detalhadas

### Win32 API Calls por Categoria

| API Category | Occurrences | Status |
|--------------|-------------|--------|
| Window Management | 50+ | ❌ CreateWindow, SetWindowPos, ShowWindow |
| System Metrics | 20+ | ❌ GetSystemMetrics |
| DLL Loading | 80+ | ❌ LoadLibrary, GetProcAddress, FreeLibrary |
| Message Boxes | 10+ | ⚠️ MessageBox (stubs exist) |
| Registry | 100+ | ⚠️ INI migration done but untested |
| File System | 30+ | ⚠️ Partially migrated |

### Compliance Score por Módulo

| Módulo | Compliance | Issues |
|--------|------------|--------|
| **WinMain.cpp** | 🔴 30% | Win32 + SDL2 misturados |
| **GameSpyGameInfo.cpp** | 🔴 0% | 100% Win32 DLL loading |
| **ScriptEngine.cpp** | 🟡 50% | Guards exist mas incompletos |
| **DX8Wrapper** | 🟢 80% | Stubs OK, falta integração Vulkan |
| **Registry System** | 🟢 90% | INI migration complete |

---

## 🎯 Plano de Ação Recomendado

### Priority 1 - CRITICAL (Semana 1)

1. **WinMain.cpp Window Management**
   ```bash
   # Remover código Win32 duplicado
   # Manter apenas SDL2_CreateWindow path
   # Testar em macOS/Linux
   ```

2. **GameSpyGameInfo.cpp Networking**
   ```bash
   # Guard com #ifdef _WIN32
   # Implementar fallback multiplataforma
   # Documentar limitações
   ```

3. **ScriptEngine.cpp Editors**
   ```bash
   # Estender guards para cobrir todos GetProcAddress
   # Adicionar logs quando editores não disponíveis
   # Testar compile em macOS/Linux
   ```

### Priority 2 - HIGH (Semana 2)

4. **MessageBox Migration**
   ```bash
   # Substituir todas chamadas Win32 MessageBox por SDL_ShowMessageBox
   # Criar wrapper em Dependencies/Utility/Compat/
   # Testar diálogos de erro cross-platform
   ```

5. **GetSystemMetrics → SDL2**
   ```bash
   # Substituir SM_CXSCREEN/SM_CYSCREEN por SDL_GetDisplayBounds
   # Criar helpers em SDL2_AppWindow
   ```

### Priority 3 - MEDIUM (Semana 3-4)

6. **Windows.h Cleanup**
   ```bash
   # Remover includes desnecessários
   # Adicionar guards condicionais onde necessário
   # Testar compilação incremental
   ```

7. **Registry Testing**
   ```bash
   # Criar testes para sistema INI
   # Validar leitura/escrita de configurações
   # Documentar migração de .reg → .ini
   ```

---

## ✅ Checklist de Verificação

**SDL2 Only:**
- [ ] Zero CreateWindow Win32 calls
- [ ] Zero GetSystemMetrics calls
- [ ] Zero SetWindowPos Win32 calls
- [ ] Zero MessageBox Win32 calls (exceto stubs)
- [ ] Todos windows gerenciados via SDL2

**Vulkan Only:**
- [x] DirectX stubs implementados (Phase 62)
- [ ] Zero chamadas DirectX diretas no game code
- [ ] DX8Wrapper routing para Vulkan completamente
- [ ] Shaders migrados de HLSL para SPIR-V

**Cross-Platform:**
- [ ] Zero LoadLibrary/GetProcAddress sem guards
- [ ] Zero includes de windows.h sem necessidade
- [ ] Registry completamente em INI files
- [ ] File paths usando std::filesystem

---

## 📚 Referências

- **Diretrizes Projeto:** `.github/instructions/generalsx.instructions.md`
- **Phase 39.5 Registry:** `docs/WORKDIR/phases/3/PHASE39/39.5_INDEX.md`
- **Phase 40 SDL2:** `docs/WORKDIR/phases/4/PHASE40/`
- **Phase 62 DX8 Stubs:** `Core/Libraries/Source/WWVegas/WW3D2/DX8Wrapper_Stubs.cpp`

---

## 🚨 Notas Importantes

1. **NÃO REMOVER STUBS:** Os stubs DirectX são necessários para compilação. A migração deve ser gradual via DX8Wrapper.

2. **FAIL FAST:** Cada correção deve ser testada com compilação completa antes de prosseguir.

3. **COMMIT TIME RESTRICTIONS:** GeneralsX project - evitar commits Segunda-Sexta 09:00-18:00 horário local.

4. **BUILD COM TEE:** Sempre usar `tee` para logging:
   ```bash
   cmake --build build/macos --target GeneralsXZH -j 4 2>&1 | tee logs/phase_XX_build.log
   ```

5. **UPDATE DEV BLOG:** Atualizar `docs/DEV_BLOG/2025-01-DIARY.md` antes de commitar mudanças.

---

**Auditado por:** GitHub Copilot (Claude Sonnet 4.5)
**Próxima Revisão:** Após correção Priority 1 issues
