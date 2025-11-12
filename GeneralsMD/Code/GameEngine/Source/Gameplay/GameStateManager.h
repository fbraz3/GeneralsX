#ifndef GAME_STATE_MANAGER_H
#define GAME_STATE_MANAGER_H

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Handle range for Game State Manager: 36000-36999 (1,000 handles) */
typedef uint32_t GameStateHandle;
#define GAMESTATE_HANDLE_MIN 36000
#define GAMESTATE_HANDLE_MAX 36999

#define GAMESTATE_MAX_SAVES 100
#define GAMESTATE_MAX_UNDO_HISTORY 50
#define GAMESTATE_FILENAME_MAX 256
#define GAMESTATE_MAX_ERROR_LEN 256

/* Game state version for compatibility */
#define GAMESTATE_CURRENT_VERSION 1

/* Save game metadata */
typedef struct {
    uint32_t version;
    time_t timestamp;
    char level_name[128];
    char player_name[128];
    uint32_t difficulty;
    float playtime_seconds;
    uint32_t checksum;
} GameStateMeta;

/* Savegame file header */
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

/* Unit serialization data */
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

/* Building serialization data */
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

/* Projectile serialization data */
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

/* Waypoint serialization data */
typedef struct {
    float x;
    float y;
    uint32_t index;
} SerializedWaypoint;

/* Undo/redo history entry */
typedef struct {
    char action_name[64];
    time_t timestamp;
    uint32_t snapshot_size;
    uint8_t* snapshot_data;
} UndoHistoryEntry;

/* Opaque game state manager */
typedef struct GameStateManager GameStateManager;

/* System Lifecycle */
GameStateManager* GameStateManager_Create(void);
void GameStateManager_Destroy(GameStateManager* manager);
void GameStateManager_Initialize(GameStateManager* manager);
void GameStateManager_Shutdown(GameStateManager* manager);

/* Save/Load Operations */
int GameStateManager_SaveGame(GameStateManager* manager, const char* filename, const char* level_name, const char* player_name);
int GameStateManager_LoadGame(GameStateManager* manager, const char* filename);
int GameStateManager_QuickSave(GameStateManager* manager);
int GameStateManager_QuickLoad(GameStateManager* manager);
int GameStateManager_DeleteSave(GameStateManager* manager, const char* filename);
int GameStateManager_SaveExists(GameStateManager* manager, const char* filename);

/* Game State Capture */
int GameStateManager_CaptureCurrentState(GameStateManager* manager);
int GameStateManager_RestoreGameState(GameStateManager* manager, uint32_t state_id);
int GameStateManager_ClearStateSnapshots(GameStateManager* manager);

/* Metadata Management */
int GameStateManager_GetSaveMetadata(GameStateManager* manager, const char* filename, GameStateMeta* out_meta);
int GameStateManager_ListSaves(GameStateManager* manager, char** out_filenames, uint32_t max_count);
uint32_t GameStateManager_GetSaveCount(GameStateManager* manager);

/* Unit Serialization */
int GameStateManager_SerializeUnit(GameStateManager* manager, uint32_t unit_id, SerializedUnit* out_unit);
int GameStateManager_DeserializeUnit(GameStateManager* manager, const SerializedUnit* unit);
int GameStateManager_SerializeAllUnits(GameStateManager* manager, SerializedUnit* out_units, uint32_t max_count, uint32_t* out_count);
int GameStateManager_DeserializeAllUnits(GameStateManager* manager, const SerializedUnit* units, uint32_t count);

/* Building Serialization */
int GameStateManager_SerializeBuilding(GameStateManager* manager, uint32_t building_id, SerializedBuilding* out_building);
int GameStateManager_DeserializeBuilding(GameStateManager* manager, const SerializedBuilding* building);
int GameStateManager_SerializeAllBuildings(GameStateManager* manager, SerializedBuilding* out_buildings, uint32_t max_count, uint32_t* out_count);
int GameStateManager_DeserializeAllBuildings(GameStateManager* manager, const SerializedBuilding* buildings, uint32_t count);

/* Projectile Serialization */
int GameStateManager_SerializeProjectile(GameStateManager* manager, uint32_t projectile_id, SerializedProjectile* out_projectile);
int GameStateManager_DeserializeProjectile(GameStateManager* manager, const SerializedProjectile* projectile);
int GameStateManager_SerializeAllProjectiles(GameStateManager* manager, SerializedProjectile* out_projectiles, uint32_t max_count, uint32_t* out_count);
int GameStateManager_DeserializeAllProjectiles(GameStateManager* manager, const SerializedProjectile* projectiles, uint32_t count);

/* Version Management */
uint32_t GameStateManager_GetFileVersion(GameStateManager* manager, const char* filename);
int GameStateManager_IsVersionCompatible(GameStateManager* manager, uint32_t file_version);
int GameStateManager_MigrateGameState(GameStateManager* manager, uint32_t from_version, uint32_t to_version);

/* Undo/Redo System */
int GameStateManager_Undo(GameStateManager* manager);
int GameStateManager_Redo(GameStateManager* manager);
int GameStateManager_CanUndo(GameStateManager* manager);
int GameStateManager_CanRedo(GameStateManager* manager);
int GameStateManager_ClearUndoHistory(GameStateManager* manager);
uint32_t GameStateManager_GetUndoHistorySize(GameStateManager* manager);
const char* GameStateManager_GetLastUndoAction(GameStateManager* manager);
const char* GameStateManager_GetLastRedoAction(GameStateManager* manager);

/* State Checksum & Validation */
uint32_t GameStateManager_CalculateChecksum(GameStateManager* manager);
int GameStateManager_ValidateSaveFile(GameStateManager* manager, const char* filename);
int GameStateManager_RepairSaveFile(GameStateManager* manager, const char* filename);

/* Statistics & Info */
uint32_t GameStateManager_GetActiveStateCount(GameStateManager* manager);
uint32_t GameStateManager_GetTotalSaveSize(GameStateManager* manager);
float GameStateManager_GetCompressionRatio(GameStateManager* manager);
uint32_t GameStateManager_GetSaveFileSize(GameStateManager* manager, const char* filename);

/* Save Game Export/Import */
int GameStateManager_ExportToJSON(GameStateManager* manager, const char* filename);
int GameStateManager_ImportFromJSON(GameStateManager* manager, const char* filename);
int GameStateManager_ExportToXML(GameStateManager* manager, const char* filename);
int GameStateManager_ImportFromXML(GameStateManager* manager, const char* filename);

/* Configuration */
void GameStateManager_SetSavePath(GameStateManager* manager, const char* save_path);
void GameStateManager_SetAutoSaveInterval(GameStateManager* manager, float seconds);
void GameStateManager_SetCompressionEnabled(GameStateManager* manager, int enabled);
void GameStateManager_SetEncryptionEnabled(GameStateManager* manager, int enabled);

/* Error Handling */
const char* GameStateManager_GetLastError(GameStateManager* manager);
int GameStateManager_ClearErrors(GameStateManager* manager);

/* Debug & Utilities */
void GameStateManager_PrintStateInfo(GameStateManager* manager);
void GameStateManager_PrintSaveInfo(GameStateManager* manager, const char* filename);
void GameStateManager_DumpStateToConsole(GameStateManager* manager);

#ifdef __cplusplus
}
#endif

#endif /* GAME_STATE_MANAGER_H */
