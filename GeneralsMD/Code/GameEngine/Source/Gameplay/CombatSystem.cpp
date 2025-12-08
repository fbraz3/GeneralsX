#include "CombatSystem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

struct CombatSystem {
    /* Combat units */
    Combat_Unit* units;
    uint32_t max_units;
    uint32_t num_units;
    
    /* Projectiles */
    Projectile* projectiles;
    uint32_t max_projectiles;
    uint32_t num_projectiles;
    
    /* Damage modifiers */
    float damage_modifiers[COMBAT_MAX_DAMAGE_TYPES][16];
    
    /* Events */
    DamageEventCallback damage_callback;
    void* damage_callback_user_data;
    
    /* Statistics */
    uint32_t total_damage_dealt;
    uint32_t total_damage_taken;
    
    /* Error tracking */
    char last_error[256];
    int error_count;
};

static CombatHandle combat_allocate_handle(void) {
    return COMBAT_HANDLE_MIN + (rand() % (COMBAT_HANDLE_MAX - COMBAT_HANDLE_MIN));
}

static int combat_find_unit(CombatSystem* system, uint32_t unit_id) {
    for (uint32_t i = 0; i < system->num_units; i++) {
        if (system->units[i].unit_id == unit_id) {
            return i;
        }
    }
    return -1;
}

static void combat_update_unit_state(Combat_Unit* unit) {
    if (unit->health <= 0) {
        unit->state = UNIT_STATE_DESTROYED;
    } else if (unit->health < unit->max_health * 0.25f) {
        unit->state = UNIT_STATE_CRITICAL;
    } else if (unit->health < unit->max_health) {
        unit->state = UNIT_STATE_DAMAGED;
    } else {
        unit->state = UNIT_STATE_ALIVE;
    }
}

CombatSystem* CombatSystem_Create(void) {
    CombatSystem* system = (CombatSystem*)malloc(sizeof(CombatSystem));
    if (!system) return NULL;
    
    memset(system, 0, sizeof(CombatSystem));
    
    system->max_units = COMBAT_MAX_ACTIVE_UNITS;
    system->max_projectiles = COMBAT_MAX_PROJECTILES;
    
    system->units = (Combat_Unit*)calloc(system->max_units, sizeof(Combat_Unit));
    system->projectiles = (Projectile*)calloc(system->max_projectiles, sizeof(Projectile));
    
    if (!system->units || !system->projectiles) {
        free(system->units);
        free(system->projectiles);
        free(system);
        return NULL;
    }
    
    /* Initialize damage modifiers to 1.0 (neutral) */
    for (int i = 0; i < COMBAT_MAX_DAMAGE_TYPES; i++) {
        for (int j = 0; j < 16; j++) {
            system->damage_modifiers[i][j] = 1.0f;
        }
    }
    
    return system;
}

void CombatSystem_Destroy(CombatSystem* system) {
    if (!system) return;
    
    for (uint32_t i = 0; i < system->num_units; i++) {
        free(system->units[i].weapons);
    }
    
    free(system->units);
    free(system->projectiles);
    free(system);
}

void CombatSystem_Initialize(CombatSystem* system) {
    if (!system) return;
    
    system->num_units = 0;
    system->num_projectiles = 0;
    
    printf("Phase 35: Combat system initialized\n");
    printf("Max units: %u, Max projectiles: %u\n", system->max_units, system->max_projectiles);
}

void CombatSystem_Shutdown(CombatSystem* system) {
    if (!system) return;
    
    system->num_units = 0;
    system->num_projectiles = 0;
    
    printf("Phase 35: Combat system shutdown\n");
}

CombatHandle CombatSystem_RegisterUnit(CombatSystem* system,
                                       uint32_t unit_id,
                                       uint32_t owner_id,
                                       float initial_health) {
    if (!system || system->num_units >= system->max_units) return 0;
    
    Combat_Unit* unit = &system->units[system->num_units];
    unit->unit_id = unit_id;
    unit->owner_id = owner_id;
    unit->health = initial_health;
    unit->max_health = initial_health;
    unit->state = UNIT_STATE_ALIVE;
    unit->veterancy = VETERANCY_LEVEL_ROOKIE;
    unit->experience = 0;
    unit->kills = 0;
    unit->deaths = 0;
    unit->weapon_count = 0;
    unit->active_weapon = 0;
    unit->target_unit = 0;
    unit->target_x = 0.0f;
    unit->target_y = 0.0f;
    unit->attacking = 0;
    unit->attack_cooldown = 0.0f;
    
    unit->weapons = (Weapon*)calloc(COMBAT_MAX_WEAPONS, sizeof(Weapon));
    if (!unit->weapons) return 0;
    
    system->num_units++;
    
    return combat_allocate_handle();
}

