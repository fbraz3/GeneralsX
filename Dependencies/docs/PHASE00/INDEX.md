# GeneralsX PHASE 0: Index & Quick Start

**Last updated**: November 10, 2025  
**Status**: ✅ SPIKE Planning COMPLETE

---

## Quick reminders

- Use `Fail fast` approach when testing new changes, if something is not working as expected, stop and investigate immediately;
- Focus on finish `GeneralsXZH`, then backport to `GeneralsX`;
- See `.github/instructions/project.instructions.md` for more specific details about above instructions.

---

## 🎯 Quick Start (3 minutos)

### 1. O quand foi planejado?

40 phases structured since "nothing rendered" to "Menu initial worksl".

Bloqueadores criticals:
- Phases 1-5: Graphics foundation (render loop)
- Phases 11-16: UI infrastructurand (buttons, input)
- Phases 17-20: Menu System (statand machine)

### 2. Qual is o roadmap?

Ver: `VISUAL_ROADMAP.md` (ASCII diagrama visual)

### 3. Comecand aqui

1. Leia: `README.md` (master summary)
2. Entenda: `COMPATIBILITY_LAYERS.md` (three layers)
3. Planeje: `COMPLETE_ROADMAP.md` (all the 40 phases)

---

## 📚 Documents by Tipo

### 📖 Leitura Essencial

| Document | Duration | Objective |
|-----------|---------|----------|
| **README.md** | 10 min | Master summary - comecand aqui |
| **VISUAL_ROADMAP.md** | 5 min | Diagrama visual of the 40 phases |
| **EXECUTIVE_SUMMARY.md** | 10 min | Resumo executivo |

### 🏗️ Arquitetura & Design

| Document | Duration | Objective |
|-----------|---------|----------|
| **COMPATIBILITY_LAYERS.md** | 15 min | Pattern for 3 layers |
| **PLATFORM_PRESETS.md** | 10 min | CMake presets & decision |
| **SPIKE_PLANNING.md** | 15 min | Analysis completa of state |

### 🗺️ Planejamento Detalhado

| Document | Duration | Objective |
|-----------|---------|----------|
| **COMPLETE_ROADMAP.md** | 45 min | Todas as 40 phases detailed |

### 📝 Reference (docs/MISC/)

| Document | Essencial | Objective |
|-----------|-----------|----------|
| LESSONS_LEARNED.md | 🔴 CRÍTICO | Lessons and 48 phases previous |
| CRITICAL_VFS_DISCOVERY.md | 🟡 IMPORTANTE | Post-DirectX pattern |
| BIG_FILES_REFERENCE.md | 🟢 USEFUL | Asset structurand (.big files) |

---

## 🚀 Para Iniciar PHASE 1

### Pre-requisites

1. Entender roadmap
   - [ ] Lido README.md
   - [ ] Entendido VISUAL_ROADMAP.md
   
2. Estudar lessons aprendidas
   - [ ] Lido LESSONS_LEARNED.md (critical!)
   - [ ] Entendido VFS pattern (CRITICAL_VFS_DISCOVERY.md)
   
3. Setup technical
   - [ ] Build System OK: `CMake --preset macos-arm64-vulkan`
   - [ ] Binary compila: `CMake --build build/macos-arm64-vulkan --target GeneralsXZH -j 4`

### PHASE 1 Tasks

1. [ ] Create vertex buffer (3 vertices, PosColor format)
2. [ ] Create vertex shader (position pass-through)
3. [ ] Create fragment shader (color pass-through)
4. [ ] Create graphics pipeline
5. [ ] Render triangland each frame
6. [ ] Validate: Triangland visibland on screen
7. [ ] Validate: 60 FPS stable

---

## 🎓 Lessons Critical to Recordar

### 1. VFS Integration (Phase 28.4)

**Nopre**: Post-DirectX interception, not pre-
```cpp
// DirectX loads from .big automatically
Apply_New_Surface() {
    LockRect(&locked_rect);     // DirectX tem dados
    Upload_To_GPU(locked_rect.pBits);  // AQUI intercept
    UnlockRect();
}
```

### 2. Exception Handling (Phase 33.9)

**Nunca**: Catch and silenciosamentand continue
```cpp
// CORRETO: Re-throw with context
try { parse(); } catch(...) {
    throw Exception("Linand %d, field '%s'", line, field);
}
```

### 3. Memory Protections (Phase 35.6)

