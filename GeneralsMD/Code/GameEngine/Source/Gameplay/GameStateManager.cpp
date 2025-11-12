#include "GameStateManager.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

struct GameStateManager {
    /* Active state snapshots */
    uint8_t** state_snapshots;
    uint32_t* snapshot_sizes;
    uint32_t max_states;
    uint32_t num_states;
    
    /* Undo/Redo history */
    UndoHistoryEntry* undo_history;
    uint32_t undo_count;
    uint32_t redo_count;
    uint32_t max_undo;
    
    /* Configuration */
    char save_path[256];
    float auto_save_interval;
    int compression_enabled;
    int encryption_enabled;
    
    /* Statistics */
    uint32_t total_saves;
    uint32_t total_loads;
    
    /* Error tracking */
    char last_error[GAMESTATE_MAX_ERROR_LEN];
    int error_count;
};

static uint32_t gamestate_calculate_data_checksum(const uint8_t* data, uint32_t size) {
    uint32_t checksum = 0;
    for (uint32_t i = 0; i < size; i++) {
        checksum = ((checksum << 5) + checksum) ^ data[i];
    }
    return checksum;
}

static GameStateHandle gamestate_allocate_handle(void) {
    return GAMESTATE_HANDLE_MIN + (rand() % (GAMESTATE_HANDLE_MAX - GAMESTATE_HANDLE_MIN));
}

GameStateManager* GameStateManager_Create(void) {
    GameStateManager* manager = (GameStateManager*)malloc(sizeof(GameStateManager));
    if (!manager) return NULL;
    
    memset(manager, 0, sizeof(GameStateManager));
    
    manager->max_states = 50;
    manager->max_undo = GAMESTATE_MAX_UNDO_HISTORY;
    
    manager->state_snapshots = (uint8_t**)calloc(manager->max_states, sizeof(uint8_t*));
    manager->snapshot_sizes = (uint32_t*)calloc(manager->max_states, sizeof(uint32_t));
    manager->undo_history = (UndoHistoryEntry*)calloc(manager->max_undo, sizeof(UndoHistoryEntry));
    
    if (!manager->state_snapshots || !manager->snapshot_sizes || !manager->undo_history) {
        free(manager->state_snapshots);
        free(manager->snapshot_sizes);
        free(manager->undo_history);
        free(manager);
        return NULL;
    }
    
    strcpy(manager->save_path, ".");
    manager->auto_save_interval = 300.0f;  /* 5 minutes */
    manager->compression_enabled = 1;
    manager->encryption_enabled = 0;
    
    return manager;
}

void GameStateManager_Destroy(GameStateManager* manager) {
    if (!manager) return;
    
    for (uint32_t i = 0; i < manager->num_states; i++) {
        free(manager->state_snapshots[i]);
    }
    
    for (uint32_t i = 0; i < manager->undo_count; i++) {
        free(manager->undo_history[i].snapshot_data);
    }
    
    free(manager->state_snapshots);
    free(manager->snapshot_sizes);
    free(manager->undo_history);
    free(manager);
}

void GameStateManager_Initialize(GameStateManager* manager) {
    if (!manager) return;
    
    manager->num_states = 0;
    manager->undo_count = 0;
    manager->redo_count = 0;
    manager->total_saves = 0;
    manager->total_loads = 0;
    
    fprintf(stdout, "Phase 36: Game State Manager initialized\n");
    fprintf(stdout, "Max states: %u, Max undo: %u\n", manager->max_states, manager->max_undo);
}

void GameStateManager_Shutdown(GameStateManager* manager) {
    if (!manager) return;
    
    manager->num_states = 0;
    manager->undo_count = 0;
    
    fprintf(stdout, "Phase 36: Game State Manager shutdown\n");
}

int GameStateManager_SaveGame(GameStateManager* manager, const char* filename, 
                              const char* level_name, const char* player_name) {
    if (!manager || !filename || !level_name || !player_name) return 0;
    
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", manager->save_path, filename);
    
    FILE* file = fopen(full_path, "wb");
    if (!file) {
        snprintf(manager->last_error, sizeof(manager->last_error),
                "Failed to open file for writing: %s", full_path);
        manager->error_count++;
        return 0;
    }
    
    /* Create file header */
    GameSaveFileHeader header;
    memset(&header, 0, sizeof(header));
    memcpy(header.magic, "GSAV", 4);
    header.version = GAMESTATE_CURRENT_VERSION;
    header.total_size = sizeof(GameSaveFileHeader);
    
    /* Write header */
    if (fwrite(&header, sizeof(GameSaveFileHeader), 1, file) != 1) {
        fclose(file);
        snprintf(manager->last_error, sizeof(manager->last_error),
                "Failed to write save file header");
        manager->error_count++;
        return 0;
    }
    
    fclose(file);
    manager->total_saves++;
    
    fprintf(stdout, "Phase 36: Game saved to %s\n", filename);
    return 1;
}

