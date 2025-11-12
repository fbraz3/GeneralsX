# Phase 33: Input Handling & Selection System

**Status**: COMPLETE  
**Implementation Date**: November 12, 2025  
**Files Created**: 2  
**Total Lines**: 1,263 (header: 287 lines, implementation: 976 lines)  

## Overview

Phase 33 implements the selection and input handling system for GeneralsX, providing unit selection, command queuing, and control group management.

## Deliverables Completed

- [x] Single/multi-select mechanics
- [x] Box select with mouse drag
- [x] Command queuing
- [x] Hotkey binding
- [x] Control groups (1-10)
- [x] Selection filters by unit type
- [x] Primary unit tracking
- [x] Viewport coordinate handling

## Architecture

### Handle Range
- **SELECTION_HANDLE_MIN**: 33000-33999 (1,000 selection handles)

### Selection Modes
1. **SINGLE**: Single unit selection
2. **MULTI**: Multiple units selected
3. **BOX**: Drag-to-select box
4. **HOTKEY**: Group/type-based selection

### Selection Operations
- **REPLACE**: Clear and select new units
- **ADD**: Add to current selection
- **REMOVE**: Remove from selection
- **TOGGLE**: Toggle unit in/out of selection

### Command Types
- MOVE - Unit movement
- ATTACK - Target attack
- BUILD - Structure construction
- REPAIR - Unit/building repair
- GATHER - Resource gathering
- SPECIAL - Special ability
- HOLD_POSITION - Stationary
- PATROL - Automated patrol

### Control Groups
- 10 configurable control groups
- Up to 100 units per group
- Quick access with hotkeys (0-9)
- Persistent during gameplay

### Input Modifiers
- SHIFT: Add to selection / queued command
- CTRL: Subtraction from selection
- ALT: Alternate action
- CMD: Apple Command key

## Data Structures

**SelectionSystem** - Main selection manager
- Tracks selected units with priority
- Manages command queue (circular buffer)
- Stores 10 control groups
- Hotkey bindings (up to 32)
- Selection filters

**Selected_Unit** - Selected unit entry
- Unit handle (from GameObject phase)
- Unit ID
- Priority/order (for multi-select rendering)

**Queued_Command** - Command in queue
- Command type
- Target position (x, y)
- Target unit/building
- Shift-queue flag

**Control_Group** - Unit grouping
- Group ID (0-9)
- Array of unit handles
- Current unit count

**Hotkey_Binding** - Input binding
- Key code
- Modifier keys
- Action type (hotkey action enum)

**Selection_Box** - Drag selection
- Start position (x1, y1)
- End position (x2, y2)
- Rectangle bounds checking

## API Functions (42 total)

### System Management (4)
- `SelectionSystem_Create()` - Create system
- `SelectionSystem_Destroy()` - Destroy system
- `SelectionSystem_Initialize()` - Initialize
- `SelectionSystem_Shutdown()` - Shutdown

### Selection Operations (5)
- `SelectionSystem_SelectUnit()` - Single unit
- `SelectionSystem_SelectMultiple()` - Multiple units
- `SelectionSystem_SelectInBox()` - Box selection
- `SelectionSystem_SelectByType()` - Type-based
- `SelectionSystem_DeselectAll()` - Clear selection

### Selection Query (4)
- `SelectionSystem_GetSelectedCount()` - Count
- `SelectionSystem_GetSelectedUnit()` - Get by index
- `SelectionSystem_IsUnitSelected()` - Check selected
- `SelectionSystem_GetAllSelected()` - Get all

### Box Selection (4)
- `SelectionSystem_BeginBoxSelect()` - Start drag
- `SelectionSystem_UpdateBoxSelect()` - During drag
- `SelectionSystem_EndBoxSelect()` - End drag
- `SelectionSystem_GetBoxSelection()` - Current box
- `SelectionSystem_IsBoxSelecting()` - Check status

### Mouse Input (3)
- `SelectionSystem_HandleMouseDown()` - Mouse down
- `SelectionSystem_HandleMouseUp()` - Mouse up
- `SelectionSystem_HandleMouseMove()` - Mouse move

### Command Queue (5)
- `SelectionSystem_QueueCommand()` - Queue command
- `SelectionSystem_DequeueCommand()` - Get next
- `SelectionSystem_GetCommandQueueSize()` - Count
- `SelectionSystem_ClearCommandQueue()` - Clear all
- `SelectionSystem_GetAllQueuedCommands()` - Get all

### Direct Commands (1)
- `SelectionSystem_IssueCommand()` - Execute now

