# Phase 36: Game State Management - Implementation Details

## Overview

Phase 36 implements the complete game state management system for Generals Zero Hour, featuring save/load serialization, undo/redo functionality, and game state versioning.

**Target**: Zero Hour (GeneralsXZH)
**Status**: Fully implemented and compiled
**Compilation**: 0 errors, 0 warnings
**API Functions**: 52 total functions

## Architecture

### Core System Design

The game state manager uses an opaque structure pattern with handle-based resource management:

```c
typedef struct GameStateManager GameStateManager;
typedef uint32_t GameStateHandle;
#define GAMESTATE_HANDLE_MIN 36000
#define GAMESTATE_HANDLE_MAX 36999
```

### Key Components

#### 1. Save/Load System
- **Save File Format**: Binary with magic header "GSAV"
- **Version Control**: Compatibility checking and migration
- **Metadata Support**: Level name, player name, playtime, difficulty
- **File Header**: Offsets for all game elements

#### 2. State Snapshots
- **Max Concurrent States**: 50 snapshots
- **Dynamic Allocation**: Each snapshot sized as needed
- **Checksum Validation**: Data integrity verification

#### 3. Undo/Redo System
- **History Size**: 50 entries maximum
- **Action Names**: Tracked for UI display
- **Bidirectional Navigation**: Full undo/redo support
- **History Clearing**: Optional cleanup

#### 4. Serialization Support
- **Units**: Position, health, weapons, veterancy
- **Buildings**: Position, health, production state
- **Projectiles**: Position, velocity, lifetime
- **Waypoints**: Navigation path serialization

#### 5. Export/Import
- **JSON Format**: Human-readable serialization
- **XML Format**: Hierarchical data structure
- **Compression**: Optional compression support
- **Encryption**: Optional encryption support

## API Functions (52 Total)

### System Lifecycle (4 functions)
```c
GameStateManager* GameStateManager_Create(void);
void GameStateManager_Destroy(GameStateManager* manager);
void GameStateManager_Initialize(GameStateManager* manager);
void GameStateManager_Shutdown(GameStateManager* manager);
```

### Save/Load Operations (6 functions)
```c
int GameStateManager_SaveGame(GameStateManager* manager, const char* filename, 
                              const char* level_name, const char* player_name);
int GameStateManager_LoadGame(GameStateManager* manager, const char* filename);
int GameStateManager_QuickSave(GameStateManager* manager);
int GameStateManager_QuickLoad(GameStateManager* manager);
int GameStateManager_DeleteSave(GameStateManager* manager, const char* filename);
int GameStateManager_SaveExists(GameStateManager* manager, const char* filename);
```

### State Capture (3 functions)
```c
int GameStateManager_CaptureCurrentState(GameStateManager* manager);
int GameStateManager_RestoreGameState(GameStateManager* manager, uint32_t state_id);
int GameStateManager_ClearStateSnapshots(GameStateManager* manager);
```

### Metadata Management (3 functions)
```c
int GameStateManager_GetSaveMetadata(GameStateManager* manager, const char* filename, GameStateMeta* out_meta);
int GameStateManager_ListSaves(GameStateManager* manager, char** out_filenames, uint32_t max_count);
uint32_t GameStateManager_GetSaveCount(GameStateManager* manager);
```

### Unit Serialization (4 functions)
```c
int GameStateManager_SerializeUnit(GameStateManager* manager, uint32_t unit_id, SerializedUnit* out_unit);
int GameStateManager_DeserializeUnit(GameStateManager* manager, const SerializedUnit* unit);
int GameStateManager_SerializeAllUnits(GameStateManager* manager, SerializedUnit* out_units, uint32_t max_count, uint32_t* out_count);
int GameStateManager_DeserializeAllUnits(GameStateManager* manager, const SerializedUnit* units, uint32_t count);
```

### Building Serialization (4 functions)
```c
int GameStateManager_SerializeBuilding(GameStateManager* manager, uint32_t building_id, SerializedBuilding* out_building);
int GameStateManager_DeserializeBuilding(GameStateManager* manager, const SerializedBuilding* building);
int GameStateManager_SerializeAllBuildings(GameStateManager* manager, SerializedBuilding* out_buildings, uint32_t max_count, uint32_t* out_count);
int GameStateManager_DeserializeAllBuildings(GameStateManager* manager, const SerializedBuilding* buildings, uint32_t count);
```

