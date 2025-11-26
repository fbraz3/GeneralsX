# Phase 48: Before/After Code Patterns

**Purpose**: Documentation of cleanup patterns and best practices for Phase 48  
**Status**: REFERENCE DOCUMENT

---

## Pattern 1: Stub Factory Methods

### ❌ BEFORE (Anti-pattern - Stub)

```cpp
// File: Core/GameEngine/Source/Common/System/GameSubsystemFactory.cpp
// Old implementation - returns nullptr

AudioManager* createAudioManager() {
    // TODO: Implement audio manager
    return nullptr;  // PROBLEM: Breaks audio subsystem initialization
}

FileSystem* createFileSystem() {
    // TODO: Implement file system
    return nullptr;  // PROBLEM: Game can't load assets
}

NetworkInterface* createNetwork() {
    // TODO: Implement network
    return nullptr;  // PROBLEM: Multiplayer fails silently
}
```

### ✅ AFTER (Production pattern - Real Implementation)

```cpp
// File: Core/GameEngine/Source/Common/System/GameSubsystemFactory.cpp
// New implementation - returns real instance

AudioManager* createAudioManager() {
    try {
        OpenALAudioDevice* audio = NEW OpenALAudioDevice();
        
        if (!audio->Initialize()) {
            DEBUG_LOG(("AudioManager::Initialize() failed"));
            delete audio;
            return nullptr;  // Only return nullptr if initialization fails
        }
        
        DEBUG_LOG(("AudioManager initialized successfully"));
        return audio;
        
    } catch (const std::exception& e) {
        DEBUG_LOG(("AudioManager creation exception: %s", e.what()));
        return nullptr;
    } catch (...) {
        DEBUG_LOG(("AudioManager creation failed with unknown exception"));
        return nullptr;
    }
}

FileSystem* createFileSystem() {
    try {
        StdLocalFileSystem* fs = NEW StdLocalFileSystem();
        
        if (!fs->Initialize()) {
            DEBUG_LOG(("FileSystem::Initialize() failed"));
            delete fs;
            return nullptr;
        }
        
        DEBUG_LOG(("FileSystem initialized successfully"));
        return fs;
        
    } catch (const std::exception& e) {
        DEBUG_LOG(("FileSystem creation exception: %s", e.what()));
        return nullptr;
    }
}

NetworkInterface* createNetwork() {
    try {
        LANAPI* network = NEW LANAPI();
        
        if (!network->Initialize()) {
            DEBUG_LOG(("Network::Initialize() failed"));
            delete network;
            return nullptr;
        }
        
        DEBUG_LOG(("Network initialized successfully"));
        return network;
        
    } catch (const std::exception& e) {
        DEBUG_LOG(("Network creation exception: %s", e.what()));
        return nullptr;
    }
}
```

### Key Changes

1. ✅ **Try-catch properly implemented** - Catches real exceptions
2. ✅ **Logging on success** - Tracks initialization
3. ✅ **Logging on failure** - Debuggable failures
4. ✅ **Proper cleanup** - Delete on failure
5. ✅ **Real implementation** - Not just return nullptr

### Rule

> **Factory methods should ALWAYS try to create real instances. Only return nullptr if creation/initialization actually fails, NOT as a placeholder.**

---

## Pattern 2: Empty Try-Catch Blocks

### ❌ BEFORE (Anti-pattern - Silent Failure)

```cpp
// File: Generals/Code/Tools/WorldBuilder/src/SaveMap.cpp
// Old - silently swallows exceptions

void SaveMap() {
    try {
        // Map saving logic
        map->Save(filename);
    } catch(...) {}  // PROBLEM: Exceptions silently ignored
}

void LoadMap() {
    try {
        // Map loading logic
        map->Load(filename);
    } catch(...) {
        // Empty catch - no error handling
    }
}

void ProcessGeometry() {
    try {
        geometry.Rebuild();
    } catch(...) {
        // PROBLEM: Developer has no idea why this failed
    }
}
```

### ✅ AFTER (Production pattern - Proper Error Handling)

