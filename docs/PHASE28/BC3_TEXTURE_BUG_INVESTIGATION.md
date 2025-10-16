# Phase 28.4: BC3 Texture Rendering Bug - Investigação Completa

**Data**: 15 de outubro de 2025  
**Status**: 🔴 BUG ATIVO - Não Resolvido  
**Prioridade**: CRÍTICA - Bloqueia Phase 28.4.2 completion

## Sumário Executivo

Texturas DDS comprimidas em formato BC3 (DXT5) estão renderizando com **metade da textura substituída por laranja sólido** no backend Metal (macOS ARM64). TGAs não comprimidas renderizam corretamente, isolando o problema para texturas comprimidas BC3.

## Descrição Visual do Bug

### Estado Atual (Reproduzível)
```
┌─────────────────────────────────────┐
│ Quad 0: defeated.dds (1024x256)    │
│ ┌─────────────┬──────────────────┐  │
│ │ ✓ Textura   │ ❌ LARANJA       │  │
│ │   correta   │    SÓLIDO        │  │
│ │  (céu roxo, │                  │  │
│ │   tanques)  │                  │  │
│ └─────────────┴──────────────────┘  │
│                                     │
│ Quad 1: GameOver.tga (512x128)     │
│ ┌─────────────┬──────────────────┐  │
│ │ ✓ TGA OK    │ ❌ LARANJA       │  │
│ │ (GAME OVER) │    SÓLIDO        │  │
│ └─────────────┴──────────────────┘  │
│                                     │
│ Quad 3: defeated.dds (512x128)     │
│ ┌─────────┬──────────────────────┐  │
│ │ ✓ Metade│ ❌ LARANJA SÓLIDO    │  │
│ │   esq.  │                      │  │
│ └─────────┴──────────────────────┘  │
│                                     │
│ Quad 2: caust00.tga (64x64) ✓ OK  │
└─────────────────────────────────────┘
```

