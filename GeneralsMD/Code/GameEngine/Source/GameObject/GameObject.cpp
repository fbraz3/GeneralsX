/**
 * @file GameObject.cpp
 * @brief Phase 27: Game Object System - Implementation
 *
 * Entity lifecycle management for units, buildings, effects.
 */

#include "GameObject.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#define MAX_GAME_OBJECTS 4096
#define MAX_CALLBACKS 32

// Error handling
static char g_error_message[256] = {0};

// Callback entries
typedef struct {
    GameObjectUpdateCallback update_callback;
    GameObjectDeathCallback death_callback;
    void* user_data;
} CallbackEntry;

// Game object system state
typedef struct {
    void* objects[MAX_GAME_OBJECTS];  // Generic object storage
    GameObjectType object_types[MAX_GAME_OBJECTS];
    uint32_t object_count;
    uint32_t max_objects;
    
    CallbackEntry callbacks[MAX_CALLBACKS];
    uint32_t callback_count;
    
    int is_initialized;
} GameObjectSystemState;

static GameObjectSystemState g_object_system = {0};

/**
 * Set error message
 */
static void SetError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(g_error_message, sizeof(g_error_message), format, args);
    va_end(args);
}

/**
 * Calculate distance between two vectors
 */
static float Distance(const Vector3D* v1, const Vector3D* v2) {
    float dx = v1->x - v2->x;
    float dy = v1->y - v2->y;
    float dz = v1->z - v2->z;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}

/**
 * Initialize game object system
 */
int GameObject_Initialize(uint32_t max_objects) {
    if (g_object_system.is_initialized) {
        return GAMEOBJECT_OK;
    }
    
    if (max_objects == 0 || max_objects > MAX_GAME_OBJECTS) {
        max_objects = MAX_GAME_OBJECTS;
    }
    
    memset(&g_object_system, 0, sizeof(GameObjectSystemState));
    g_object_system.max_objects = max_objects;
    g_object_system.is_initialized = 1;
    
    return GAMEOBJECT_OK;
}

/**
 * Shutdown game object system
 */
int GameObject_Shutdown(void) {
    if (!g_object_system.is_initialized) {
        SetError("Game object system not initialized");
        return GAMEOBJECT_ERROR_INVALID_PARAM;
    }
    
    // Free all objects
    for (uint32_t i = 0; i < g_object_system.object_count; i++) {
        if (g_object_system.objects[i]) {
            free(g_object_system.objects[i]);
            g_object_system.objects[i] = NULL;
        }
    }
    
    g_object_system.object_count = 0;
    g_object_system.callback_count = 0;
    g_object_system.is_initialized = 0;
    
    return GAMEOBJECT_OK;
}

/**
 * Create game object
 */