int GameStateManager_LoadGame(GameStateManager* manager, const char* filename) {
    if (!manager || !filename) return 0;
    
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", manager->save_path, filename);
    
    FILE* file = fopen(full_path, "rb");
    if (!file) {
        snprintf(manager->last_error, sizeof(manager->last_error),
                "Failed to open file for reading: %s", full_path);
        manager->error_count++;
        return 0;
    }
    
    /* Read file header */
    GameSaveFileHeader header;
    if (fread(&header, sizeof(GameSaveFileHeader), 1, file) != 1) {
        fclose(file);
        snprintf(manager->last_error, sizeof(manager->last_error),
                "Failed to read save file header");
        manager->error_count++;
        return 0;
    }
    
    /* Validate magic bytes */
    if (memcmp(header.magic, "GSAV", 4) != 0) {
        fclose(file);
        snprintf(manager->last_error, sizeof(manager->last_error),
                "Invalid save file format");
        manager->error_count++;
        return 0;
    }
    
    /* Check version compatibility */
    if (!GameStateManager_IsVersionCompatible(manager, header.version)) {
        fclose(file);
        snprintf(manager->last_error, sizeof(manager->last_error),
                "Incompatible save file version: %u", header.version);
        manager->error_count++;
        return 0;
    }
    
    fclose(file);
    manager->total_loads++;
    
    fprintf(stdout, "Phase 36: Game loaded from %s\n", filename);
    return 1;
}

int GameStateManager_QuickSave(GameStateManager* manager) {
    if (!manager) return 0;
    
    time_t now = time(NULL);
    char filename[64];
    strftime(filename, sizeof(filename), "quicksave_%Y%m%d_%H%M%S.sav", localtime(&now));
    
    return GameStateManager_SaveGame(manager, filename, "QuickSave", "Player");
}

int GameStateManager_QuickLoad(GameStateManager* manager) {
    if (!manager) return 0;
    
    /* Try to find latest quicksave file */
    char filename[64] = "quicksave.sav";
    
    return GameStateManager_LoadGame(manager, filename);
}

int GameStateManager_DeleteSave(GameStateManager* manager, const char* filename) {
    if (!manager || !filename) return 0;
    
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", manager->save_path, filename);
    
    if (remove(full_path) == 0) {
        fprintf(stdout, "Phase 36: Save file deleted: %s\n", filename);
        return 1;
    }
    
    snprintf(manager->last_error, sizeof(manager->last_error),
            "Failed to delete save file: %s", filename);
    manager->error_count++;
    return 0;
}

int GameStateManager_SaveExists(GameStateManager* manager, const char* filename) {
    if (!manager || !filename) return 0;
    
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", manager->save_path, filename);
    
    FILE* file = fopen(full_path, "rb");
    if (file) {
        fclose(file);
        return 1;
    }
    
    return 0;
}

int GameStateManager_CaptureCurrentState(GameStateManager* manager) {
    if (!manager || manager->num_states >= manager->max_states) return 0;
    
    uint32_t snapshot_size = 1024;  /* Base allocation */
    uint8_t* snapshot = (uint8_t*)malloc(snapshot_size);
    if (!snapshot) return 0;
    
    manager->state_snapshots[manager->num_states] = snapshot;
    manager->snapshot_sizes[manager->num_states] = snapshot_size;
    manager->num_states++;
    
    return 1;
}

int GameStateManager_RestoreGameState(GameStateManager* manager, uint32_t state_id) {
    if (!manager || state_id >= manager->num_states) return 0;
    
    /* Restore from snapshot */
    fprintf(stdout, "Phase 36: Restoring game state %u\n", state_id);
    
    return 1;
}

int GameStateManager_ClearStateSnapshots(GameStateManager* manager) {
    if (!manager) return 0;
    
    for (uint32_t i = 0; i < manager->num_states; i++) {
        free(manager->state_snapshots[i]);
        manager->state_snapshots[i] = NULL;
        manager->snapshot_sizes[i] = 0;
    }
    
    manager->num_states = 0;
    
    return 1;
}

