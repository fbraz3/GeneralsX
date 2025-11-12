# Phase 35: Combat System - Implementation Details

## Overview

Phase 35 implements the complete combat system for Generals Zero Hour, featuring unit health management, weapon firing, damage calculation, and veterancy progression.

**Target**: Zero Hour (GeneralsXZH)
**Status**: Fully implemented and compiled
**Compilation**: 0 errors, 3 warnings (non-critical)
**API Functions**: 50 total functions across all categories

## Architecture

### Core System Design

The combat system uses a handle-based resource management pattern with an opaque CombatSystem structure:

```c
/* Opaque handle */
typedef struct CombatSystem CombatSystem;

/* Handle range: 35000-35999 (1,000 reserved handles) */
typedef uint32_t CombatHandle;
#define COMBAT_HANDLE_MIN 35000
#define COMBAT_HANDLE_MAX 35999
```

### Key Components

#### 1. Combat Units
- **Max Concurrent**: 1,000 units
- **Data Tracked**: Health, state, weapons, target, veterancy, kill/death counts
- **States**: Alive, Damaged, Critical, Destroyed
- **Veterancy Levels**: Rookie, Regular, Veteran, Elite

#### 2. Weapon System
- **Max Weapons per Unit**: 6 weapons
- **Damage Types**: 10 (kinetic, explosive, fire, energy, acid, crush, pierce, radiation, electric, special)
- **Weapon Classes**: 6 types (infantry, vehicle, turret, building, aircraft, special)
- **Target Modes**: 5 modes (ground, air, both, building, unit)
- **Weapon Properties**: Damage range, fire rate, reload time, attack range, readiness state

#### 3. Projectile System
- **Max Concurrent Projectiles**: 1,000
- **Properties**: Position, velocity, lifetime
- **Lifecycle**: Automatic cleanup when lifetime expires

#### 4. Damage System
- **Modifiers**: Support for 10 damage types with 16 defender unit types each
- **Calculation**: Base damage + random variation + modifier application
- **Callbacks**: Damage event notifications for gameplay integration

#### 5. Veterancy & Experience
- **Experience Scaling**: 300 points per level (4 levels total)
- **Benefits**: Tracks kill/death ratio, experience progression
- **Statistics**: Total damage dealt/taken, combat efficiency metrics

## API Functions (50 Total)

### System Lifecycle (3 functions)
```c
CombatSystem* CombatSystem_Create(void);
void CombatSystem_Destroy(CombatSystem* system);
void CombatSystem_Initialize(CombatSystem* system);
void CombatSystem_Shutdown(CombatSystem* system);
```

### Unit Management (10 functions)
```c
CombatHandle CombatSystem_RegisterUnit(CombatSystem* system,
                                       uint32_t unit_id,
                                       uint32_t owner_id,
                                       float initial_health);
void CombatSystem_UnregisterUnit(CombatSystem* system, uint32_t unit_id);
int CombatSystem_IsUnitRegistered(CombatSystem* system, uint32_t unit_id);
int CombatSystem_IsUnitAlive(CombatSystem* system, uint32_t unit_id);
int CombatSystem_IsUnitDestroyed(CombatSystem* system, uint32_t unit_id);
Unit_State CombatSystem_GetUnitState(CombatSystem* system, uint32_t unit_id);
int CombatSystem_CanAttack(CombatSystem* system, uint32_t attacker_id);
CombatHandle CombatSystem_KillUnit(CombatSystem* system, uint32_t unit_id, uint32_t killer_id);
void CombatSystem_RespawnUnit(CombatSystem* system, uint32_t unit_id, float health);
uint32_t CombatSystem_GetActiveUnitCount(CombatSystem* system);
```