void CombatSystem_UnregisterUnit(CombatSystem* system, uint32_t unit_id) {
    if (!system) return;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return;
    
    free(system->units[idx].weapons);
    
    if (idx < (int)system->num_units - 1) {
        memmove(&system->units[idx],
               &system->units[idx + 1],
               (system->num_units - idx - 1) * sizeof(Combat_Unit));
    }
    system->num_units--;
}

int CombatSystem_IsUnitRegistered(CombatSystem* system, uint32_t unit_id) {
    if (!system) return 0;
    return combat_find_unit(system, unit_id) >= 0;
}

void CombatSystem_SetUnitHealth(CombatSystem* system, uint32_t unit_id, float health) {
    if (!system) return;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return;
    
    system->units[idx].health = health;
    if (system->units[idx].health > system->units[idx].max_health) {
        system->units[idx].health = system->units[idx].max_health;
    }
    if (system->units[idx].health < 0) {
        system->units[idx].health = 0;
    }
    
    combat_update_unit_state(&system->units[idx]);
}

float CombatSystem_GetUnitHealth(CombatSystem* system, uint32_t unit_id) {
    if (!system) return 0.0f;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return 0.0f;
    
    return system->units[idx].health;
}

float CombatSystem_GetUnitMaxHealth(CombatSystem* system, uint32_t unit_id) {
    if (!system) return 0.0f;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return 0.0f;
    
    return system->units[idx].max_health;
}

float CombatSystem_GetUnitHealthPercent(CombatSystem* system, uint32_t unit_id) {
    if (!system) return 0.0f;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return 0.0f;
    
    if (system->units[idx].max_health <= 0) return 0.0f;
    
    return (system->units[idx].health / system->units[idx].max_health) * 100.0f;
}

void CombatSystem_SetUnitMaxHealth(CombatSystem* system, uint32_t unit_id, float max_health) {
    if (!system) return;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return;
    
    system->units[idx].max_health = max_health;
    if (system->units[idx].health > max_health) {
        system->units[idx].health = max_health;
    }
}

Unit_State CombatSystem_GetUnitState(CombatSystem* system, uint32_t unit_id) {
    if (!system) return UNIT_STATE_DESTROYED;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return UNIT_STATE_DESTROYED;
    
    return system->units[idx].state;
}

int CombatSystem_IsUnitAlive(CombatSystem* system, uint32_t unit_id) {
    if (!system) return 0;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return 0;
    
    return system->units[idx].health > 0;
}

int CombatSystem_IsUnitDestroyed(CombatSystem* system, uint32_t unit_id) {
    if (!system) return 1;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return 1;
    
    return system->units[idx].state == UNIT_STATE_DESTROYED;
}

uint32_t CombatSystem_AddWeapon(CombatSystem* system,
                                uint32_t unit_id,
                                const char* weapon_name,
                                float damage_min, float damage_max,
                                Damage_Type damage_type,
                                Weapon_Class weapon_class,
                                Target_Mode target_mode,
                                float range, float fire_rate) {
    if (!system || !weapon_name) return 0;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0 || system->units[idx].weapon_count >= COMBAT_MAX_WEAPONS) return 0;
    
    Combat_Unit* unit = &system->units[idx];
    Weapon* weapon = &unit->weapons[unit->weapon_count];
    
    weapon->weapon_id = unit->weapon_count;
    strncpy(weapon->name, weapon_name, sizeof(weapon->name) - 1);
    weapon->damage_min = damage_min;
    weapon->damage_max = damage_max;
    weapon->damage_type = damage_type;
    weapon->weapon_class = weapon_class;
    weapon->target_mode = target_mode;
    weapon->range = range;
    weapon->fire_rate = fire_rate;
    weapon->reload_time = 1.0f / fire_rate;
    weapon->current_reload = 0.0f;
    weapon->is_ready = 1;
    
    unit->weapon_count++;
    
    return weapon->weapon_id;
}

