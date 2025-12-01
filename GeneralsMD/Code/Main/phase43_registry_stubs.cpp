/**
 * Phase 43/54: Registry & Platform Stubs for macOS/Linux Compatibility
 * Stub implementations of Windows-specific functions for cross-platform support
 *
 * This file provides stubs for:
 * - Windows Registry functions (GetStringFromRegistry, SetStringInRegistry, etc.)
 * - String conversion utilities (WideCharStringToMultiByte, MultiByteToWideCharSingleLine)
 * - Stack dump stubs (StackDumpFromAddresses, FillStackAddresses)
 * - Network stubs (GameSpyBuddyMessageQueueInterface, etc.)
 * - Buddy system stubs (InitBuddyControls, BuddyControlSystem, etc.)
 *
 * NOTE: GameSpy overlay functions are provided by phase43_5_gamespy_ui.cpp
 * NOTE: OSDisplay functions are provided by CrossPlatformOSDisplay.cpp
 */

#if !defined(_WIN32)

#include <string>
#include <cstring>
#include <cwchar>
#include <vector>
#include <unordered_map>
#include <Utility/compat.h>

 // Forward declarations for game types
class AsciiString;
class UnicodeString;
class GameWindow;

// ============================================================================
// Registry Function Stubs (Windows Registry not available on Linux/macOS)
// These functions store settings in memory only (not persistent)
// ============================================================================

static std::unordered_map<std::string, std::string> g_registryStrings;
static std::unordered_map<std::string, unsigned int> g_registryInts;

void GetStringFromRegistry(std::string key, std::string name, std::string& value)
{
  std::string fullKey = key + GET_PATH_SEPARATOR() + name;
  auto it = g_registryStrings.find(fullKey);
  if (it != g_registryStrings.end()) {
    value = it->second;
  }
}

void SetStringInRegistry(std::string key, std::string name, std::string value)
{
  std::string fullKey = key + GET_PATH_SEPARATOR() + name;
  g_registryStrings[fullKey] = value;
}

void GetUnsignedIntFromRegistry(std::string key, std::string name, unsigned int& value)
{
  std::string fullKey = key + GET_PATH_SEPARATOR() + name;
  auto it = g_registryInts.find(fullKey);
  if (it != g_registryInts.end()) {
    value = it->second;
  }
}

void SetUnsignedIntInRegistry(std::string key, std::string name, unsigned int value)
{
  std::string fullKey = key + GET_PATH_SEPARATOR() + name;
  g_registryInts[fullKey] = value;
}

// ============================================================================
// String Conversion Utilities
// These convert between wide strings (wchar_t*) and multibyte strings (char*)
// ============================================================================

std::string WideCharStringToMultiByte(const wchar_t* wstr)
{
  if (!wstr) return std::string();

  std::string result;
  size_t len = std::wcslen(wstr);
  result.reserve(len);

  for (size_t i = 0; i < len; i++) {
    wchar_t wc = wstr[i];
    if (wc < 128) {
      result += static_cast<char>(wc);
    }
    else {
      result += '?';  // Simple replacement for non-ASCII
    }
  }
  return result;
}

std::wstring MultiByteToWideCharSingleLine(const char* str)
{
  if (!str) return std::wstring();

  std::wstring result;
  size_t len = std::strlen(str);
  result.reserve(len);

  for (size_t i = 0; i < len; i++) {
    unsigned char c = static_cast<unsigned char>(str[i]);
    result += static_cast<wchar_t>(c);
  }
  return result;
}

// ============================================================================
// Stack Dump Functions (Debug only - stubs for Linux)
// ============================================================================

void StackDumpFromAddresses(void** /*addresses*/, unsigned int /*count*/,
  void (*/*callback*/)(const char*))
{
  // Stub: Stack dump not implemented on Linux (would need libunwind or similar)
}

void FillStackAddresses(void** addresses, unsigned int count, unsigned int /*skip*/)
{
  if (addresses) {
    for (unsigned int i = 0; i < count; i++) {
      addresses[i] = nullptr;
    }
  }
}

// ============================================================================
// GameSpy Network Interface Stubs
// NOTE: GameSpyBuddyMessageQueueInterface, GameSpyPeerMessageQueueInterface,
//       GameSpyConfigInterface are implemented in phase43_5_gamespy_globals.cpp
// ============================================================================

// PingerInterface is still needed here as it's not in phase43_5_gamespy_globals
class PingerInterface {
public:
  static PingerInterface* createNewPingerInterface();
};

PingerInterface* PingerInterface::createNewPingerInterface() { return nullptr; }