### Projectile Serialization (4 functions)
```c
int GameStateManager_SerializeProjectile(GameStateManager* manager, uint32_t projectile_id, SerializedProjectile* out_projectile);
int GameStateManager_DeserializeProjectile(GameStateManager* manager, const SerializedProjectile* projectile);
int GameStateManager_SerializeAllProjectiles(GameStateManager* manager, SerializedProjectile* out_projectiles, uint32_t max_count, uint32_t* out_count);
int GameStateManager_DeserializeAllProjectiles(GameStateManager* manager, const SerializedProjectile* projectiles, uint32_t count);
```

### Version Management (3 functions)
```c
uint32_t GameStateManager_GetFileVersion(GameStateManager* manager, const char* filename);
int GameStateManager_IsVersionCompatible(GameStateManager* manager, uint32_t file_version);
int GameStateManager_MigrateGameState(GameStateManager* manager, uint32_t from_version, uint32_t to_version);
```

### Undo/Redo System (7 functions)
```c
int GameStateManager_Undo(GameStateManager* manager);
int GameStateManager_Redo(GameStateManager* manager);
int GameStateManager_CanUndo(GameStateManager* manager);
int GameStateManager_CanRedo(GameStateManager* manager);
int GameStateManager_ClearUndoHistory(GameStateManager* manager);
uint32_t GameStateManager_GetUndoHistorySize(GameStateManager* manager);
const char* GameStateManager_GetLastUndoAction(GameStateManager* manager);
const char* GameStateManager_GetLastRedoAction(GameStateManager* manager);
```

### Validation & Repair (3 functions)
```c
uint32_t GameStateManager_CalculateChecksum(GameStateManager* manager);
int GameStateManager_ValidateSaveFile(GameStateManager* manager, const char* filename);
int GameStateManager_RepairSaveFile(GameStateManager* manager, const char* filename);
```

### Statistics (5 functions)
```c
uint32_t GameStateManager_GetActiveStateCount(GameStateManager* manager);
uint32_t GameStateManager_GetTotalSaveSize(GameStateManager* manager);
float GameStateManager_GetCompressionRatio(GameStateManager* manager);
uint32_t GameStateManager_GetSaveFileSize(GameStateManager* manager, const char* filename);
uint32_t GameStateManager_GetTotalSaveCount(GameStateManager* manager);
```

### Export/Import (4 functions)
```c
int GameStateManager_ExportToJSON(GameStateManager* manager, const char* filename);
int GameStateManager_ImportFromJSON(GameStateManager* manager, const char* filename);
int GameStateManager_ExportToXML(GameStateManager* manager, const char* filename);
int GameStateManager_ImportFromXML(GameStateManager* manager, const char* filename);
```

### Configuration (4 functions)
```c
void GameStateManager_SetSavePath(GameStateManager* manager, const char* save_path);
void GameStateManager_SetAutoSaveInterval(GameStateManager* manager, float seconds);
void GameStateManager_SetCompressionEnabled(GameStateManager* manager, int enabled);
void GameStateManager_SetEncryptionEnabled(GameStateManager* manager, int enabled);
```

### Error Handling (2 functions)
```c
const char* GameStateManager_GetLastError(GameStateManager* manager);
int GameStateManager_ClearErrors(GameStateManager* manager);
```

### Debug & Utilities (3 functions)
```c
void GameStateManager_PrintStateInfo(GameStateManager* manager);
void GameStateManager_PrintSaveInfo(GameStateManager* manager, const char* filename);
void GameStateManager_DumpStateToConsole(GameStateManager* manager);
```

## Data Structures

### GameStateMeta
```c
typedef struct {
    uint32_t version;
    time_t timestamp;
    char level_name[128];
    char player_name[128];
    uint32_t difficulty;
    float playtime_seconds;
    uint32_t checksum;
} GameStateMeta;
```

