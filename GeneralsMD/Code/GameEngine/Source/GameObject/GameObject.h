/**
 * @file GameObject.h
 * @brief Phase 27: Game Object System - Base Entity System
 *
 * Base classes for game objects (units, buildings, effects) with lifecycle management.
 */

#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
#define GAMEOBJECT_OK                  0
#define GAMEOBJECT_ERROR_INVALID_PARAM -1
#define GAMEOBJECT_ERROR_INVALID_ID    -2
#define GAMEOBJECT_ERROR_LIMIT_EXCEEDED -3

// Game object types
typedef enum {
    GAMEOBJECT_TYPE_UNIT,
    GAMEOBJECT_TYPE_BUILDING,
    GAMEOBJECT_TYPE_EFFECT,
    GAMEOBJECT_TYPE_PROJECTILE,
    GAMEOBJECT_TYPE_PROP
} GameObjectType;

// Unit movement states
typedef enum {
    UNIT_STATE_IDLE = 0,
    UNIT_STATE_MOVING,
    UNIT_STATE_ATTACKING,
    UNIT_STATE_DEAD,
    UNIT_STATE_DISABLED
} UnitState;

// Building construction states
typedef enum {
    BUILDING_STATE_BUILDING = 0,
    BUILDING_STATE_COMPLETE,
    BUILDING_STATE_DAMAGED,
    BUILDING_STATE_DESTROYED,
    BUILDING_STATE_SELLING
} BuildingState;

// Vector2D for position/direction
typedef struct {
    float x;
    float y;
} Vector2D;

// Vector3D for 3D position
typedef struct {
    float x;
    float y;
    float z;
} Vector3D;

// Base game object structure
typedef struct {
    uint32_t id;
    GameObjectType type;
    uint32_t owner_team;
    Vector3D position;
    Vector3D velocity;
    float rotation;
    float health;
    float max_health;
    int is_active;
    int is_visible;
    float creation_time_ms;
    float lifetime_ms;  // 0 = infinite
} GameObject;

// Unit structure
typedef struct {
    GameObject base;
    UnitState unit_state;
    uint32_t target_id;
    Vector3D target_position;
    float move_speed;
    float attack_range;
    float attack_damage;
    float attack_cooldown_ms;
    float attack_remaining_ms;
} Unit;

// Building structure
typedef struct {
    GameObject base;
    BuildingState building_state;
    float construction_progress;  // 0.0 - 1.0
    float construction_time_ms;
    float remaining_construction_ms;
    uint32_t armor_type;
    float armor_value;
} Building;

// Effect structure
typedef struct {
    GameObject base;
    uint32_t effect_type;
    float particle_count;
    Vector3D emitter_position;
    float emission_rate;
    float particle_lifetime_ms;
} Effect;

// Projectile structure
typedef struct {
    GameObject base;
    uint32_t source_unit_id;
    uint32_t target_unit_id;
    float projectile_speed;
    Vector3D launch_position;
    Vector3D target_position;
} Projectile;

// Game object callbacks
typedef void (*GameObjectUpdateCallback)(GameObject* obj, float delta_time_ms, void* user_data);
typedef void (*GameObjectDeathCallback)(GameObject* obj, void* user_data);
typedef void (*GameObjectCollisionCallback)(GameObject* obj1, GameObject* obj2, void* user_data);

/**
 * Initialize game object system
 * @return GAMEOBJECT_OK on success
 */
int GameObject_Initialize(uint32_t max_objects);

/**
 * Shutdown game object system
 * @return GAMEOBJECT_OK on success
 */
int GameObject_Shutdown(void);

/**
 * Create game object
 * @param type Object type
 * @param owner_team Team owner
 * @param position Initial position
 * @return Object ID, or 0 on failure
 */
uint32_t GameObject_Create(GameObjectType type, uint32_t owner_team, const Vector3D* position);

/**
 * Destroy game object
 * @param object_id Object ID
 * @return GAMEOBJECT_OK on success
 */
int GameObject_Destroy(uint32_t object_id);

/**
 * Get game object
 * @param object_id Object ID
 * @return Pointer to game object, or NULL if not found
 */
GameObject* GameObject_Get(uint32_t object_id);

/**
 * Get unit object
 * @param object_id Object ID
 * @return Pointer to unit object, or NULL if not a unit
 */
Unit* GameObject_GetUnit(uint32_t object_id);

/**
 * Get building object
 * @param object_id Object ID
 * @return Pointer to building object, or NULL if not a building
 */
Building* GameObject_GetBuilding(uint32_t object_id);

/**
 * Get effect object
 * @param object_id Object ID
 * @return Pointer to effect object, or NULL if not an effect
 */
Effect* GameObject_GetEffect(uint32_t object_id);

/**
 * Update all game objects
 * @param delta_time_ms Time delta in milliseconds
 * @return GAMEOBJECT_OK on success
 */