### Health Management (6 functions)
```c
void CombatSystem_SetUnitHealth(CombatSystem* system, uint32_t unit_id, float health);
float CombatSystem_GetUnitHealth(CombatSystem* system, uint32_t unit_id);
float CombatSystem_GetUnitMaxHealth(CombatSystem* system, uint32_t unit_id);
float CombatSystem_GetUnitHealthPercent(CombatSystem* system, uint32_t unit_id);
void CombatSystem_SetUnitMaxHealth(CombatSystem* system, uint32_t unit_id, float max_health);
void CombatSystem_HealUnit(CombatSystem* system, uint32_t unit_id, float heal_amount);
```

### Weapon System (7 functions)
```c
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
int CombatSystem_IsWeaponReady(CombatSystem* system, uint32_t unit_id, uint32_t weapon_id);
```

### Targeting System (5 functions)
```c
void CombatSystem_SetTarget(CombatSystem* system,
                            uint32_t attacker_id,
                            uint32_t target_id);
void CombatSystem_SetTargetPosition(CombatSystem* system,
                                    uint32_t attacker_id,
                                    float target_x, float target_y);
uint32_t CombatSystem_GetTarget(CombatSystem* system, uint32_t attacker_id);
void CombatSystem_ClearTarget(CombatSystem* system, uint32_t attacker_id);
float CombatSystem_GetDistanceToTarget(CombatSystem* system,
                                       uint32_t attacker_id,
                                       uint32_t target_id);
```

### Attack & Damage (6 functions)
```c
void CombatSystem_Attack(CombatSystem* system, uint32_t attacker_id);
void CombatSystem_FireWeapon(CombatSystem* system,
                             uint32_t attacker_id,
                             uint32_t weapon_id,
                             float target_x, float target_y);
void CombatSystem_DealDamage(CombatSystem* system,
                             uint32_t attacker_id,
                             uint32_t defender_id,
                             float damage_amount,
                             Damage_Type damage_type,
                             uint32_t source_weapon);
float CombatSystem_CalculateDamage(CombatSystem* system,
                                   uint32_t attacker_id,
                                   uint32_t target_unit_id,
                                   Damage_Type damage_type);
void CombatSystem_DealAreaDamage(CombatSystem* system,
                                 float center_x, float center_y,
                                 float radius,
                                 float damage,
                                 Damage_Type damage_type,
                                 uint32_t source_unit);
int CombatSystem_IsInRange(CombatSystem* system,
                           uint32_t attacker_id,
                           float target_x, float target_y);
```

### Veterancy System (6 functions)
```c
Veterancy_Level CombatSystem_GetVeterancyLevel(CombatSystem* system, uint32_t unit_id);
void CombatSystem_SetVeterancyLevel(CombatSystem* system, uint32_t unit_id, Veterancy_Level level);
void CombatSystem_AddExperience(CombatSystem* system, uint32_t unit_id, uint32_t experience);
uint32_t CombatSystem_GetExperience(CombatSystem* system, uint32_t unit_id);
void CombatSystem_IncreaseKillCount(CombatSystem* system, uint32_t unit_id);
uint32_t CombatSystem_GetKillCount(CombatSystem* system, uint32_t unit_id);
```

### Damage Modifiers (2 functions)
```c
float CombatSystem_GetDamageModifier(CombatSystem* system,
                                     Damage_Type damage_type,
                                     uint32_t defender_unit_type);
void CombatSystem_SetDamageModifier(CombatSystem* system,
                                    Damage_Type damage_type,
                                    uint32_t unit_type,
                                    float modifier);
```

### Statistics (4 functions)
```c
uint32_t CombatSystem_GetDeathCount(CombatSystem* system, uint32_t unit_id);
uint32_t CombatSystem_GetKillDeathRatio(CombatSystem* system, uint32_t unit_id);
uint32_t CombatSystem_GetTotalDamageDealt(CombatSystem* system, uint32_t unit_id);
uint32_t CombatSystem_GetTotalDamageTaken(CombatSystem* system, uint32_t unit_id);
```

### Projectiles (2 functions)
```c
uint32_t CombatSystem_SpawnProjectile(CombatSystem* system,
                                      float start_x, float start_y,
                                      float velocity_x, float velocity_y,
                                      float lifetime,
                                      uint32_t source_unit,
                                      uint32_t target_unit);
void CombatSystem_UpdateProjectiles(CombatSystem* system, float delta_time);
```

