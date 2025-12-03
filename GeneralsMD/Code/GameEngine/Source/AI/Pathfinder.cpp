#include "Pathfinder.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>

typedef struct {
    uint32_t x, y;
    float g_cost;
    float h_cost;
    float f_cost;
    int open;
    int closed;
    uint32_t parent_x, parent_y;
} AStarNode;

typedef struct {
    uint32_t unit_id;
    float x, y;
    float vx, vy;
    float desired_speed;
    float max_speed;
    float acceleration;
    float radius;
    Movement_State state;
    PathHandle current_path;
} Unit;

struct Pathfinder {
    /* Grid */
    GridCellData* grid;
    uint32_t grid_width, grid_height;
    float cell_size;
    
    /* Paths */
    Path* paths;
    uint32_t max_paths;
    uint32_t num_paths;
    
    /* Units */
    Unit* units;
    uint32_t max_units;
    uint32_t num_units;
    
    /* Formation */
    Formation_Type current_formation;
    float formation_spacing;
    
    /* A* scratch space */
    AStarNode* astar_open;
    AStarNode* astar_closed;
    uint32_t astar_open_count;
    uint32_t astar_closed_count;
    
    /* Error tracking */
    char last_error[256];
    int error_count;
};

static PathHandle pathfinder_allocate_handle(void) {
    return PATHFINDING_HANDLE_MIN + (rand() % (PATHFINDING_HANDLE_MAX - PATHFINDING_HANDLE_MIN));
}

static float heuristic_distance(float dx, float dy) {
    return sqrtf(dx * dx + dy * dy);
}

static float grid_distance(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2) {
    int dx = (int)x1 - (int)x2;
    int dy = (int)y1 - (int)y2;
    return sqrtf((float)(dx * dx + dy * dy));
}

Pathfinder* Pathfinder_Create(void) {
    Pathfinder* pathfinder = (Pathfinder*)malloc(sizeof(Pathfinder));
    if (!pathfinder) return NULL;
    
    memset(pathfinder, 0, sizeof(Pathfinder));
    
    pathfinder->max_paths = PATHFINDING_MAX_PATHS;
    pathfinder->max_units = 1000;
    
    pathfinder->grid = (GridCellData*)calloc(PATHFINDING_GRID_WIDTH * PATHFINDING_GRID_HEIGHT, 
                                             sizeof(GridCellData));
    pathfinder->paths = (Path*)calloc(pathfinder->max_paths, sizeof(Path));
    pathfinder->units = (Unit*)calloc(pathfinder->max_units, sizeof(Unit));
    
    pathfinder->astar_open = (AStarNode*)calloc(PATHFINDING_GRID_WIDTH * PATHFINDING_GRID_HEIGHT, 
                                                sizeof(AStarNode));
    pathfinder->astar_closed = (AStarNode*)calloc(PATHFINDING_GRID_WIDTH * PATHFINDING_GRID_HEIGHT, 
                                                  sizeof(AStarNode));
    
    if (!pathfinder->grid || !pathfinder->paths || !pathfinder->units || 
        !pathfinder->astar_open || !pathfinder->astar_closed) {
        free(pathfinder->grid);
        free(pathfinder->paths);
        free(pathfinder->units);
        free(pathfinder->astar_open);
        free(pathfinder->astar_closed);
        free(pathfinder);
        return NULL;
    }
    
    pathfinder->grid_width = PATHFINDING_GRID_WIDTH;
    pathfinder->grid_height = PATHFINDING_GRID_HEIGHT;
    pathfinder->cell_size = PATHFINDING_CELL_SIZE;
    
    /* Initialize all cells as walkable */
    for (uint32_t i = 0; i < pathfinder->grid_width * pathfinder->grid_height; i++) {
        pathfinder->grid[i].walkable = 1;
        pathfinder->grid[i].cost = 1;
        pathfinder->grid[i].region_id = 0;
    }
    
    /* Initialize paths with waypoint storage */
    for (uint32_t i = 0; i < pathfinder->max_paths; i++) {
        pathfinder->paths[i].waypoints = (Waypoint*)calloc(PATHFINDING_MAX_WAYPOINTS, sizeof(Waypoint));
        pathfinder->paths[i].handle = 0;
        pathfinder->paths[i].waypoint_count = 0;
    }
    
    return pathfinder;
}