uint32_t GameObject_Create(GameObjectType type, uint32_t owner_team, const Vector3D* position) {
    if (!g_object_system.is_initialized) {
        SetError("Game object system not initialized");
        return 0;
    }
    
    if (!position) {
        SetError("Position pointer is NULL");
        return 0;
    }
    
    if (g_object_system.object_count >= g_object_system.max_objects) {
        SetError("Object limit exceeded");
        return 0;
    }
    
    uint32_t obj_idx = g_object_system.object_count;
    uint32_t object_id = 27000 + obj_idx;
    
    // Allocate object based on type
    void* new_object = NULL;
    
    switch (type) {
        case GAMEOBJECT_TYPE_UNIT: {
            Unit* unit = (Unit*)malloc(sizeof(Unit));
            if (!unit) {
                SetError("Failed to allocate unit");
                return 0;
            }
            memset(unit, 0, sizeof(Unit));
            unit->base.id = object_id;
            unit->base.type = type;
            unit->base.owner_team = owner_team;
            unit->base.position = *position;
            unit->base.max_health = 100.0f;
            unit->base.health = 100.0f;
            unit->base.is_active = 1;
            unit->base.is_visible = 1;
            unit->unit_state = UNIT_STATE_IDLE;
            unit->move_speed = 5.0f;
            unit->attack_range = 10.0f;
            new_object = unit;
            break;
        }
        
        case GAMEOBJECT_TYPE_BUILDING: {
            Building* building = (Building*)malloc(sizeof(Building));
            if (!building) {
                SetError("Failed to allocate building");
                return 0;
            }
            memset(building, 0, sizeof(Building));
            building->base.id = object_id;
            building->base.type = type;
            building->base.owner_team = owner_team;
            building->base.position = *position;
            building->base.max_health = 500.0f;
            building->base.health = 500.0f;
            building->base.is_active = 1;
            building->base.is_visible = 1;
            building->building_state = BUILDING_STATE_COMPLETE;
            building->armor_value = 0.5f;
            new_object = building;
            break;
        }
        
        case GAMEOBJECT_TYPE_EFFECT: {
            Effect* effect = (Effect*)malloc(sizeof(Effect));
            if (!effect) {
                SetError("Failed to allocate effect");
                return 0;
            }
            memset(effect, 0, sizeof(Effect));
            effect->base.id = object_id;
            effect->base.type = type;
            effect->base.position = *position;
            effect->base.is_active = 1;
            effect->base.is_visible = 1;
            effect->emission_rate = 100.0f;
            new_object = effect;
            break;
        }
        
        case GAMEOBJECT_TYPE_PROJECTILE: {
            Projectile* projectile = (Projectile*)malloc(sizeof(Projectile));
            if (!projectile) {
                SetError("Failed to allocate projectile");
                return 0;
            }
            memset(projectile, 0, sizeof(Projectile));
            projectile->base.id = object_id;
            projectile->base.type = type;
            projectile->base.position = *position;
            projectile->base.is_active = 1;
            projectile->base.is_visible = 1;
            projectile->projectile_speed = 20.0f;
            new_object = projectile;
            break;
        }
        
        case GAMEOBJECT_TYPE_PROP: {
            GameObject* prop = (GameObject*)malloc(sizeof(GameObject));
            if (!prop) {
                SetError("Failed to allocate prop");
                return 0;
            }
            memset(prop, 0, sizeof(GameObject));
            prop->id = object_id;
            prop->type = type;
            prop->position = *position;
            prop->is_active = 1;
            prop->is_visible = 1;
            new_object = prop;
            break;
        }
        
        default:
            SetError("Unknown object type");
            return 0;
    }
    
    if (!new_object) {
        return 0;
    }
    
    g_object_system.objects[obj_idx] = new_object;
    g_object_system.object_types[obj_idx] = type;
    g_object_system.object_count++;
    
    return object_id;
}

/**
 * Destroy game object
 */
int GameObject_Destroy(uint32_t object_id) {
    if (!g_object_system.is_initialized) {
        SetError("Game object system not initialized");
        return GAMEOBJECT_ERROR_INVALID_PARAM;
    }
    
    uint32_t obj_idx = object_id - 27000;
    if (obj_idx >= g_object_system.object_count) {
        SetError("Invalid object ID");
        return GAMEOBJECT_ERROR_INVALID_ID;
    }
    
    if (g_object_system.objects[obj_idx]) {
        free(g_object_system.objects[obj_idx]);
        g_object_system.objects[obj_idx] = NULL;
    }
    
    // Swap with last object
    if (obj_idx < g_object_system.object_count - 1) {
        g_object_system.objects[obj_idx] = g_object_system.objects[g_object_system.object_count - 1];
        g_object_system.object_types[obj_idx] = g_object_system.object_types[g_object_system.object_count - 1];
    }
    
    g_object_system.object_count--;
    
    return GAMEOBJECT_OK;
}

/**
 * Get game object
 */
GameObject* GameObject_Get(uint32_t object_id) {
    if (!g_object_system.is_initialized) {
        SetError("Game object system not initialized");
        return NULL;
    }
    
    uint32_t obj_idx = object_id - 27000;
    if (obj_idx >= g_object_system.object_count) {
        SetError("Invalid object ID");
        return NULL;
    }
    
    // All types start with GameObject base struct
    return (GameObject*)g_object_system.objects[obj_idx];
}

/**
 * Get unit object
 */
Unit* GameObject_GetUnit(uint32_t object_id) {
    if (!g_object_system.is_initialized) {
        SetError("Game object system not initialized");
        return NULL;
    }
    
    uint32_t obj_idx = object_id - 27000;
    if (obj_idx >= g_object_system.object_count || g_object_system.object_types[obj_idx] != GAMEOBJECT_TYPE_UNIT) {
        SetError("Not a unit object");
        return NULL;
    }
    
    return (Unit*)g_object_system.objects[obj_idx];
}