void CombatSystem_RemoveWeapon(CombatSystem* system, uint32_t unit_id, uint32_t weapon_id) {
    if (!system) return;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return;
    
    Combat_Unit* unit = &system->units[idx];
    if (weapon_id >= unit->weapon_count) return;
    
    if (weapon_id < unit->weapon_count - 1) {
        memmove(&unit->weapons[weapon_id],
               &unit->weapons[weapon_id + 1],
               (unit->weapon_count - weapon_id - 1) * sizeof(Weapon));
    }
    unit->weapon_count--;
    
    if (unit->active_weapon >= unit->weapon_count) {
        unit->active_weapon = 0;
    }
}

void CombatSystem_SelectWeapon(CombatSystem* system, uint32_t unit_id, uint32_t weapon_id) {
    if (!system) return;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return;
    
    if (weapon_id < system->units[idx].weapon_count) {
        system->units[idx].active_weapon = weapon_id;
    }
}

uint32_t CombatSystem_GetActiveWeapon(CombatSystem* system, uint32_t unit_id) {
    if (!system) return 0;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return 0;
    
    return system->units[idx].active_weapon;
}

uint32_t CombatSystem_GetWeaponCount(CombatSystem* system, uint32_t unit_id) {
    if (!system) return 0;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return 0;
    
    return system->units[idx].weapon_count;
}

Weapon* CombatSystem_GetWeapon(CombatSystem* system, uint32_t unit_id, uint32_t weapon_id) {
    if (!system) return NULL;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0 || weapon_id >= system->units[idx].weapon_count) return NULL;
    
    return &system->units[idx].weapons[weapon_id];
}

void CombatSystem_SetTarget(CombatSystem* system,
                            uint32_t attacker_id,
                            uint32_t target_id) {
    if (!system) return;
    
    int idx = combat_find_unit(system, attacker_id);
    if (idx < 0) return;
    
    system->units[idx].target_unit = target_id;
}

void CombatSystem_SetTargetPosition(CombatSystem* system,
                                    uint32_t attacker_id,
                                    float target_x, float target_y) {
    if (!system) return;
    
    int idx = combat_find_unit(system, attacker_id);
    if (idx < 0) return;
    
    system->units[idx].target_x = target_x;
    system->units[idx].target_y = target_y;
}

uint32_t CombatSystem_GetTarget(CombatSystem* system, uint32_t attacker_id) {
    if (!system) return 0;
    
    int idx = combat_find_unit(system, attacker_id);
    if (idx < 0) return 0;
    
    return system->units[idx].target_unit;
}

void CombatSystem_ClearTarget(CombatSystem* system, uint32_t attacker_id) {
    if (!system) return;
    
    int idx = combat_find_unit(system, attacker_id);
    if (idx < 0) return;
    
    system->units[idx].target_unit = 0;
    system->units[idx].attacking = 0;
}

int CombatSystem_CanAttack(CombatSystem* system, uint32_t attacker_id) {
    if (!system) return 0;
    
    int idx = combat_find_unit(system, attacker_id);
    if (idx < 0) return 0;
    
    Combat_Unit* unit = &system->units[idx];
    return unit->health > 0 && unit->weapon_count > 0;
}

int CombatSystem_IsWeaponReady(CombatSystem* system, uint32_t unit_id, uint32_t weapon_id) {
    if (!system) return 0;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return 0;
    
    Combat_Unit* unit = &system->units[idx];
    if (weapon_id >= unit->weapon_count) return 0;
    
    return unit->weapons[weapon_id].is_ready;
}

void CombatSystem_Attack(CombatSystem* system, uint32_t attacker_id) {
    if (!system) return;
    
    int idx = combat_find_unit(system, attacker_id);
    if (idx < 0) return;
    
    Combat_Unit* attacker = &system->units[idx];
    
    if (!CombatSystem_CanAttack(system, attacker_id)) return;
    
    Weapon* weapon = &attacker->weapons[attacker->active_weapon];
    
    if (!weapon->is_ready) return;
    
    attacker->attacking = 1;
    weapon->is_ready = 0;
    weapon->current_reload = weapon->reload_time;
}