void Pathfinder_Destroy(Pathfinder* pathfinder) {
    if (!pathfinder) return;
    
    for (uint32_t i = 0; i < pathfinder->max_paths; i++) {
        free(pathfinder->paths[i].waypoints);
    }
    
    free(pathfinder->grid);
    free(pathfinder->paths);
    free(pathfinder->units);
    free(pathfinder->astar_open);
    free(pathfinder->astar_closed);
    free(pathfinder);
}

void Pathfinder_Initialize(Pathfinder* pathfinder) {
    if (!pathfinder) return;
    
    pathfinder->num_paths = 0;
    pathfinder->num_units = 0;
    pathfinder->current_formation = FORMATION_NONE;
    pathfinder->formation_spacing = 3.0f;
    
    printf("Phase 34: Pathfinder system initialized\n");
    printf("Grid: %ux%u, Cell size: %.1f\n", pathfinder->grid_width, 
            pathfinder->grid_height, pathfinder->cell_size);
}

void Pathfinder_Shutdown(Pathfinder* pathfinder) {
    if (!pathfinder) return;
    
    pathfinder->num_paths = 0;
    pathfinder->num_units = 0;
    
    printf("Phase 34: Pathfinder system shutdown\n");
}

void Pathfinder_SetGridDimensions(Pathfinder* pathfinder, uint32_t width, uint32_t height, float cell_size) {
    if (!pathfinder) return;
    
    pathfinder->grid_width = (width > PATHFINDING_GRID_WIDTH) ? PATHFINDING_GRID_WIDTH : width;
    pathfinder->grid_height = (height > PATHFINDING_GRID_HEIGHT) ? PATHFINDING_GRID_HEIGHT : height;
    pathfinder->cell_size = cell_size;
}

void Pathfinder_SetCellWalkable(Pathfinder* pathfinder, uint32_t x, uint32_t y, int walkable) {
    if (!pathfinder || x >= pathfinder->grid_width || y >= pathfinder->grid_height) return;
    
    uint32_t index = y * pathfinder->grid_width + x;
    pathfinder->grid[index].walkable = walkable ? 1 : 0;
}

void Pathfinder_SetCellCost(Pathfinder* pathfinder, uint32_t x, uint32_t y, uint8_t cost) {
    if (!pathfinder || x >= pathfinder->grid_width || y >= pathfinder->grid_height) return;
    
    uint32_t index = y * pathfinder->grid_width + x;
    pathfinder->grid[index].cost = cost;
}

int Pathfinder_IsCellWalkable(Pathfinder* pathfinder, uint32_t x, uint32_t y) {
    if (!pathfinder || x >= pathfinder->grid_width || y >= pathfinder->grid_height) return 0;
    
    uint32_t index = y * pathfinder->grid_width + x;
    return pathfinder->grid[index].walkable;
}

uint8_t Pathfinder_GetCellCost(Pathfinder* pathfinder, uint32_t x, uint32_t y) {
    if (!pathfinder || x >= pathfinder->grid_width || y >= pathfinder->grid_height) return 255;
    
    uint32_t index = y * pathfinder->grid_width + x;
    return pathfinder->grid[index].cost;
}

void Pathfinder_WorldToGrid(Pathfinder* pathfinder, float world_x, float world_y, 
                           uint32_t* out_grid_x, uint32_t* out_grid_y) {
    if (!pathfinder || !out_grid_x || !out_grid_y) return;
    
    *out_grid_x = (uint32_t)(world_x / pathfinder->cell_size);
    *out_grid_y = (uint32_t)(world_y / pathfinder->cell_size);
    
    if (*out_grid_x >= pathfinder->grid_width) *out_grid_x = pathfinder->grid_width - 1;
    if (*out_grid_y >= pathfinder->grid_height) *out_grid_y = pathfinder->grid_height - 1;
}

