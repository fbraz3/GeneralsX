# Logs Directory

Este diretório contém logs de debug e execução do GeneralsXZH para análise de problemas e validação de funcionalidades.

## Arquivos Ignorados pelo Git

Os seguintes padrões estão no `.gitignore`:
- `*.log` - Arquivos de log gerais
- `*.txt` - Arquivos de texto temporários

## Logs Gerados Durante Debug (Outubro 17, 2025)

### Graphics Crash Investigation

#### `build_window_debug.log`
- **Conteúdo**: Saída de compilação com debug logs adicionados
- **Resultado**: 115 warnings (37 pre-existentes), compilação bem-sucedida
- **Arquivos modificados**: W3DDisplay.cpp com logs de criação de janela

#### `window_creation.log`
- **Conteúdo**: Log inicial de execução (150 linhas, timeout 8s)
- **Resultado**: Capturado carregamento de .big files, mas timeout cortou antes de W3DDisplay::init()
- **Lição**: Timeouts curtos inadequados para captura completa de inicialização

#### `full_init.log`
- **Conteúdo**: Log completo de execução sem timeout
- **Resultado**: **SUCESSO** - Janela criada, Metal inicializado, game loop rodando
- **Evidências chave**:
  ```
  W3DDisplay::init - About to create SDL window (800x600, flags=0x24)...
  W3DDisplay::init - SDL window created successfully (handle=0xa62db1a00)
  Phase 29: Metal backend initialized successfully
  ```
- **Performance**: ~30 FPS, BeginFrame/EndFrame cycling continuamente

## Conclusão da Investigação

✅ **Nenhum crash existe** - O jogo roda perfeitamente com backend Metal  
✅ **SDL2 Window**: Criada com sucesso (800x600, windowed mode)  
✅ **Metal Backend**: Inicializado e renderizando frames  
✅ **Game Loop**: Todos os subsistemas operacionais  

**Root Cause da Confusão**: 
- Logs com timeout curto aparentavam crash quando na verdade o jogo estava inicializando normalmente
- Grep em logs repetitivos (BeginFrame/EndFrame) ocultou progressão através das fases de inicialização
- O crash original AGXMetal13_3 já havia sido resolvido pela Phase 30.6 (memory protections)

## Como Usar Este Diretório

```bash
# Executar jogo e salvar log completo
cd $HOME/GeneralsX/GeneralsMD
USE_METAL=1 ./GeneralsXZH 2>&1 | tee /path/to/GeneralsGameCode/logs/nome_do_teste.log

# Executar compilação e salvar log
cd /path/to/GeneralsGameCode
cmake --build build/macos-arm64 --target GeneralsXZH -j 4 2>&1 | tee logs/build_$(date +%Y%m%d_%H%M%S).log

# Buscar por patterns específicos
grep -i "error\|fatal\|crash" logs/full_init.log
grep -i "metal\|window\|sdl" logs/full_init.log | head -50
```

## Boas Práticas

1. **Não use timeouts curtos** para captura inicial - deixe o jogo rodar até estabilizar
2. **Use nomes descritivos** para logs (ex: `metal_texture_test_20251017.log`)
3. **Documente o contexto** em comentários no topo do log quando necessário
4. **Mantenha logs relevantes** para referência futura de bugs resolvidos
5. **Delete logs antigos** periodicamente para economizar espaço