/**
 * Get building object
 */
Building* GameObject_GetBuilding(uint32_t object_id) {
    if (!g_object_system.is_initialized) {
        SetError("Game object system not initialized");
        return NULL;
    }
    
    uint32_t obj_idx = object_id - 27000;
    if (obj_idx >= g_object_system.object_count || g_object_system.object_types[obj_idx] != GAMEOBJECT_TYPE_BUILDING) {
        SetError("Not a building object");
        return NULL;
    }
    
    return (Building*)g_object_system.objects[obj_idx];
}

/**
 * Get effect object
 */
Effect* GameObject_GetEffect(uint32_t object_id) {
    if (!g_object_system.is_initialized) {
        SetError("Game object system not initialized");
        return NULL;
    }
    
    uint32_t obj_idx = object_id - 27000;
    if (obj_idx >= g_object_system.object_count || g_object_system.object_types[obj_idx] != GAMEOBJECT_TYPE_EFFECT) {
        SetError("Not an effect object");
        return NULL;
    }
    
    return (Effect*)g_object_system.objects[obj_idx];
}

/**
 * Update all game objects
 */
int GameObject_UpdateAll(float delta_time_ms) {
    if (!g_object_system.is_initialized) {
        SetError("Game object system not initialized");
        return GAMEOBJECT_ERROR_INVALID_PARAM;
    }
    
    for (uint32_t i = 0; i < g_object_system.object_count; i++) {
        GameObject* obj = (GameObject*)g_object_system.objects[i];
        
        if (!obj->is_active) continue;
        
        // Update position based on velocity
        obj->position.x += obj->velocity.x * (delta_time_ms / 1000.0f);
        obj->position.y += obj->velocity.y * (delta_time_ms / 1000.0f);
        obj->position.z += obj->velocity.z * (delta_time_ms / 1000.0f);
        
        // Check lifetime
        if (obj->lifetime_ms > 0.0f) {
            obj->creation_time_ms += delta_time_ms;
            if (obj->creation_time_ms >= obj->lifetime_ms) {
                obj->is_active = 0;
            }
        }
        
        // Handle building construction
        if (obj->type == GAMEOBJECT_TYPE_BUILDING) {
            Building* building = (Building*)obj;
            if (building->building_state == BUILDING_STATE_BUILDING && building->remaining_construction_ms > 0.0f) {
                building->remaining_construction_ms -= delta_time_ms;
                if (building->remaining_construction_ms <= 0.0f) {
                    building->building_state = BUILDING_STATE_COMPLETE;
                    building->construction_progress = 1.0f;
                } else {
                    building->construction_progress = 1.0f - (building->remaining_construction_ms / building->construction_time_ms);
                }
            }
        }
        
        // Trigger update callbacks
        for (uint32_t c = 0; c < g_object_system.callback_count; c++) {
            if (g_object_system.callbacks[c].update_callback) {
                g_object_system.callbacks[c].update_callback(obj, delta_time_ms, g_object_system.callbacks[c].user_data);
            }
        }
    }
    
    return GAMEOBJECT_OK;
}

/**
 * Set object position
 */
int GameObject_SetPosition(uint32_t object_id, const Vector3D* position) {
    GameObject* obj = GameObject_Get(object_id);
    if (!obj) return GAMEOBJECT_ERROR_INVALID_ID;
    obj->position = *position;
    return GAMEOBJECT_OK;
}

/**
 * Get object position
 */
Vector3D GameObject_GetPosition(uint32_t object_id) {
    GameObject* obj = GameObject_Get(object_id);
    if (!obj) {
        SetError("Object not found");
        return (Vector3D){0, 0, 0};
    }
    return obj->position;
}

/**
 * Set object rotation
 */
int GameObject_SetRotation(uint32_t object_id, float rotation) {
    GameObject* obj = GameObject_Get(object_id);
    if (!obj) return GAMEOBJECT_ERROR_INVALID_ID;
    obj->rotation = rotation;
    return GAMEOBJECT_OK;
}

/**
 * Apply damage
 */
int GameObject_ApplyDamage(uint32_t object_id, float damage_amount) {
    GameObject* obj = GameObject_Get(object_id);
    if (!obj) return GAMEOBJECT_ERROR_INVALID_ID;
    
    obj->health -= damage_amount;
    if (obj->health <= 0.0f) {
        obj->health = 0.0f;
        obj->is_active = 0;
    }
    
    return GAMEOBJECT_OK;
}