void Pathfinder_GridToWorld(Pathfinder* pathfinder, uint32_t grid_x, uint32_t grid_y, 
                           float* out_world_x, float* out_world_y) {
    if (!pathfinder || !out_world_x || !out_world_y) return;
    
    *out_world_x = (float)grid_x * pathfinder->cell_size + pathfinder->cell_size * 0.5f;
    *out_world_y = (float)grid_y * pathfinder->cell_size + pathfinder->cell_size * 0.5f;
}

static int pathfinder_find_path_astar(Pathfinder* pathfinder,
                                       uint32_t start_x, uint32_t start_y,
                                       uint32_t goal_x, uint32_t goal_y,
                                       Waypoint* out_waypoints,
                                       uint32_t* out_waypoint_count) {
    if (*out_waypoint_count == 0) return 0;
    
    memset(pathfinder->astar_open, 0, pathfinder->grid_width * pathfinder->grid_height * sizeof(AStarNode));
    memset(pathfinder->astar_closed, 0, pathfinder->grid_width * pathfinder->grid_height * sizeof(AStarNode));
    
    pathfinder->astar_open_count = 0;
    pathfinder->astar_closed_count = 0;
    
    /* Add start node to open list */
    AStarNode* start_node = &pathfinder->astar_open[pathfinder->astar_open_count++];
    start_node->x = start_x;
    start_node->y = start_y;
    start_node->g_cost = 0.0f;
    start_node->h_cost = grid_distance(start_x, start_y, goal_x, goal_y);
    start_node->f_cost = start_node->h_cost;
    start_node->open = 1;
    
    int found = 0;
    uint32_t iterations = 0;
    uint32_t max_iterations = 10000;
    
    while (pathfinder->astar_open_count > 0 && iterations < max_iterations) {
        iterations++;
        
        /* Find node with lowest f_cost */
        int current_idx = 0;
        float lowest_f = pathfinder->astar_open[0].f_cost;
        
        for (uint32_t i = 1; i < pathfinder->astar_open_count; i++) {
            if (pathfinder->astar_open[i].f_cost < lowest_f) {
                lowest_f = pathfinder->astar_open[i].f_cost;
                current_idx = i;
            }
        }
        
        AStarNode* current = &pathfinder->astar_open[current_idx];
        
        if (current->x == goal_x && current->y == goal_y) {
            found = 1;
            break;
        }
        
        /* Move from open to closed */
        pathfinder->astar_closed[pathfinder->astar_closed_count++] = *current;
        
        /* Remove from open list */
        if (current_idx < pathfinder->astar_open_count - 1) {
            memmove(&pathfinder->astar_open[current_idx],
                   &pathfinder->astar_open[current_idx + 1],
                   (pathfinder->astar_open_count - current_idx - 1) * sizeof(AStarNode));
        }
        pathfinder->astar_open_count--;
        
        /* Check neighbors */
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                
                int nx = (int)current->x + dx;
                int ny = (int)current->y + dy;
                
                if (nx < 0 || nx >= (int)pathfinder->grid_width || 
                    ny < 0 || ny >= (int)pathfinder->grid_height) continue;
                
                if (!pathfinder->grid[ny * pathfinder->grid_width + nx].walkable) continue;
                
                float movement_cost = (dx != 0 && dy != 0) ? 1.414f : 1.0f;
                float g_cost = current->g_cost + movement_cost;
                float h_cost = grid_distance(nx, ny, goal_x, goal_y);
                float f_cost = g_cost + h_cost;
                
                /* Check if in closed list with lower g_cost */
                int in_closed = 0;
                for (uint32_t i = 0; i < pathfinder->astar_closed_count; i++) {
                    if (pathfinder->astar_closed[i].x == (uint32_t)nx && 
                        pathfinder->astar_closed[i].y == (uint32_t)ny) {
                        in_closed = 1;
                        if (g_cost >= pathfinder->astar_closed[i].g_cost) break;
                    }
                }
                
                if (in_closed) continue;
                
                /* Add to open list if not already there or if better path */
                int in_open = -1;
                for (uint32_t i = 0; i < pathfinder->astar_open_count; i++) {
                    if (pathfinder->astar_open[i].x == (uint32_t)nx && 
                        pathfinder->astar_open[i].y == (uint32_t)ny) {
                        in_open = i;
                        break;
                    }
                }
                
                if (in_open >= 0) {
                    if (g_cost < pathfinder->astar_open[in_open].g_cost) {
                        pathfinder->astar_open[in_open].g_cost = g_cost;
                        pathfinder->astar_open[in_open].f_cost = f_cost;
                        pathfinder->astar_open[in_open].parent_x = current->x;
                        pathfinder->astar_open[in_open].parent_y = current->y;
                    }
                } else if (pathfinder->astar_open_count < pathfinder->grid_width * pathfinder->grid_height) {
                    AStarNode* new_node = &pathfinder->astar_open[pathfinder->astar_open_count++];
                    new_node->x = nx;
                    new_node->y = ny;
                    new_node->g_cost = g_cost;
                    new_node->h_cost = h_cost;
                    new_node->f_cost = f_cost;
                    new_node->parent_x = current->x;
                    new_node->parent_y = current->y;
                    new_node->open = 1;
                }
            }
        }
    }
    
    if (!found) return 0;
    
    /* Reconstruct path */
    uint32_t path_length = 0;
    uint32_t current_x = goal_x;
    uint32_t current_y = goal_y;
    
    while (path_length < *out_waypoint_count && 
          !(current_x == start_x && current_y == start_y)) {
        
        out_waypoints[path_length].x = (float)current_x * pathfinder->cell_size + pathfinder->cell_size * 0.5f;
        out_waypoints[path_length].y = (float)current_y * pathfinder->cell_size + pathfinder->cell_size * 0.5f;
        path_length++;
        
        /* Find parent */
        int parent_found = 0;
        for (uint32_t i = 0; i < pathfinder->astar_closed_count; i++) {
            if (pathfinder->astar_closed[i].x == current_x && 
                pathfinder->astar_closed[i].y == current_y) {
                current_x = pathfinder->astar_closed[i].parent_x;
                current_y = pathfinder->astar_closed[i].parent_y;
                parent_found = 1;
                break;
            }
        }
        
        if (!parent_found) break;
    }
    
    /* Reverse waypoints */
    for (uint32_t i = 0; i < path_length / 2; i++) {
        Waypoint temp = out_waypoints[i];
        out_waypoints[i] = out_waypoints[path_length - 1 - i];
        out_waypoints[path_length - 1 - i] = temp;
    }
    
    *out_waypoint_count = path_length;
    return 1;
}