void CombatSystem_FireWeapon(CombatSystem* system,
                             uint32_t attacker_id,
                             uint32_t weapon_id,
                             float target_x, float target_y) {
    if (!system) return;
    
    int idx = combat_find_unit(system, attacker_id);
    if (idx < 0) return;
    
    Combat_Unit* attacker = &system->units[idx];
    if (weapon_id >= attacker->weapon_count) return;
    
    Weapon* weapon = &attacker->weapons[weapon_id];
    
    /* Calculate damage */
    float damage = weapon->damage_min + 
                  (rand() % (int)(weapon->damage_max - weapon->damage_min + 1));
    
    /* Fire projectile */
    CombatSystem_SpawnProjectile(system,
                                 attacker->health, 0,  /* Placeholder position */
                                 0, 0,  /* Velocity */
                                 1.0f,  /* Lifetime */
                                 attacker_id,
                                 0);
}

void CombatSystem_DealDamage(CombatSystem* system,
                             uint32_t attacker_id,
                             uint32_t defender_id,
                             float damage_amount,
                             Damage_Type damage_type,
                             uint32_t source_weapon) {
    if (!system || attacker_id == defender_id) return;
    
    int defender_idx = combat_find_unit(system, defender_id);
    if (defender_idx < 0) return;
    
    int attacker_idx = combat_find_unit(system, attacker_id);
    
    /* Apply damage modifiers */
    float modifier = 1.0f;
    if (damage_type < COMBAT_MAX_DAMAGE_TYPES && attacker_idx >= 0) {
        modifier = system->damage_modifiers[damage_type][attacker_idx % 16];
    }
    
    float final_damage = damage_amount * modifier;
    
    Combat_Unit* defender = &system->units[defender_idx];
    defender->health -= final_damage;
    
    if (defender->health < 0) {
        defender->health = 0;
    }
    
    combat_update_unit_state(defender);
    
    system->total_damage_dealt += (uint32_t)final_damage;
    
    /* Call damage callback */
    if (system->damage_callback) {
        Damage_Event event;
        event.attacker_id = attacker_id;
        event.defender_id = defender_id;
        event.damage_amount = final_damage;
        event.damage_type = damage_type;
        event.source_weapon = source_weapon;
        
        system->damage_callback(&event, system->damage_callback_user_data);
    }
}

void CombatSystem_HealUnit(CombatSystem* system, uint32_t unit_id, float heal_amount) {
    if (!system) return;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return;
    
    Combat_Unit* unit = &system->units[idx];
    unit->health += heal_amount;
    
    if (unit->health > unit->max_health) {
        unit->health = unit->max_health;
    }
    
    combat_update_unit_state(unit);
}

float CombatSystem_CalculateDamage(CombatSystem* system,
                                   uint32_t attacker_id,
                                   uint32_t target_unit_id,
                                   Damage_Type damage_type) {
    if (!system) return 0.0f;
    
    int attacker_idx = combat_find_unit(system, attacker_id);
    if (attacker_idx < 0) return 0.0f;
    
    Combat_Unit* attacker = &system->units[attacker_idx];
    
    if (attacker->weapon_count == 0) return 0.0f;
    
    Weapon* weapon = &attacker->weapons[attacker->active_weapon];
    
    float damage = weapon->damage_min + 
                  (rand() % (int)(weapon->damage_max - weapon->damage_min + 1));
    
    return damage;
}

int CombatSystem_KillUnit(CombatSystem* system, uint32_t unit_id, uint32_t killer_id) {
    if (!system) return 0;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return 0;
    
    Combat_Unit* unit = &system->units[idx];
    
    if (unit->health > 0) {
        unit->health = 0;
    }
    
    unit->state = UNIT_STATE_DESTROYED;
    unit->deaths++;
    
    if (killer_id != 0) {
        int killer_idx = combat_find_unit(system, killer_id);
        if (killer_idx >= 0) {
            system->units[killer_idx].kills++;
            system->units[killer_idx].experience += 50;
        }
    }
    
    return 1;
}

void CombatSystem_RespawnUnit(CombatSystem* system, uint32_t unit_id, float health) {
    if (!system) return;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return;
    
    Combat_Unit* unit = &system->units[idx];
    unit->health = health;
    unit->state = UNIT_STATE_ALIVE;
}

Veterancy_Level CombatSystem_GetVeterancyLevel(CombatSystem* system, uint32_t unit_id) {
    if (!system) return VETERANCY_LEVEL_ROOKIE;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return VETERANCY_LEVEL_ROOKIE;
    
    return system->units[idx].veterancy;
}