/**
 * Heal object
 */
int GameObject_Heal(uint32_t object_id, float heal_amount) {
    GameObject* obj = GameObject_Get(object_id);
    if (!obj) return GAMEOBJECT_ERROR_INVALID_ID;
    
    obj->health += heal_amount;
    if (obj->health > obj->max_health) {
        obj->health = obj->max_health;
    }
    
    return GAMEOBJECT_OK;
}

/**
 * Set velocity
 */
int GameObject_SetVelocity(uint32_t object_id, const Vector3D* velocity) {
    GameObject* obj = GameObject_Get(object_id);
    if (!obj) return GAMEOBJECT_ERROR_INVALID_ID;
    obj->velocity = *velocity;
    return GAMEOBJECT_OK;
}

/**
 * Get velocity
 */
Vector3D GameObject_GetVelocity(uint32_t object_id) {
    GameObject* obj = GameObject_Get(object_id);
    if (!obj) {
        SetError("Object not found");
        return (Vector3D){0, 0, 0};
    }
    return obj->velocity;
}

/**
 * Activate unit
 */
int GameObject_ActivateUnit(uint32_t object_id) {
    Unit* unit = GameObject_GetUnit(object_id);
    if (!unit) return GAMEOBJECT_ERROR_INVALID_ID;
    unit->base.is_active = 1;
    unit->unit_state = UNIT_STATE_IDLE;
    return GAMEOBJECT_OK;
}

/**
 * Deactivate unit
 */
int GameObject_DeactivateUnit(uint32_t object_id) {
    Unit* unit = GameObject_GetUnit(object_id);
    if (!unit) return GAMEOBJECT_ERROR_INVALID_ID;
    unit->base.is_active = 0;
    unit->unit_state = UNIT_STATE_DISABLED;
    return GAMEOBJECT_OK;
}

/**
 * Move unit to position
 */
int GameObject_MoveUnit(uint32_t object_id, const Vector3D* target_position) {
    Unit* unit = GameObject_GetUnit(object_id);
    if (!unit) return GAMEOBJECT_ERROR_INVALID_ID;
    
    unit->target_position = *target_position;
    unit->unit_state = UNIT_STATE_MOVING;
    
    // Calculate direction
    Vector3D dir;
    dir.x = target_position->x - unit->base.position.x;
    dir.y = target_position->y - unit->base.position.y;
    dir.z = target_position->z - unit->base.position.z;
    
    float dist = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
    if (dist > 0.01f) {
        unit->base.velocity.x = (dir.x / dist) * unit->move_speed;
        unit->base.velocity.y = (dir.y / dist) * unit->move_speed;
        unit->base.velocity.z = (dir.z / dist) * unit->move_speed;
    }
    
    return GAMEOBJECT_OK;
}

/**
 * Attack with unit
 */
int GameObject_AttackWithUnit(uint32_t object_id, uint32_t target_id) {
    Unit* unit = GameObject_GetUnit(object_id);
    if (!unit) return GAMEOBJECT_ERROR_INVALID_ID;
    
    unit->target_id = target_id;
    unit->unit_state = UNIT_STATE_ATTACKING;
    unit->attack_remaining_ms = unit->attack_cooldown_ms;
    
    return GAMEOBJECT_OK;
}

/**
 * Cancel unit action
 */
int GameObject_CancelUnitAction(uint32_t object_id) {
    Unit* unit = GameObject_GetUnit(object_id);
    if (!unit) return GAMEOBJECT_ERROR_INVALID_ID;
    
    unit->unit_state = UNIT_STATE_IDLE;
    unit->target_id = 0;
    unit->base.velocity.x = 0.0f;
    unit->base.velocity.y = 0.0f;
    unit->base.velocity.z = 0.0f;
    
    return GAMEOBJECT_OK;
}

/**
 * Start building construction
 */
int GameObject_StartBuildingConstruction(uint32_t object_id, float construction_time_ms) {
    Building* building = GameObject_GetBuilding(object_id);
    if (!building) return GAMEOBJECT_ERROR_INVALID_ID;
    
    building->building_state = BUILDING_STATE_BUILDING;
    building->construction_time_ms = construction_time_ms;
    building->remaining_construction_ms = construction_time_ms;
    building->construction_progress = 0.0f;
    
    return GAMEOBJECT_OK;
}