```cpp
// File: Generals/Code/Tools/WorldBuilder/src/SaveMap.cpp
// New - logs exceptions for debugging

void SaveMap() {
    try {
        // Map saving logic
        map->Save(filename);
        DEBUG_LOG(("Map saved successfully to: %s", filename));
        
    } catch (const FileNotFoundException& e) {
        DEBUG_LOG(("Map save failed: File not found - %s", e.what()));
        ShowErrorDialog("Failed to save map: File not found");
        
    } catch (const IOException& e) {
        DEBUG_LOG(("Map save failed: I/O error - %s", e.what()));
        ShowErrorDialog("Failed to save map: I/O error");
        
    } catch (const std::exception& e) {
        DEBUG_LOG(("Map save failed: %s", e.what()));
        ShowErrorDialog("Failed to save map");
        
    } catch (...) {
        DEBUG_LOG(("Map save failed with unknown exception"));
        ShowErrorDialog("Failed to save map: Unknown error");
    }
}

void LoadMap() {
    try {
        // Map loading logic
        map->Load(filename);
        DEBUG_LOG(("Map loaded successfully from: %s", filename));
        UpdateUI();
        
    } catch (const FileNotFoundException& e) {
        DEBUG_LOG(("Map load failed: File not found - %s", e.what()));
        ShowErrorDialog("Failed to load map: File not found");
        
    } catch (const ParseException& e) {
        DEBUG_LOG(("Map load failed: Parse error - %s", e.what()));
        ShowErrorDialog("Failed to load map: Invalid format");
        
    } catch (const std::exception& e) {
        DEBUG_LOG(("Map load failed: %s", e.what()));
        ShowErrorDialog("Failed to load map");
        
    } catch (...) {
        DEBUG_LOG(("Map load failed with unknown exception"));
        ShowErrorDialog("Failed to load map: Unknown error");
    }
}

void ProcessGeometry() {
    try {
        geometry.Rebuild();
        DEBUG_LOG(("Geometry rebuilt successfully"));
        NotifyGeometryUpdated();
        
    } catch (const InvalidGeometryException& e) {
        DEBUG_LOG(("Geometry rebuild failed: Invalid geometry - %s", e.what()));
        ShowErrorDialog("Failed to rebuild geometry: Invalid data");
        
    } catch (const std::exception& e) {
        DEBUG_LOG(("Geometry rebuild failed: %s", e.what()));
        ShowErrorDialog("Failed to rebuild geometry");
        
    } catch (...) {
        DEBUG_LOG(("Geometry rebuild failed with unknown exception"));
        ShowErrorDialog("Failed to rebuild geometry: Unknown error");
    }
}
```

### Key Changes

1. ✅ **Specific exception types** - Different handling per error
2. ✅ **Logging on success** - Track successful operations
3. ✅ **Logging on failure** - Capture exception details
4. ✅ **User feedback** - Show error dialogs
5. ✅ **Cleanup** - Proper state management

### Rule

> **NEVER use empty catch blocks. At minimum, add `DEBUG_LOG` for debugging. Ideally, handle specific exceptions and provide user feedback.**

---

## Pattern 3: Unnecessary Try-Catch Removal

### ❌ BEFORE (Anti-pattern - Defensive Overkill)

```cpp
// File: Some game code
// Problem: Try-catching operations that never throw

void ProcessString() {
    try {
        std::string name = "Player";  // Never throws
        int length = name.length();   // Never throws
        bool empty = name.empty();    // Never throws
        name.clear();                 // Never throws
        return length;
    } catch(...) {
        return -1;  // PROBLEM: This catch never executes
    }
}

void ProcessNumbers() {
    try {
        int x = 10;      // Never throws
        int y = 20;      // Never throws
        int sum = x + y; // Never throws
        return sum;
    } catch(...) {
        return 0;  // PROBLEM: Unnecessary defensive programming
    }
}

void GetConfigValue() {
    try {
        if (config.has("key")) {         // May throw
            return config.get("key");    // May throw
        }
        return default_value;            // Never throws
    } catch(...) {
        return default_value;
    }
}
```

### ✅ AFTER (Production pattern - Appropriate Error Handling)

```cpp
// File: Some game code
// Clean - only wrap code that actually throws

void ProcessString() {
    std::string name = "Player";  // ✅ No try - never throws
    int length = name.length();   // ✅ No try - never throws
    bool empty = name.empty();    // ✅ No try - never throws
    name.clear();                 // ✅ No try - never throws
    return length;
}

void ProcessNumbers() {
    int x = 10;      // ✅ No try - never throws
    int y = 20;      // ✅ No try - never throws
    int sum = x + y; // ✅ No try - never throws
    return sum;
}

void GetConfigValue() {
    try {
        // ✅ Only wrap code that can throw
        if (config.has("key")) {
            return config.get("key");  // May throw
        }
        return default_value;  // Never throws, outside try
        
    } catch (const std::exception& e) {
        DEBUG_LOG(("Config access failed: %s", e.what()));
        return default_value;
    }
}
```

### Decision Tree

```
Does this code throw exceptions?
├─ YES → Keep try-catch with proper logging
├─ NO, but uses external library → Wrap if library may throw
└─ NO, only basic operations → Remove try-catch
```

### Rule

> **Only use try-catch for code that actually throws exceptions. Unnecessary try-catch adds confusion and maintenance burden without benefit.**

---

## Pattern 4: Commented Code Removal

### ❌ BEFORE (Anti-pattern - Dead Code)

```cpp
// File: Core/GameEngine/Source/GameClient/ObjectiveTracker.cpp
// Old - contains commented-out implementations

void ObjectiveTracker::Update() {
    // Old implementation - replaced with new system
    /*
    for (int i = 0; i < m_objectives.size(); i++) {
        Objective* obj = m_objectives[i];
        if (obj->IsActive()) {
            obj->Update();
        }
    }
    */
    
    // New implementation - uses iterator pattern
    for (auto& obj : m_objectives) {
        if (obj.IsActive()) {
            obj.Update();  // Vector-based, cleaner
        }
    }
}

void ObjectiveTracker::Display() {
    // TODO: Fix this for new rendering system
    /*
    for (int i = 0; i < m_objectives.size(); i++) {
        m_objectives[i]->Render(device, 100, 50 + i * 20);
    }
    */
    
    // Temporary workaround while new UI system is developed
    // This should be replaced with proper UI framework
    RenderObjectivesOldWay();  // PROBLEM: References commented-out code
}

// Backup implementation - kept just in case
/*
Bool ObjectiveTracker::SetObjectiveStatus_Old(UnsignedInt id, ObjectiveStatus status) {
    // Old implementation
    ...
}
*/
```