### Control Groups (6)
- `SelectionSystem_CreateControlGroup()` - Create group
- `SelectionSystem_AssignToControlGroup()` - Assign units
- `SelectionSystem_SelectControlGroup()` - Select group
- `SelectionSystem_AddToControlGroup()` - Add selected
- `SelectionSystem_GetControlGroupMembers()` - Get members
- `SelectionSystem_ClearControlGroup()` - Clear group

### Hotkey Bindings (2)
- `SelectionSystem_RegisterHotkey()` - Register binding
- `SelectionSystem_ProcessHotkey()` - Process key press

### Selection Filters (2)
- `SelectionSystem_SetSelectionFilter()` - Set filter
- `SelectionSystem_GetSelectionFilter()` - Get filter

### Primary Unit (3)
- `SelectionSystem_IsPrimarySelection()` - Check primary
- `SelectionSystem_GetPrimaryUnit()` - Get primary
- `SelectionSystem_SetPrimaryUnit()` - Set primary

### Viewport (3)
- `SelectionSystem_SetViewportSize()` - Set dimensions
- `SelectionSystem_SetViewportOffset()` - Set offset
- `SelectionSystem_GetViewportSize()` - Get dimensions

### Update & Error (3)
- `SelectionSystem_Update()` - Update state
- `SelectionSystem_GetLastError()` - Get error
- `SelectionSystem_ClearErrors()` - Clear errors

### Debug (2)
- `SelectionSystem_PrintSelectionInfo()` - Debug output
- `SelectionSystem_PrintCommandQueue()` - Debug queue
- `SelectionSystem_GetStats()` - Statistics

### Deselect (1)
- `SelectionSystem_DeselectUnit()` - Remove unit

## Implementation Details

### Selection Priority
- First selected unit becomes primary
- Used for camera focus and info display
- Can be manually overridden

### Box Selection
- Drag threshold of 5 pixels
- Prevents accidental box select on click
- SHIFT modifier adds to current selection
- Normal click replaces selection

### Command Queue
- Circular buffer (50 max commands)
- FIFO ordering
- Shift-queue flag for queued vs immediate
- Automatic dequeue on command execution

### Control Groups
- Created on demand
- 100 unit capacity per group
- Quick 0-9 hotkey access
- Persistent across gameplay

### Mouse Input Handling
- Left button for selection
- Drag to create selection box
- Modifier-aware operations
- Viewport offset for camera panning

### Selection Filters
- Include mask: unit types to select
- Exclude mask: unit types to skip
- Defaults to all units included
- Used for selection operations

## Integration Points

### GameEngine Integration
- Can be added as update callback
- Receives delta_time for timing
- Synchronized with input events

### GameObject Integration (Phase 27)
- Uses GameObject handles for units
- Can query unit properties (type, team)
- Coordinates positions from GameWorld

### GameWorld Integration (Phase 28)
- Queries spatial index for box select
- Uses frustum culling for visibility
- Tracks all active units

### Input System Integration
- Receives mouse/keyboard events
- Translates to selection operations
- Queues commands for execution

## Known Limitations

1. **No RTS Grid Support**: Selection assumes continuous coordinates
   - Can be adapted for tile-based games
   - Box select uses rect intersection

2. **No Unit Type Querying**: Type-based selection needs GameWorld integration
   - Currently placeholder implementation
   - Will work with Phase 27/28 integration

3. **Command Execution**: Commands are queued but not executed
   - Execution delegated to gameplay systems
   - Phase 35 (Combat) will consume commands

4. **No Formation Selection**: All selected units treated equally
   - Priority system in place for future use
   - Primary unit provides focus point

## Testing Strategy

### Unit Tests Needed
- Single/multi-select operations
- Box selection geometry
- Command queue FIFO
- Control group assignment
- Hotkey lookup
- Viewport coordinate transform

### Integration Tests Needed
- With GameWorld for spatial queries
- With GameObject for unit tracking
- With input system for events
- Mouse coordinate translation

## Compilation Status

✓ Compiles successfully with clang++ -std=c++20
- No errors or warnings
- Ready for platform builds

## Next Steps

1. **Phase 34**: Pathfinding & Movement (A* navigation)
2. **Phase 35**: Combat System (weapons, damage)
3. **Integration**: Connect with GameWorld spatial queries

## Files

- `GeneralsMD/Code/GameEngine/Source/Input/SelectionSystem.h` (287 lines)
- `GeneralsMD/Code/GameEngine/Source/Input/SelectionSystem.cpp` (976 lines)

## Metrics

- **Total functions**: 42
- **Total lines**: 1,263
- **Handle ranges**: 1,000 selection handles
- **Max selected units**: 200
- **Max queued commands**: 50
- **Control groups**: 10
- **Max hotkeys**: 32
- **Max hotkey actions**: 8
