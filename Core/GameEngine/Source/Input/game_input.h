/**
 * Game Input System - Unit selection and command execution
 * 
 * Extends CameraInputController with game-specific commands:
 * - Unit selection (single click, multi-select box)
 * - Unit commands (move, attack, build, etc)
 * - Mini-map interaction
 * - Hotkeys
 * 
 * Architecture:
 * - Inherits/wraps CameraInputController for camera commands
 * - Adds GameWorld queries for unit picking
 * - Implements command queue for unit orders
 */

#pragma once

#include "../../GameEngine/Source/Input/camera_input.h"
#include "../../GameEngine/Source/GameObject/gameobject.h"
#include <vector>
#include <cstdint>

namespace GeneralsX {

class GameWorld;
class Unit;
class Building;
class Camera;

/**
 * Game input manager - Handles gameplay commands and unit selection
 * 
 * Extends camera input with:
 * - Unit/building selection
 * - Click detection for unit picking
 * - Command queue (move, attack, build)
 * - Hotkeys for quick commands
 */
class GameInputManager {
public:
    /**
     * Selection mode
     */
    enum class SelectionMode : uint8_t {
        SINGLE = 0,      // Single unit selected
        MULTIPLE = 1,    // Multiple units selected
        BOX = 2,         // Box selection in progress
    };
    
    /**
     * Command type
     */
    enum class CommandType : uint8_t {
        MOVE = 0,        // Move to position
        ATTACK = 1,      // Attack target unit
        HOLD = 2,        // Hold position
        GUARD = 3,       // Guard area
        BUILD = 4,       // Build structure
        REPAIR = 5,      // Repair unit/building
        STOP = 6,        // Stop current action
    };
    
    /**
     * Unit command
     */
    struct UnitCommand {
        uint32_t unit_id;
        CommandType command;
        Vector3 target_position;
        uint32_t target_unit_id;  // For attack/repair
        
        UnitCommand() = default;
        UnitCommand(uint32_t id, CommandType cmd, const Vector3& pos)
            : unit_id(id), command(cmd), target_position(pos), target_unit_id(0) {}
        UnitCommand(uint32_t id, CommandType cmd, uint32_t target_id)
            : unit_id(id), command(cmd), target_position(0, 0, 0), target_unit_id(target_id) {}
    };
    
    /**
     * Constructor
     * @param world GameWorld instance (not owned)
     * @param camera Active camera (not owned)
     */
    GameInputManager(GameWorld* world, Camera* camera);
    
    /**
     * Destructor
     */
    ~GameInputManager();
    
    // ----- Selection Management -----
    
    /**
     * Select unit at world position
     * @param position World position to click
     * @param add If true, add to selection; if false, replace selection
     */
    void SelectAtPosition(const Vector3& position, bool add = false);
    
    /**
     * Select units in box
     * @param min Box corner 1
     * @param max Box corner 2
     * @param add If true, add to selection; if false, replace
     */
    void SelectInBox(const Vector3& min, const Vector3& max, bool add = false);
    
    /**
     * Clear current selection
     */
    void ClearSelection();
    
    /**
     * Get selected units
     * @return Vector of selected Unit pointers
     */
    const std::vector<Unit*>& GetSelectedUnits() const { return m_selected_units; }
    
    /**
     * Get selected unit count
     * @return Number of selected units
     */
    size_t GetSelectedUnitCount() const { return m_selected_units.size(); }
    
    /**
     * Check if unit is selected
     * @param unit Unit to check
     * @return True if unit is in selection
     */
    bool IsUnitSelected(Unit* unit) const;
    
    /**
     * Get selection mode
     * @return Current selection mode
     */
    SelectionMode GetSelectionMode() const { return m_selection_mode; }
    
    // ----- Command Execution -----
    
    /**
     * Issue move command to selected units
     * @param target Target position
     */
    void CommandMoveSelection(const Vector3& target);
    
    /**
     * Issue attack command to selected units
     * @param target Target unit to attack
     */
    void CommandAttackSelection(Unit* target);
    
    /**
     * Issue hold position command to selected units
     */
    void CommandHoldSelection();
    
    /**
     * Issue guard command to selected units
     * @param guard_point Point to guard
     * @param radius Guard radius
     */
    void CommandGuardSelection(const Vector3& guard_point, float radius);
    
    /**
     * Issue stop command to selected units
     */
    void CommandStopSelection();
    
    /**
     * Issue repair command to selected repair units
     * @param target Target unit/building to repair
     */
    void CommandRepairSelection(GameObject* target);
    
    // ----- Command Queue -----
    
    /**
     * Get pending command queue
     * @return Vector of pending commands
     */
    const std::vector<UnitCommand>& GetCommandQueue() const { return m_command_queue; }
    
    /**
     * Process command queue
     * Called each frame to execute queued commands
     */
    void ProcessCommandQueue();
    
    /**
     * Clear command queue
     */
    void ClearCommandQueue() { m_command_queue.clear(); }
    
    // ----- Input Processing -----
    
    /**
     * Process mouse click
     * @param screen_pos Screen coordinates of click
     * @param button Mouse button (0=left, 1=middle, 2=right)
     * @param ctrl Control key pressed
     * @param shift Shift key pressed
     * @param alt Alt key pressed
     */
    void OnMouseClick(const Vector2& screen_pos, int button, bool ctrl, bool shift, bool alt);
    
    /**
     * Process mouse drag
     * @param start_pos Start screen position
     * @param end_pos Current screen position
     * @param button Mouse button (0=left, 1=middle, 2=right)
     */
    void OnMouseDrag(const Vector2& start_pos, const Vector2& end_pos, int button);
    
    /**
     * Process mouse release
     * @param screen_pos Final screen position
     * @param button Mouse button
     */
    void OnMouseRelease(const Vector2& screen_pos, int button);
    
    /**
     * Process keyboard key
     * @param key_code Key code
     * @param is_pressed True if pressed, false if released
     */
    void OnKeyboard(int key_code, bool is_pressed);
    
    // ----- Debug -----
    
    /**
     * Get debug information
     * @return Debug string
     */
    std::string GetDebugInfo() const;

private:
    // Systems (not owned)
    GameWorld* m_world;
    Camera* m_camera;
    CameraInputController* m_camera_input;
    
    // Selection
    std::vector<Unit*> m_selected_units;
    SelectionMode m_selection_mode;
    
    // Command queue
    std::vector<UnitCommand> m_command_queue;
    
    // Input state
    Vector2 m_last_mouse_pos;
    bool m_mouse_dragging;
    Vector2 m_drag_start;
    
    /**
     * Convert screen coordinates to world position on ground plane
     * @param screen_pos Screen coordinates
     * @return World position on ground (Y=0 plane)
     */
    Vector3 ScreenToWorldPosition(const Vector2& screen_pos);
    
    /**
     * Perform ray cast from camera through screen position
     * @param screen_pos Screen coordinates
     * @return Ray in world space
     */
    Ray CastRayFromScreen(const Vector2& screen_pos);
    
    /**
     * Execute single command
     * @param command Command to execute
     */
    void ExecuteCommand(const UnitCommand& command);
    
    /**
     * Dequeue and execute pending commands
     */
    void FlushCommandQueue();

    // Prevent copying
    GameInputManager(const GameInputManager&) = delete;
    GameInputManager& operator=(const GameInputManager&) = delete;
};

} // namespace GeneralsX
