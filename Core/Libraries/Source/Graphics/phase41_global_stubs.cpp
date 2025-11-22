/**
 * Phase 41: Global stub symbols
 * 
 * These are global variables and weak implementations needed by the game
 * but not yet implemented in the graphics/network backends.
 */

#include <cstdint>

// ============================================================================
// Global Variables (weak symbols)
// ============================================================================

// DX8 Wrapper globals
void* _DX8Wrapper_PreserveFPU = nullptr;
bool _DontShowMainMenu = false;
void* _GameSpyColor = nullptr;

// FunctionLexicon global registry
void* _TheFunctionLexicon = nullptr;

// GameSpy queues
void* _TheGameResultsQueue = nullptr;
void* _TheGameSpyBuddyMessageQueue = nullptr;
void* _TheGameSpyConfig = nullptr;
void* _TheGameSpyGame = nullptr;
void* _TheGameSpyInfo = nullptr;
void* _TheGameSpyPSMessageQueue = nullptr;
void* _TheGameSpyPeerMessageQueue = nullptr;

// IME Manager
void* _TheIMEManager = nullptr;

// GameSpy Pinger
void* _ThePinger = nullptr;

// Debug crash dump
void* _g_LastErrorDump = nullptr;

// ============================================================================
// Function Stubs
// ============================================================================

void _DX8Wrapper_PreserveFPU_func() { }
void CancelPatchCheckCallback() { }
void StartPatchCheck() { }
void TearDownGameSpy() { }
void updateBuddyInfo() { }
void updateLAN() { }

// GameSpy overlay stubs
void GameSpyCloseAllOverlays() { }
void GameSpyCloseOverlay(int type) { }
bool GameSpyIsOverlayOpen(int type) { return false; }
void GameSpyOpenOverlay(int type) { }
void GameSpyToggleOverlay(int type) { }
void GameSpyUpdateOverlays() { }

// Registry stubs
int GetRegistryLanguage() { return 0; }
bool GetStringFromRegistry(void* key1, void* key2, void* result) { return false; }

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