void CombatSystem_SetVeterancyLevel(CombatSystem* system, uint32_t unit_id, Veterancy_Level level) {
    if (!system) return;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return;
    
    system->units[idx].veterancy = level;
}

void CombatSystem_AddExperience(CombatSystem* system, uint32_t unit_id, uint32_t experience) {
    if (!system) return;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return;
    
    system->units[idx].experience += experience;
    
    /* Level up based on experience */
    if (system->units[idx].experience >= 300 && system->units[idx].veterancy < VETERANCY_LEVEL_ELITE) {
        system->units[idx].veterancy = (Veterancy_Level)(system->units[idx].veterancy + 1);
        system->units[idx].experience = 0;
    }
}

uint32_t CombatSystem_GetExperience(CombatSystem* system, uint32_t unit_id) {
    if (!system) return 0;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return 0;
    
    return system->units[idx].experience;
}

void CombatSystem_IncreaseKillCount(CombatSystem* system, uint32_t unit_id) {
    if (!system) return;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return;
    
    system->units[idx].kills++;
}

uint32_t CombatSystem_GetKillCount(CombatSystem* system, uint32_t unit_id) {
    if (!system) return 0;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return 0;
    
    return system->units[idx].kills;
}

float CombatSystem_GetDamageModifier(CombatSystem* system,
                                     Damage_Type damage_type,
                                     uint32_t defender_unit_type) {
    if (!system || damage_type >= COMBAT_MAX_DAMAGE_TYPES) return 1.0f;
    
    return system->damage_modifiers[damage_type][defender_unit_type % 16];
}

void CombatSystem_SetDamageModifier(CombatSystem* system,
                                    Damage_Type damage_type,
                                    uint32_t unit_type,
                                    float modifier) {
    if (!system || damage_type >= COMBAT_MAX_DAMAGE_TYPES) return;
    
    system->damage_modifiers[damage_type][unit_type % 16] = modifier;
}

int CombatSystem_IsInRange(CombatSystem* system,
                           uint32_t attacker_id,
                           float target_x, float target_y) {
    if (!system) return 0;
    
    int idx = combat_find_unit(system, attacker_id);
    if (idx < 0 || system->units[idx].weapon_count == 0) return 0;
    
    Weapon* weapon = &system->units[idx].weapons[system->units[idx].active_weapon];
    
    float dx = target_x;  /* Would need attacker position */
    float dy = target_y;
    float distance = sqrtf(dx * dx + dy * dy);
    
    return distance <= weapon->range;
}

float CombatSystem_GetDistanceToTarget(CombatSystem* system,
                                       uint32_t attacker_id,
                                       uint32_t target_id) {
    if (!system) return 0.0f;
    
    return 0.0f;  /* Would need position data */
}

uint32_t CombatSystem_GetDeathCount(CombatSystem* system, uint32_t unit_id) {
    if (!system) return 0;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return 0;
    
    return system->units[idx].deaths;
}

uint32_t CombatSystem_GetKillDeathRatio(CombatSystem* system, uint32_t unit_id) {
    if (!system) return 0;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return 0;
    
    if (system->units[idx].deaths == 0) return system->units[idx].kills * 100;
    
    return (system->units[idx].kills * 100) / system->units[idx].deaths;
}

uint32_t CombatSystem_GetTotalDamageDealt(CombatSystem* system, uint32_t unit_id) {
    if (!system) return 0;
    return system->total_damage_dealt;
}

uint32_t CombatSystem_GetTotalDamageTaken(CombatSystem* system, uint32_t unit_id) {
    if (!system) return 0;
    return system->total_damage_taken;
}

uint32_t CombatSystem_SpawnProjectile(CombatSystem* system,
                                      float start_x, float start_y,
                                      float velocity_x, float velocity_y,
                                      float lifetime,
                                      uint32_t source_unit,
                                      uint32_t target_unit) {
    if (!system || system->num_projectiles >= system->max_projectiles) return 0;
    
    Projectile* proj = &system->projectiles[system->num_projectiles];
    proj->x = start_x;
    proj->y = start_y;
    proj->vx = velocity_x;
    proj->vy = velocity_y;
    proj->lifetime = lifetime;
    
    system->num_projectiles++;
    
    return system->num_projectiles - 1;
}

