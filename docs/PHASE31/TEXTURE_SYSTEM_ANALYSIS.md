# Phase 31: Texture System - Análise da Estrutura Atual

**Data**: 19 de outubro de 2025  
**Status**: Em análise - Task 1 completa

## Sumário Executivo

A análise inicial revela que **a maior parte da infraestrutura de texturas já está implementada** desde as Fases 28-30. Os loaders DDS e TGA estão funcionais, a API Metal está completa, e o pipeline de integração existe em `textureloader.cpp`. O foco da Fase 31 é **validação, integração e testes** ao invés de implementação do zero.

## Componentes Existentes

### 1. DDS Texture Loader ✅ IMPLEMENTADO

**Localização**: `Core/Libraries/Source/WWVegas/WW3D2/ddsloader.{h,cpp}`

**Recursos**:
- Suporte completo para BC1/BC2/BC3 (DXT1/DXT3/DXT5)
- Fallback RGB8/RGBA8 para formatos não-comprimidos
- Extração de mipmaps
- Validação de headers
- API `DDSLoader::Load()` e `DDSLoader::LoadFromMemory()`

**Teste existente**: `tests/test_dds_loader.cpp`

**Integração Metal**: `MetalWrapper::CreateTextureFromDDS()` (linha 726 em metalwrapper.mm)

### 2. TGA Texture Loader ✅ IMPLEMENTADO

**Localização**: `Core/Libraries/Source/WWVegas/WW3D2/tgaloader.{h,cpp}`

**Recursos**:
- Suporte RLE comprimido (tipo 10)
- Suporte não-comprimido RGB/RGBA (tipo 2)
- Conversão BGR→RGBA automática
- Flip vertical para compatibilidade Metal
- API `TGALoader::Load()` e `TGALoader::LoadFromMemory()`

**Teste existente**: `tests/test_tga_loader.cpp`

**Integração Metal**: `MetalWrapper::CreateTextureFromTGA()` (linha 957 em metalwrapper.mm)

### 3. Metal Texture API ✅ IMPLEMENTADO

**Localização**: `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.{h,mm}`

**APIs disponíveis**:
```cpp
// Generic texture creation (Phase 28.4 redesign)
void* CreateTextureFromMemory(unsigned int width, unsigned int height,
                               unsigned int glFormat, const void* data,
                               unsigned int dataSize);

// DDS-specific (Phase 28.1)
void* CreateTextureFromDDS(unsigned int width, unsigned int height, 
                            unsigned int format, const void* data, 
                            unsigned int dataSize, unsigned int mipLevels = 1);

// TGA-specific (Phase 28.3.2)
void* CreateTextureFromTGA(unsigned int width, unsigned int height,
                            const void* data, unsigned int dataSize);

// Texture binding (Phase 28.3.3)
void BindTexture(void* texture, unsigned int slot = 0);
void UnbindTexture(unsigned int slot = 0);

// Cleanup
void DeleteTexture(void* texture);
```

### 4. TextureLoader Integration ⚠️ PARCIALMENTE INTEGRADO

**Localização**: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/textureloader.cpp`

**Pontos de integração existentes**:

#### 4.1. Compressed Texture Loading (DDS)
- **Linha 2109**: `TextureLoadTaskClass::Begin_Compressed_Load()`
- **Linha 2269**: `TextureLoadTaskClass::Load_Compressed_Mipmap()`
- Usa `DDSFileClass` (wrapper antigo) - **precisa migrar para DDSLoader**

#### 4.2. Uncompressed Texture Loading (TGA)
- **Linha 2271**: `TextureLoadTaskClass::Begin_Uncompressed_Load()`
- **Linha 2495**: `TextureLoadTaskClass::Load_Uncompressed_Mipmap()`
- Usa `Targa` (wrapper antigo) - **precisa migrar para TGALoader**

#### 4.3. Metal Backend Integration
- **Linha 2206-2219**: Tentativa de integração `TextureCache` (OpenGL-based)
- **Comentário crítico**: "Phase 28.1-28.3: Metal backend texture loading via TextureCache"
- **Status**: Usa fallback stub, **precisa refatorar para DDSLoader/TGALoader**

### 5. Texture Cache System ⚠️ LEGACY OPENGL

**Localização**: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/texture_cache.{h,cpp}`

