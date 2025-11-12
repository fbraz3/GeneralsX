#pragma once

#include <cstdint>
#include <cstddef>

/* Phase 34: Pathfinding & Movement System
 * 
 * Provides A* pathfinding and unit movement:
 * - A* algorithm implementation
 * - Navigation grid/graph
 * - Collision avoidance
 * - Formation movement
 * - Speed/acceleration control
 */

typedef struct Pathfinder Pathfinder;
typedef struct PathfindingGrid PathfindingGrid;
typedef uint32_t PathHandle;

#define PATHFINDING_MAX_PATHS 1000
#define PATHFINDING_MAX_WAYPOINTS 256
#define PATHFINDING_GRID_WIDTH 256
#define PATHFINDING_GRID_HEIGHT 256
#define PATHFINDING_CELL_SIZE 2.0f

/* Pathfinding handle ranges (Phase 34: 34000-34999) */
#define PATHFINDING_HANDLE_MIN 34000
#define PATHFINDING_HANDLE_MAX 34999

typedef enum {
    PATHFINDING_STATUS_IDLE = 0,
    PATHFINDING_STATUS_FINDING = 1,
    PATHFINDING_STATUS_FOUND = 2,
    PATHFINDING_STATUS_PARTIAL = 3,
    PATHFINDING_STATUS_FAILED = 4,
} Pathfinding_Status;

typedef enum {
    MOVEMENT_STATE_IDLE = 0,
    MOVEMENT_STATE_MOVING = 1,
    MOVEMENT_STATE_AT_DESTINATION = 2,
    MOVEMENT_STATE_BLOCKED = 3,
    MOVEMENT_STATE_WAITING = 4,
} Movement_State;

typedef enum {
    FORMATION_NONE = 0,
    FORMATION_LINE = 1,
    FORMATION_COLUMN = 2,
    FORMATION_WEDGE = 3,
    FORMATION_BOX = 4,
} Formation_Type;

typedef struct {
    float x, y;
} Waypoint;

typedef struct {
    uint16_t x, y;
} GridCell;

typedef struct {
    float x, y;
    float vx, vy;
    float desired_speed;
    float max_speed;
    float acceleration;
    float radius;
} Moving_Unit;

typedef struct {
    PathHandle handle;
    Waypoint* waypoints;
    uint32_t waypoint_count;
    uint32_t current_waypoint;
    Pathfinding_Status status;
    float length;
} Path;

typedef struct {
    uint8_t walkable;
    uint8_t cost;
    uint16_t region_id;
} GridCellData;

/* System Management */
Pathfinder* Pathfinder_Create(void);
void Pathfinder_Destroy(Pathfinder* pathfinder);
void Pathfinder_Initialize(Pathfinder* pathfinder);
void Pathfinder_Shutdown(Pathfinder* pathfinder);

/* Grid Management */
void Pathfinder_SetGridDimensions(Pathfinder* pathfinder, uint32_t width, uint32_t height, float cell_size);
void Pathfinder_SetCellWalkable(Pathfinder* pathfinder, uint32_t x, uint32_t y, int walkable);
void Pathfinder_SetCellCost(Pathfinder* pathfinder, uint32_t x, uint32_t y, uint8_t cost);
int Pathfinder_IsCellWalkable(Pathfinder* pathfinder, uint32_t x, uint32_t y);
uint8_t Pathfinder_GetCellCost(Pathfinder* pathfinder, uint32_t x, uint32_t y);
void Pathfinder_WorldToGrid(Pathfinder* pathfinder, float world_x, float world_y, uint32_t* out_grid_x, uint32_t* out_grid_y);
void Pathfinder_GridToWorld(Pathfinder* pathfinder, uint32_t grid_x, uint32_t grid_y, float* out_world_x, float* out_world_y);

/* Pathfinding A* Algorithm */
PathHandle Pathfinder_FindPath(Pathfinder* pathfinder,
                               float start_x, float start_y,
                               float goal_x, float goal_y,
                               float unit_radius);
PathHandle Pathfinder_FindPathAvoidingObjects(Pathfinder* pathfinder,
                                              float start_x, float start_y,
                                              float goal_x, float goal_y,
                                              float unit_radius,
                                              float* obstacle_positions,
                                              float* obstacle_radii,
                                              uint32_t obstacle_count);
void Pathfinder_CancelPath(Pathfinder* pathfinder, PathHandle handle);

/* Path Query */
Pathfinding_Status Pathfinder_GetPathStatus(Pathfinder* pathfinder, PathHandle handle);
uint32_t Pathfinder_GetPathWaypointCount(Pathfinder* pathfinder, PathHandle handle);
Waypoint* Pathfinder_GetPathWaypoints(Pathfinder* pathfinder, PathHandle handle);
float Pathfinder_GetPathLength(Pathfinder* pathfinder, PathHandle handle);
Waypoint Pathfinder_GetNextWaypoint(Pathfinder* pathfinder, PathHandle handle);

