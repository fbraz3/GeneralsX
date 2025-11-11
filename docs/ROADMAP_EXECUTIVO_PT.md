# 🎯 Roadmap Executivo: Tela Inicial Funcional com Vulkan

**Data**: 11 de Novembro 2025  
**Status**: ✅ Plano Completo Pronto para Implementação  
**Estratégia**: Vulkan-only cross-platform (macOS/Linux/Windows)

---

## 📊 Visão Geral: 21 Phases até MVP

```
PHASE 00.5 ✅ (Build)
    ↓ [~1 dia]
PHASE 01-07 (Vulkan Core)      [2-3 weeks]
    ↓ [render pipeline estável]
PHASE 08 (Asset Loading)        [3-4 dias]
    ↓ [textures/meshes carregando]
PHASE 09 (INI Parser Fix) 🚨    [2-3 dias] ← CRÍTICO!
    ↓ [configs lendo correto]
PHASE 10-15 (Rendering)         [2-3 weeks]
    ↓ [geometria renderizando]
PHASE 16-20 (Menu System)       [2-3 weeks]
    ↓ [menu inicializado]
PHASE 21 (Main Menu MVP)        [2-3 dias]
    ↓ [tudo conectado]
🎉 TELA INICIAL FUNCIONAL!
```

**Total: 8-11 semanas até MVP**

---

## 🚨 Bloqueador Crítico Identificado

### Phase 09: INI Parser Fix

**Problema**:
- Código usa `catch(...)` blanket (engole TODAS exceções)
- Resultado: campos ficam com valores padrão (0.00, '')
- Audio volumes 0.00 → sem som
- Audio filenames '' → sem música
- Menu INI quebrado → menu não aparece

**Localização**: `Core/Libraries/Source/WWVegas/WWLib/Source/Ini.cpp`

**Solução** (2-3 dias):
1. Remover `catch(...)`
2. Implementar re-throw específico com contexto
3. Testar AudioSettings.ini + Music.ini
4. Validar menu parsing

**Impacto**: Bloqueia Phase 16-20 (menu system) inteira

**Referência**: `docs/Misc/INI_PARSER_BLOCKER_ANALYSIS.md` (análise completa)

---

## 📋 As 21 Phases

### Grupo 1: Vulkan Core (Phases 01-07) [2-3 weeks]

| Phase | Nome | Objetivo |
|-------|------|----------|
| 01 | Instance & Device | Inicializar Vulkan |
| 02 | Command Buffers | Recording de comandos |
| 03 | Shader System | GLSL → SPIR-V compilation |
| 04 | Vertex/Index Buffers | GPU buffers allocation |
| 05 | Textures & Sampling | Load + bind textures |
| 06 | Pipeline State | Graphics pipeline |
| 07 | Drawing & Presentation | Frame rendering |

### Grupo 2: Asset Loading (Phase 08) [3-4 days]

| Phase | Nome | Objetivo |
|-------|------|----------|
| 08 | Asset Loading | VFS + mesh loading + error handling |

### Grupo 3: Critical Fix (Phase 09) [2-3 days] 🚨

| Phase | Nome | Objetivo |
|-------|------|----------|
| 09 | INI Parser Fix | Remove exception swallowing |

### Grupo 4: Rendering Systems (Phases 10-15) [2-3 weeks]

| Phase | Nome | Objetivo |
|-------|------|----------|
| 10 | Window & Input | SDL2 + Vulkan surface |
| 11 | Camera System | View/projection matrices |
| 12 | Lighting | Phong shaders |
| 13 | Mesh Rendering | Multi-mesh support |
| 14 | Materials & Textures | Descriptor sets |
| 15 | GameObject System | Entity abstraction |

### Grupo 5: Menu System (Phases 16-20) [2-3 weeks]

| Phase | Nome | Objetivo |
|-------|------|----------|
| 16 | Menu INI Parser | Parse MenuDefinition.ini |
| 17 | State Machine | MainMenu + transitions |
| 18 | UI Rendering | 2D ortho projection |
| 19 | Button Interaction | Clicking + keyboard |
| 20 | Audio Integration | Music + sounds |

### Grupo 6: MVP Integration (Phase 21) [2-3 days]

| Phase | Nome | Objetivo |
|-------|------|----------|
| 21 | Main Menu MVP | Tie everything together |

---

## ✨ Lições Aprendidas (Consolidadas)

### 1. VFS Discovery (Oct 17)
- ❌ Problema: Assumed "fix DirectX loader to use VFS"
- ✅ Solução: "Intercept AFTER DirectX loads via VFS"
- 📊 Resultado: 7 textures loaded sem modificação VFS
- 🔑 Lição: Entender arquitetura existente antes de modificar

### 2. INI Parser Silent Failures (Oct 19)
- ❌ Problema: `catch(...)` engole exceções
- ✅ Solução: Re-throw específico com contexto
- 📊 Resultado: Audio + menu bloqueados
- 🔑 Lição: Nunca swallow exceptions silenciosamente

### 3. ARC Memory Management (Oct 24)
- ❌ Problema: Storing local ARC objects em globals
- ✅ Solução: Deixe ARC gerenciar ciclo de vida
- 📊 Resultado: Crashes em Metal driver resolvidos
- 🔑 Lição: Confia em ARC, não force retention

### 4. Vulkan Viability (Oct 30)
- ✅ Phase 38 interface (47 métodos) = 1:1 mapping Vulkan
- ✅ DXVK viável em macOS ARM64
- ✅ MoltenVK + Vulkan SDK testado & working
- 🔑 Lição: Arquitetura bem-desenhada permite mudanças

---

## 📚 Documentação

### Principal
- **`docs/MAIN_SCREEN_ROADMAP.md`** (1000+ linhas)
  - Cada phase com: dependências, deliverables, validação, tech details
  - Timeline per-phase
  - Critical success factors