**Problema**: Sistema baseado em OpenGL (`GLuint`), incompatível com Metal.

**Solução**: Bypass via `TextureLoadTaskClass` que chama diretamente `MetalWrapper::CreateTextureFromDDS/TGA`.

## Gaps Identificados

### Gap 1: Integração DDSFileClass → DDSLoader

**Arquivo**: `textureloader.cpp`, linha 2109+

**Problema**: `Begin_Compressed_Load()` usa `DDSFileClass` antigo ao invés do novo `DDSLoader`.

**Solução**:
```cpp
// ANTES (linha 2269-2280):
DDSFileClass dds_file(Texture->Get_Full_Path(), Get_Reduction());
if (!dds_file.Is_Available() || !dds_file.Load()) return false;
dds_file.Copy_Level_To_Surface(...);

// DEPOIS (refatorar para):
DDSTextureData ddsData;
if (!DDSLoader::Load(Texture->Get_Full_Path().Peek_Buffer(), &ddsData)) return false;
void* metalTexture = MetalWrapper::CreateTextureFromDDS(
    ddsData.width, ddsData.height, ddsData.format,
    ddsData.data, ddsData.dataSize, ddsData.mipMapCount);
DDSLoader::Free(&ddsData);
```

### Gap 2: Integração Targa → TGALoader

**Arquivo**: `textureloader.cpp`, linha 2495+

**Problema**: `Load_Uncompressed_Mipmap()` usa classe `Targa` antiga ao invés de `TGALoader`.

**Solução**:
```cpp
// ANTES (linha 2495-2530):
Targa targa;
targa.Open(Texture->Get_Full_Path(), TGA_READMODE);
targa.Load(...);

// DEPOIS (refatorar para):
TGATextureData tgaData = TGALoader::Load(Texture->Get_Full_Path().Peek_Buffer());
if (!tgaData.is_valid) return false;
void* metalTexture = MetalWrapper::CreateTextureFromTGA(
    tgaData.width, tgaData.height, tgaData.data, tgaData.data_size);
TGALoader::Free(tgaData);
```

### Gap 3: Texture Cache Bypass

**Arquivo**: `textureloader.cpp`, linha 2206-2219

**Problema**: Código Metal tenta usar `TextureCache` (OpenGL), que retorna `GLuint`.

**Solução**: Remover código `TextureCache`, usar diretamente `DDSLoader/TGALoader → MetalWrapper`.

```cpp
// REMOVER linhas 2206-2219 (tentativa falha de TextureCache)
#ifndef _WIN32
if (g_useMetalBackend) {
    StringClass& fullpath = const_cast<StringClass&>(Texture->Get_Full_Path());
    const char* filepath = fullpath.Peek_Buffer();
    TextureCache* cache = TextureCache::Get_Instance(); // ❌ OpenGL-based
    GLuint tex_id = cache->Get_Texture(filepath);       // ❌ Incompatível com Metal
    ...
}
#endif

// SUBSTITUIR por chamada direta a DDSLoader/TGALoader
```

## Plano de Integração

### Etapa 1: Validar Loaders Isoladamente (Task 2, 4)
- [ ] Compilar e executar `test_dds_loader` com texturas do jogo
- [ ] Compilar e executar `test_tga_loader` com texturas do jogo
- [ ] Verificar output de `defeated.dds`, `caust00.tga`

