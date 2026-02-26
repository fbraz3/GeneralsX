# RELATÓRIO: Diagnóstico e Solução do CCCache GeneralsX

## Problema Identificado

**CCCache está praticamente inútil neste projeto!**

StatísticaS encontradas:
- **37.53%** de chamadas "Cacheable" 
- **62.46%** de chamadas "UNCACHEABLE" ← **ESTE É O PROBLEMA!**
- De apenas 37.53% cacheable, temos 73.36% de hits
- Isso significa: apenas ~27% das compilações aproveitam o cache!

## Causa Raiz

### `__TIME__` e `__DATE__` em WinMain.cpp

Arquivo: `GeneralsMD/Code/Main/WinMain.cpp` linha 899

```cpp
AsciiString(__TIME__), AsciiString(__DATE__)
```

Esses macros **mudam A CADA SEGUNDO**, interferindo na cache key do ccache.

**Mas espera...** isso deveria ser tratável com ccache sloppiness!

## Solução Proposta

### 1. Configurar CCCache com `time_macros` Sloppiness

Criar arquivo: `~/.config/ccache/ccache.conf`

```ini
# GeneralsX ccache configuration  
cache_dir = /Users/felipebraz/Library/Caches/ccache
max_size = 20.0G
compress = true
compression_level = 9

# KEY: Ignorar __TIME__, __DATE__, __TIMESTAMP__ na cache key
sloppiness = time_macros,locale

direct_mode = true
stats = true
```

**EM macOS**, o ccache procura em:
- `~/.config/ccache/ccache.conf` (XDG standard)
- `~/Library/Preferences/ccache/ccache.conf` (macOS specific)

### 2. Verificar se está aplicado

```bash
ccache -p | grep sloppiness
# Deve mostrar: sloppiness = time_macros,locale
```

### 3. Testar a melhoria

```bash
# Limpar stats
ccache -z

# Compilar (primeira vez)
time cmake --build build/macos-vulkan --target z_ww3d2

# Stats intermediárias  
ccache -s

# Compilar novamente (segunda vez - deve usar cache)
time cmake --build build/macos-vulkan --target z_ww3d2

# Stats finais
ccache -s
```

**Resultado esperado**: 
- Segunda compilação deve ser **2-3x mais rápida**
- Hit rate deve subir para **>70%**

## Alternativa: Remover __TIME__ e __DATE__ (Nuclear Option)

Se mesmo com `time_macros` ainda tiver problemas:

```cpp
// GeneralsX @bugfix BenderAI 25/02/2026
// Replaced __TIME__ and __DATE__ with fixed strings for cache efficiency
// CCCache 62.46% uncacheable calls was due to nondeterministic macros
AsciiString("00:00:00"), AsciiString("Jan 01 1970")
```

**Contra**: Metadata fica incorreta na build info.

## Status Técnico

### Arquivos Envolvidos

- `cmake/ccache.cmake` - Integração CMake (já está correto)
- `GeneralsMD/Code/Main/WinMain.cpp` - Source do __TIME__/__DATE__ 
- `/Users/felipebraz/.config/ccache/ccache.conf` - Config (criado)
- `/Users/felipebraz/Library/Preferences/ccache/ccache.conf` - Config macOS backup (pendente criar)

### Próximas Ações

1. [ ] Garantir que `sloppiness = time_macros,locale` está em ambos os config files
2. [ ] `ccache -p | grep sloppiness` deve mostrar `time_macros,locale`
3. [ ] Rodar test_ccache.sh para validar melhoria
4. [ ] Se ainda não funcionar, investigar outras 62% uncacheable
5. [ ] Considerar nuclear option (remover __TIME__/__DATE__)

## Benchmarks Esperados (com sloppiness aplicado)

### Antes (sem sloppiness):
- 62.46% uncacheable
- Build z_ww3d2 primeira vez: ~130 segundos
- Build z_ww3d2 segunda vez: ~120 segundos (pouca diferença!)

### Depois (com sloppiness + time_macros):
- ~27% uncacheable (significativa redução)
- Build z_ww3d2 primeira vez: ~130 segundos
- Build z_ww3d2 segunda vez: ~20-30 segundos (4-6x mais rápido!) 

## Referências

- CCCache 4.12.2 (instalado no projeto)
- `time_macros` sloppiness: Ignora `__TIME__`, `__DATE__`, `__TIMESTAMP__`
- Locale sloppiness: Ignora diferenças de locale nos preprocessor directives