- **`docs/PHASE_INDEX.md`** (atualizado)
  - 21 phases Vulkan-only
  - Dependency graph
  - Status tracking

### Referências Críticas
- `docs/MISC/LESSONS_LEARNED.md` - 4 lições consolidadas
- `docs/MISC/INI_PARSER_BLOCKER_ANALYSIS.md` - Phase 09 análise
- `docs/MISC/VULKAN_ANALYSIS.md` - Arquitetura Vulkan
- `docs/MISC/BIG_FILES_REFERENCE.md` - Asset structure
- `docs/MISC/CRITICAL_VFS_DISCOVERY.md` - VFS breakthrough

---

## 🎯 Timeline Estimada

| Milestone | Duração | Cumulativo | Dependências |
|-----------|---------|-----------|--------------|
| Vulkan Core (01-07) | 2-3 weeks | 2-3 weeks | PHASE00.5 ✅ |
| Asset + INI (08-09) | 1 week | 3-4 weeks | 01-07 |
| Rendering (10-15) | 2-3 weeks | 5-7 weeks | 08-09 |
| Menu System (16-20) | 2-3 weeks | 7-10 weeks | 10-15 |
| Final (21) | 2-3 days | **7-10 weeks** | 16-20 |

**TOTAL: 8-11 semanas** até main menu funcional com Vulkan

---

## 🔑 Critical Success Factors

### 1. Phase 09 (INI Parser) é Bottleneck
- Se tomar 1 week: menu system atrasado 1 week
- Se fix 2-3 days: no schedule
- **Prioridade máxima** após Phase 08

### 2. Vulkan Foundation Deve Ser Robust
- Fases 01-07 são 40% do esforço
- Se arquitetura wrong: retrabalho massivo
- Validação cuidadosa de cada component

### 3. Menu System Depende de Tudo
- Phase 16-20 requerem:
  - Rendering estável (Phase 10-15)
  - INI parser correto (Phase 09)
  - Audio funcionando (Phase 20)

### 4. Testing Incrementado é Essencial
- Cada phase: validação imediata
- Não acumular bugs
- Logs estruturados (tee to files)

---

## ✅ Próximos Passos

### Hoje (Nov 11)
1. ✅ Análise completa
2. ✅ Plano detalhado criado
3. ✅ Documentação pronta

### Esta Semana
4. ⏳ Review & approve plano
5. ⏳ Criar Phase 01-21 READMEs detalhados
6. ⏳ Setup inicial Phase 01

### Próximas 2 Semanas
7. ⏳ Iniciar Phase 01 implementation
8. ⏳ Atualizações diárias em `docs/MACOS_PORT_DIARY.md`
9. ⏳ Completar Phases 01-07

### Semana 4-5
10. ⏳ Phase 08 (asset loading)
11. ⏳ Phase 09 (INI fix) 🚨
12. ⏳ Validar audio + menu configs

### Semana 6-10
13. ⏳ Phases 10-21 (rendering + menu)
14. ⏳ Main menu MVP funcional
15. 🎉 **TELA INICIAL FUNCIONAL!**

---

## 📞 Referências Rápidas

**Vulkan Documentation**:
- Local: `docs/Vulkan/` (Vulkan SDK 1.4.328.1)
- Online: https://vulkan-tutorial.com/

**Asset Structure**:
- `docs/MISC/BIG_FILES_REFERENCE.md`
- `.big files`: INI.big, INIZH.big, Textures.big, Music.big, Window.big

**Build System**:
- Presets: `macos-arm64`, `macos-x64`, `linux`, `windows`
- ccache enabled (10GB cache)
- `scripts/build_zh.sh` para builds

**Development Diary**:
- `docs/MACOS_PORT_DIARY.md` - update daily

---

## 🎖️ Status Geral

| Aspecto | Status | Observação |
|---------|--------|-----------|
| Planejamento | ✅ Completo | 21 phases mapeadas |
| Arquitetura | ✅ Aprovada | Vulkan-only, Vulkan SDK installed |
| Build System | ✅ Funcional | CMake, ccache, cross-platform |
| Asset Loading | ✅ Working | VFS verified, 7 textures loaded |
| Bloqueadores | 🚨 Crítico | Phase 09 (INI parser) must fix |
| Documentação | ✅ Completa | 50KB+ análise consolidada |
| **Status MVP** | ⏳ Pronto | Start Phase 01 após approval |

---

## 💡 Insights Principais

1. **Decisão Vulkan foi correta**
   - DirectX 8 → Vulkan 1:1 mapping (no surprises)
   - Cross-platform viável (MoltenVK tested)
   - DXVK pode ser fallback se needed

2. **Arquitetura existente é sólida**
   - Build system robust
   - VFS working (7 textures prove it)
   - Audio 100% done (apenas bloqueado por INI)

3. **Phase 09 é projeto differentiator**
   - Se fix rápido: menu development on schedule
   - Se take 1 week: entire project delayed
   - Deve ser prioridade máxima

4. **Testing strategy é crítico**
   - Each phase needs immediate validation
   - Logs essential (tee to files, grep later)
   - Incremental progress tracking

---

## 🚀 Conclusão

**Temos um plano claro, realista e pronto para execução.**

- 21 phases bem-definidas
- 8-11 semanas até MVP
- Bloqueadores identificados e mitigáveis
- Documentação completa
- Próximos passos óbvios

**Pronto para começar Phase 01 após aprovação deste plano.**

---

**Documento**: `docs/ROADMAP_EXECUTIVO_PT.md`  
**Data**: 11 de Novembro 2025  
**Responsável**: Análise detalhada completa  
**Status**: ✅ **PRONTO PARA IMPLEMENTAÇÃO**

