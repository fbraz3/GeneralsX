#pragma once

#include <cstdint>
#include <cstddef>

/* Phase 35: Combat System
 * 
 * Provides weapon firing and damage calculation:
 * - Weapon selection and firing
 * - Damage calculation
 * - Health tracking
 * - Death handling
 * - Experience/veterancy
 */

typedef struct CombatSystem CombatSystem;
typedef uint32_t CombatHandle;

#define COMBAT_MAX_ACTIVE_UNITS 5000
#define COMBAT_MAX_WEAPONS 256
#define COMBAT_MAX_PROJECTILES 2000
#define COMBAT_MAX_DAMAGE_TYPES 16

/* Combat handle ranges (Phase 35: 35000-35999) */
#define COMBAT_HANDLE_MIN 35000
#define COMBAT_HANDLE_MAX 35999

typedef enum {
    DAMAGE_TYPE_KINETIC = 0,
    DAMAGE_TYPE_EXPLOSIVE = 1,
    DAMAGE_TYPE_FIRE = 2,
    DAMAGE_TYPE_ENERGY = 3,
    DAMAGE_TYPE_ACID = 4,
    DAMAGE_TYPE_CRUSH = 5,
    DAMAGE_TYPE_PIERCE = 6,
    DAMAGE_TYPE_RADIATION = 8,
    DAMAGE_TYPE_ELECTRIC = 9,
    DAMAGE_TYPE_SPECIAL = 15,
} Damage_Type;

typedef enum {
    WEAPON_CLASS_INFANTRY = 0,
    WEAPON_CLASS_VEHICLE = 1,
    WEAPON_CLASS_TURRET = 2,
    WEAPON_CLASS_BUILDING = 3,
    WEAPON_CLASS_AIRCRAFT = 4,
    WEAPON_CLASS_SPECIAL = 15,
} Weapon_Class;

typedef enum {
    TARGET_MODE_GROUND = 0,
    TARGET_MODE_AIR = 1,
    TARGET_MODE_BOTH = 2,
    TARGET_MODE_BUILDING = 3,
    TARGET_MODE_UNIT = 4,
} Target_Mode;

typedef enum {
    UNIT_STATE_ALIVE = 0,
    UNIT_STATE_DAMAGED = 1,
    UNIT_STATE_CRITICAL = 2,
    UNIT_STATE_DESTROYED = 3,
} Unit_State;

typedef enum {
    VETERANCY_LEVEL_ROOKIE = 0,
    VETERANCY_LEVEL_REGULAR = 1,
    VETERANCY_LEVEL_VETERAN = 2,
    VETERANCY_LEVEL_ELITE = 3,
} Veterancy_Level;

typedef struct {
    float x, y;
    float vx, vy;
    float lifetime;
} Projectile;

typedef struct {
    uint32_t weapon_id;
    char name[64];
    float damage_min, damage_max;
    Damage_Type damage_type;
    Weapon_Class weapon_class;
    Target_Mode target_mode;
    float range;
    float fire_rate;
    float reload_time;
    float current_reload;
    int is_ready;
} Weapon;

typedef struct {
    uint32_t unit_id;
    uint32_t owner_id;
    float health;
    float max_health;
    Unit_State state;
    Veterancy_Level veterancy;
    uint32_t experience;
    uint32_t kills;
    uint32_t deaths;
    
    /* Weapons */
    Weapon* weapons;
    uint32_t weapon_count;
    uint32_t active_weapon;
    
    /* Combat state */
    uint32_t target_unit;
    float target_x, target_y;
    int attacking;
    float attack_cooldown;
} Combat_Unit;

typedef struct {
    uint32_t attacker_id;
    uint32_t defender_id;
    float damage_amount;
    Damage_Type damage_type;
    uint32_t source_weapon;
} Damage_Event;

/* System Management */
CombatSystem* CombatSystem_Create(void);
void CombatSystem_Destroy(CombatSystem* system);
void CombatSystem_Initialize(CombatSystem* system);
void CombatSystem_Shutdown(CombatSystem* system);

/* Unit Registration */
CombatHandle CombatSystem_RegisterUnit(CombatSystem* system,
                                       uint32_t unit_id,
                                       uint32_t owner_id,
                                       float initial_health);
void CombatSystem_UnregisterUnit(CombatSystem* system, uint32_t unit_id);
int CombatSystem_IsUnitRegistered(CombatSystem* system, uint32_t unit_id);

/* Unit Health Management */
void CombatSystem_SetUnitHealth(CombatSystem* system, uint32_t unit_id, float health);
float CombatSystem_GetUnitHealth(CombatSystem* system, uint32_t unit_id);
float CombatSystem_GetUnitMaxHealth(CombatSystem* system, uint32_t unit_id);
float CombatSystem_GetUnitHealthPercent(CombatSystem* system, uint32_t unit_id);
void CombatSystem_SetUnitMaxHealth(CombatSystem* system, uint32_t unit_id, float max_health);

/* Unit State */
Unit_State CombatSystem_GetUnitState(CombatSystem* system, uint32_t unit_id);
int CombatSystem_IsUnitAlive(CombatSystem* system, uint32_t unit_id);
int CombatSystem_IsUnitDestroyed(CombatSystem* system, uint32_t unit_id);

/* Weapon Management */
uint32_t CombatSystem_AddWeapon(CombatSystem* system,
                                uint32_t unit_id,
                                const char* weapon_name,
                                float damage_min, float damage_max,
                                Damage_Type damage_type,
                                Weapon_Class weapon_class,
                                Target_Mode target_mode,
                                float range, float fire_rate);