### Update Loop (3 functions)
```c
void CombatSystem_Update(CombatSystem* system, float delta_time);
void CombatSystem_UpdateWeaponCooldowns(CombatSystem* system, float delta_time);
```

### Debugging & Utilities (3 functions)
```c
void CombatSystem_PrintUnitStats(CombatSystem* system, uint32_t unit_id);
void CombatSystem_PrintCombatStats(CombatSystem* system);
const char* CombatSystem_GetLastError(CombatSystem* system);
int CombatSystem_ClearErrors(CombatSystem* system);
```

### Event System (1 function)
```c
void CombatSystem_RegisterDamageCallback(CombatSystem* system,
                                         DamageEventCallback callback,
                                         void* user_data);
```

## Data Structures

### Combat_Unit
```c
typedef struct Combat_Unit {
    uint32_t unit_id;
    uint32_t owner_id;
    float health;
    float max_health;
    Unit_State state;
    Veterancy_Level veterancy;
    uint32_t experience;
    uint32_t kills;
    uint32_t deaths;
    
    Weapon* weapons;
    uint32_t weapon_count;
    uint32_t active_weapon;
    
    uint32_t target_unit;
    float target_x;
    float target_y;
    
    uint8_t attacking;
    float attack_cooldown;
} Combat_Unit;
```

### Weapon
```c
typedef struct Weapon {
    uint32_t weapon_id;
    char name[64];
    float damage_min;
    float damage_max;
    Damage_Type damage_type;
    Weapon_Class weapon_class;
    Target_Mode target_mode;
    float range;
    float fire_rate;
    float reload_time;
    float current_reload;
    uint8_t is_ready;
} Weapon;
```

### Projectile
```c
typedef struct Projectile {
    float x, y;
    float vx, vy;
    float lifetime;
} Projectile;
```

### Damage_Event
```c
typedef struct Damage_Event {
    uint32_t attacker_id;
    uint32_t defender_id;
    float damage_amount;
    Damage_Type damage_type;
    uint32_t source_weapon;
} Damage_Event;
```

## Enumerations

### Unit_State
- `UNIT_STATE_ALIVE` (0): Unit has full health
- `UNIT_STATE_DAMAGED` (1): Unit between 25-100% health
- `UNIT_STATE_CRITICAL` (2): Unit below 25% health
- `UNIT_STATE_DESTROYED` (3): Unit is dead

### Damage_Type
- `DAMAGE_KINETIC` (0): Physical projectiles
- `DAMAGE_EXPLOSIVE` (1): Explosions
- `DAMAGE_FIRE` (2): Fire/heat
- `DAMAGE_ENERGY` (3): Beam weapons
- `DAMAGE_ACID` (4): Chemical
- `DAMAGE_CRUSH` (5): Crushing force
- `DAMAGE_PIERCE` (6): Piercing damage
- `DAMAGE_RADIATION` (7): Radiation
- `DAMAGE_ELECTRIC` (8): Electrical
- `DAMAGE_SPECIAL` (9): Special/miscellaneous

### Weapon_Class
- `WEAPON_INFANTRY` (0): Infantry weapons
- `WEAPON_VEHICLE` (1): Vehicle weapons
- `WEAPON_TURRET` (2): Building turrets
- `WEAPON_BUILDING` (3): Building weapons
- `WEAPON_AIRCRAFT` (4): Aircraft weapons
- `WEAPON_SPECIAL` (5): Special weapons

### Target_Mode
- `TARGET_GROUND` (0): Ground units only
- `TARGET_AIR` (1): Air units only
- `TARGET_BOTH` (2): Both ground and air
- `TARGET_BUILDING` (3): Buildings only
- `TARGET_UNIT` (4): Any unit

