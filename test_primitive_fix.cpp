// Teste simples para verificar correção do primitive type (3→4)
// Compila direto sem dependências complexas

#include <SDL.h>
#include <stdio.h>

#ifdef __APPLE__
#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

extern "C" {
    // Funções mínimas do MetalWrapper necessárias
    namespace MetalWrapper {
        void Initialize(void* config);
        void Shutdown();
        void BeginFrame(float r, float g, float b, float a);
        void EndFrame();
        void* CreateVertexBuffer(unsigned long size, const void* data);
        void* CreateIndexBuffer(unsigned long size, const void* data);
        void SetVertexBuffer(void* buffer, unsigned int offset, unsigned int slot);
        void SetIndexBuffer(void* buffer, unsigned int offset);
        void DrawIndexedPrimitive(unsigned int primitiveType, int baseVertexIndex,
                                 unsigned int minVertex, unsigned int numVertices,
                                 unsigned int startIndex, unsigned int primitiveCount);
    }
}

int main(int argc, char* argv[]) {
    printf("=== TESTE: Correção Primitive Type (3→4) ===\n");
    
    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("ERRO: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_Window* window = SDL_CreateWindow("Test Primitive Fix",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 768, SDL_WINDOW_SHOWN);
    
    if (!window) {
        printf("ERRO: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    
    // Inicializar Metal
    MetalWrapper::Initialize(window);
    
    // Dados de vertex - 1 quad RED (4 vértices)
    struct Vertex {
        float position[3];
        float normal[3];
        float color[4];
        float texcoord[2];
    };
    
    Vertex vertices[4] = {
        {{-0.5f,  0.5f, 0.0f}, {0,0,1}, {1,0,0,1}, {0,0}},  // Top-left RED
        {{ 0.5f,  0.5f, 0.0f}, {0,0,1}, {1,0,0,1}, {1,0}},  // Top-right RED
        {{ 0.5f, -0.5f, 0.0f}, {0,0,1}, {1,0,0,1}, {1,1}},  // Bottom-right RED
        {{-0.5f, -0.5f, 0.0f}, {0,0,1}, {1,0,0,1}, {0,1}}   // Bottom-left RED
    };
    
    unsigned short indices[6] = {0, 1, 2, 0, 2, 3};  // 2 triângulos
    
    void* vb = MetalWrapper::CreateVertexBuffer(sizeof(vertices), vertices);
    void* ib = MetalWrapper::CreateIndexBuffer(sizeof(indices), indices);
    
    printf("Vertex Buffer: %p\n", vb);
    printf("Index Buffer: %p\n", ib);
    
    // Renderizar por 3 segundos
    Uint32 startTime = SDL_GetTicks();
    int frameCount = 0;
    
    while (SDL_GetTicks() - startTime < 3000) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) goto cleanup;
        }
        
        MetalWrapper::BeginFrame(0.0f, 0.0f, 0.3f, 1.0f);  // Blue background
        
        MetalWrapper::SetVertexBuffer(vb, 0, 0);
        MetalWrapper::SetIndexBuffer(ib, 0);
        
        // TESTE: Primitive type = 4 (D3DPT_TRIANGLELIST)
        // Deve desenhar quad RED no centro da tela
        MetalWrapper::DrawIndexedPrimitive(
            4,  // D3DPT_TRIANGLELIST (foi 3 antes - BUG!)
            0,  // base_vertex_index
            0,  // min_vertex
            4,  // num_vertices
            0,  // start_index
            2   // primitive_count (2 triangles)
        );
        
        MetalWrapper::EndFrame();
        frameCount++;
        
        SDL_Delay(16);  // ~60 FPS
    }
    
cleanup:
    printf("\n=== RESULTADO ===\n");
    printf("Frames renderizados: %d\n", frameCount);
    printf("Se viu um QUAD VERMELHO no centro → ✅ PRIMITIVE TYPE CORRETO!\n");
    printf("Se viu apenas fundo azul → ❌ BUG ainda presente\n");
    
    MetalWrapper::Shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}

#else
int main() {
    printf("Este teste é apenas para macOS Metal\n");
    return 0;
}
#endif
