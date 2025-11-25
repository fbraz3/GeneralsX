/**
 * Phase 41: Global stub symbols
 * 
 * These are global variables and weak implementations needed by the game
 * but not yet implemented in the graphics/network backends.
 */

#include <cstdint>
#include <cstring>

// ============================================================================
// Namespace declaration for C++ name mangling fix
// ============================================================================
// These functions are called from C++ code with C++ name mangling,
// but defined with extern "C" for C linkage. We need to export them properly.

// ============================================================================
// Global Variables (with correct C linkage)
// ============================================================================

extern "C" {
    // These must match the types expected by callers
    int DX8Wrapper_PreserveFPU = 0;
    bool DontShowMainMenu = false;
}

// Forward declarations of C++ classes
class FunctionLexicon;
class GameResultsInterface;
class IMEManagerInterface;
class PingerInterface;

extern "C" {
    // Pointers to C++ objects - use void* to avoid C++ name mangling issues
    void* TheFunctionLexicon = nullptr;
    void* TheGameResultsQueue = nullptr;
    void* TheIMEManager = nullptr;
    void* TheGameSpyBuddyMessageQueue = nullptr;
    void* TheGameSpyConfig = nullptr;
    void* TheGameSpyGame = nullptr;
    void* TheGameSpyInfo = nullptr;
    void* TheGameSpyPSMessageQueue = nullptr;
    void* TheGameSpyPeerMessageQueue = nullptr;
    void* ThePinger = nullptr;
    void* TheLadderList = nullptr;
    void* _g_LastErrorDump = nullptr;
    void* GameSpyColor = nullptr;
}

// ============================================================================
// Function Stubs with PROPER C++ linkage
// ============================================================================
// These functions are called from C++ code, so they need C++ name mangling.
// However, we use extern "C" wrapper functions with C linkage for compatibility.

// C++ implementation functions (these get proper C++ name mangling)
void PopBackToLobby() { }
void StartPatchCheck() { }
void CancelPatchCheckCallback() { }
void TearDownGameSpy() { }
void updateBuddyInfo() { }
void HTTPThinkWrapper() { }
void InitBuddyControls(int param) { }
void StopAsyncDNSCheck() { }
void RefreshGameListBoxes() { }
void deleteNotificationBox() { }
void PopulateOldBuddyMessages() { }
void LookupSmallRankImage(int rank, int side) { }
int MultiByteToWideCharSingleLine(const char* str) { return str ? strlen(str) : 0; }

int GetStringFromRegistry(const char* key1, const char* key2, char* result)
{
    if (result) *result = '\0';
    return 0;
}

int SetStringInRegistry(const char* key1, const char* key2, const char* value)
{
    return 0;
}

int GetUnsignedIntFromRegistry(const char* key1, const char* key2, unsigned int* result)
{
    if (result) *result = 0;
    return 0;
}

int SetUnsignedIntInRegistry(const char* key1, const char* key2, unsigned int value)
{
    return 0;
}

int OSDisplayWarningBox(const char* title, const char* message, unsigned int type, unsigned int flags)
{
    return 0;
}

void OSDisplaySetBusyState(bool busy, bool complete) { }

void StackDumpFromAddresses(void** addresses, unsigned int count, void (*callback)(const char*)) { }

// GameSpy overlay stubs
void GameSpyCloseAllOverlays() { }
void GameSpyCloseOverlay(int type) { }
int GameSpyIsOverlayOpen(int type) { return 0; }
void GameSpyOpenOverlay(int type) { }
void GameSpyToggleOverlay(int type) { }
void GameSpyUpdateOverlays() { }

// Registry stubs
int GetRegistryLanguage() { return 0; }

// File system stubs
void Load_Texture(void* chunk) { }

// Allocator stubs
void* FastAllocatorGeneral_Get_Allocator() { return nullptr; }

// Stack trace stubs
void FillStackAddresses(void** addresses, unsigned int count, unsigned int skip) { }

// GameSpy interface stubs
void* GameResultsInterface_createNewGameResultsInterface() { return nullptr; }
void* GameSpyStagingRoom_generateGameSpyGameResultsPacket() { return nullptr; }
void* GameSpyStagingRoom_generateLadderGameResultsPacket() { return nullptr; }
int GameSpyStagingRoom_getGameSpySlot(int slot) { return -1; }