PathHandle Pathfinder_FindPath(Pathfinder* pathfinder,
                               float start_x, float start_y,
                               float goal_x, float goal_y,
                               float unit_radius) {
    if (!pathfinder || pathfinder->num_paths >= pathfinder->max_paths) return 0;
    
    uint32_t start_grid_x, start_grid_y, goal_grid_x, goal_grid_y;
    
    Pathfinder_WorldToGrid(pathfinder, start_x, start_y, &start_grid_x, &start_grid_y);
    Pathfinder_WorldToGrid(pathfinder, goal_x, goal_y, &goal_grid_x, &goal_grid_y);
    
    Path* path = &pathfinder->paths[pathfinder->num_paths];
    path->waypoint_count = PATHFINDING_MAX_WAYPOINTS;
    
    int found = pathfinder_find_path_astar(pathfinder,
                                          start_grid_x, start_grid_y,
                                          goal_grid_x, goal_grid_y,
                                          path->waypoints,
                                          &path->waypoint_count);
    
    if (!found) {
        path->status = PATHFINDING_STATUS_FAILED;
        path->handle = 0;
        return 0;
    }
    
    path->status = PATHFINDING_STATUS_FOUND;
    path->handle = pathfinder_allocate_handle();
    path->current_waypoint = 0;
    path->length = 0.0f;
    
    for (uint32_t i = 0; i < path->waypoint_count - 1; i++) {
        float dx = path->waypoints[i+1].x - path->waypoints[i].x;
        float dy = path->waypoints[i+1].y - path->waypoints[i].y;
        path->length += sqrtf(dx * dx + dy * dy);
    }
    
    pathfinder->num_paths++;
    
    return path->handle;
}

