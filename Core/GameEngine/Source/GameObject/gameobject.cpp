/**
 * GameObject implementation
 * 
 * Provides lifecycle management, transform calculations, and rendering
 * for all game entities in the world.
 */

#include "gameobject.h"
#include "../../Math/math.h"
#include "../Camera/camera.h"
#include <sstream>
#include <cmath>

namespace GeneralsX {

// ============================================================================
// GameObject Implementation
// ============================================================================

GameObject::GameObject(uint32_t id, const std::string& name, ObjectType type, const Vector3& position)
    : m_id(id)
    , m_name(name)
    , m_type(type)
    , m_position(position)
    , m_rotation(0.0f, 0.0f, 0.0f, 1.0f)  // Identity quaternion
    , m_scale(1.0f, 1.0f, 1.0f)
    , m_transform_dirty(true)
    , m_model(nullptr)
    , m_material(nullptr)
    , m_health(100.0f)
    , m_max_health(100.0f)
    , m_is_alive(true)
    , m_visibility_range(1000.0f)
{
}

GameObject::~GameObject() {
    // Model and Material are owned by their managers, not this object
    m_model = nullptr;
    m_material = nullptr;
}

// ----- Transform Management -----

void GameObject::SetPosition(const Vector3& position) {
    if (m_position != position) {
        m_position = position;
        m_transform_dirty = true;
    }
}

void GameObject::SetRotation(const Quaternion& rotation) {
    if (m_rotation != rotation) {
        m_rotation = rotation.Normalize();
        m_transform_dirty = true;
    }
}

void GameObject::SetScale(const Vector3& scale) {
    if (m_scale != scale) {
        m_scale = scale;
        m_transform_dirty = true;
    }
}

Matrix4x4 GameObject::GetWorldTransform() const {
    if (m_transform_dirty) {
        RecalculateWorldTransform();
    }
    return m_world_transform;
}

void GameObject::RecalculateWorldTransform() {
    // Build transform matrix: M = T * R * S
    // Where T is translation, R is rotation, S is scale
    
    // Convert quaternion to rotation matrix
    Matrix4x4 rotation_matrix = m_rotation.ToMatrix();
    
    // Apply scale to rotation matrix
    Matrix4x4 rs_matrix = rotation_matrix;
    rs_matrix.m[0][0] *= m_scale.x;
    rs_matrix.m[0][1] *= m_scale.y;
    rs_matrix.m[0][2] *= m_scale.z;
    
    rs_matrix.m[1][0] *= m_scale.x;
    rs_matrix.m[1][1] *= m_scale.y;
    rs_matrix.m[1][2] *= m_scale.z;
    
    rs_matrix.m[2][0] *= m_scale.x;
    rs_matrix.m[2][1] *= m_scale.y;
    rs_matrix.m[2][2] *= m_scale.z;
    
    // Apply translation
    m_world_transform = rs_matrix;
    m_world_transform.m[3][0] = m_position.x;
    m_world_transform.m[3][1] = m_position.y;
    m_world_transform.m[3][2] = m_position.z;
    m_world_transform.m[3][3] = 1.0f;
    
    m_transform_dirty = false;
}

Vector3 GameObject::GetForward() const {
    // Extract forward vector from rotation quaternion
    // Forward = (0, 0, 1) rotated by m_rotation
    Vector3 forward(0.0f, 0.0f, 1.0f);
    return m_rotation.RotateVector(forward);
}

Vector3 GameObject::GetRight() const {
    // Extract right vector from rotation quaternion
    // Right = (1, 0, 0) rotated by m_rotation
    Vector3 right(1.0f, 0.0f, 0.0f);
    return m_rotation.RotateVector(right);
}

Vector3 GameObject::GetUp() const {
    // Extract up vector from rotation quaternion
    // Up = (0, 1, 0) rotated by m_rotation
    Vector3 up(0.0f, 1.0f, 0.0f);
    return m_rotation.RotateVector(up);
}

// ----- Model & Rendering -----

void GameObject::SetModel(Model* model) {
    m_model = model;
}

void GameObject::SetMaterial(Material* material) {
    m_material = material;
}

void GameObject::SetAnimation(uint32_t anim_id, bool loop, float speed) {
    m_animation.animation_id = anim_id;
    m_animation.animation_time = 0.0f;
    m_animation.is_looping = loop;
    m_animation.animation_speed = speed;
}

// ----- Game State -----

void GameObject::SetHealth(float health) {
    m_health = health;
    if (m_health <= 0.0f) {
        m_health = 0.0f;
        m_is_alive = false;
        OnDeath();
    }
}

void GameObject::TakeDamage(float damage) {
    if (damage < 0.0f) return;  // Ignore negative damage
    
    float old_health = m_health;
    SetHealth(m_health - damage);
    
    if (m_health > 0.0f && old_health > 0.0f) {
        OnDamaged(damage);
    }
}

void GameObject::Heal(float amount) {
    if (amount < 0.0f) return;
    
    m_health += amount;
    if (m_health > m_max_health) {
        m_health = m_max_health;
    }
}

void GameObject::Kill() {
    SetHealth(0.0f);
    m_is_alive = false;
}

// ----- Bounding Volume -----

float GameObject::GetBoundingRadius() const {
    if (m_model == nullptr) {
        return 1.0f;  // Default radius
    }
    
    // TODO: Get bounding radius from model
    // For now return fixed value
    return 2.0f;
}

void GameObject::GetBoundingBox(Vector3& out_min, Vector3& out_max) const {
    if (m_model == nullptr) {
        // Default unit cube around position
        out_min = m_position - Vector3(1.0f, 1.0f, 1.0f);
        out_max = m_position + Vector3(1.0f, 1.0f, 1.0f);
        return;
    }
    
    // TODO: Get bounding box from model and transform by world matrix
    // For now use approximate
    float radius = GetBoundingRadius();
    out_min = m_position - Vector3(radius, radius, radius);
    out_max = m_position + Vector3(radius, radius, radius);
}

// ----- Lifecycle -----

void GameObject::Update(float delta_time) {
    // Update animation
    if (m_model != nullptr && m_animation.animation_id > 0) {
        m_animation.animation_time += delta_time * m_animation.animation_speed;
        
        // TODO: Check animation duration and loop if needed
        // if (m_animation.animation_time > animation_duration && m_animation.is_looping) {
        //     m_animation.animation_time = 0.0f;
        // }
    }
}

void GameObject::Render(Camera* camera) {
    if (m_model == nullptr || !IsVisible()) {
        return;
    }
    
    // TODO: Bind material and textures
    // TODO: Set world transform in shader
    // TODO: Call model->Draw()
}

bool GameObject::IsVisible() const {
    // Object is visible if:
    // 1. It has a model
    // 2. It is alive
    // 3. It is within visibility range (TODO: implement distance check)
    return m_model != nullptr && m_is_alive;
}

std::string GameObject::GetDebugInfo() const {
    std::stringstream ss;
    ss << "GameObject[" << m_id << "]" << std::endl;
    ss << "  Name: " << m_name << std::endl;
    ss << "  Type: " << static_cast<int>(m_type) << std::endl;
    ss << "  Position: (" << m_position.x << ", " << m_position.y << ", " << m_position.z << ")" << std::endl;
    ss << "  Health: " << m_health << "/" << m_max_health << std::endl;
    ss << "  Alive: " << (m_is_alive ? "true" : "false") << std::endl;
    ss << "  Model: " << (m_model != nullptr ? "attached" : "none") << std::endl;
    return ss.str();
}

void GameObject::DrawDebug() {
    // TODO: Draw debug visualization
    // - Bounding sphere
    // - Coordinate axes
    // - Health bar above head
}

void GameObject::OnDeath() {
    // Default: just mark as dead
    // Subclasses can override for death animations/effects
}

void GameObject::OnDamaged(float damage) {
    // Default: no reaction
    // Subclasses can override for damage animations/sounds
}

// ============================================================================
// Unit Implementation
// ============================================================================

Unit::Unit(uint32_t id, const std::string& name, const Vector3& position, float speed)
    : GameObject(id, name, ObjectType::UNIT, position)
    , m_move_target(position)
    , m_speed(speed)
    , m_movement_state(MovementState::IDLE)
    , m_rotation_speed(3.0f)  // 3 radians per second
    , m_attack_target(nullptr)
    , m_attack_range(5.0f)
    , m_attack_cooldown(0.0f)
{
    // Units default to 100 health
    m_health = 100.0f;
    m_max_health = 100.0f;
}

Unit::~Unit() {
    // Clear attack target (but don't delete, it's owned by GameWorld)
    m_attack_target = nullptr;
}

void Unit::SetMoveTarget(const Vector3& target) {
    m_move_target = target;
    
    // Only set to MOVING if target is different from current position
    float distance = (target - m_position).Length();
    if (distance > 0.1f) {
        m_movement_state = MovementState::MOVING;
    }
}

void Unit::SetAttackTarget(Unit* target) {
    m_attack_target = target;
    if (target != nullptr) {
        m_movement_state = MovementState::ATTACKING;
    }
}

void Unit::Attack(Unit* target, float damage) {
    if (target != nullptr && target->IsAlive()) {
        target->TakeDamage(damage);
    }
}

void Unit::Update(float delta_time) {
    // Call base update (handles animation)
    GameObject::Update(delta_time);
    
    if (!m_is_alive) {
        return;
    }
    
    // Update attack cooldown
    if (m_attack_cooldown > 0.0f) {
        m_attack_cooldown -= delta_time;
    }
    
    // Update state based on current activity
    switch (m_movement_state) {
        case MovementState::IDLE:
            // Do nothing
            break;
            
        case MovementState::MOVING:
            UpdateMovement(delta_time);
            UpdateRotation(delta_time);
            break;
            
        case MovementState::ATTACKING:
            UpdateCombat(delta_time);
            break;
            
        default:
            break;
    }
}

void Unit::Render(Camera* camera) {
    // Call base render
    GameObject::Render(camera);
}

void Unit::UpdateMovement(float delta_time) {
    Vector3 to_target = m_move_target - m_position;
    float distance = to_target.Length();
    
    // Check if reached target
    if (distance < 0.5f) {
        m_movement_state = MovementState::IDLE;
        return;
    }
    
    // Move towards target
    Vector3 direction = to_target.Normalize();
    Vector3 new_position = m_position + direction * m_speed * delta_time;
    
    // Don't overshoot target
    if ((new_position - m_move_target).Length() < distance) {
        SetPosition(new_position);
    } else {
        SetPosition(m_move_target);
        m_movement_state = MovementState::IDLE;
    }
}

void Unit::UpdateRotation(float delta_time) {
    Vector3 direction = (m_move_target - m_position).Normalize();
    
    // Get current forward direction
    Vector3 current_forward = GetForward();
    
    // Calculate angle between current and target direction
    float dot = Vector3::Dot(current_forward, direction);
    dot = std::clamp(dot, -1.0f, 1.0f);
    float angle = std::acos(dot);
    
    // Rotate towards direction
    if (angle > 0.01f) {
        float rotation_amount = m_rotation_speed * delta_time;
        if (rotation_amount > angle) {
            rotation_amount = angle;
        }
        
        // Interpolate rotation (SLERP would be more advanced)
        // For now, just rotate towards target
        Vector3 axis = Vector3::Cross(current_forward, direction).Normalize();
        if (axis.Length() > 0.01f) {
            // Create rotation quaternion
            Quaternion delta_rot;
            delta_rot.FromAxisAngle(axis, rotation_amount);
            m_rotation = (delta_rot * m_rotation).Normalize();
            m_transform_dirty = true;
        }
    }
}

void Unit::UpdateCombat(float delta_time) {
    if (m_attack_target == nullptr || !m_attack_target->IsAlive()) {
        m_attack_target = nullptr;
        m_movement_state = MovementState::IDLE;
        return;
    }
    
    Vector3 to_target = m_attack_target->GetPosition() - m_position;
    float distance = to_target.Length();
    
    // Move closer if out of range
    if (distance > m_attack_range) {
        SetMoveTarget(m_attack_target->GetPosition());
        m_movement_state = MovementState::MOVING;
        UpdateMovement(delta_time);
    } else {
        // In range, perform attack if cooldown elapsed
        if (m_attack_cooldown <= 0.0f) {
            Attack(m_attack_target, 10.0f);  // 10 damage per hit
            m_attack_cooldown = 0.5f;  // 0.5 second cooldown
        }
    }
}

void Unit::OnDeath() {
    m_movement_state = MovementState::IDLE;
    m_attack_target = nullptr;
    
    // TODO: Play death animation
    // TODO: Spawn debris/corpse
}

// ============================================================================
// Building Implementation
// ============================================================================

Building::Building(uint32_t id, const std::string& name, const Vector3& position, float build_time)
    : GameObject(id, name, ObjectType::BUILDING, position)
    , m_build_progress(0.0f)
    , m_build_time(build_time)
    , m_state(BuildingState::CONSTRUCTING)
    , m_aoe_radius(10.0f)
{
    // Buildings start with reduced health during construction
    m_health = 50.0f;
    m_max_health = 100.0f;
}

Building::~Building() {
}

float Building::GetBuildProgress() const {
    return m_build_progress;
}

void Building::SetBuildProgress(float progress) {
    m_build_progress = std::clamp(progress, 0.0f, 1.0f);
    
    if (m_build_progress >= 1.0f) {
        FinishConstruction();
    }
}

void Building::FinishConstruction() {
    m_build_progress = 1.0f;
    m_state = BuildingState::OPERATIONAL;
    m_health = m_max_health;  // Full health when construction complete
    
    OnConstructionComplete();
}

void Building::Update(float delta_time) {
    GameObject::Update(delta_time);
    
    if (m_state == BuildingState::CONSTRUCTING) {
        UpdateConstruction(delta_time);
    }
}

void Building::Render(Camera* camera) {
    GameObject::Render(camera);
    
    // TODO: Draw construction progress bar if still building
}

void Building::UpdateConstruction(float delta_time) {
    // Automatically progress construction (in real game, would be triggered by workers)
    // For now, just auto-complete for testing
    m_build_progress += delta_time / m_build_time;
    
    if (m_build_progress >= 1.0f) {
        FinishConstruction();
    }
}

void Building::OnConstructionComplete() {
    // TODO: Play construction complete animation/sound
    // TODO: Update radar
}

// ============================================================================
// Effect Implementation
// ============================================================================

Effect::Effect(uint32_t id, const std::string& name, const Vector3& position, float duration)
    : GameObject(id, name, ObjectType::EFFECT, position)
    , m_duration(duration)
    , m_time_remaining(duration)
{
    // Effects always start alive
    m_is_alive = true;
    m_health = 1.0f;
    m_max_health = 1.0f;
}

Effect::~Effect() {
}

float Effect::GetTimeRemaining() const {
    return m_time_remaining;
}

void Effect::Update(float delta_time) {
    GameObject::Update(delta_time);
    
    m_time_remaining -= delta_time;
    if (m_time_remaining <= 0.0f) {
        m_time_remaining = 0.0f;
        m_is_alive = false;
    }
}

void Effect::Render(Camera* camera) {
    GameObject::Render(camera);
}

} // namespace GeneralsX