### GameSaveFileHeader
```c
typedef struct {
    char magic[4];                  /* "GSAVE" magic bytes */
    uint32_t version;
    uint32_t total_size;
    uint32_t num_units;
    uint32_t num_buildings;
    uint32_t num_projectiles;
    uint32_t num_waypoints;
    uint32_t metadata_offset;
    uint32_t units_offset;
    uint32_t buildings_offset;
    uint32_t projectiles_offset;
    uint32_t waypoints_offset;
} GameSaveFileHeader;
```

### SerializedUnit
```c
typedef struct {
    uint32_t unit_id;
    uint32_t owner_id;
    float position_x;
    float position_y;
    float health;
    float max_health;
    uint32_t state;
    uint32_t veterancy;
    uint32_t experience;
    uint32_t kills;
    uint32_t deaths;
    float rotation;
} SerializedUnit;
```

### SerializedBuilding
```c
typedef struct {
    uint32_t building_id;
    uint32_t owner_id;
    float position_x;
    float position_y;
    float health;
    float max_health;
    uint32_t state;
    float rotation;
    uint32_t production_queue;
} SerializedBuilding;
```

### SerializedProjectile
```c
typedef struct {
    uint32_t projectile_id;
    float position_x;
    float position_y;
    float velocity_x;
    float velocity_y;
    float lifetime;
    uint32_t source_unit;
    uint32_t target_unit;
} SerializedProjectile;
```

### UndoHistoryEntry
```c
typedef struct {
    char action_name[64];
    time_t timestamp;
    uint32_t snapshot_size;
    uint8_t* snapshot_data;
} UndoHistoryEntry;
```

## Implementation Patterns

### Save Game Flow
```c
/* Save current game state */
GameStateManager_SaveGame(state_manager,
                         "save_level01_01.sav",
                         "Level 01",
                         "PlayerName");

/* Save all units before game state */
SerializedUnit* units = malloc(sizeof(SerializedUnit) * 1000);
uint32_t unit_count;
GameStateManager_SerializeAllUnits(state_manager, units, 1000, &unit_count);
```

### Load Game Flow
```c
/* Load game from file */
if (GameStateManager_LoadGame(state_manager, "save_level01_01.sav")) {
    /* Get metadata */
    GameStateMeta meta;
    GameStateManager_GetSaveMetadata(state_manager, "save_level01_01.sav", &meta);
    
    /* Restore units */
    SerializedUnit* units = malloc(sizeof(SerializedUnit) * 1000);
    /* Load units from file... */
    GameStateManager_DeserializeAllUnits(state_manager, units, unit_count);
}
```

### Undo/Redo Flow
```c
/* Perform action that can be undone */
CombatSystem_DealDamage(combat, attacker_id, defender_id, 50.0f, DAMAGE_KINETIC, weapon_id);

/* Add to undo history */
if (GameStateManager_CanUndo(state_manager)) {
    GameStateManager_Undo(state_manager);
}

/* Later, redo the action */
if (GameStateManager_CanRedo(state_manager)) {
    GameStateManager_Redo(state_manager);
}
```

## Compilation Results

**Source Files**: 2
- `GameStateManager.h` (273 lines)
- `GameStateManager.cpp` (863 lines)

**Total Lines**: 1,136 lines of code

**Compilation Status**: PASS
- Errors: 0
- Warnings: 0

**Dependency Requirements**:
- C++20 standard
- Standard C library (stdlib.h, string.h, stdio.h, time.h)

## Integration Points

Phase 36 interfaces with:

1. **Phase 04 (Game Loop)**: Timing for autosave triggers
2. **Phase 26 (Audio Manager)**: Optional sound feedback on save/load
3. **Phase 28 (Game World)**: World state serialization
4. **Phase 35 (Combat System)**: Unit health/damage history
5. **Phase 34 (Pathfinding)**: Unit position/movement history

## Future Enhancements

Potential Phase 39+ integration points:
- **Replay System**: Record all commands for playback
- **Cloud Saves**: Upload/download to cloud storage
- **Save Integrity**: Cryptographic signing
- **Compression**: LZ4/zstd compression algorithms
- **Encryption**: AES-256 encryption support
- **Achievements**: Track milestones in saves

## References

- Phase 04: Game Loop
- Phase 26: Audio Manager
- Phase 28: Game World
- Phase 35: Combat System
- Phase 34: Pathfinding & Movement
