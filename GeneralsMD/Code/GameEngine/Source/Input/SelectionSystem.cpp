#include "SelectionSystem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct {
    UnitHandle unit_handle;
    uint32_t unit_id;
    int priority;
} Selected_Unit_Internal;

typedef struct {
    int key_code;
    Input_Modifier modifier;
    Hotkey_Action action;
} Hotkey_Binding;

struct SelectionSystem {
    /* Selected units */
    Selected_Unit_Internal* selected_units;
    uint32_t selected_count;
    uint32_t max_selected;
    
    /* Command queue */
    Queued_Command* command_queue;
    uint32_t queue_size;
    uint32_t queue_head;
    uint32_t queue_max;
    
    /* Control groups */
    Control_Group* control_groups;
    uint32_t num_groups;
    
    /* Box selection state */
    Selection_Box current_box;
    int is_box_selecting;
    float drag_start_x, drag_start_y;
    float mouse_x, mouse_y;
    
    /* Hotkey bindings */
    Hotkey_Binding* hotkeys;
    uint32_t num_hotkeys;
    uint32_t max_hotkeys;
    
    /* Selection filters */
    uint32_t include_mask;
    uint32_t exclude_mask;
    
    /* Primary selection */
    UnitHandle primary_unit;
    
    /* Viewport info */
    float viewport_width, viewport_height;
    float viewport_offset_x, viewport_offset_y;
    
    /* Error tracking */
    char last_error[256];
    int error_count;
};

static SelectionHandle selection_allocate_handle(void) {
    return SELECTION_HANDLE_MIN + (rand() % (SELECTION_HANDLE_MAX - SELECTION_HANDLE_MIN));
}

static int selection_box_contains_point(Selection_Box* box, float x, float y) {
    float min_x = (box->x1 < box->x2) ? box->x1 : box->x2;
    float max_x = (box->x1 > box->x2) ? box->x1 : box->x2;
    float min_y = (box->y1 < box->y2) ? box->y1 : box->y2;
    float max_y = (box->y1 > box->y2) ? box->y1 : box->y2;
    
    return (x >= min_x && x <= max_x && y >= min_y && y <= max_y);
}