PathHandle Pathfinder_FindPathAvoidingObjects(Pathfinder* pathfinder,
                                              float start_x, float start_y,
                                              float goal_x, float goal_y,
                                              float unit_radius,
                                              float* obstacle_positions,
                                              float* obstacle_radii,
                                              uint32_t obstacle_count) {
    if (!pathfinder) return 0;
    
    return Pathfinder_FindPath(pathfinder, start_x, start_y, goal_x, goal_y, unit_radius);
}

void Pathfinder_CancelPath(Pathfinder* pathfinder, PathHandle handle) {
    if (!pathfinder) return;
    
    for (uint32_t i = 0; i < pathfinder->num_paths; i++) {
        if (pathfinder->paths[i].handle == handle) {
            pathfinder->paths[i].waypoint_count = 0;
            pathfinder->paths[i].handle = 0;
            pathfinder->paths[i].status = PATHFINDING_STATUS_FAILED;
            break;
        }
    }
}

Pathfinding_Status Pathfinder_GetPathStatus(Pathfinder* pathfinder, PathHandle handle) {
    if (!pathfinder) return PATHFINDING_STATUS_FAILED;
    
    for (uint32_t i = 0; i < pathfinder->num_paths; i++) {
        if (pathfinder->paths[i].handle == handle) {
            return pathfinder->paths[i].status;
        }
    }
    
    return PATHFINDING_STATUS_FAILED;
}

uint32_t Pathfinder_GetPathWaypointCount(Pathfinder* pathfinder, PathHandle handle) {
    if (!pathfinder) return 0;
    
    for (uint32_t i = 0; i < pathfinder->num_paths; i++) {
        if (pathfinder->paths[i].handle == handle) {
            return pathfinder->paths[i].waypoint_count;
        }
    }
    
    return 0;
}

Waypoint* Pathfinder_GetPathWaypoints(Pathfinder* pathfinder, PathHandle handle) {
    if (!pathfinder) return NULL;
    
    for (uint32_t i = 0; i < pathfinder->num_paths; i++) {
        if (pathfinder->paths[i].handle == handle) {
            return pathfinder->paths[i].waypoints;
        }
    }
    
    return NULL;
}

float Pathfinder_GetPathLength(Pathfinder* pathfinder, PathHandle handle) {
    if (!pathfinder) return 0.0f;
    
    for (uint32_t i = 0; i < pathfinder->num_paths; i++) {
        if (pathfinder->paths[i].handle == handle) {
            return pathfinder->paths[i].length;
        }
    }
    
    return 0.0f;
}

Waypoint Pathfinder_GetNextWaypoint(Pathfinder* pathfinder, PathHandle handle) {
    Waypoint wp = {0};
    
    if (!pathfinder) return wp;
    
    for (uint32_t i = 0; i < pathfinder->num_paths; i++) {
        if (pathfinder->paths[i].handle == handle) {
            Path* path = &pathfinder->paths[i];
            if (path->current_waypoint < path->waypoint_count) {
                return path->waypoints[path->current_waypoint];
            }
        }
    }
    
    return wp;
}

void Pathfinder_AddMovingUnit(Pathfinder* pathfinder,
                              uint32_t unit_id,
                              float x, float y,
                              float max_speed,
                              float acceleration,
                              float radius) {
    if (!pathfinder || pathfinder->num_units >= pathfinder->max_units) return;
    
    Unit* unit = &pathfinder->units[pathfinder->num_units];
    unit->unit_id = unit_id;
    unit->x = x;
    unit->y = y;
    unit->vx = 0.0f;
    unit->vy = 0.0f;
    unit->desired_speed = 0.0f;
    unit->max_speed = max_speed;
    unit->acceleration = acceleration;
    unit->radius = radius;
    unit->state = MOVEMENT_STATE_IDLE;
    unit->current_path = 0;
    
    pathfinder->num_units++;
}