int GameObject_UpdateAll(float delta_time_ms);

/**
 * Set object position
 * @param object_id Object ID
 * @param position New position
 * @return GAMEOBJECT_OK on success
 */
int GameObject_SetPosition(uint32_t object_id, const Vector3D* position);

/**
 * Get object position
 * @param object_id Object ID
 * @return Object position
 */
Vector3D GameObject_GetPosition(uint32_t object_id);

/**
 * Set object rotation
 * @param object_id Object ID
 * @param rotation Rotation angle (degrees)
 * @return GAMEOBJECT_OK on success
 */
int GameObject_SetRotation(uint32_t object_id, float rotation);

/**
 * Apply damage to object
 * @param object_id Object ID
 * @param damage_amount Damage amount
 * @return GAMEOBJECT_OK on success
 */
int GameObject_ApplyDamage(uint32_t object_id, float damage_amount);

/**
 * Heal object
 * @param object_id Object ID
 * @param heal_amount Heal amount
 * @return GAMEOBJECT_OK on success
 */
int GameObject_Heal(uint32_t object_id, float heal_amount);

/**
 * Set object velocity
 * @param object_id Object ID
 * @param velocity New velocity vector
 * @return GAMEOBJECT_OK on success
 */
int GameObject_SetVelocity(uint32_t object_id, const Vector3D* velocity);

/**
 * Get object velocity
 * @param object_id Object ID
 * @return Object velocity vector
 */
Vector3D GameObject_GetVelocity(uint32_t object_id);

/**
 * Activate unit
 * @param object_id Unit object ID
 * @return GAMEOBJECT_OK on success
 */
int GameObject_ActivateUnit(uint32_t object_id);

/**
 * Deactivate unit
 * @param object_id Unit object ID
 * @return GAMEOBJECT_OK on success
 */
int GameObject_DeactivateUnit(uint32_t object_id);

/**
 * Move unit to position
 * @param object_id Unit object ID
 * @param target_position Target position
 * @return GAMEOBJECT_OK on success
 */
int GameObject_MoveUnit(uint32_t object_id, const Vector3D* target_position);

/**
 * Attack with unit
 * @param object_id Unit object ID
 * @param target_id Target unit object ID
 * @return GAMEOBJECT_OK on success
 */
int GameObject_AttackWithUnit(uint32_t object_id, uint32_t target_id);

/**
 * Cancel unit action
 * @param object_id Unit object ID
 * @return GAMEOBJECT_OK on success
 */
int GameObject_CancelUnitAction(uint32_t object_id);

/**
 * Start building construction
 * @param object_id Building object ID
 * @param construction_time_ms Total construction time in milliseconds
 * @return GAMEOBJECT_OK on success
 */
int GameObject_StartBuildingConstruction(uint32_t object_id, float construction_time_ms);

/**
 * Get building construction progress
 * @param object_id Building object ID
 * @return Construction progress (0.0 - 1.0), or -1 on error
 */
float GameObject_GetBuildingProgress(uint32_t object_id);

/**
 * Create effect at position
 * @param effect_type Effect type ID
 * @param position Effect position
 * @param lifetime_ms Effect lifetime in milliseconds
 * @return Effect object ID, or 0 on failure
 */
uint32_t GameObject_CreateEffect(uint32_t effect_type, const Vector3D* position, float lifetime_ms);

/**
 * Get object count by type
 * @param type Object type to count
 * @return Number of objects of given type
 */
uint32_t GameObject_GetCountByType(GameObjectType type);

/**
 * Get object count by team
 * @param team_id Team ID
 * @return Number of objects owned by team
 */
uint32_t GameObject_GetCountByTeam(uint32_t team_id);

/**
 * Register object update callback
 * @param callback Update callback function
 * @param user_data User data to pass to callback
 * @return GAMEOBJECT_OK on success
 */
int GameObject_RegisterUpdateCallback(GameObjectUpdateCallback callback, void* user_data);

/**
 * Register object death callback
 * @param callback Death callback function
 * @param user_data User data to pass to callback
 * @return GAMEOBJECT_OK on success
 */
int GameObject_RegisterDeathCallback(GameObjectDeathCallback callback, void* user_data);

/**
 * Get all objects in radius
 * @param center Center position
 * @param radius Search radius
 * @param result_ids Array to receive object IDs
 * @param max_results Maximum number of results
 * @return Number of objects found
 */
uint32_t GameObject_GetObjectsInRadius(const Vector3D* center, float radius,
                                       uint32_t* result_ids, uint32_t max_results);

/**
 * Get distance between objects
 * @param object_id1 First object ID
 * @param object_id2 Second object ID
 * @return Distance between objects, or -1 on error
 */
float GameObject_GetDistance(uint32_t object_id1, uint32_t object_id2);

/**
 * Get error message from last operation
 * @return Error message string
 */
const char* GameObject_GetError(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_OBJECT_H
