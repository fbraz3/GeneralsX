/**
 * Game input manager implementation
 * 
 * Unit selection and command processing
 */

#include "game_input.h"
#include "../GameWorld/gameworld.h"
#include "../Camera/camera.h"
#include <algorithm>
#include <sstream>
#include <cmath>

namespace GeneralsX {

GameInputManager::GameInputManager(GameWorld* world, Camera* camera)
    : m_world(world)
    , m_camera(camera)
    , m_camera_input(nullptr)
    , m_selection_mode(SelectionMode::SINGLE)
    , m_last_mouse_pos(0, 0)
    , m_mouse_dragging(false)
    , m_drag_start(0, 0)
{
    // Create camera input controller if camera provided
    if (m_camera != nullptr) {
        m_camera_input = new CameraInputController(m_camera);
        if (m_camera_input != nullptr) {
            m_camera_input->Initialize();
        }
    }
}

GameInputManager::~GameInputManager() {
    if (m_camera_input != nullptr) {
        m_camera_input->Shutdown();
        delete m_camera_input;
        m_camera_input = nullptr;
    }
    
    ClearCommandQueue();
    ClearSelection();
}

// ----- Selection Management -----

void GameInputManager::SelectAtPosition(const Vector3& position, bool add) {
    // Find unit/building at position
    GameObject* obj = m_world->GetObjectAt(position, 2.0f);
    Unit* unit = dynamic_cast<Unit*>(obj);
    
    if (!add) {
        ClearSelection();
    }
    
    if (unit != nullptr && unit->IsAlive()) {
        // Check if already selected
        if (!IsUnitSelected(unit)) {
            m_selected_units.push_back(unit);
            m_selection_mode = m_selected_units.size() > 1 ? SelectionMode::MULTIPLE : SelectionMode::SINGLE;
        }
    }
}

void GameInputManager::SelectInBox(const Vector3& min, const Vector3& max, bool add) {
    std::vector<GameObject*> objects = m_world->GetObjectsInBox(min, max);
    
    if (!add) {
        ClearSelection();
    }
    
    for (GameObject* obj : objects) {
        Unit* unit = dynamic_cast<Unit*>(obj);
        if (unit != nullptr && unit->IsAlive() && !IsUnitSelected(unit)) {
            m_selected_units.push_back(unit);
        }
    }
    
    m_selection_mode = m_selected_units.size() > 1 ? SelectionMode::MULTIPLE : SelectionMode::SINGLE;
}

void GameInputManager::ClearSelection() {
    m_selected_units.clear();
    m_selection_mode = SelectionMode::SINGLE;
}

bool GameInputManager::IsUnitSelected(Unit* unit) const {
    return std::find(m_selected_units.begin(), m_selected_units.end(), unit) != m_selected_units.end();
}

// ----- Command Execution -----

void GameInputManager::CommandMoveSelection(const Vector3& target) {
    for (Unit* unit : m_selected_units) {
        UnitCommand cmd(unit->GetID(), CommandType::MOVE, target);
        m_command_queue.push_back(cmd);
    }
}

void GameInputManager::CommandAttackSelection(Unit* target) {
    if (target == nullptr) return;
    
    for (Unit* unit : m_selected_units) {
        UnitCommand cmd(unit->GetID(), CommandType::ATTACK, target->GetID());
        m_command_queue.push_back(cmd);
    }
}

void GameInputManager::CommandHoldSelection() {
    for (Unit* unit : m_selected_units) {
        UnitCommand cmd(unit->GetID(), CommandType::HOLD, unit->GetPosition());
        m_command_queue.push_back(cmd);
    }
}

void GameInputManager::CommandGuardSelection(const Vector3& guard_point, float radius) {
    for (Unit* unit : m_selected_units) {
        UnitCommand cmd(unit->GetID(), CommandType::GUARD, guard_point);
        m_command_queue.push_back(cmd);
    }
}

void GameInputManager::CommandStopSelection() {
    for (Unit* unit : m_selected_units) {
        UnitCommand cmd(unit->GetID(), CommandType::STOP, unit->GetPosition());
        m_command_queue.push_back(cmd);
    }
}

void GameInputManager::CommandRepairSelection(GameObject* target) {
    if (target == nullptr) return;
    
    for (Unit* unit : m_selected_units) {
        UnitCommand cmd(unit->GetID(), CommandType::REPAIR, target->GetID());
        m_command_queue.push_back(cmd);
    }
}

// ----- Command Queue -----

void GameInputManager::ProcessCommandQueue() {
    for (const UnitCommand& cmd : m_command_queue) {
        ExecuteCommand(cmd);
    }
    m_command_queue.clear();
}

// ----- Input Processing -----

void GameInputManager::OnMouseClick(const Vector2& screen_pos, int button, bool ctrl, bool shift, bool alt) {
    m_last_mouse_pos = screen_pos;
    
    switch (button) {
        case 0:  // Left click - select
            m_mouse_dragging = true;
            m_drag_start = screen_pos;
            
            // Single click selection
            if (!shift && !ctrl) {
                Vector3 world_pos = ScreenToWorldPosition(screen_pos);
                SelectAtPosition(world_pos, false);
            }
            break;
            
        case 1:  // Middle click - pan
            // Camera pan handled by camera input
            break;
            
        case 2:  // Right click - command
            {
                Vector3 world_pos = ScreenToWorldPosition(screen_pos);
                
                // Check what's at target position
                GameObject* target = m_world->GetObjectAt(world_pos, 2.0f);
                Unit* target_unit = dynamic_cast<Unit*>(target);
                
                if (target_unit != nullptr && !IsUnitSelected(target_unit)) {
                    // Attack target
                    CommandAttackSelection(target_unit);
                } else {
                    // Move command
                    CommandMoveSelection(world_pos);
                }
            }
            break;
    }
}

void GameInputManager::OnMouseDrag(const Vector2& start_pos, const Vector2& end_pos, int button) {
    m_last_mouse_pos = end_pos;
    
    if (button == 0 && m_mouse_dragging) {
        // Left drag - box selection
        // TODO: Show selection box visualization
    }
}

void GameInputManager::OnMouseRelease(const Vector2& screen_pos, int button) {
    if (button == 0 && m_mouse_dragging) {
        // Box selection on release
        Vector3 start_world = ScreenToWorldPosition(m_drag_start);
        Vector3 end_world = ScreenToWorldPosition(screen_pos);
        
        // Create bounding box
        Vector3 min(
            std::min(start_world.x, end_world.x),
            std::min(start_world.y, end_world.y),
            std::min(start_world.z, end_world.z)
        );
        Vector3 max(
            std::max(start_world.x, end_world.x),
            std::max(start_world.y, end_world.y),
            std::max(start_world.z, end_world.z)
        );
        
        SelectInBox(min, max, false);
        m_selection_mode = SelectionMode::BOX;
        
        m_mouse_dragging = false;
    }
}

void GameInputManager::OnKeyboard(int key_code, bool is_pressed) {
    if (!is_pressed) return;
    
    // Common hotkeys
    switch (key_code) {
        case 'H':
            // H for Hold
            CommandHoldSelection();
            break;
        case 'S':
            // S for Stop
            CommandStopSelection();
            break;
        case 'A':
            // A for Attack (if target selected)
            // TODO: Implement attack mode
            break;
        case 'M':
            // M for Move mode
            // TODO: Implement move mode
            break;
        default:
            break;
    }
    
    // Delegate camera input to camera input controller
    if (m_camera_input != nullptr) {
        // TODO: Route keyboard to camera input controller
    }
}

// ----- Debug -----

std::string GameInputManager::GetDebugInfo() const {
    std::stringstream ss;
    ss << "Game Input Manager:" << std::endl;
    ss << "  Selected Units: " << m_selected_units.size() << std::endl;
    ss << "  Selection Mode: " << static_cast<int>(m_selection_mode) << std::endl;
    ss << "  Pending Commands: " << m_command_queue.size() << std::endl;
    ss << "  Mouse Position: (" << m_last_mouse_pos.x << ", " << m_last_mouse_pos.y << ")" << std::endl;
    return ss.str();
}

// ----- Private Methods -----

Vector3 GameInputManager::ScreenToWorldPosition(const Vector2& screen_pos) {
    if (m_camera == nullptr) {
        return Vector3(0, 0, 0);
    }
    
    // TODO: Use camera ray casting to find ground plane intersection
    // For now, return approximate world position
    Vector3 forward = m_camera->GetForward();
    Vector3 right = m_camera->GetRight();
    
    // Simple projection assuming camera looking down at ground
    Vector3 world_pos = m_camera->GetPosition() + forward * 10.0f;
    return world_pos;
}

Ray GameInputManager::CastRayFromScreen(const Vector2& screen_pos) {
    if (m_camera == nullptr) {
        return Ray(Vector3(0, 0, 0), Vector3(0, 0, 1));
    }
    
    // TODO: Use camera's CastRay method
    // return m_camera->CastRay(screen_pos.x, screen_pos.y);
    
    return Ray(m_camera->GetPosition(), m_camera->GetForward());
}

void GameInputManager::ExecuteCommand(const UnitCommand& command) {
    Unit* unit = dynamic_cast<Unit*>(m_world->GetObjectByID(command.unit_id));
    if (unit == nullptr || !unit->IsAlive()) {
        return;
    }
    
    switch (command.command) {
        case CommandType::MOVE:
            unit->SetMoveTarget(command.target_position);
            break;
            
        case CommandType::ATTACK:
            {
                Unit* target = dynamic_cast<Unit*>(m_world->GetObjectByID(command.target_unit_id));
                if (target != nullptr) {
                    unit->SetAttackTarget(target);
                }
            }
            break;
            
        case CommandType::HOLD:
            unit->SetMoveTarget(unit->GetPosition());  // Stop moving
            break;
            
        case CommandType::STOP:
            unit->SetMoveTarget(unit->GetPosition());
            unit->SetAttackTarget(nullptr);
            break;
            
        default:
            break;
    }
}

void GameInputManager::FlushCommandQueue() {
    ProcessCommandQueue();
}

} // namespace GeneralsX
