# Plano B — Usar referência fighter19 como workaround enquanto cmake executa

## Contexto
Se o `cmake --preset macos-vulkan` levar > 20 min, podemos usar a build fighter19 como base.

## Passos

### 1. Verificar se fighter19 tem DXVK compilado
```bash
ls -la references/fighter19-dxvk-port/GeneralsMD/Run/
# Deve ter libdxvk_d3d8.0.dylib + libdxvk_d3d9.0.dylib
```

### 2. Se sim, copiar para nossa build
```bash
mkdir -p build/macos-vulkan-backup
cp -r build/macos-vulkan build/macos-vulkan-backup

# copiar DXVK dylibs
cp references/fighter19-dxvk-port/GeneralsMD/Run/libdxvk_*.d*.llib \
   build/macos-vulkan/_run/ 2>/dev/null || echo "Not available"
```

### 3. Se fighter19 não tem build, clonar e compilar
```bash
cd references/fighter19-dxvk-port
cmake --preset linux64-deploy  # Para builds nativas
# OU
cmake --preset macos-vulkan    # Se tiver preset macOS
cmake --build build/macos-vulkan --target z_generals
```

### 4. Se tudo falhar, usar build anterior
```bash
git log --oneline build/macos-vulkan/ 2>/dev/null | head -5
# Tentar resetar para commit anterior que tinha DXVK compilado
```

## Verificação Rápida
```bash
file build/macos-vulkan/_deps/dxvk-src/src/dxso/dxso_compiler.cpp 2>/dev/null && \
  echo "DXVK source fetched" || echo "DXVK not yet ready"
```