void CombatSystem_RemoveWeapon(CombatSystem* system, uint32_t unit_id, uint32_t weapon_id);
void CombatSystem_SelectWeapon(CombatSystem* system, uint32_t unit_id, uint32_t weapon_id);
uint32_t CombatSystem_GetActiveWeapon(CombatSystem* system, uint32_t unit_id);
uint32_t CombatSystem_GetWeaponCount(CombatSystem* system, uint32_t unit_id);
Weapon* CombatSystem_GetWeapon(CombatSystem* system, uint32_t unit_id, uint32_t weapon_id);

/* Targeting */
void CombatSystem_SetTarget(CombatSystem* system,
                            uint32_t attacker_id,
                            uint32_t target_id);
void CombatSystem_SetTargetPosition(CombatSystem* system,
                                    uint32_t attacker_id,
                                    float target_x, float target_y);
uint32_t CombatSystem_GetTarget(CombatSystem* system, uint32_t attacker_id);
void CombatSystem_ClearTarget(CombatSystem* system, uint32_t attacker_id);

/* Attack/Firing */
int CombatSystem_CanAttack(CombatSystem* system, uint32_t attacker_id);
int CombatSystem_IsWeaponReady(CombatSystem* system, uint32_t unit_id, uint32_t weapon_id);
void CombatSystem_Attack(CombatSystem* system, uint32_t attacker_id);
void CombatSystem_FireWeapon(CombatSystem* system,
                             uint32_t attacker_id,
                             uint32_t weapon_id,
                             float target_x, float target_y);

/* Damage */
void CombatSystem_DealDamage(CombatSystem* system,
                             uint32_t attacker_id,
                             uint32_t defender_id,
                             float damage_amount,
                             Damage_Type damage_type,
                             uint32_t source_weapon);
void CombatSystem_HealUnit(CombatSystem* system, uint32_t unit_id, float heal_amount);
float CombatSystem_CalculateDamage(CombatSystem* system,
                                   uint32_t attacker_id,
                                   uint32_t defender_id,
                                   Damage_Type damage_type);

/* Death Handling */
int CombatSystem_KillUnit(CombatSystem* system, uint32_t unit_id, uint32_t killer_id);
void CombatSystem_RespawnUnit(CombatSystem* system, uint32_t unit_id, float health);

/* Veterancy System */
Veterancy_Level CombatSystem_GetVeterancyLevel(CombatSystem* system, uint32_t unit_id);
void CombatSystem_SetVeterancyLevel(CombatSystem* system, uint32_t unit_id, Veterancy_Level level);
void CombatSystem_AddExperience(CombatSystem* system, uint32_t unit_id, uint32_t experience);
uint32_t CombatSystem_GetExperience(CombatSystem* system, uint32_t unit_id);
void CombatSystem_IncreaseKillCount(CombatSystem* system, uint32_t unit_id);
uint32_t CombatSystem_GetKillCount(CombatSystem* system, uint32_t unit_id);

/* Damage Type Modifiers */
float CombatSystem_GetDamageModifier(CombatSystem* system,
                                     Damage_Type damage_type,
                                     uint32_t defender_unit_type);
void CombatSystem_SetDamageModifier(CombatSystem* system,
                                    Damage_Type damage_type,
                                    uint32_t unit_type,
                                    float modifier);

/* Range Checking */
int CombatSystem_IsInRange(CombatSystem* system,
                           uint32_t attacker_id,
                           float target_x, float target_y);
float CombatSystem_GetDistanceToTarget(CombatSystem* system,
                                       uint32_t attacker_id,
                                       uint32_t target_id);

/* Combat Statistics */
uint32_t CombatSystem_GetDeathCount(CombatSystem* system, uint32_t unit_id);
uint32_t CombatSystem_GetKillDeathRatio(CombatSystem* system, uint32_t unit_id);
uint32_t CombatSystem_GetTotalDamageDealt(CombatSystem* system, uint32_t unit_id);
uint32_t CombatSystem_GetTotalDamageTaken(CombatSystem* system, uint32_t unit_id);

/* Projectiles */
uint32_t CombatSystem_SpawnProjectile(CombatSystem* system,
                                      float start_x, float start_y,
                                      float velocity_x, float velocity_y,
                                      float lifetime,
                                      uint32_t source_unit,
                                      uint32_t target_unit);
void CombatSystem_UpdateProjectiles(CombatSystem* system, float delta_time);

/* Update */
void CombatSystem_Update(CombatSystem* system, float delta_time);
void CombatSystem_UpdateWeaponCooldowns(CombatSystem* system, float delta_time);

/* Area of Effect */
void CombatSystem_DealAreaDamage(CombatSystem* system,
                                 float center_x, float center_y,
                                 float radius,
                                 float damage,
                                 Damage_Type damage_type,
                                 uint32_t source_unit);

/* Debugging */
const char* CombatSystem_GetLastError(CombatSystem* system);
int CombatSystem_ClearErrors(CombatSystem* system);
void CombatSystem_PrintUnitStats(CombatSystem* system, uint32_t unit_id);
void CombatSystem_PrintCombatStats(CombatSystem* system);
uint32_t CombatSystem_GetActiveUnitCount(CombatSystem* system);

/* Damage Events Callback */
typedef void (*DamageEventCallback)(Damage_Event* event, void* user_data);
void CombatSystem_RegisterDamageCallback(CombatSystem* system, DamageEventCallback callback, void* user_data);