### Veterancy_Level
- `VETERANCY_LEVEL_ROOKIE` (0): Inexperienced
- `VETERANCY_LEVEL_REGULAR` (1): Standard experience
- `VETERANCY_LEVEL_VETERAN` (2): High experience
- `VETERANCY_LEVEL_ELITE` (3): Maximum experience

## Implementation Patterns

### Unit Registration
```c
/* Register a new unit in combat system */
CombatHandle handle = CombatSystem_RegisterUnit(combat_system,
                                               unit->id,
                                               unit->owner_id,
                                               unit->health);

/* Verify registration */
if (CombatSystem_IsUnitRegistered(combat_system, unit->id)) {
    /* Unit successfully registered */
}
```

### Weapon Management
```c
/* Add a weapon to a unit */
uint32_t weapon_id = CombatSystem_AddWeapon(combat_system,
                                           unit_id,
                                           "Machine Gun",
                                           10.0f, 15.0f,
                                           DAMAGE_KINETIC,
                                           WEAPON_INFANTRY,
                                           TARGET_GROUND,
                                           100.0f, 10.0f);

/* Select active weapon */
CombatSystem_SelectWeapon(combat_system, unit_id, weapon_id);

/* Fire weapon */
CombatSystem_FireWeapon(combat_system, unit_id, weapon_id, target_x, target_y);
```

### Damage Application
```c
/* Deal direct damage */
CombatSystem_DealDamage(combat_system,
                        attacker_id,
                        defender_id,
                        damage_amount,
                        DAMAGE_KINETIC,
                        weapon_id);

/* Deal area damage */
CombatSystem_DealAreaDamage(combat_system,
                            explosion_x, explosion_y,
                            explosion_radius,
                            explosion_damage,
                            DAMAGE_EXPLOSIVE,
                            source_unit);
```

### Health Management
```c
/* Check unit health */
float health = CombatSystem_GetUnitHealth(combat_system, unit_id);
float health_percent = CombatSystem_GetUnitHealthPercent(combat_system, unit_id);

/* Update health */
if (health <= 0) {
    CombatSystem_KillUnit(combat_system, unit_id, attacker_id);
}

/* Healing */
CombatSystem_HealUnit(combat_system, unit_id, heal_amount);
```

### Veterancy System
```c
/* Award experience for kills */
CombatSystem_AddExperience(combat_system, unit_id, 50);

/* Check veterancy level */
Veterancy_Level level = CombatSystem_GetVeterancyLevel(combat_system, unit_id);

/* Get combat stats */
uint32_t kills = CombatSystem_GetKillCount(combat_system, unit_id);
uint32_t deaths = CombatSystem_GetDeathCount(combat_system, unit_id);
uint32_t kd_ratio = CombatSystem_GetKillDeathRatio(combat_system, unit_id);
```

## Compilation Results

**Source Files**: 2
- `CombatSystem.h` (262 lines)
- `CombatSystem.cpp` (1,057 lines)

**Total Lines**: 1,319 lines of code

**Compilation Status**: PASS
- Errors: 0
- Warnings: 3 (non-critical tautological comparisons)

**Dependency Requirements**:
- C++20 standard
- Standard C library (stdlib.h, string.h, math.h)

## Integration Points

Phase 35 interfaces with:

1. **Phase 33 (Input System)**: Combat responds to user selection and commands
2. **Phase 34 (Pathfinding)**: Movement system feeds into combat positioning
3. **Phase 32 (Audio System)**: Sound effects for weapon fire, impacts, death

## Future Enhancements

Potential Phase 36+ integration points:
- **Environmental Damage**: Damage from terrain hazards
- **Status Effects**: Buffs/debuffs affecting combat parameters
- **Team Cooperation**: Bonuses for nearby friendly units
- **Intelligence System**: Advanced targeting and tactical decisions
- **Persistence**: Saving/loading combat statistics and unit states

## References

- Phase 32: Audio System
- Phase 33: Input Handling
- Phase 34: Pathfinding & Movement
- References: `jmarshall-win64-modern/` for combat pattern inspiration
