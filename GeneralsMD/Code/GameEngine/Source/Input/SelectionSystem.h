#pragma once

#include <cstdint>
#include <cstddef>

/* Phase 33: Input Handling & Selection System
 * 
 * Provides unit selection, command queuing, and control group management:
 * - Single/multi-select mechanics
 * - Box select with mouse drag
 * - Command queuing
 * - Hotkey binding
 * - Control groups (1-10)
 */

typedef struct SelectionSystem SelectionSystem;
typedef uint32_t SelectionHandle;
typedef uint32_t UnitHandle;

#define SELECTION_MAX_SELECTED 200
#define SELECTION_MAX_GROUPS 10
#define SELECTION_MAX_QUEUED_COMMANDS 50
#define SELECTION_DRAG_THRESHOLD 5
#define SELECTION_GROUP_SIZE 100

/* Selection system handle ranges (Phase 33: 33000-33999) */
#define SELECTION_HANDLE_MIN 33000
#define SELECTION_HANDLE_MAX 33999

typedef enum {
    SELECTION_MODE_NONE = 0,
    SELECTION_MODE_SINGLE = 1,
    SELECTION_MODE_MULTI = 2,
    SELECTION_MODE_BOX = 3,
    SELECTION_MODE_HOTKEY = 4,
} Selection_Mode;

typedef enum {
    SELECTION_OP_REPLACE = 0,
    SELECTION_OP_ADD = 1,
    SELECTION_OP_REMOVE = 2,
    SELECTION_OP_TOGGLE = 3,
} Selection_Operation;

typedef enum {
    COMMAND_TYPE_MOVE = 0,
    COMMAND_TYPE_ATTACK = 1,
    COMMAND_TYPE_BUILD = 2,
    COMMAND_TYPE_REPAIR = 3,
    COMMAND_TYPE_GATHER = 4,
    COMMAND_TYPE_SPECIAL = 5,
    COMMAND_TYPE_HOLD_POSITION = 6,
    COMMAND_TYPE_PATROL = 7,
} Command_Type;

typedef enum {
    MODIFIER_NONE = 0,
    MODIFIER_SHIFT = 1,
    MODIFIER_CTRL = 2,
    MODIFIER_ALT = 4,
    MODIFIER_CMD = 8,
} Input_Modifier;

typedef enum {
    HOTKEY_SELECT_ALL = 0,
    HOTKEY_SELECT_IDLE = 1,
    HOTKEY_SELECT_BUILDINGS = 2,
    HOTKEY_SELECT_UNITS = 3,
    HOTKEY_SELECT_INFANTRY = 4,
    HOTKEY_SELECT_VEHICLES = 5,
    HOTKEY_SELECT_AIRCRAFT = 6,
    HOTKEY_DESELECT_ALL = 7,
} Hotkey_Action;

typedef struct {
    float x1, y1;
    float x2, y2;
} Selection_Box;

typedef struct {
    UnitHandle unit_handle;
    uint32_t unit_id;
    int priority;
} Selected_Unit;

typedef struct {
    Command_Type type;
    float target_x, target_y;
    uint32_t target_unit;
    uint32_t target_building;
    int shift_queue;
} Queued_Command;

typedef struct {
    uint32_t group_id;
    UnitHandle* unit_handles;
    uint32_t unit_count;
    uint32_t max_units;
} Control_Group;

/* System Management */
SelectionSystem* SelectionSystem_Create(void);
void SelectionSystem_Destroy(SelectionSystem* system);
void SelectionSystem_Initialize(SelectionSystem* system);
void SelectionSystem_Shutdown(SelectionSystem* system);

/* Selection Operations */
SelectionHandle SelectionSystem_SelectUnit(SelectionSystem* system,
                                           UnitHandle unit_handle,
                                           Selection_Operation operation);
SelectionHandle SelectionSystem_SelectMultiple(SelectionSystem* system,
                                               UnitHandle* unit_handles,
                                               uint32_t count,
                                               Selection_Operation operation);
SelectionHandle SelectionSystem_SelectInBox(SelectionSystem* system,
                                            Selection_Box box,
                                            Selection_Operation operation);
SelectionHandle SelectionSystem_SelectByType(SelectionSystem* system,
                                             uint32_t unit_type,
                                             Selection_Operation operation);
void SelectionSystem_DeselectAll(SelectionSystem* system);
void SelectionSystem_DeselectUnit(SelectionSystem* system, UnitHandle unit_handle);