/* Movement Simulation */
void Pathfinder_AddMovingUnit(Pathfinder* pathfinder,
                              uint32_t unit_id,
                              float x, float y,
                              float max_speed,
                              float acceleration,
                              float radius);
void Pathfinder_RemoveMovingUnit(Pathfinder* pathfinder, uint32_t unit_id);
void Pathfinder_SetUnitTarget(Pathfinder* pathfinder, uint32_t unit_id, float target_x, float target_y);
void Pathfinder_SetUnitSpeed(Pathfinder* pathfinder, uint32_t unit_id, float speed);
void Pathfinder_SetUnitMaxSpeed(Pathfinder* pathfinder, uint32_t unit_id, float max_speed);
void Pathfinder_SetUnitAcceleration(Pathfinder* pathfinder, uint32_t unit_id, float acceleration);

/* Movement Query */
Movement_State Pathfinder_GetUnitMovementState(Pathfinder* pathfinder, uint32_t unit_id);
void Pathfinder_GetUnitPosition(Pathfinder* pathfinder, uint32_t unit_id, float* out_x, float* out_y);
void Pathfinder_GetUnitVelocity(Pathfinder* pathfinder, uint32_t unit_id, float* out_vx, float* out_vy);
float Pathfinder_GetUnitSpeed(Pathfinder* pathfinder, uint32_t unit_id);

/* Collision Avoidance */
int Pathfinder_CheckCollision(Pathfinder* pathfinder,
                              float x1, float y1, float radius1,
                              float x2, float y2, float radius2);
void Pathfinder_AvoidCollision(Pathfinder* pathfinder,
                               uint32_t unit_id,
                               float* out_desired_vx, float* out_desired_vy);
void Pathfinder_SetCollisionRadius(Pathfinder* pathfinder, uint32_t unit_id, float radius);

/* Formation Movement */
void Pathfinder_CreateFormation(Pathfinder* pathfinder,
                                Formation_Type type,
                                uint32_t* unit_ids,
                                uint32_t unit_count);
void Pathfinder_MoveFormation(Pathfinder* pathfinder,
                              uint32_t leader_unit_id,
                              float target_x, float target_y);
void Pathfinder_SetFormationSpacing(Pathfinder* pathfinder,
                                    Formation_Type type,
                                    float spacing);
void Pathfinder_DisbandFormation(Pathfinder* pathfinder);

/* Steering Behaviors */
void Pathfinder_ApplySteering(Pathfinder* pathfinder,
                              uint32_t unit_id,
                              float steer_x, float steer_y,
                              float steer_magnitude);
void Pathfinder_ApplySeekBehavior(Pathfinder* pathfinder,
                                  uint32_t unit_id,
                                  float target_x, float target_y,
                                  float max_force);
void Pathfinder_ApplyAvoidanceBehavior(Pathfinder* pathfinder,
                                       uint32_t unit_id,
                                       float* obstacle_x,
                                       float* obstacle_y,
                                       uint32_t obstacle_count,
                                       float max_force);

/* Spatial Queries */
int Pathfinder_IsPathObstructed(Pathfinder* pathfinder,
                                float start_x, float start_y,
                                float end_x, float end_y,
                                float unit_radius);
int Pathfinder_LineOfSight(Pathfinder* pathfinder,
                           float x1, float y1,
                           float x2, float y2,
                           float unit_radius);
void Pathfinder_GetNearbyUnits(Pathfinder* pathfinder,
                               uint32_t unit_id,
                               float search_radius,
                               uint32_t* out_nearby_ids,
                               uint32_t* out_count,
                               uint32_t max_results);

/* Optimization & Smoothing */
void Pathfinder_SmoothPath(Pathfinder* pathfinder, PathHandle handle);
void Pathfinder_SimplifyPath(Pathfinder* pathfinder, PathHandle handle, float max_deviation);

/* Navigation Mesh Generation */
void Pathfinder_BuildNavigationMesh(Pathfinder* pathfinder,
                                    GridCellData* grid_data,
                                    uint32_t width, uint32_t height);
void Pathfinder_UpdateRegions(Pathfinder* pathfinder);

/* Update */
void Pathfinder_Update(Pathfinder* pathfinder, float delta_time);
void Pathfinder_UpdateMovement(Pathfinder* pathfinder, float delta_time);

/* Debugging & Analysis */
const char* Pathfinder_GetLastError(Pathfinder* pathfinder);
int Pathfinder_ClearErrors(Pathfinder* pathfinder);
void Pathfinder_PrintGridInfo(Pathfinder* pathfinder);
void Pathfinder_PrintPathInfo(Pathfinder* pathfinder, PathHandle handle);
void Pathfinder_PrintUnitInfo(Pathfinder* pathfinder, uint32_t unit_id);
uint32_t Pathfinder_GetStats(Pathfinder* pathfinder);

/* Performance Metrics */
float Pathfinder_GetPathComputeTime(Pathfinder* pathfinder, PathHandle handle);
uint32_t Pathfinder_GetActivePathCount(Pathfinder* pathfinder);
uint32_t Pathfinder_GetMovingUnitCount(Pathfinder* pathfinder);