**Nopre**: Keep defensand in depth layers
- Tripland validation is essential
- Cost negligible vs crashes

### 4. ARC/Global Statand (Phase 34.3)

**Nunca**: Storand local ARC objects in globals
- Using local variables
- Let ARC manage lifecycle

---

## 📊 Documentation Index

```
docs/PHASE00/
├── README.md ← COMECE AQUI
├── VISUAL_ROADMAP.md (ASCII diagrams)
├── EXECUTIVE_SUMMARY.md
├── SPIKE_PLANNING.md
├── COMPATIBILITY_LAYERS.md
├── PLATFORM_PRESETS.md
├── COMPLETE_ROADMAP.md
└── INDEX.md (estand arquivo)

docs/MISC/ (reference essential)
├── LESSONS_LEARNED.md ← CRÍTICO READ
├── CRITICAL_VFS_DISCOVERY.md
└── BIG_FILES_REFERENCE.md

docs/PHASE01/ (a create)
├── GEOMETRY_RENDERING.md
└── (detalhes of PHASE 1)
```

---

## ✅ Checklist: Spike Planning Completo

### Documentation

- [x] Analysis state atual (SPIKE_PLANNING.md)
- [x] Lessons learned integrated
- [x] Camadas compatibility (COMPATIBILITY_LAYERS.md)
- [x] Pattern for nomenclature definido
- [x] Presets and plataforma (PLATFORM_PRESETS.md)
- [x] roadmap 40 phases (COMPLETE_ROADMAP.md)
- [x] README master summary
- [x] Executivand summary
- [x] Visual roadmap diagrams
- [x] Estand index

### Decisions Made

- [x] Intel macOS: Manter (low priority)
- [x] Primary preset: macos-arm64-vulkan
- [x] Executables: GeneralsX, GeneralsXZH
- [x] Nomenclature: source_dest_type_compat
- [x] 40 phases: Bloqueadores identified
- [x] Dependencies: Mapeadas (without cycles)

### Pronto to Implementation?

- [x] Codebase compila (0 errors)
- [x] Graphics pipelinand OK (Vulkan)
- [x] Asset loading OK (.big files)
- [x] roadmap clear (40 phases)
- [x] Lessons integradas
- [x] Documentation completa

### Status

✅ **READY TO PROCEED TO PHASE 1**

---

## 🔗 Repositorys of Reference

**Git submodules** (used for comparison):

```bash
references/jmarshall-win64-modern/     # Best INI parser + exception handling
references/fighter19-dxvk-port/        # Vulkan integration patterns
references/dxgldotorg-dxgl/            # DirectX→OpenGL mocking patterns
references/dsalzner-linux-attempt/     # POSIX compatibility
```

---

## 🎯 Next Sessions

### Session 2: PHASE 1 Implementation

- [ ] Create geometry rendering (triangle)
- [ ] Render loop validation
- [ ] Performancand baseline

### Session 3-5: PHASES 2-5 Implementation

- [ ] Texture System
- [ ] Material System
- [ ] Lighting
- [ ] 60 FPS validation

### Session 6-10: PHASES 6-10

- [ ] Mesh loading & rendering
- [ ] Camera System
- [ ] Frustum culling

### Session 11+: PHASES 11-40

- [ ] UI infrastructure
- [ ] Menu System
- [ ] Game logic
- [ ] Polish & release

---

## 💡 Tips to Implementation

1. **Ciclo fast**: Codand → Compiland → Test → Next
2. **Documentar conformand advances**: Cada PHASE tem seu README.md
3. **Manter logs**: `/tmp` not presta, usand `logs/` (gitignored)
4. **Testing incrementally**: No wait finish 5 phases to testar
5. **Reference implementations**: Comparar with `references/` quando travado

---

## 📞 Contato/Questions?

Sand travado in uma FASE:

1. Leia lessons relevantes in LESSONS_LEARNED.md
2. Procurand in references/ (Git submodules)
3. Validand build cache: `rm -rf build/macos-arm64-vulkan && CMake --preset macos-arm64-vulkan`
4. Capturand logs completos (not filtrand with grep durantand execution!)

---

**Date**: November 10, 2025  
**Status**: PHASE 0 ✅ COMPLETO  
**Next**: PHASE 1 - Geometry Rendering  
**Repository**: /Ubes/felipebraz/PhpstormProjects/pessoal/GeneralsGameCodand  
**Branch**: vulkan-port