void Pathfinder_RemoveMovingUnit(Pathfinder* pathfinder, uint32_t unit_id) {
    if (!pathfinder) return;
    
    for (uint32_t i = 0; i < pathfinder->num_units; i++) {
        if (pathfinder->units[i].unit_id == unit_id) {
            if (i < pathfinder->num_units - 1) {
                memmove(&pathfinder->units[i],
                       &pathfinder->units[i + 1],
                       (pathfinder->num_units - i - 1) * sizeof(Unit));
            }
            pathfinder->num_units--;
            break;
        }
    }
}

void Pathfinder_SetUnitTarget(Pathfinder* pathfinder, uint32_t unit_id, float target_x, float target_y) {
    if (!pathfinder) return;
    
    for (uint32_t i = 0; i < pathfinder->num_units; i++) {
        if (pathfinder->units[i].unit_id == unit_id) {
            pathfinder->units[i].state = MOVEMENT_STATE_MOVING;
            break;
        }
    }
}

void Pathfinder_SetUnitSpeed(Pathfinder* pathfinder, uint32_t unit_id, float speed) {
    if (!pathfinder) return;
    
    for (uint32_t i = 0; i < pathfinder->num_units; i++) {
        if (pathfinder->units[i].unit_id == unit_id) {
            pathfinder->units[i].desired_speed = speed;
            break;
        }
    }
}

void Pathfinder_SetUnitMaxSpeed(Pathfinder* pathfinder, uint32_t unit_id, float max_speed) {
    if (!pathfinder) return;
    
    for (uint32_t i = 0; i < pathfinder->num_units; i++) {
        if (pathfinder->units[i].unit_id == unit_id) {
            pathfinder->units[i].max_speed = max_speed;
            break;
        }
    }
}

void Pathfinder_SetUnitAcceleration(Pathfinder* pathfinder, uint32_t unit_id, float acceleration) {
    if (!pathfinder) return;
    
    for (uint32_t i = 0; i < pathfinder->num_units; i++) {
        if (pathfinder->units[i].unit_id == unit_id) {
            pathfinder->units[i].acceleration = acceleration;
            break;
        }
    }
}

Movement_State Pathfinder_GetUnitMovementState(Pathfinder* pathfinder, uint32_t unit_id) {
    if (!pathfinder) return MOVEMENT_STATE_IDLE;
    
    for (uint32_t i = 0; i < pathfinder->num_units; i++) {
        if (pathfinder->units[i].unit_id == unit_id) {
            return pathfinder->units[i].state;
        }
    }
    
    return MOVEMENT_STATE_IDLE;
}

void Pathfinder_GetUnitPosition(Pathfinder* pathfinder, uint32_t unit_id, float* out_x, float* out_y) {
    if (!pathfinder || !out_x || !out_y) return;
    
    for (uint32_t i = 0; i < pathfinder->num_units; i++) {
        if (pathfinder->units[i].unit_id == unit_id) {
            *out_x = pathfinder->units[i].x;
            *out_y = pathfinder->units[i].y;
            return;
        }
    }
}

void Pathfinder_GetUnitVelocity(Pathfinder* pathfinder, uint32_t unit_id, float* out_vx, float* out_vy) {
    if (!pathfinder || !out_vx || !out_vy) return;
    
    for (uint32_t i = 0; i < pathfinder->num_units; i++) {
        if (pathfinder->units[i].unit_id == unit_id) {
            *out_vx = pathfinder->units[i].vx;
            *out_vy = pathfinder->units[i].vy;
            return;
        }
    }
}

float Pathfinder_GetUnitSpeed(Pathfinder* pathfinder, uint32_t unit_id) {
    if (!pathfinder) return 0.0f;
    
    for (uint32_t i = 0; i < pathfinder->num_units; i++) {
        if (pathfinder->units[i].unit_id == unit_id) {
            return sqrtf(pathfinder->units[i].vx * pathfinder->units[i].vx +
                        pathfinder->units[i].vy * pathfinder->units[i].vy);
        }
    }
    
    return 0.0f;
}

int Pathfinder_CheckCollision(Pathfinder* pathfinder,
                              float x1, float y1, float radius1,
                              float x2, float y2, float radius2) {
    if (!pathfinder) return 0;
    
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dist_sq = dx * dx + dy * dy;
    float min_dist = radius1 + radius2;
    
    return dist_sq < min_dist * min_dist;
}

