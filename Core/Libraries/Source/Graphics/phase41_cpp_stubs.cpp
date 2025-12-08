/**
 * Phase 41: C++ Class Stub Implementations
 * 
 * Provides complete stub implementations for classes that were declared
 * but never implemented. These allow the linker to succeed and create
 * a working executable.
 * 
 * Strategy: Define minimal vtables and methods for all unimplemented classes.
 * 
 * NOTE: Many stubs have moved to dedicated files:
 * - Transport: phase43_4_transport.cpp
 * - FillStackAddresses, Registry: phase43_registry_stubs.cpp
 * - GameSpy Overlays: phase43_5_gamespy_ui.cpp
 * - PSRequest: PersistentStorageThread.cpp
 * - Targa: TARGA.cpp (original implementation)
 */

#include <cstdint>
#include <cstring>

// ============================================================================
// Texture Loading
// ============================================================================

struct ChunkLoadClass { };
struct GameAssetManager { };
struct WW3D_FormatSpecifier { };

class TextureLoadTaskClass {
public:
    TextureLoadTaskClass(const char* name, const ChunkLoadClass& chunk, GameAssetManager* asset_mgr, const WW3D_FormatSpecifier& format);
    ~TextureLoadTaskClass();
};

TextureLoadTaskClass::TextureLoadTaskClass(const char* name, const ChunkLoadClass& chunk, GameAssetManager* asset_mgr, const WW3D_FormatSpecifier& format) { }
TextureLoadTaskClass::~TextureLoadTaskClass() { }

// ============================================================================
// CubeTexture Constructor (most problematic)
// ============================================================================

// Minimal MipCountType and WW3DFormat stubs
enum MipCountType { MIP_LEVELS_ALL = 0 };
enum WW3DFormat { WW3D_FORMAT_UNKNOWN = 0 };

class CubeTextureClass {
public:
    CubeTextureClass(const char* name1, const char* name2, MipCountType mip_count, WW3DFormat format, bool render_target, bool allow_reduction);
    virtual ~CubeTextureClass();
    virtual unsigned Get_Texture_Memory_Usage() const { return 0; }
    virtual void* Peek_D3D_Base_Texture() const { return nullptr; }
};

CubeTextureClass::CubeTextureClass(const char* name1, const char* name2, MipCountType mip_count, WW3DFormat format, bool render_target, bool allow_reduction) { }
CubeTextureClass::~CubeTextureClass() { }

// ============================================================================
// UnicodeString helpers
// ============================================================================

class UnicodeString {
public:
    UnicodeString() { }
    ~UnicodeString() { }
    void Append(const UnicodeString& other) { }
};

// ============================================================================
// GameSpy Interface Classes
// ============================================================================

struct PSPlayerStats { };
struct GSOverlayType { };

class GameSpyPSMessageQueueInterface {
public:
    virtual ~GameSpyPSMessageQueueInterface() = default;
    virtual void formatPlayerKVPairs(PSPlayerStats stats) { }
};

// NOTE: GameResultsInterface is now implemented in phase51_game_results_stub.cpp
// The original stub here was incorrect - it declared a different class instead of
// implementing the factory method for the real GameResultsInterface class.

class GameSpyStagingRoom {
public:
    static void* generateGameSpyGameResultsPacket() { return nullptr; }
    static void* generateLadderGameResultsPacket() { return nullptr; }
    static int getGameSpySlot(int slot) { return -1; }
};

// ============================================================================
// Window/UI Classes
// ============================================================================

// NOTE: BuddyControlSystem is now in phase43_registry_stubs.cpp

// ============================================================================
// Allocator classes
// ============================================================================

class FastAllocatorGeneral {
public:
    static void* Get_Allocator() { return nullptr; }
};

// ============================================================================
// GameSpy Message Box (C-style)
// ============================================================================

class UnicodeStringStub {
public:
    UnicodeStringStub() { }
};

void GSMessageBoxOk(const UnicodeStringStub& title, const UnicodeStringStub& message, void (*callback)()) {
    // Stub: do nothing - no UI in headless game
}

// NOTE: PSRequest is now in PersistentStorageThread.cpp
// NOTE: FillStackAddresses is now in phase43_registry_stubs.cpp