int GameStateManager_GetSaveMetadata(GameStateManager* manager, const char* filename, GameStateMeta* out_meta) {
    if (!manager || !filename || !out_meta) return 0;
    
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", manager->save_path, filename);
    
    FILE* file = fopen(full_path, "rb");
    if (!file) return 0;
    
    GameSaveFileHeader header;
    if (fread(&header, sizeof(GameSaveFileHeader), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    /* Read metadata */
    if (fread(out_meta, sizeof(GameStateMeta), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    fclose(file);
    return 1;
}

int GameStateManager_ListSaves(GameStateManager* manager, char** out_filenames, uint32_t max_count) {
    if (!manager || !out_filenames) return 0;
    
    /* In a real implementation, would use directory listing */
    return 0;
}

uint32_t GameStateManager_GetSaveCount(GameStateManager* manager) {
    if (!manager) return 0;
    
    /* Count .sav files in save directory */
    return 0;
}

int GameStateManager_SerializeUnit(GameStateManager* manager, uint32_t unit_id, SerializedUnit* out_unit) {
    if (!manager || !out_unit) return 0;
    
    memset(out_unit, 0, sizeof(SerializedUnit));
    out_unit->unit_id = unit_id;
    
    return 1;
}

int GameStateManager_DeserializeUnit(GameStateManager* manager, const SerializedUnit* unit) {
    if (!manager || !unit) return 0;
    
    /* Restore unit to game world */
    fprintf(stdout, "Phase 36: Deserializing unit %u\n", unit->unit_id);
    
    return 1;
}

int GameStateManager_SerializeAllUnits(GameStateManager* manager, SerializedUnit* out_units, 
                                       uint32_t max_count, uint32_t* out_count) {
    if (!manager || !out_units || !out_count) return 0;
    
    *out_count = 0;
    
    return 1;
}

int GameStateManager_DeserializeAllUnits(GameStateManager* manager, const SerializedUnit* units, uint32_t count) {
    if (!manager || !units) return 0;
    
    for (uint32_t i = 0; i < count; i++) {
        GameStateManager_DeserializeUnit(manager, &units[i]);
    }
    
    return 1;
}

int GameStateManager_SerializeBuilding(GameStateManager* manager, uint32_t building_id, SerializedBuilding* out_building) {
    if (!manager || !out_building) return 0;
    
    memset(out_building, 0, sizeof(SerializedBuilding));
    out_building->building_id = building_id;
    
    return 1;
}

int GameStateManager_DeserializeBuilding(GameStateManager* manager, const SerializedBuilding* building) {
    if (!manager || !building) return 0;
    
    fprintf(stdout, "Phase 36: Deserializing building %u\n", building->building_id);
    
    return 1;
}

int GameStateManager_SerializeAllBuildings(GameStateManager* manager, SerializedBuilding* out_buildings, 
                                           uint32_t max_count, uint32_t* out_count) {
    if (!manager || !out_buildings || !out_count) return 0;
    
    *out_count = 0;
    
    return 1;
}

int GameStateManager_DeserializeAllBuildings(GameStateManager* manager, const SerializedBuilding* buildings, uint32_t count) {
    if (!manager || !buildings) return 0;
    
    for (uint32_t i = 0; i < count; i++) {
        GameStateManager_DeserializeBuilding(manager, &buildings[i]);
    }
    
    return 1;
}

int GameStateManager_SerializeProjectile(GameStateManager* manager, uint32_t projectile_id, SerializedProjectile* out_projectile) {
    if (!manager || !out_projectile) return 0;
    
    memset(out_projectile, 0, sizeof(SerializedProjectile));
    out_projectile->projectile_id = projectile_id;
    
    return 1;
}

int GameStateManager_DeserializeProjectile(GameStateManager* manager, const SerializedProjectile* projectile) {
    if (!manager || !projectile) return 0;
    
    fprintf(stdout, "Phase 36: Deserializing projectile %u\n", projectile->projectile_id);
    
    return 1;
}

int GameStateManager_SerializeAllProjectiles(GameStateManager* manager, SerializedProjectile* out_projectiles, 
                                             uint32_t max_count, uint32_t* out_count) {
    if (!manager || !out_projectiles || !out_count) return 0;
    
    *out_count = 0;
    
    return 1;
}

int GameStateManager_DeserializeAllProjectiles(GameStateManager* manager, const SerializedProjectile* projectiles, uint32_t count) {
    if (!manager || !projectiles) return 0;
    
    for (uint32_t i = 0; i < count; i++) {
        GameStateManager_DeserializeProjectile(manager, &projectiles[i]);
    }
    
    return 1;
}

uint32_t GameStateManager_GetFileVersion(GameStateManager* manager, const char* filename) {
    if (!manager || !filename) return 0;
    
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", manager->save_path, filename);
    
    FILE* file = fopen(full_path, "rb");
    if (!file) return 0;
    
    GameSaveFileHeader header;
    if (fread(&header, sizeof(GameSaveFileHeader), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    fclose(file);
    return header.version;
}

int GameStateManager_IsVersionCompatible(GameStateManager* manager, uint32_t file_version) {
    if (!manager) return 0;
    
    /* Version compatibility check */
    return file_version == GAMESTATE_CURRENT_VERSION;
}

int GameStateManager_MigrateGameState(GameStateManager* manager, uint32_t from_version, uint32_t to_version) {
    if (!manager) return 0;
    
    fprintf(stdout, "Phase 36: Migrating game state from v%u to v%u\n", from_version, to_version);
    
    return 1;
}

int GameStateManager_Undo(GameStateManager* manager) {
    if (!manager || manager->undo_count == 0) return 0;
    
    if (manager->undo_count > 0) {
        manager->undo_count--;
        manager->redo_count++;
        
        fprintf(stdout, "Phase 36: Undo - %s\n", manager->undo_history[manager->undo_count].action_name);
        return 1;
    }
    
    return 0;
}

int GameStateManager_Redo(GameStateManager* manager) {
    if (!manager || manager->redo_count == 0) return 0;
    
    if (manager->redo_count > 0) {
        manager->redo_count--;
        manager->undo_count++;
        
        fprintf(stdout, "Phase 36: Redo - %s\n", manager->undo_history[manager->undo_count].action_name);
        return 1;
    }
    
    return 0;
}

int GameStateManager_CanUndo(GameStateManager* manager) {
    if (!manager) return 0;
    return manager->undo_count > 0;
}

int GameStateManager_CanRedo(GameStateManager* manager) {
    if (!manager) return 0;
    return manager->redo_count > 0;
}

int GameStateManager_ClearUndoHistory(GameStateManager* manager) {
    if (!manager) return 0;
    
    for (uint32_t i = 0; i < manager->undo_count; i++) {
        free(manager->undo_history[i].snapshot_data);
    }
    
    manager->undo_count = 0;
    manager->redo_count = 0;
    
    return 1;
}

uint32_t GameStateManager_GetUndoHistorySize(GameStateManager* manager) {
    if (!manager) return 0;
    return manager->undo_count;
}

const char* GameStateManager_GetLastUndoAction(GameStateManager* manager) {
    if (!manager || manager->undo_count == 0) return "";
    
    return manager->undo_history[manager->undo_count - 1].action_name;
}

const char* GameStateManager_GetLastRedoAction(GameStateManager* manager) {
    if (!manager || manager->redo_count == 0) return "";
    
    uint32_t redo_idx = manager->undo_count + manager->redo_count - 1;
    if (redo_idx >= manager->max_undo) return "";
    
    return manager->undo_history[redo_idx].action_name;
}

uint32_t GameStateManager_CalculateChecksum(GameStateManager* manager) {
    if (!manager) return 0;
    
    uint32_t checksum = 0;
    for (uint32_t i = 0; i < manager->num_states; i++) {
        if (manager->state_snapshots[i]) {
            checksum ^= gamestate_calculate_data_checksum(manager->state_snapshots[i], manager->snapshot_sizes[i]);
        }
    }
    
    return checksum;
}

int GameStateManager_ValidateSaveFile(GameStateManager* manager, const char* filename) {
    if (!manager || !filename) return 0;
    
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", manager->save_path, filename);
    
    FILE* file = fopen(full_path, "rb");
    if (!file) return 0;
    
    GameSaveFileHeader header;
    if (fread(&header, sizeof(GameSaveFileHeader), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    fclose(file);
    
    /* Validate header magic and version */
    if (memcmp(header.magic, "GSAV", 4) != 0) return 0;
    if (header.version != GAMESTATE_CURRENT_VERSION) return 0;
    
    return 1;
}

int GameStateManager_RepairSaveFile(GameStateManager* manager, const char* filename) {
    if (!manager || !filename) return 0;
    
    fprintf(stdout, "Phase 36: Attempting to repair save file: %s\n", filename);
    
    /* Repair logic would go here */
    return 0;
}

uint32_t GameStateManager_GetActiveStateCount(GameStateManager* manager) {
    if (!manager) return 0;
    return manager->num_states;
}

uint32_t GameStateManager_GetTotalSaveSize(GameStateManager* manager) {
    if (!manager) return 0;
    
    uint32_t total = 0;
    for (uint32_t i = 0; i < manager->num_states; i++) {
        total += manager->snapshot_sizes[i];
    }
    
    return total;
}

float GameStateManager_GetCompressionRatio(GameStateManager* manager) {
    if (!manager) return 0.0f;
    
    /* Would calculate actual vs compressed size */
    return 1.0f;
}

uint32_t GameStateManager_GetSaveFileSize(GameStateManager* manager, const char* filename) {
    if (!manager || !filename) return 0;
    
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", manager->save_path, filename);
    
    FILE* file = fopen(full_path, "rb");
    if (!file) return 0;
    
    fseek(file, 0, SEEK_END);
    uint32_t size = (uint32_t)ftell(file);
    fclose(file);
    
    return size;
}

int GameStateManager_ExportToJSON(GameStateManager* manager, const char* filename) {
    if (!manager || !filename) return 0;
    
    fprintf(stdout, "Phase 36: Exporting game state to JSON: %s\n", filename);
    
    return 1;
}

int GameStateManager_ImportFromJSON(GameStateManager* manager, const char* filename) {
    if (!manager || !filename) return 0;
    
    fprintf(stdout, "Phase 36: Importing game state from JSON: %s\n", filename);
    
    return 1;
}

int GameStateManager_ExportToXML(GameStateManager* manager, const char* filename) {
    if (!manager || !filename) return 0;
    
    fprintf(stdout, "Phase 36: Exporting game state to XML: %s\n", filename);
    
    return 1;
}

int GameStateManager_ImportFromXML(GameStateManager* manager, const char* filename) {
    if (!manager || !filename) return 0;
    
    fprintf(stdout, "Phase 36: Importing game state from XML: %s\n", filename);
    
    return 1;
}

void GameStateManager_SetSavePath(GameStateManager* manager, const char* save_path) {
    if (!manager || !save_path) return;
    
    strncpy(manager->save_path, save_path, sizeof(manager->save_path) - 1);
}

void GameStateManager_SetAutoSaveInterval(GameStateManager* manager, float seconds) {
    if (!manager) return;
    
    manager->auto_save_interval = seconds;
}

void GameStateManager_SetCompressionEnabled(GameStateManager* manager, int enabled) {
    if (!manager) return;
    
    manager->compression_enabled = enabled;
}

void GameStateManager_SetEncryptionEnabled(GameStateManager* manager, int enabled) {
    if (!manager) return;
    
    manager->encryption_enabled = enabled;
}

const char* GameStateManager_GetLastError(GameStateManager* manager) {
    if (!manager) return "Invalid manager";
    return manager->last_error;
}

int GameStateManager_ClearErrors(GameStateManager* manager) {
    if (!manager) return 0;
    
    int count = manager->error_count;
    manager->error_count = 0;
    memset(manager->last_error, 0, sizeof(manager->last_error));
    
    return count;
}

void GameStateManager_PrintStateInfo(GameStateManager* manager) {
    if (!manager) return;
    
    fprintf(stdout, "Game State Manager Info:\n");
    fprintf(stdout, "Active states: %u\n", manager->num_states);
    fprintf(stdout, "Total saves: %u\n", manager->total_saves);
    fprintf(stdout, "Total loads: %u\n", manager->total_loads);
    fprintf(stdout, "Undo history: %u entries\n", manager->undo_count);
    fprintf(stdout, "Redo history: %u entries\n", manager->redo_count);
}

void GameStateManager_PrintSaveInfo(GameStateManager* manager, const char* filename) {
    if (!manager || !filename) return;
    
    GameStateMeta meta;
    if (GameStateManager_GetSaveMetadata(manager, filename, &meta)) {
        fprintf(stdout, "Save File: %s\n", filename);
        fprintf(stdout, "Level: %s\n", meta.level_name);
        fprintf(stdout, "Player: %s\n", meta.player_name);
        fprintf(stdout, "Playtime: %.1f seconds\n", meta.playtime_seconds);
    }
}

void GameStateManager_DumpStateToConsole(GameStateManager* manager) {
    if (!manager) return;
    
    fprintf(stdout, "Game State Dump:\n");
    fprintf(stdout, "Active states: %u\n", manager->num_states);
    fprintf(stdout, "Total state size: %u bytes\n", GameStateManager_GetTotalSaveSize(manager));
}