void Pathfinder_AvoidCollision(Pathfinder* pathfinder,
                               uint32_t unit_id,
                               float* out_desired_vx, float* out_desired_vy) {
    if (!pathfinder || !out_desired_vx || !out_desired_vy) return;
    
    *out_desired_vx = 0.0f;
    *out_desired_vy = 0.0f;
}

void Pathfinder_SetCollisionRadius(Pathfinder* pathfinder, uint32_t unit_id, float radius) {
    if (!pathfinder) return;
    
    for (uint32_t i = 0; i < pathfinder->num_units; i++) {
        if (pathfinder->units[i].unit_id == unit_id) {
            pathfinder->units[i].radius = radius;
            break;
        }
    }
}

void Pathfinder_CreateFormation(Pathfinder* pathfinder,
                                Formation_Type type,
                                uint32_t* unit_ids,
                                uint32_t unit_count) {
    if (!pathfinder) return;
    
    pathfinder->current_formation = type;
}

void Pathfinder_MoveFormation(Pathfinder* pathfinder,
                              uint32_t leader_unit_id,
                              float target_x, float target_y) {
    if (!pathfinder) return;
    
    /* Move all units in formation */
}

void Pathfinder_SetFormationSpacing(Pathfinder* pathfinder,
                                    Formation_Type type,
                                    float spacing) {
    if (!pathfinder) return;
    
    pathfinder->formation_spacing = spacing;
}

void Pathfinder_DisbandFormation(Pathfinder* pathfinder) {
    if (!pathfinder) return;
    
    pathfinder->current_formation = FORMATION_NONE;
}

void Pathfinder_ApplySteering(Pathfinder* pathfinder,
                              uint32_t unit_id,
                              float steer_x, float steer_y,
                              float steer_magnitude) {
    if (!pathfinder) return;
}

void Pathfinder_ApplySeekBehavior(Pathfinder* pathfinder,
                                  uint32_t unit_id,
                                  float target_x, float target_y,
                                  float max_force) {
    if (!pathfinder) return;
}

void Pathfinder_ApplyAvoidanceBehavior(Pathfinder* pathfinder,
                                       uint32_t unit_id,
                                       float* obstacle_x,
                                       float* obstacle_y,
                                       uint32_t obstacle_count,
                                       float max_force) {
    if (!pathfinder) return;
}

int Pathfinder_IsPathObstructed(Pathfinder* pathfinder,
                                float start_x, float start_y,
                                float end_x, float end_y,
                                float unit_radius) {
    if (!pathfinder) return 1;
    
    return 0;
}

int Pathfinder_LineOfSight(Pathfinder* pathfinder,
                           float x1, float y1,
                           float x2, float y2,
                           float unit_radius) {
    if (!pathfinder) return 0;
    
    return !Pathfinder_IsPathObstructed(pathfinder, x1, y1, x2, y2, unit_radius);
}

void Pathfinder_GetNearbyUnits(Pathfinder* pathfinder,
                               uint32_t unit_id,
                               float search_radius,
                               uint32_t* out_nearby_ids,
                               uint32_t* out_count,
                               uint32_t max_results) {
    if (!pathfinder || !out_count) return;
    
    *out_count = 0;
}

void Pathfinder_SmoothPath(Pathfinder* pathfinder, PathHandle handle) {
    if (!pathfinder) return;
}

void Pathfinder_SimplifyPath(Pathfinder* pathfinder, PathHandle handle, float max_deviation) {
    if (!pathfinder) return;
}

void Pathfinder_BuildNavigationMesh(Pathfinder* pathfinder,
                                    GridCellData* grid_data,
                                    uint32_t width, uint32_t height) {
    if (!pathfinder || !grid_data) return;
}

void Pathfinder_UpdateRegions(Pathfinder* pathfinder) {
    if (!pathfinder) return;
}

void Pathfinder_Update(Pathfinder* pathfinder, float delta_time) {
    if (!pathfinder) return;
    
    Pathfinder_UpdateMovement(pathfinder, delta_time);
}