void CombatSystem_UpdateProjectiles(CombatSystem* system, float delta_time) {
    if (!system) return;
    
    for (int i = (int)system->num_projectiles - 1; i >= 0; i--) {
        Projectile* proj = &system->projectiles[i];
        
        proj->x += proj->vx * delta_time;
        proj->y += proj->vy * delta_time;
        proj->lifetime -= delta_time;
        
        if (proj->lifetime <= 0) {
            if (i < (int)system->num_projectiles - 1) {
                memmove(proj,
                       &system->projectiles[i + 1],
                       (system->num_projectiles - i - 1) * sizeof(Projectile));
            }
            system->num_projectiles--;
        }
    }
}

void CombatSystem_Update(CombatSystem* system, float delta_time) {
    if (!system) return;
    
    CombatSystem_UpdateWeaponCooldowns(system, delta_time);
    CombatSystem_UpdateProjectiles(system, delta_time);
}

void CombatSystem_UpdateWeaponCooldowns(CombatSystem* system, float delta_time) {
    if (!system) return;
    
    for (uint32_t i = 0; i < system->num_units; i++) {
        Combat_Unit* unit = &system->units[i];
        
        for (uint32_t j = 0; j < unit->weapon_count; j++) {
            Weapon* weapon = &unit->weapons[j];
            
            if (!weapon->is_ready) {
                weapon->current_reload -= delta_time;
                if (weapon->current_reload <= 0) {
                    weapon->is_ready = 1;
                    weapon->current_reload = 0;
                }
            }
        }
    }
}

void CombatSystem_DealAreaDamage(CombatSystem* system,
                                 float center_x, float center_y,
                                 float radius,
                                 float damage,
                                 Damage_Type damage_type,
                                 uint32_t source_unit) {
    if (!system) return;
    
    for (uint32_t i = 0; i < system->num_units; i++) {
        if (system->units[i].unit_id == source_unit) continue;
        if (!CombatSystem_IsUnitAlive(system, system->units[i].unit_id)) continue;
        
        /* Calculate distance (would need position data) */
        /* if (distance <= radius) { ... } */
    }
}

const char* CombatSystem_GetLastError(CombatSystem* system) {
    if (!system) return "Invalid system";
    return system->last_error;
}

int CombatSystem_ClearErrors(CombatSystem* system) {
    if (!system) return 0;
    
    int count = system->error_count;
    system->error_count = 0;
    memset(system->last_error, 0, sizeof(system->last_error));
    
    return count;
}

void CombatSystem_PrintUnitStats(CombatSystem* system, uint32_t unit_id) {
    if (!system) return;
    
    int idx = combat_find_unit(system, unit_id);
    if (idx < 0) return;
    
    Combat_Unit* unit = &system->units[idx];
    
    printf("Combat Stats for Unit %u:\n", unit_id);
    printf("Health: %.1f / %.1f (%.1f%%)\n", 
           unit->health, unit->max_health,
           CombatSystem_GetUnitHealthPercent(system, unit_id));
    printf("State: %d\n", unit->state);
    printf("Weapons: %u\n", unit->weapon_count);
    printf("Kills: %u\n", unit->kills);
    printf("Deaths: %u\n", unit->deaths);
    printf("Experience: %u\n", unit->experience);
    printf("Veterancy: %d\n", unit->veterancy);
}

void CombatSystem_PrintCombatStats(CombatSystem* system) {
    if (!system) return;
    
    printf("Combat System Stats:\n");
    printf("Active Units: %u\n", system->num_units);
    printf("Active Projectiles: %u\n", system->num_projectiles);
    printf("Total Damage Dealt: %u\n", system->total_damage_dealt);
    
    uint32_t alive_count = 0;
    uint32_t dead_count = 0;
    
    for (uint32_t i = 0; i < system->num_units; i++) {
        if (system->units[i].health > 0) {
            alive_count++;
        } else {
            dead_count++;
        }
    }
    
    printf("Alive Units: %u\n", alive_count);
    printf("Dead Units: %u\n", dead_count);
}

uint32_t CombatSystem_GetActiveUnitCount(CombatSystem* system) {
    if (!system) return 0;
    return system->num_units;
}

void CombatSystem_RegisterDamageCallback(CombatSystem* system, DamageEventCallback callback, void* user_data) {
    if (!system) return;
    
    system->damage_callback = callback;
    system->damage_callback_user_data = user_data;
}