/**
 * Get building progress
 */
float GameObject_GetBuildingProgress(uint32_t object_id) {
    Building* building = GameObject_GetBuilding(object_id);
    if (!building) return -1.0f;
    return building->construction_progress;
}

/**
 * Create effect
 */
uint32_t GameObject_CreateEffect(uint32_t effect_type, const Vector3D* position, float lifetime_ms) {
    if (!position) return 0;
    
    uint32_t effect_id = GameObject_Create(GAMEOBJECT_TYPE_EFFECT, 0, position);
    if (effect_id) {
        Effect* effect = GameObject_GetEffect(effect_id);
        if (effect) {
            effect->effect_type = effect_type;
            effect->base.lifetime_ms = lifetime_ms;
        }
    }
    
    return effect_id;
}

/**
 * Get count by type
 */
uint32_t GameObject_GetCountByType(GameObjectType type) {
    if (!g_object_system.is_initialized) return 0;
    
    uint32_t count = 0;
    for (uint32_t i = 0; i < g_object_system.object_count; i++) {
        if (g_object_system.object_types[i] == type) count++;
    }
    return count;
}

/**
 * Get count by team
 */
uint32_t GameObject_GetCountByTeam(uint32_t team_id) {
    if (!g_object_system.is_initialized) return 0;
    
    uint32_t count = 0;
    for (uint32_t i = 0; i < g_object_system.object_count; i++) {
        GameObject* obj = (GameObject*)g_object_system.objects[i];
        if (obj->owner_team == team_id) count++;
    }
    return count;
}

/**
 * Register update callback
 */
int GameObject_RegisterUpdateCallback(GameObjectUpdateCallback callback, void* user_data) {
    if (!g_object_system.is_initialized) return GAMEOBJECT_ERROR_INVALID_PARAM;
    
    if (g_object_system.callback_count >= MAX_CALLBACKS) {
        SetError("Callback limit exceeded");
        return GAMEOBJECT_ERROR_LIMIT_EXCEEDED;
    }
    
    g_object_system.callbacks[g_object_system.callback_count].update_callback = callback;
    g_object_system.callbacks[g_object_system.callback_count].user_data = user_data;
    g_object_system.callback_count++;
    
    return GAMEOBJECT_OK;
}

/**
 * Register death callback
 */
int GameObject_RegisterDeathCallback(GameObjectDeathCallback callback, void* user_data) {
    if (!g_object_system.is_initialized) return GAMEOBJECT_ERROR_INVALID_PARAM;
    
    if (g_object_system.callback_count >= MAX_CALLBACKS) {
        SetError("Callback limit exceeded");
        return GAMEOBJECT_ERROR_LIMIT_EXCEEDED;
    }
    
    g_object_system.callbacks[g_object_system.callback_count].death_callback = callback;
    g_object_system.callbacks[g_object_system.callback_count].user_data = user_data;
    g_object_system.callback_count++;
    
    return GAMEOBJECT_OK;
}

/**
 * Get objects in radius
 */
uint32_t GameObject_GetObjectsInRadius(const Vector3D* center, float radius,
                                       uint32_t* result_ids, uint32_t max_results) {
    if (!g_object_system.is_initialized || !center || !result_ids) return 0;
    
    uint32_t count = 0;
    for (uint32_t i = 0; i < g_object_system.object_count && count < max_results; i++) {
        GameObject* obj = (GameObject*)g_object_system.objects[i];
        float dist = Distance(center, &obj->position);
        if (dist <= radius) {
            result_ids[count++] = obj->id;
        }
    }
    
    return count;
}

/**
 * Get distance between objects
 */
float GameObject_GetDistance(uint32_t object_id1, uint32_t object_id2) {
    GameObject* obj1 = GameObject_Get(object_id1);
    GameObject* obj2 = GameObject_Get(object_id2);
    
    if (!obj1 || !obj2) return -1.0f;
    
    return Distance(&obj1->position, &obj2->position);
}

/**
 * Get error message
 */
const char* GameObject_GetError(void) {
    if (g_error_message[0] == '\0') {
        return "No error";
    }
    return g_error_message;
}