void Pathfinder_UpdateMovement(Pathfinder* pathfinder, float delta_time) {
    if (!pathfinder || delta_time <= 0.0f) return;
    
    for (uint32_t i = 0; i < pathfinder->num_units; i++) {
        Unit* unit = &pathfinder->units[i];
        
        if (unit->state == MOVEMENT_STATE_MOVING) {
            /* Accelerate towards desired speed */
            if (unit->desired_speed > 0.0f) {
                float current_speed = sqrtf(unit->vx * unit->vx + unit->vy * unit->vy);
                if (current_speed < unit->desired_speed) {
                    /* Accelerate */
                }
            }
        }
    }
}

const char* Pathfinder_GetLastError(Pathfinder* pathfinder) {
    if (!pathfinder) return "Invalid pathfinder";
    return pathfinder->last_error;
}

int Pathfinder_ClearErrors(Pathfinder* pathfinder) {
    if (!pathfinder) return 0;
    
    int count = pathfinder->error_count;
    pathfinder->error_count = 0;
    memset(pathfinder->last_error, 0, sizeof(pathfinder->last_error));
    
    return count;
}

void Pathfinder_PrintGridInfo(Pathfinder* pathfinder) {
    if (!pathfinder) return;
    
    printf("Pathfinding Grid Info:\n");
    printf("Dimensions: %ux%u\n", pathfinder->grid_width, pathfinder->grid_height);
    printf("Cell Size: %.1f\n", pathfinder->cell_size);
    
    uint32_t walkable_count = 0;
    for (uint32_t i = 0; i < pathfinder->grid_width * pathfinder->grid_height; i++) {
        if (pathfinder->grid[i].walkable) walkable_count++;
    }
    printf("Walkable Cells: %u / %u\n", walkable_count, 
            pathfinder->grid_width * pathfinder->grid_height);
}

void Pathfinder_PrintPathInfo(Pathfinder* pathfinder, PathHandle handle) {
    if (!pathfinder) return;
    
    for (uint32_t i = 0; i < pathfinder->num_paths; i++) {
        if (pathfinder->paths[i].handle == handle) {
            Path* path = &pathfinder->paths[i];
            printf("Path Info:\n");
            printf("Handle: %u\n", handle);
            printf("Status: %d\n", path->status);
            printf("Waypoints: %u\n", path->waypoint_count);
            printf("Length: %.1f\n", path->length);
            return;
        }
    }
}

void Pathfinder_PrintUnitInfo(Pathfinder* pathfinder, uint32_t unit_id) {
    if (!pathfinder) return;
    
    for (uint32_t i = 0; i < pathfinder->num_units; i++) {
        if (pathfinder->units[i].unit_id == unit_id) {
            Unit* unit = &pathfinder->units[i];
            printf("Unit Info:\n");
            printf("ID: %u\n", unit_id);
            printf("Position: (%.1f, %.1f)\n", unit->x, unit->y);
            printf("Velocity: (%.1f, %.1f)\n", unit->vx, unit->vy);
            printf("Speed: %.1f / %.1f\n", Pathfinder_GetUnitSpeed(pathfinder, unit_id), unit->max_speed);
            printf("State: %d\n", unit->state);
            return;
        }
    }
}

uint32_t Pathfinder_GetStats(Pathfinder* pathfinder) {
    if (!pathfinder) return 0;
    return pathfinder->num_units;
}

float Pathfinder_GetPathComputeTime(Pathfinder* pathfinder, PathHandle handle) {
    if (!pathfinder) return 0.0f;
    return 0.0f;
}

uint32_t Pathfinder_GetActivePathCount(Pathfinder* pathfinder) {
    if (!pathfinder) return 0;
    
    uint32_t count = 0;
    for (uint32_t i = 0; i < pathfinder->num_paths; i++) {
        if (pathfinder->paths[i].handle != 0) count++;
    }
    return count;
}

uint32_t Pathfinder_GetMovingUnitCount(Pathfinder* pathfinder) {
    if (!pathfinder) return 0;
    
    uint32_t count = 0;
    for (uint32_t i = 0; i < pathfinder->num_units; i++) {
        if (pathfinder->units[i].state == MOVEMENT_STATE_MOVING) count++;
    }
    return count;
}