### ✅ AFTER (Production pattern - Clean Code)

```cpp
// File: Core/GameEngine/Source/GameClient/ObjectiveTracker.cpp
// New - clean, no dead code

void ObjectiveTracker::Update() {
    // Uses iterator pattern for safety and clarity
    for (auto& obj : m_objectives) {
        if (obj.IsActive()) {
            obj.Update();
        }
    }
}

void ObjectiveTracker::Display() {
    // Renders objectives using modern UI framework
    for (auto& obj : m_objectives) {
        obj.Render(m_ui_context);
    }
}

// ✅ No backup implementations - git history tracks old versions
// ✅ No commented-out code - cleaner to read and maintain
```

### When to Keep Comments

```cpp
// ✅ KEEP - Explains why code exists
// Changed from linked list to vector for O(1) lookup performance
for (auto& obj : m_objectives) {
    obj.Update();
}

// ✅ KEEP - References issue tracker
// TODO-123: Replace with proper audio system when Phase 47 complete
AudioManager* audio = CreateAudioManager();

// ✅ KEEP - Explains non-obvious logic
// Must iterate backwards to safely remove elements during loop
for (int i = m_objectives.size() - 1; i >= 0; i--) {
    if (m_objectives[i].IsComplete()) {
        m_objectives.erase(i);
    }
}
```

### Rule

> **Delete ALL commented-out code blocks (>3 lines). Git history tracks old implementations. Clean code is more important than keeping backups.**

---

## Pattern 5: Stale TODO/FIXME Cleanup

### ❌ BEFORE (Anti-pattern - Stale Comments)

```cpp
// File: Core/GameEngine/Source/GameClient/ObjectiveTracker.cpp

// TODO: Implement objective tracker
// FIXME: Make this work with new render system
// HACK: This is temporary until Phase 48
class ObjectiveTracker {
public:
    // TODO: Add comments to all methods
    void Update();
    
    // FIXME: Memory leak here if exception thrown
    Objective* CreateObjective();
    
    // TODO: Optimize this O(n²) algorithm
    void CheckCompletionConditions();
};

void ObjectiveTracker::Update() {
    // HACK: Workaround for bug #456
    // TODO: Remove when graphics layer fixed
    // FIXME: This causes 10% performance hit
    for (auto& obj : m_objectives) {
        if (obj.IsActive()) {
            obj.Update();
        }
    }
}
```

### ✅ AFTER (Production pattern - Clean Comments)

```cpp
// File: Core/GameEngine/Source/GameClient/ObjectiveTracker.cpp

// Manages mission objectives and tracks completion status
// Integrated with campaign system in Phase 47
class ObjectiveTracker {
public:
    // Performs per-frame objective updates and checks completion conditions
    void Update();
    
    // Creates new objective and registers for tracking
    // Returns nullptr if creation fails (logged via DEBUG_LOG)
    Objective* CreateObjective();
    
    // Checks if all objectives are completed (O(n) iteration)
    // Performance acceptable for typical mission (10-20 objectives)
    void CheckCompletionConditions();
};

void ObjectiveTracker::Update() {
    // Iterate through active objectives and update state
    for (auto& obj : m_objectives) {
        if (obj.IsActive()) {
            obj.Update();  // Maintains ~0.1ms per objective
        }
    }
}

// Reference: GitHub issue #456 - Resolved in Phase 39.3 graphics refactor
// No longer applies - leaving for historical reference
```

### TODO Comment Policy

**Keep TODOs only if**:
- Blocked by external factor (GitHub issue reference)
- Cross-phase dependency (with explicit phase number)
- Specific, actionable, with context

**Remove TODOs if**:
- Already implemented (remove comment)
- Stale (no context, no issue reference)
- General/vague (too broad)

### Rule

> **Clean up TODO/FIXME comments. Keep only those referencing GitHub issues or specific blockers. Delete vague comments.**

---

## Summary: Code Quality Pattern Checklist

| Pattern | Before | After | Rule |
|---------|--------|-------|------|
| **Factory Methods** | `return nullptr;` | Real instance | Always try to create real instances |
| **Try-Catch** | `catch(...) {}` | `DEBUG_LOG + handling` | Always log exceptions |
| **Unnecessary Try** | `try { safe_op; }` | `safe_op;` | Only wrap throwing code |
| **Commented Code** | `/* old impl */` | Deleted | Delete dead code |
| **TODO Comments** | `// TODO: fix` | Removed/referenced | Keep only if actionable |

---

**Document Version**: 1.0  
**Created**: November 26, 2025  
**Purpose**: Reference for Phase 48 implementation