/* Selection Query */
uint32_t SelectionSystem_GetSelectedCount(SelectionSystem* system);
UnitHandle SelectionSystem_GetSelectedUnit(SelectionSystem* system, uint32_t index);
int SelectionSystem_IsUnitSelected(SelectionSystem* system, UnitHandle unit_handle);
Selected_Unit* SelectionSystem_GetAllSelected(SelectionSystem* system);

/* Box Selection */
void SelectionSystem_BeginBoxSelect(SelectionSystem* system, float start_x, float start_y);
void SelectionSystem_UpdateBoxSelect(SelectionSystem* system, float current_x, float current_y);
void SelectionSystem_EndBoxSelect(SelectionSystem* system, Input_Modifier modifier);
Selection_Box SelectionSystem_GetBoxSelection(SelectionSystem* system);
int SelectionSystem_IsBoxSelecting(SelectionSystem* system);

/* Mouse Input */
void SelectionSystem_HandleMouseDown(SelectionSystem* system,
                                     float mouse_x,
                                     float mouse_y,
                                     int button,
                                     Input_Modifier modifier);
void SelectionSystem_HandleMouseUp(SelectionSystem* system,
                                   float mouse_x,
                                   float mouse_y,
                                   int button,
                                   Input_Modifier modifier);
void SelectionSystem_HandleMouseMove(SelectionSystem* system,
                                     float mouse_x,
                                     float mouse_y,
                                     Input_Modifier modifier);

/* Command Queuing */
void SelectionSystem_QueueCommand(SelectionSystem* system, Queued_Command command);
Queued_Command SelectionSystem_DequeueCommand(SelectionSystem* system);
uint32_t SelectionSystem_GetCommandQueueSize(SelectionSystem* system);
void SelectionSystem_ClearCommandQueue(SelectionSystem* system);
Queued_Command* SelectionSystem_GetAllQueuedCommands(SelectionSystem* system);

/* Direct Commands (Immediate Execution) */
void SelectionSystem_IssueCommand(SelectionSystem* system, Queued_Command command);

/* Control Groups */
void SelectionSystem_CreateControlGroup(SelectionSystem* system, uint32_t group_id);
void SelectionSystem_AssignToControlGroup(SelectionSystem* system,
                                          uint32_t group_id,
                                          UnitHandle* unit_handles,
                                          uint32_t count);
void SelectionSystem_SelectControlGroup(SelectionSystem* system,
                                        uint32_t group_id,
                                        Input_Modifier modifier);
void SelectionSystem_AddToControlGroup(SelectionSystem* system,
                                       uint32_t group_id);
UnitHandle* SelectionSystem_GetControlGroupMembers(SelectionSystem* system,
                                                   uint32_t group_id,
                                                   uint32_t* out_count);
void SelectionSystem_ClearControlGroup(SelectionSystem* system, uint32_t group_id);

/* Hotkey Bindings */
void SelectionSystem_RegisterHotkey(SelectionSystem* system,
                                    int key_code,
                                    Input_Modifier modifier,
                                    Hotkey_Action action);
void SelectionSystem_ProcessHotkey(SelectionSystem* system,
                                   int key_code,
                                   Input_Modifier modifier);

/* Selection Filters */
void SelectionSystem_SetSelectionFilter(SelectionSystem* system,
                                        uint32_t unit_type_mask,
                                        uint32_t exclude_mask);
void SelectionSystem_GetSelectionFilter(SelectionSystem* system,
                                        uint32_t* out_include_mask,
                                        uint32_t* out_exclude_mask);

/* Priority & Display */
int SelectionSystem_IsPrimarySelection(SelectionSystem* system, UnitHandle unit_handle);
UnitHandle SelectionSystem_GetPrimaryUnit(SelectionSystem* system);
void SelectionSystem_SetPrimaryUnit(SelectionSystem* system, UnitHandle unit_handle);

/* Viewport & Screen Coordinates */
void SelectionSystem_SetViewportSize(SelectionSystem* system, float width, float height);
void SelectionSystem_SetViewportOffset(SelectionSystem* system, float offset_x, float offset_y);
void SelectionSystem_GetViewportSize(SelectionSystem* system, float* out_width, float* out_height);

/* Update */
void SelectionSystem_Update(SelectionSystem* system, float delta_time);

/* Error Handling */
const char* SelectionSystem_GetLastError(SelectionSystem* system);
int SelectionSystem_ClearErrors(SelectionSystem* system);

/* Debug */
void SelectionSystem_PrintSelectionInfo(SelectionSystem* system);
void SelectionSystem_PrintCommandQueue(SelectionSystem* system);
uint32_t SelectionSystem_GetStats(SelectionSystem* system);