void deleteNotificationBox() { /* stub */ }
void PopBackToLobby() { /* stub */ }
void StartPatchCheck() { /* stub */ }
void StopAsyncDNSCheck() { /* stub */ }
void CancelPatchCheckCallback() { /* stub */ }
void HTTPThinkWrapper() { /* stub */ }
void UpdateLocalPlayerStats() { /* stub */ }
void* LookupSmallRankImage(int /*rank*/, int /*mode*/) { return nullptr; }

// ============================================================================
// Buddy System Stubs (Friend list functionality)
// ============================================================================

void InitBuddyControls(int /*mode*/) { /* stub */ }
void PopulateOldBuddyMessages() { /* stub */ }
void updateBuddyInfo() { /* stub */ }
long BuddyControlSystem(GameWindow* /*window*/, unsigned int /*msg*/,
  unsigned long /*wparam*/, unsigned long /*lparam*/) {
  return 0;
}

// ============================================================================
// RegistryClass Stubs (WW3D uses this for video settings)
// Implementation of registry.h interface for non-Windows platforms
// ============================================================================

#include "registry.h"
#include <Utility/compat.h>

bool RegistryClass::IsLocked = false;

bool RegistryClass::Exists(const char* /*sub_key*/) {
  return false;  // Registry keys don't exist on non-Windows
}

RegistryClass::RegistryClass(const char* /*sub_key*/, bool /*create*/) :
  IsValid(false),
  Key(0)
{
  // No registry on Linux/macOS - stub
}

RegistryClass::~RegistryClass() {
  IsValid = false;
}

int RegistryClass::Get_Int(const char* /*name*/, int def_value) {
  return def_value;
}

void RegistryClass::Set_Int(const char* /*name*/, int /*value*/) {
  // No-op on non-Windows
}

bool RegistryClass::Get_Bool(const char* /*name*/, bool def_value) {
  return def_value;
}

void RegistryClass::Set_Bool(const char* /*name*/, bool /*value*/) {
  // No-op on non-Windows
}

float RegistryClass::Get_Float(const char* /*name*/, float def_value) {
  return def_value;
}

void RegistryClass::Set_Float(const char* /*name*/, float /*value*/) {
  // No-op on non-Windows
}

char* RegistryClass::Get_String(const char* /*name*/, char* value, int value_size,
  const char* default_string) {
  if (default_string && value && value_size > 0) {
    strncpy(value, default_string, value_size - 1);
    value[value_size - 1] = '\0';
  }
  return value;
}

void RegistryClass::Get_String(const char* /*name*/, StringClass& string, const char* default_string) {
  if (default_string) {
    string = default_string;
  }
}

void RegistryClass::Set_String(const char* /*name*/, const char* /*value*/) {
  // No-op on non-Windows
}

void RegistryClass::Get_String(const WCHAR* /*name*/, WideStringClass& string, const WCHAR* default_string) {
  if (default_string) {
    string = default_string;
  }
}

void RegistryClass::Set_String(const WCHAR* /*name*/, const WCHAR* /*value*/) {
  // No-op on non-Windows
}

void RegistryClass::Get_Bin(const char* /*name*/, void* /*buffer*/, int /*buffer_size*/) {
  // No-op on non-Windows
}

int RegistryClass::Get_Bin_Size(const char* /*name*/) {
  return 0;
}

void RegistryClass::Set_Bin(const char* /*name*/, const void* /*buffer*/, int /*buffer_size*/) {
  // No-op on non-Windows
}

void RegistryClass::Get_Value_List(DynamicVectorClass<StringClass>& /*list*/) {
  // No-op on non-Windows
}

void RegistryClass::Delete_Value(const char* /*name*/) {
  // No-op on non-Windows
}

void RegistryClass::Deleta_All_Values(void) {
  // No-op on non-Windows
}

void RegistryClass::Delete_Registry_Tree(char* /*path*/) {
  // No-op on non-Windows
}

void RegistryClass::Load_Registry(const char* /*filename*/, char* /*old_path*/, char* /*new_path*/) {
  // No-op on non-Windows
}

void RegistryClass::Save_Registry(const char* /*filename*/, char* /*path*/) {
  // No-op on non-Windows
}

void RegistryClass::Delete_Registry_Values(void* /*key*/) {
  // No-op on non-Windows
}

void RegistryClass::Save_Registry_Tree(char* /*path*/, INIClass* /*ini*/) {
  // No-op on non-Windows
}

void RegistryClass::Save_Registry_Values(void* /*key*/, char* /*path*/, INIClass* /*ini*/) {
  // No-op on non-Windows
}

// ============================================================================
// FastAllocatorGeneral Stub (Memory allocator)
// ============================================================================

class FastAllocatorGeneral {
public:
  static FastAllocatorGeneral* Get_Allocator() {
    static FastAllocatorGeneral instance;
    return &instance;
  }
};

#endif // !_WIN32