static float selection_distance_squared(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

SelectionSystem* SelectionSystem_Create(void) {
    SelectionSystem* system = (SelectionSystem*)malloc(sizeof(SelectionSystem));
    if (!system) return NULL;
    
    memset(system, 0, sizeof(SelectionSystem));
    
    system->max_selected = SELECTION_MAX_SELECTED;
    system->queue_max = SELECTION_MAX_QUEUED_COMMANDS;
    system->num_groups = SELECTION_MAX_GROUPS;
    system->max_hotkeys = 32;
    
    system->selected_units = (Selected_Unit_Internal*)calloc(system->max_selected, sizeof(Selected_Unit_Internal));
    system->command_queue = (Queued_Command*)calloc(system->queue_max, sizeof(Queued_Command));
    system->control_groups = (Control_Group*)calloc(system->num_groups, sizeof(Control_Group));
    system->hotkeys = (Hotkey_Binding*)calloc(system->max_hotkeys, sizeof(Hotkey_Binding));
    
    if (!system->selected_units || !system->command_queue || !system->control_groups || !system->hotkeys) {
        free(system->selected_units);
        free(system->command_queue);
        free(system->control_groups);
        free(system->hotkeys);
        free(system);
        return NULL;
    }
    
    /* Initialize control groups */
    for (uint32_t i = 0; i < system->num_groups; i++) {
        system->control_groups[i].group_id = i;
        system->control_groups[i].max_units = SELECTION_GROUP_SIZE;
        system->control_groups[i].unit_handles = (UnitHandle*)calloc(SELECTION_GROUP_SIZE, sizeof(UnitHandle));
    }
    
    system->include_mask = 0xFFFFFFFF;
    system->exclude_mask = 0;
    system->viewport_width = 1920.0f;
    system->viewport_height = 1080.0f;
    
    return system;
}

void SelectionSystem_Destroy(SelectionSystem* system) {
    if (!system) return;
    
    for (uint32_t i = 0; i < system->num_groups; i++) {
        free(system->control_groups[i].unit_handles);
    }
    
    free(system->selected_units);
    free(system->command_queue);
    free(system->control_groups);
    free(system->hotkeys);
    free(system);
}

void SelectionSystem_Initialize(SelectionSystem* system) {
    if (!system) return;
    
    system->selected_count = 0;
    system->queue_size = 0;
    system->queue_head = 0;
    system->is_box_selecting = 0;
    system->num_hotkeys = 0;
    
    printf("Phase 33: Selection system initialized\n");
}

void SelectionSystem_Shutdown(SelectionSystem* system) {
    if (!system) return;
    
    SelectionSystem_DeselectAll(system);
    SelectionSystem_ClearCommandQueue(system);
    
    printf("Phase 33: Selection system shutdown\n");
}

SelectionHandle SelectionSystem_SelectUnit(SelectionSystem* system,
                                           UnitHandle unit_handle,
                                           Selection_Operation operation) {
    if (!system) return 0;
    
    switch (operation) {
        case SELECTION_OP_REPLACE: {
            system->selected_count = 0;
            if (system->selected_count < system->max_selected) {
                system->selected_units[system->selected_count].unit_handle = unit_handle;
                system->selected_units[system->selected_count].unit_id = unit_handle;
                system->selected_units[system->selected_count].priority = 0;
                system->selected_count++;
                system->primary_unit = unit_handle;
            }
            break;
        }
        
        case SELECTION_OP_ADD: {
            if (!SelectionSystem_IsUnitSelected(system, unit_handle)) {
                if (system->selected_count < system->max_selected) {
                    system->selected_units[system->selected_count].unit_handle = unit_handle;
                    system->selected_units[system->selected_count].unit_id = unit_handle;
                    system->selected_units[system->selected_count].priority = system->selected_count;
                    system->selected_count++;
                    if (system->primary_unit == 0) {
                        system->primary_unit = unit_handle;
                    }
                }
            }
            break;
        }
        
        case SELECTION_OP_REMOVE: {
            for (uint32_t i = 0; i < system->selected_count; i++) {
                if (system->selected_units[i].unit_handle == unit_handle) {
                    if (i < system->selected_count - 1) {
                        memmove(&system->selected_units[i], 
                               &system->selected_units[i + 1],
                               (system->selected_count - i - 1) * sizeof(Selected_Unit_Internal));
                    }
                    system->selected_count--;
                    
                    if (system->primary_unit == unit_handle && system->selected_count > 0) {
                        system->primary_unit = system->selected_units[0].unit_handle;
                    }
                    break;
                }
            }
            break;
        }
        
        case SELECTION_OP_TOGGLE: {
            if (SelectionSystem_IsUnitSelected(system, unit_handle)) {
                SelectionSystem_DeselectUnit(system, unit_handle);
            } else {
                SelectionSystem_SelectUnit(system, unit_handle, SELECTION_OP_ADD);
            }
            break;
        }
    }
    
    return selection_allocate_handle();
}

SelectionHandle SelectionSystem_SelectMultiple(SelectionSystem* system,
                                               UnitHandle* unit_handles,
                                               uint32_t count,
                                               Selection_Operation operation) {
    if (!system || !unit_handles) return 0;
    
    if (operation == SELECTION_OP_REPLACE) {
        system->selected_count = 0;
    }
    
    for (uint32_t i = 0; i < count; i++) {
        SelectionSystem_SelectUnit(system, unit_handles[i], 
                                 (i == 0 && operation == SELECTION_OP_REPLACE) ? 
                                 SELECTION_OP_REPLACE : SELECTION_OP_ADD);
    }
    
    return selection_allocate_handle();
}

SelectionHandle SelectionSystem_SelectInBox(SelectionSystem* system,
                                            Selection_Box box,
                                            Selection_Operation operation) {
    if (!system) return 0;
    
    if (operation == SELECTION_OP_REPLACE) {
        system->selected_count = 0;
    }
    
    /* In real implementation, would query GameWorld for units in box */
    /* For now, just record the box */
    system->current_box = box;
    
    return selection_allocate_handle();
}

SelectionHandle SelectionSystem_SelectByType(SelectionSystem* system,
                                             uint32_t unit_type,
                                             Selection_Operation operation) {
    if (!system) return 0;
    
    /* In real implementation, would query GameWorld for units by type */
    
    return selection_allocate_handle();
}

void SelectionSystem_DeselectAll(SelectionSystem* system) {
    if (!system) return;
    
    system->selected_count = 0;
    system->primary_unit = 0;
}

void SelectionSystem_DeselectUnit(SelectionSystem* system, UnitHandle unit_handle) {
    if (!system) return;
    
    SelectionSystem_SelectUnit(system, unit_handle, SELECTION_OP_REMOVE);
}

uint32_t SelectionSystem_GetSelectedCount(SelectionSystem* system) {
    if (!system) return 0;
    return system->selected_count;
}

UnitHandle SelectionSystem_GetSelectedUnit(SelectionSystem* system, uint32_t index) {
    if (!system || index >= system->selected_count) return 0;
    return system->selected_units[index].unit_handle;
}

int SelectionSystem_IsUnitSelected(SelectionSystem* system, UnitHandle unit_handle) {
    if (!system) return 0;
    
    for (uint32_t i = 0; i < system->selected_count; i++) {
        if (system->selected_units[i].unit_handle == unit_handle) {
            return 1;
        }
    }
    
    return 0;
}

Selected_Unit* SelectionSystem_GetAllSelected(SelectionSystem* system) {
    if (!system) return NULL;
    return (Selected_Unit*)system->selected_units;
}

void SelectionSystem_BeginBoxSelect(SelectionSystem* system, float start_x, float start_y) {
    if (!system) return;
    
    system->is_box_selecting = 1;
    system->drag_start_x = start_x;
    system->drag_start_y = start_y;
    system->mouse_x = start_x;
    system->mouse_y = start_y;
    
    system->current_box.x1 = start_x;
    system->current_box.y1 = start_y;
    system->current_box.x2 = start_x;
    system->current_box.y2 = start_y;
}

void SelectionSystem_UpdateBoxSelect(SelectionSystem* system, float current_x, float current_y) {
    if (!system || !system->is_box_selecting) return;
    
    system->mouse_x = current_x;
    system->mouse_y = current_y;
    
    system->current_box.x2 = current_x;
    system->current_box.y2 = current_y;
}

void SelectionSystem_EndBoxSelect(SelectionSystem* system, Input_Modifier modifier) {
    if (!system || !system->is_box_selecting) return;
    
    system->is_box_selecting = 0;
    
    float distance = sqrtf(selection_distance_squared(
        system->drag_start_x, system->drag_start_y,
        system->mouse_x, system->mouse_y));
    
    if (distance > SELECTION_DRAG_THRESHOLD) {
        Selection_Operation op = (modifier & MODIFIER_SHIFT) ? SELECTION_OP_ADD : SELECTION_OP_REPLACE;
        SelectionSystem_SelectInBox(system, system->current_box, op);
    }
}

Selection_Box SelectionSystem_GetBoxSelection(SelectionSystem* system) {
    if (!system) {
        return (Selection_Box){0, 0, 0, 0};
    }
    return system->current_box;
}

int SelectionSystem_IsBoxSelecting(SelectionSystem* system) {
    if (!system) return 0;
    return system->is_box_selecting;
}

void SelectionSystem_HandleMouseDown(SelectionSystem* system,
                                     float mouse_x,
                                     float mouse_y,
                                     int button,
                                     Input_Modifier modifier) {
    if (!system) return;
    
    if (button == 0) {  /* Left mouse button */
        SelectionSystem_BeginBoxSelect(system, mouse_x, mouse_y);
    }
}

void SelectionSystem_HandleMouseUp(SelectionSystem* system,
                                   float mouse_x,
                                   float mouse_y,
                                   int button,
                                   Input_Modifier modifier) {
    if (!system) return;
    
    if (button == 0) {  /* Left mouse button */
        SelectionSystem_EndBoxSelect(system, modifier);
    }
}

void SelectionSystem_HandleMouseMove(SelectionSystem* system,
                                     float mouse_x,
                                     float mouse_y,
                                     Input_Modifier modifier) {
    if (!system) return;
    
    if (system->is_box_selecting) {
        SelectionSystem_UpdateBoxSelect(system, mouse_x, mouse_y);
    }
}

void SelectionSystem_QueueCommand(SelectionSystem* system, Queued_Command command) {
    if (!system || system->queue_size >= system->queue_max) return;
    
    uint32_t index = (system->queue_head + system->queue_size) % system->queue_max;
    system->command_queue[index] = command;
    system->queue_size++;
}

Queued_Command SelectionSystem_DequeueCommand(SelectionSystem* system) {
    Queued_Command cmd;
    memset(&cmd, 0, sizeof(Queued_Command));
    
    if (!system || system->queue_size == 0) return cmd;
    
    cmd = system->command_queue[system->queue_head];
    system->queue_head = (system->queue_head + 1) % system->queue_max;
    system->queue_size--;
    
    return cmd;
}

uint32_t SelectionSystem_GetCommandQueueSize(SelectionSystem* system) {
    if (!system) return 0;
    return system->queue_size;
}

void SelectionSystem_ClearCommandQueue(SelectionSystem* system) {
    if (!system) return;
    
    system->queue_size = 0;
    system->queue_head = 0;
}

Queued_Command* SelectionSystem_GetAllQueuedCommands(SelectionSystem* system) {
    if (!system) return NULL;
    return system->command_queue;
}

void SelectionSystem_IssueCommand(SelectionSystem* system, Queued_Command command) {
    if (!system) return;
    
    /* In real implementation, would dispatch command to selected units */
    SelectionSystem_QueueCommand(system, command);
}

void SelectionSystem_CreateControlGroup(SelectionSystem* system, uint32_t group_id) {
    if (!system || group_id >= system->num_groups) return;
    
    system->control_groups[group_id].group_id = group_id;
    system->control_groups[group_id].unit_count = 0;
}

void SelectionSystem_AssignToControlGroup(SelectionSystem* system,
                                          uint32_t group_id,
                                          UnitHandle* unit_handles,
                                          uint32_t count) {
    if (!system || group_id >= system->num_groups) return;
    
    Control_Group* group = &system->control_groups[group_id];
    group->unit_count = 0;
    
    for (uint32_t i = 0; i < count && group->unit_count < group->max_units; i++) {
        group->unit_handles[group->unit_count++] = unit_handles[i];
    }
}

void SelectionSystem_SelectControlGroup(SelectionSystem* system,
                                        uint32_t group_id,
                                        Input_Modifier modifier) {
    if (!system || group_id >= system->num_groups) return;
    
    Control_Group* group = &system->control_groups[group_id];
    
    Selection_Operation op = (modifier & MODIFIER_SHIFT) ? SELECTION_OP_ADD : SELECTION_OP_REPLACE;
    
    SelectionSystem_SelectMultiple(system, group->unit_handles, group->unit_count, op);
}

void SelectionSystem_AddToControlGroup(SelectionSystem* system, uint32_t group_id) {
    if (!system || group_id >= system->num_groups) return;
    
    Control_Group* group = &system->control_groups[group_id];
    
    for (uint32_t i = 0; i < system->selected_count && group->unit_count < group->max_units; i++) {
        group->unit_handles[group->unit_count++] = system->selected_units[i].unit_handle;
    }
}

UnitHandle* SelectionSystem_GetControlGroupMembers(SelectionSystem* system,
                                                   uint32_t group_id,
                                                   uint32_t* out_count) {
    if (!system || group_id >= system->num_groups || !out_count) return NULL;
    
    *out_count = system->control_groups[group_id].unit_count;
    return system->control_groups[group_id].unit_handles;
}

void SelectionSystem_ClearControlGroup(SelectionSystem* system, uint32_t group_id) {
    if (!system || group_id >= system->num_groups) return;
    
    system->control_groups[group_id].unit_count = 0;
}

void SelectionSystem_RegisterHotkey(SelectionSystem* system,
                                    int key_code,
                                    Input_Modifier modifier,
                                    Hotkey_Action action) {
    if (!system || system->num_hotkeys >= system->max_hotkeys) return;
    
    system->hotkeys[system->num_hotkeys].key_code = key_code;
    system->hotkeys[system->num_hotkeys].modifier = modifier;
    system->hotkeys[system->num_hotkeys].action = action;
    system->num_hotkeys++;
}

void SelectionSystem_ProcessHotkey(SelectionSystem* system,
                                   int key_code,
                                   Input_Modifier modifier) {
    if (!system) return;
    
    for (uint32_t i = 0; i < system->num_hotkeys; i++) {
        if (system->hotkeys[i].key_code == key_code &&
            system->hotkeys[i].modifier == modifier) {
            
            switch (system->hotkeys[i].action) {
                case HOTKEY_SELECT_ALL:
                    /* Select all units */
                    break;
                case HOTKEY_SELECT_IDLE:
                    /* Select idle units */
                    break;
                case HOTKEY_DESELECT_ALL:
                    SelectionSystem_DeselectAll(system);
                    break;
                default:
                    break;
            }
        }
    }
}

void SelectionSystem_SetSelectionFilter(SelectionSystem* system,
                                        uint32_t unit_type_mask,
                                        uint32_t exclude_mask) {
    if (!system) return;
    
    system->include_mask = unit_type_mask;
    system->exclude_mask = exclude_mask;
}

void SelectionSystem_GetSelectionFilter(SelectionSystem* system,
                                        uint32_t* out_include_mask,
                                        uint32_t* out_exclude_mask) {
    if (!system || !out_include_mask || !out_exclude_mask) return;
    
    *out_include_mask = system->include_mask;
    *out_exclude_mask = system->exclude_mask;
}

int SelectionSystem_IsPrimarySelection(SelectionSystem* system, UnitHandle unit_handle) {
    if (!system) return 0;
    return (system->primary_unit == unit_handle);
}

UnitHandle SelectionSystem_GetPrimaryUnit(SelectionSystem* system) {
    if (!system) return 0;
    return system->primary_unit;
}

void SelectionSystem_SetPrimaryUnit(SelectionSystem* system, UnitHandle unit_handle) {
    if (!system) return;
    
    system->primary_unit = unit_handle;
}

void SelectionSystem_SetViewportSize(SelectionSystem* system, float width, float height) {
    if (!system) return;
    
    system->viewport_width = width;
    system->viewport_height = height;
}

void SelectionSystem_SetViewportOffset(SelectionSystem* system, float offset_x, float offset_y) {
    if (!system) return;
    
    system->viewport_offset_x = offset_x;
    system->viewport_offset_y = offset_y;
}

void SelectionSystem_GetViewportSize(SelectionSystem* system, float* out_width, float* out_height) {
    if (!system || !out_width || !out_height) return;
    
    *out_width = system->viewport_width;
    *out_height = system->viewport_height;
}

void SelectionSystem_Update(SelectionSystem* system, float delta_time) {
    if (!system) return;
    
    /* Update selection timers or other state */
}

const char* SelectionSystem_GetLastError(SelectionSystem* system) {
    if (!system) return "Invalid system";
    return system->last_error;
}

int SelectionSystem_ClearErrors(SelectionSystem* system) {
    if (!system) return 0;
    
    int count = system->error_count;
    system->error_count = 0;
    memset(system->last_error, 0, sizeof(system->last_error));
    
    return count;
}

void SelectionSystem_PrintSelectionInfo(SelectionSystem* system) {
    if (!system) return;
    
    printf("Selection System Info:\n");
    printf("Selected Units: %u\n", system->selected_count);
    printf("Primary Unit: %u\n", system->primary_unit);
    printf("Command Queue Size: %u\n", system->queue_size);
    printf("Control Groups: %u\n", system->num_groups);
    
    for (uint32_t i = 0; i < system->selected_count && i < 5; i++) {
        printf("  Unit %u: Handle %u\n", i, system->selected_units[i].unit_handle);
    }
    
    if (system->selected_count > 5) {
        printf("  ... and %u more\n", system->selected_count - 5);
    }
}

void SelectionSystem_PrintCommandQueue(SelectionSystem* system) {
    if (!system) return;
    
    printf("Command Queue (%u commands):\n", system->queue_size);
    
    uint32_t idx = system->queue_head;
    for (uint32_t i = 0; i < system->queue_size && i < 10; i++) {
        Queued_Command* cmd = &system->command_queue[idx];
        printf("  [%u] Type: %d, Target: (%.1f, %.1f)\n", 
               i, cmd->type, cmd->target_x, cmd->target_y);
        idx = (idx + 1) % system->queue_max;
    }
    
    if (system->queue_size > 10) {
        printf("  ... and %u more\n", system->queue_size - 10);
    }
}

uint32_t SelectionSystem_GetStats(SelectionSystem* system) {
    if (!system) return 0;
    
    return system->selected_count;
}