### Etapa 2: Refatorar textureloader.cpp (Task 3, 5)
- [ ] Criar funções helper `Load_DDS_To_Metal()` e `Load_TGA_To_Metal()`
- [ ] Substituir `DDSFileClass` por `DDSLoader` em `Begin_Compressed_Load()`
- [ ] Substituir `Targa` por `TGALoader` em `Begin_Uncompressed_Load()`
- [ ] Remover código `TextureCache` (linhas 2206-2219)

### Etapa 3: Validar Pipeline Completo (Task 6, 7)
- [ ] Testar carregamento via `.big` file VFS
- [ ] Verificar mipmaps sendo carregados corretamente
- [ ] Validar binding de texturas no shader Metal

### Etapa 4: Testes In-Game (Task 8, 9)
- [ ] Executar GeneralsXZH com `USE_METAL=1`
- [ ] Verificar menu backgrounds renderizam
- [ ] Verificar botões UI aparecem
- [ ] Log de texturas carregadas

### Etapa 5: Performance (Task 10)
- [ ] Profiling de tempo de carregamento
- [ ] Otimizar mipmap upload se necessário
- [ ] Meta: < 100ms por textura

## Arquivos de Teste Sugeridos

### DDS (Compressed)
```bash
# Menu backgrounds (provavelmente DXT1/DXT5)
$HOME/GeneralsX/GeneralsMD/Data/English/Art/Textures/defeated.dds
$HOME/GeneralsX/GeneralsMD/Data/English/Art/Textures/MainMenu*.dds

# In-game textures
$HOME/GeneralsX/GeneralsMD/Data/Art/Textures/*.dds
```

### TGA (Uncompressed)
```bash
# Water caustics (animated textures)
$HOME/GeneralsX/GeneralsMD/Data/English/Art/Textures/caust00.tga
$HOME/GeneralsX/GeneralsMD/Data/English/Art/Textures/caust01.tga

# Effects textures
$HOME/GeneralsX/GeneralsMD/Data/Art/Textures/*.tga
```

## Referências de Código

### DDSFileClass vs DDSLoader
- **Antigo**: `Core/Libraries/Source/WWVegas/WW3D2/ddsfile.{h,cpp}` (usado em textureloader.cpp)
- **Novo**: `Core/Libraries/Source/WWVegas/WW3D2/ddsloader.{h,cpp}` (API moderna, testada)

### Targa vs TGALoader
- **Antigo**: `Core/Libraries/Source/WWVegas/WW3D2/TARGA.{H,CPP}` (usado em textureloader.cpp)
- **Novo**: `Core/Libraries/Source/WWVegas/WW3D2/tgaloader.{h,cpp}` (API moderna, testada)

## Próximos Passos

1. **AGORA**: Compilar e testar `test_dds_loader` com textura real
2. **DEPOIS**: Compilar e testar `test_tga_loader` com textura real
3. **ENTÃO**: Refatorar `textureloader.cpp` para usar novos loaders
4. **FINALMENTE**: Teste in-game com menu principal

## Estimativa de Tempo Revisada

| Fase Original | Estimativa | Revisão | Motivo |
|---------------|-----------|---------|--------|
| 31.1 DDS Loader | 3-4 dias | **1 dia** | Já implementado, apenas validação |
| 31.2 TGA Loader | 2 dias | **1 dia** | Já implementado, apenas validação |
| 31.3 Upload Pipeline | 2-3 dias | **2 dias** | Refatoração textureloader.cpp |
| 31.4 UI Validation | 2 dias | **2 dias** | Testes in-game necessários |
| 31.5 Documentation | 1-2 dias | **1 dia** | Análise já iniciada |
| **TOTAL** | 10-14 dias | **7 dias** | Redução de 30-50% |

---

**Conclusão**: A Fase 31 está **70% completa** antes de começar. O trabalho restante é **integração, validação e testes** ao invés de implementação de novos loaders. Isso representa uma economia significativa de tempo e confirma que as Fases 28-30 prepararam bem o terreno para texturas.