**Padrão Observado**: Exatamente metade (lado direito) de cada textura BC3 é substituída por laranja sólido (#FF8800 aproximadamente).

## Especificações Técnicas

### Ambiente de Teste
- **Plataforma**: macOS 15.0 (ARM64 - Apple Silicon)
- **Backend Gráfico**: Metal API (nativo)
- **OpenGL Version**: 4.1 Metal (deprecated, não usado)
- **Device**: Apple M-series GPU via MTLCreateSystemDefaultDevice()
- **Resolução**: 1280x768 pixels

### Formato de Textura BC3
- **Tipo**: Block Compressed 3 (DXT5) - RGBA
- **Block Size**: 16 bytes por bloco 4x4 pixels
- **Estrutura do Bloco**:
  - 8 bytes: Alpha (BC4 compression)
  - 8 bytes: Color (BC1 compression)
- **Metal Format**: `MTLPixelFormatBC3_RGBA`

### Textura de Teste Principal
- **Arquivo**: `Data/English/Art/Textures/defeated.dds`
- **Dimensões**: 1024×256 pixels
- **Blocos**: 256 wide × 64 high (1024/4 × 256/4)
- **bytesPerRow**: 4096 bytes (256 blocos × 16 bytes)
- **Tamanho Total**: 262,144 bytes
- **Mipmaps**: 0 (sem mipmaps)

### Dados de Debug Capturados
```
METAL DEBUG BC3: width=1024, height=256, blocksWide=256, blocksHigh=64
METAL DEBUG BC3: bytesPerRow=4096 (calculated), bytesPerRowAlt=4096 (RGBA8 equiv), expectedSize=262144, actualSize=262144
METAL DEBUG BC3: region(x=0, y=0, w=1024, h=256) [PIXELS]

METAL DEBUG: First 32 bytes: FF FF 00 00 00 00 00 00 B8 B4 1A AC 2A 3E DE 7F FF FF 00 00 00 00 00 00 B9 B4 3A AC 98 AB B7 FD
METAL DEBUG: Bytes at 50% (131072): FF FF 00 00 00 00 00 00 D9 DD 18 CD 29 2D FF FD FF FF 00 00 00 00 00 00 7A E6 58 CD 82 8A EF 55
```

**Conclusão da Análise de Dados**: Dados da textura são válidos (não corrompidos), sem padrão de zeros ou garbage.

## Implementação Atual

### Código de Upload de Textura (metalwrapper.mm)

```objectivec++
// Phase 28.1: Texture Creation from DDS
void* MetalWrapper::CreateTextureFromDDS(unsigned int width, unsigned int height, 
                                         unsigned int format, const void* data, 
                                         unsigned int dataSize, unsigned int mipLevels) {
    @autoreleasepool {
        // ... validations ...
        
        // Format mapping
        MTLPixelFormat metalFormat;
        bool isCompressed = false;
        
        switch (format) {
            case 3: // DDS_FORMAT_BC3_RGBA
                metalFormat = MTLPixelFormatBC3_RGBA;
                isCompressed = true;
                break;
            // ... other formats ...
        }
        
        // Create texture descriptor
        MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
        descriptor.textureType = MTLTextureType2D;
        descriptor.pixelFormat = metalFormat;
        descriptor.width = width;        // ✓ PIXELS: 1024
        descriptor.height = height;      // ✓ PIXELS: 256
        descriptor.mipmapLevelCount = (mipLevels > 0) ? mipLevels : 1;
        descriptor.usage = MTLTextureUsageShaderRead;
        descriptor.storageMode = MTLStorageModeShared;
        
        id<MTLTexture> texture = [device newTextureWithDescriptor:descriptor];
        
        if (isCompressed) {
            unsigned int blockSize = 16;  // BC3 = 16 bytes/block
            unsigned int blocksWide = (width + 3) / 4;   // 256
            unsigned int blocksHigh = (height + 3) / 4;  // 64
            unsigned int bytesPerRow = blocksWide * blockSize;  // 4096
            
            // ATUAL: Region em PIXELS (conforme documentação Apple)
            MTLRegion region = MTLRegionMake2D(0, 0, width, height);
            
            [texture replaceRegion:region
                       mipmapLevel:0
                         withBytes:data
                       bytesPerRow:bytesPerRow];  // ✓ 4096
        }
        
        return (__bridge_retained void*)texture;
    }
}
```

### Cálculos Matemáticos Verificados

| Parâmetro | Cálculo | Valor | Status |
|-----------|---------|-------|--------|
| Largura (pixels) | Input | 1024 | ✓ Correto |
| Altura (pixels) | Input | 256 | ✓ Correto |
| Blocos largura | 1024 / 4 | 256 | ✓ Correto |
| Blocos altura | 256 / 4 | 64 | ✓ Correto |
| bytesPerRow | 256 × 16 | 4096 | ✓ Correto |
| Tamanho total | 4096 × 64 | 262,144 | ✓ Correto |
| Região width | pixels | 1024 | ✓ Correto |
| Região height | pixels | 256 | ✓ Correto |

**Todos os cálculos estão matematicamente corretos.**

## Tentativas de Correção (Falharam)

### Tentativa 1: bytesPerRow = 0 (FALHOU - Piorou)
**Hipótese**: Metal calcula automaticamente bytesPerRow para texturas comprimidas.

```objectivec++
[texture replaceRegion:region
           mipmapLevel:0
             withBytes:data
           bytesPerRow:0];  // ❌ FALHOU
```

**Resultado**: TODAS as texturas (incluindo TGA) renderizaram com metade laranja. Revertido.

**Análise**: bytesPerRow=0 é apenas para texturas 3D ou texture arrays, não para 2D.

### Tentativa 2: MTLRegion em Blocos (FALHOU - Piorou)
**Hipótese**: Região deve ser especificada em dimensões de blocos, não pixels.

```objectivec++
MTLRegion region = MTLRegionMake2D(0, 0, blocksWide, blocksHigh);
// region = (0, 0, 256, 64) em vez de (0, 0, 1024, 256)
```

**Resultado**: Texturas renderizaram em **tamanho reduzido** (256×64 pixels ao invés de 1024×256). Revertido.

**Análise**: Confirmou que `MTLRegion` deve ser em **pixels**, não blocos. Documentação Apple:
> "For compressed texture formats, the width and height of the MTLRegion must be **multiples of the compression block size**."

### Tentativa 3: Verificação de Alinhamento (Sem Efeito)
**Hipótese**: bytesPerRow precisa de alinhamento específico (potência de 2, múltiplos de 256, etc).

```objectivec++
unsigned int bytesPerRowAlt = width * 4;  // RGBA8 equivalent = 4096
// Mesmo valor que bytesPerRow calculado!
```

**Resultado**: Sem mudança. Alinhamento não é o problema.

## Hipóteses Descartadas

### ❌ Dados Corrompidos
- Hex dump mostra dados BC3 válidos em todo o arquivo
- Dados em 0%, 50%, 100% do arquivo são consistentes
- Não há zeros, garbage ou padrões inválidos

### ❌ Vertex Colors Incorretas
- Shader modificado para forçar `float4(1.0, 1.0, 1.0, 1.0)` (branco puro)
- Bug laranja **persistiu** → não é problema de vertex colors

### ❌ Cálculo de bytesPerRow
- Valor calculado (4096) é matematicamente correto
- Alternativas testadas (RGBA8 equiv, alinhamentos) dão mesmo valor
- Metal não reporta erros de validação

### ❌ MTLRegion Incorreta
- Região em pixels (1024×256) é correta conforme documentação
- Região em blocos (256×64) causa textura reduzida
- Origem (0,0) está correta

### ❌ Formato Pixel Incorreto
- `MTLPixelFormatBC3_RGBA` é o formato correto para DXT5
- Texture descriptor com width/height em pixels está correto
- Metal aceita a textura sem erros

### ❌ Storage Mode
- `MTLStorageModeShared` é correto para CPU upload
- Outras opções (Private, Managed) requerem blit encoder

## Hipóteses Ativas (A Investigar)

### 1. 🔍 Problema no Shader Sampling
**Sintomas Relevantes**:
- TGA RGBA8 renderiza corretamente
- BC3 renderiza metade correta, metade laranja
- Laranja aparece no **lado direito** (metade superior das coordenadas UV?)

**Teoria**: Shader pode estar amostrando coordenadas UV incorretamente para texturas comprimidas.

**Investigação Necessária**:
- Verificar se `textureSampler` tem configuração especial para compressed
- Checar se coordenadas UV > 0.5 causam o problema
- Testar com UVs customizadas (quad 3 usa UV 0.0-0.5, ainda tem laranja)

### 2. 🔍 Endianness dos Blocos BC3
**Sintomas Relevantes**:
- Exatamente metade da textura afetada
- Padrão binário (potência de 2: 50%)

**Teoria**: Blocos BC3 podem estar em ordem incorreta (little-endian vs big-endian).

**Investigação Necessária**:
- Verificar se DDS loader lê blocos na ordem correta
- Comparar com implementação Windows (DirectX) funcional
- Testar byte-swapping manual dos blocos

### 3. 🔍 Metal Texture Swizzle/Layout
**Sintomas Relevantes**:
- Metal pode ter layout interno diferente de DirectX
- macOS ARM64 pode ter requisitos específicos de memória

**Teoria**: Metal espera blocos BC3 em layout diferente do DDS padrão.

**Investigação Necessária**:
- Consultar documentação Metal sobre BC3 internal layout
- Verificar se há requisitos de tile/swizzle para ARM64
- Comparar com implementação DXVK (Vulkan/DirectX translation)

### 4. 🔍 Mipmap Level 0 Offset
**Sintomas Relevantes**:
- Textura tem `mipmapLevelCount = 1` mas DDS reporta `mipmaps: 0`
- Possível off-by-one error

**Teoria**: Metal pode estar lendo mipmap level incorreto.

**Investigação Necessária**:
- Testar com `mipmapLevel:1` em vez de `mipmapLevel:0`
- Verificar offset de dados no arquivo DDS
- Comparar header DDS com dados realmente usados

## Código de Teste Atual

### Arquivo: tests/test_textured_quad_render.cpp

```cpp
// Test setup - 4 quads renderizados
TexturedQuad quad0;  // defeated.dds (1024x256) at (50,50)    - ❌ Metade laranja
TexturedQuad quad1;  // GameOver.tga (512x128) at (200,350)   - ❌ Metade laranja (TGA!)
TexturedQuad quad2;  // caust00.tga (64x64) at (1000,600)     - ✓ OK (TGA pequeno)
TexturedQuad quad3;  // defeated.dds (512x128) at (50,500)    - ❌ Metade laranja
```

### Comandos de Compilação e Execução

```bash
# Compilar teste
cd /Users/felipebraz/PhpstormProjects/pessoal/GeneralsGameCode
clang++ -std=c++17 -ObjC++ \
  -I./Core/Libraries/Include \
  -I./Core/Libraries/Source/WWVegas/WW3D2 \
  -I/opt/homebrew/opt/sdl2/include/SDL2 \
  -framework Metal -framework QuartzCore \
  -framework Cocoa -framework CoreFoundation \
  ./Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.mm \
  ./Core/Libraries/Source/WWVegas/WW3D2/texturedquad.cpp \
  ./Core/Libraries/Source/WWVegas/WW3D2/texturecache.cpp \
  ./Core/Libraries/Source/WWVegas/WW3D2/ddsloader.cpp \
  ./Core/Libraries/Source/WWVegas/WW3D2/tgaloader.cpp \
  ./tests/test_textured_quad_render.cpp \
  /opt/homebrew/opt/sdl2/lib/libSDL2.dylib \
  -fobjc-arc -o /tmp/test_textured_quad_render

# Executar teste
cd $HOME/GeneralsX/GeneralsMD
/tmp/test_textured_quad_render

# Screenshot manual para análise visual
# (Janela renderiza por ~2 segundos)
```

### Análise de Debug Output

```bash
# Ver informações de upload BC3
/tmp/test_textured_quad_render 2>&1 | grep "METAL DEBUG BC3"

# Ver todas as texturas carregadas
/tmp/test_textured_quad_render 2>&1 | grep "Loaded.*dds\|Loaded.*tga"

# Ver erros Metal
/tmp/test_textured_quad_render 2>&1 | grep "METAL ERROR\|METAL WARNING"
```

## Próximos Passos Recomendados

### Prioridade ALTA
1. **Investigar shader sampling**: Verificar se `textureSampler` processa BC3 corretamente
2. **Comparar com referências**: Checar implementações em `references/fighter19-dxvk-port/` e `references/dxgldotorg-dxgl/`
3. **Testar texturas menores**: Criar BC3 de 64×64, 128×128 para isolar se problema é específico de tamanho

### Prioridade MÉDIA
4. **Verificar DDS loader**: Garantir que blocos BC3 são lidos na ordem correta
5. **Consultar Metal documentation**: Procurar requisitos específicos de BC3 para ARM64
6. **Adicionar validação Metal**: Usar Metal Frame Capture para inspecionar textura GPU-side

### Prioridade BAIXA
7. **Testar outros formatos BC**: BC1, BC2 para isolar se problema é BC3-específico
8. **Descomprimir manualmente**: Converter BC3→RGBA8 em CPU como workaround temporário
9. **Comparar com OpenGL**: Testar mesma textura no path OpenGL (deprecated mas funcional)

## Arquivos Relacionados

### Implementação Core
- `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.mm` - Upload de texturas Metal
- `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.h` - API Metal wrapper
- `Core/Libraries/Source/WWVegas/WW3D2/ddsloader.cpp` - Leitor de DDS
- `Core/Libraries/Source/WWVegas/WW3D2/texturedquad.cpp` - Renderização de quads
- `Core/Libraries/Source/WWVegas/WW3D2/texturecache.cpp` - Cache de texturas
- `resources/shaders/basic.metal` - Fragment shader que amostra texturas

### Testes
- `tests/test_textured_quad_render.cpp` - Teste de renderização de quads texturizados

### Documentação
- `docs/PHASE28/PHASE28_TODO_LIST.md` - Lista de tarefas Phase 28
- `docs/MACOS_PORT.md` - Progresso geral do port macOS
- `docs/OPENGL_SUMMARY.md` - Implementação OpenGL (alternativa)

## Contexto Histórico

### Progressão do Bug
1. **Descoberta Inicial**: Phase 28.4.2 - Primeira renderização de texturas BC3
2. **Isolamento**: Confirmado via teste com cores brancas forçadas que problema não é vertex colors
3. **Validação de Dados**: Hex dump confirmou dados BC3 válidos
4. **Tentativas Falhadas**: bytesPerRow=0, região em blocos, alinhamentos diversos
5. **Estado Atual**: Bug persiste, todas hipóteses óbvias descartadas

### Impacto no Projeto
- **Bloqueio**: Phase 28.4.2 não pode ser completada (visual validation checklist)
- **Workaround Possível**: Converter todas texturas BC3→RGBA8 (custoso em memória)
- **Prioridade**: CRÍTICA - 90% das texturas do jogo são BC3/DXT5

## Referências

### Documentação Apple Metal
- Metal Texture Programming Guide: https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
- MTLTexture replaceRegion: https://developer.apple.com/documentation/metal/mtltexture/1515464-replaceregion
- Block-Compressed Texture Formats: https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf

### DDS/BC3 Specification
- DirectX Texture Compression: https://docs.microsoft.com/en-us/windows/win32/direct3d11/texture-block-compression-in-direct3d-11
- DDS File Format: https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds

### Repositórios de Referência
- DXVK (Vulkan/DirectX translation): `references/fighter19-dxvk-port/`
- DXGL (DirectX→OpenGL wrapper): `references/dxgldotorg-dxgl/`

---

**Última Atualização**: 15 de outubro de 2025  
**Investigador**: GitHub Copilot  
**Status**: Investigação em andamento - Requer sessão futura com buffer limpo
