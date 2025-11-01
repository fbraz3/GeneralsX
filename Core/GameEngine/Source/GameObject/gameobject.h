/**
 * GameObject System - Core game object implementation for GeneralsX
 * 
 * Provides base GameObject class and specialized subclasses (Unit, Building) for
 * representing game entities with transforms, models, and game state.
 * 
 * Architecture:
 * - GameObject: Base class with transform and lifecycle management
 * - Unit: Mobile objects under player control (soldiers, vehicles, tanks)
 * - Building: Static structures (barracks, factories, power plants)
 * - Effect: Temporary visual effects (explosions, smoke, particles)
 * 
 * Transform System:
 * - Position: World space (X=East, Y=Up, Z=North)
 * - Rotation: Quaternion-based for smooth animations
 * - Scale: Non-uniform support for special objects
 * - GetWorldTransform(): Combines position, rotation, scale into 4x4 matrix
 * 
 * Rendering:
 * - Model: Pointer to Model resource from Phase 44
 * - Material: Material variations for damage states, colors, effects
 * - AnimationState: Current playing animation and progress
 * - Render(): Called by game renderer each frame
 * 
 * Game State:
 * - Health/MaxHealth: Damage tracking
 * - IsAlive: Determines if object is active
 * - Type: Unit/Building/Effect classification
 * - ID: Unique identifier for networking/lookup
 */

#pragma once

#include "../../Math/math.h"
#include "../../Graphics/model.h"
#include "../../Graphics/material.h"

namespace GeneralsX {

class GameWorld;  // Forward declaration

/**
 * Animation state tracking
 * 
 * Tracks which animation is playing, current frame, and progress
 */
struct AnimationState {
    uint32_t animation_id;      // Index into model's animation list
    float animation_time;       // Elapsed time in current animation (seconds)
    float animation_speed;      // Playback speed multiplier (default: 1.0)
    bool is_looping;           // Whether animation repeats
    
    AnimationState()
        : animation_id(0)
        , animation_time(0.0f)
        , animation_speed(1.0f)
        , is_looping(true)
    {}
    
    /**
     * Get animation progress as normalized value [0, 1]
     * @param animation_duration Total duration of animation in seconds
     * @return Progress from 0.0 (start) to 1.0 (end)
     */
    float GetProgress(float animation_duration) const {
        if (animation_duration <= 0.0f) return 1.0f;
        return (animation_time / animation_duration);
    }
};

/**
 * Base GameObject class
 * 
 * Represents any game entity in the world with position, rotation, model,
 * and basic game state (health, alive status, etc).
 * 
 * Lifecycle:
 * 1. Constructed with initial parameters
 * 2. Added to GameWorld
 * 3. Update() called each frame
 * 4. Render() called each frame
 * 5. Destroyed when health reaches 0 or explicitly removed
 */
class GameObject {
public:
    /**
     * Object type classification
     */
    enum class ObjectType : uint8_t {
        UNIT = 0,         // Mobile controllable objects
        BUILDING = 1,     // Static structures
        EFFECT = 2,       // Temporary visual effects
        PROJECTILE = 3,   // Bullets, missiles, etc
        DEBRIS = 4,       // Dead unit remains
        TERRAIN = 5,      // Terrain features
    };
    
    /**
     * Constructor
     * 
     * @param id Unique identifier for this object
     * @param name Display name for debugging
     * @param type Object type (Unit/Building/Effect)
     * @param position Initial world position
     */
    GameObject(uint32_t id, const std::string& name, ObjectType type, const Vector3& position);
    
    /**
     * Virtual destructor - derived classes should override
     */
    virtual ~GameObject();
    
    // ----- Transform Management -----
    
    /**
     * Set world position
     * @param position New position in world space
     */
    void SetPosition(const Vector3& position);
    
    /**
     * Get world position
     * @return Current position in world space
     */
    const Vector3& GetPosition() const { return m_position; }
    
    /**
     * Set rotation
     * @param rotation New rotation as quaternion
     */
    void SetRotation(const Quaternion& rotation);
    
    /**
     * Get rotation
     * @return Current rotation as quaternion
     */
    const Quaternion& GetRotation() const { return m_rotation; }
    
    /**
     * Set scale (non-uniform)
     * @param scale New scale factors (default: 1,1,1)
     */
    void SetScale(const Vector3& scale);
    
    /**
     * Get scale
     * @return Current scale factors
     */
    const Vector3& GetScale() const { return m_scale; }
    
    /**
     * Calculate world transform matrix
     * Combines position, rotation, and scale into single 4x4 matrix
     * 
     * @return World transform: M = T * R * S
     */
    Matrix4x4 GetWorldTransform() const;
    
    /**
     * Get forward vector (direction object is facing)
     * @return Unit direction vector
     */
    Vector3 GetForward() const;
    
    /**
     * Get right vector
     * @return Unit direction vector
     */
    Vector3 GetRight() const;
    
    /**
     * Get up vector
     * @return Unit direction vector
     */
    Vector3 GetUp() const;
    
    // ----- Model & Rendering -----
    
    /**
     * Set model for this object
     * @param model Pointer to Model resource (managed by ModelManager)
     */
    void SetModel(Model* model);
    
    /**
     * Get model
     * @return Current model or nullptr
     */
    Model* GetModel() const { return m_model; }
    
    /**
     * Set material for this object
     * @param material Pointer to Material resource
     */
    void SetMaterial(Material* material);
    
    /**
     * Get material
     * @return Current material or nullptr
     */
    Material* GetMaterial() const { return m_material; }
    
    /**
     * Set animation state
     * @param anim_id Animation index in model
     * @param loop Whether animation loops
     * @param speed Playback speed (default: 1.0)
     */
    void SetAnimation(uint32_t anim_id, bool loop = true, float speed = 1.0f);
    
    /**
     * Get current animation state
     * @return Animation state struct
     */
    const AnimationState& GetAnimationState() const { return m_animation; }
    
    // ----- Game State -----
    
    /**
     * Get unique object ID
     * @return Unique 32-bit identifier
     */
    uint32_t GetID() const { return m_id; }
    
    /**
     * Get display name
     * @return Name string for debugging
     */
    const std::string& GetName() const { return m_name; }
    
    /**
     * Get object type
     * @return Type classification
     */
    ObjectType GetType() const { return m_type; }
    
    /**
     * Set health
     * @param health New health value
     */
    void SetHealth(float health);
    
    /**
     * Get current health
     * @return Current health value
     */
    float GetHealth() const { return m_health; }
    
    /**
     * Get maximum health
     * @return Max health value
     */
    float GetMaxHealth() const { return m_max_health; }
    
    /**
     * Get health as normalized value [0, 1]
     * @return Health percentage
     */
    float GetHealthPercentage() const {
        if (m_max_health <= 0.0f) return 1.0f;
        return m_health / m_max_health;
    }
    
    /**
     * Take damage
     * @param damage Damage amount (will be subtracted from health)
     */
    void TakeDamage(float damage);
    
    /**
     * Restore health
     * @param amount Health to restore
     */
    void Heal(float amount);
    
    /**
     * Check if object is alive
     * @return True if health > 0
     */
    bool IsAlive() const { return m_is_alive && m_health > 0.0f; }
    
    /**
     * Set alive state explicitly
     * @param alive New alive state
     */
    void SetAlive(bool alive) { m_is_alive = alive; }
    
    /**
     * Kill object (sets health to 0 and alive to false)
     */
    void Kill();
    
    // ----- Bounding Volume -----
    
    /**
     * Get bounding sphere radius
     * Used for frustum culling, collision detection
     * 
     * @return Radius in world units
     */
    float GetBoundingRadius() const;
    
    /**
     * Get axis-aligned bounding box
     * @param out_min Output minimum corner
     * @param out_max Output maximum corner
     */
    void GetBoundingBox(Vector3& out_min, Vector3& out_max) const;
    
    // ----- Lifecycle -----
    
    /**
     * Update object state
     * Called each frame before rendering
     * 
     * Typically handles:
     * - Animation updates
     * - Physics (if needed)
     * - Status effects
     * - Despawn logic
     * 
     * @param delta_time Time elapsed since last frame (seconds)
     */
    virtual void Update(float delta_time);
    
    /**
     * Render object
     * Called by game renderer if visible
     * 
     * Binds model, material, and draws using provided camera
     * 
     * @param camera Camera for view/projection matrices
     */
    virtual void Render(class Camera* camera);
    
    /**
     * Get object visibility
     * Used to determine if object should be rendered
     * 
     * @return True if object should be rendered
     */
    bool IsVisible() const;
    
    // ----- Debug -----
    
    /**
     * Get debug information string
     * @return Formatted string with object details
     */
    std::string GetDebugInfo() const;
    
    /**
     * Draw debug visualization (bounding sphere, axes, etc)
     * Only in debug builds
     */
    virtual void DrawDebug();

protected:
    // Identity & Type
    uint32_t m_id;              // Unique identifier
    std::string m_name;         // Debug name
    ObjectType m_type;          // Unit/Building/Effect
    
    // Transform
    Vector3 m_position;         // World position
    Quaternion m_rotation;      // Orientation
    Vector3 m_scale;            // Non-uniform scale (default: 1,1,1)
    bool m_transform_dirty;     // Flag to recalculate matrix
    mutable Matrix4x4 m_world_transform;  // Cached transform matrix
    
    // Model & Rendering
    Model* m_model;             // Model resource (owned by ModelManager)
    Material* m_material;       // Material override
    AnimationState m_animation; // Animation state
    
    // Game State
    float m_health;             // Current health
    float m_max_health;         // Maximum health
    bool m_is_alive;            // Alive state
    float m_visibility_range;   // Distance for visibility check
    
    /**
     * Recalculate cached world transform
     * Called when transform is marked dirty
     */
    void RecalculateWorldTransform();
    
    /**
     * Called when object dies
     * Can be overridden by subclasses for death effects
     */
    virtual void OnDeath();
    
    /**
     * Called when object takes damage
     * Can be overridden for damage reactions
     */
    virtual void OnDamaged(float damage);

private:
    // Prevent copying
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;
};

/**
 * Unit GameObject - Mobile controllable object
 * 
 * Extends GameObject with:
 * - Movement commands and pathfinding
 * - Speed and movement state
 * - Target tracking
 * - Attack capabilities
 */
class Unit : public GameObject {
public:
    /**
     * Movement state enumeration
     */
    enum class MovementState : uint8_t {
        IDLE = 0,           // Not moving
        MOVING = 1,         // Moving towards target
        ROTATING = 2,       // Turning to face direction
        ATTACKING = 3,      // In combat
        DAMAGED = 4,        // Knocked back or stunned
    };
    
    /**
     * Constructor
     * @param id Unique identifier
     * @param name Unit name
     * @param position Initial position
     * @param speed Movement speed (units/second)
     */
    Unit(uint32_t id, const std::string& name, const Vector3& position, float speed = 10.0f);
    
    virtual ~Unit();
    
    // ----- Movement -----
    
    /**
     * Set movement target position
     * @param target Target position in world space
     */
    void SetMoveTarget(const Vector3& target);
    
    /**
     * Get current movement target
     * @return Target position or current position if idle
     */
    const Vector3& GetMoveTarget() const { return m_move_target; }
    
    /**
     * Set movement speed
     * @param speed Speed in units per second
     */
    void SetSpeed(float speed) { m_speed = speed; }
    
    /**
     * Get current movement speed
     * @return Speed in units per second
     */
    float GetSpeed() const { return m_speed; }
    
    /**
     * Get movement state
     * @return Current movement state enum
     */
    MovementState GetMovementState() const { return m_movement_state; }
    
    /**
     * Check if unit is moving
     * @return True if currently moving
     */
    bool IsMoving() const { return m_movement_state == MovementState::MOVING; }
    
    // ----- Combat -----
    
    /**
     * Set attack target
     * @param target Unit to attack (nullptr to clear)
     */
    void SetAttackTarget(Unit* target);
    
    /**
     * Get current attack target
     * @return Target unit or nullptr
     */
    Unit* GetAttackTarget() const { return m_attack_target; }
    
    /**
     * Perform attack
     * @param damage Damage to inflict
     */
    void Attack(Unit* target, float damage);
    
    // ----- Lifecycle -----
    
    /**
     * Update unit state
     * Handles movement, animation, combat
     * 
     * @param delta_time Time elapsed since last frame (seconds)
     */
    void Update(float delta_time) override;
    
    /**
     * Render unit
     * @param camera Rendering camera
     */
    void Render(class Camera* camera) override;
    
protected:
    // Movement
    Vector3 m_move_target;      // Target position for movement
    float m_speed;              // Movement speed (units/sec)
    MovementState m_movement_state;
    float m_rotation_speed;     // Speed of rotation (rad/sec), default 3.0
    
    // Combat
    Unit* m_attack_target;      // Unit currently being attacked
    float m_attack_range;       // Range to attack from (world units)
    float m_attack_cooldown;    // Time until next attack possible (seconds)
    
    /**
     * Update movement towards target
     * @param delta_time Time step
     */
    void UpdateMovement(float delta_time);
    
    /**
     * Update facing direction to match movement
     * @param delta_time Time step
     */
    void UpdateRotation(float delta_time);
    
    /**
     * Update attack logic
     * @param delta_time Time step
     */
    void UpdateCombat(float delta_time);
    
    /**
     * Called when unit dies
     * Triggers death animation/effects
     */
    void OnDeath() override;
};

/**
 * Building GameObject - Static structure
 * 
 * Extends GameObject with:
 * - Construction progress
 * - Production queue
 * - Power consumption
 * - Area of effect abilities
 */
class Building : public GameObject {
public:
    /**
     * Building state enumeration
     */
    enum class BuildingState : uint8_t {
        CONSTRUCTING = 0,   // Being built
        OPERATIONAL = 1,    // Finished, operational
        DAMAGED = 2,        // Taking damage
        DESTROYED = 3,      // Destroyed
        SELLING = 4,        // Being sold
    };
    
    /**
     * Constructor
     * @param id Unique identifier
     * @param name Building name
     * @param position World position
     * @param build_time Time to construct (seconds)
     */
    Building(uint32_t id, const std::string& name, const Vector3& position, float build_time = 10.0f);
    
    virtual ~Building();
    
    // ----- Construction -----
    
    /**
     * Get construction progress [0, 1]
     * @return Progress from 0.0 (not started) to 1.0 (complete)
     */
    float GetBuildProgress() const;
    
    /**
     * Set construction progress
     * @param progress Progress [0, 1]
     */
    void SetBuildProgress(float progress);
    
    /**
     * Get construction state
     * @return Current building state
     */
    BuildingState GetBuildingState() const { return m_state; }
    
    /**
     * Finish construction
     * Sets progress to 1.0 and state to OPERATIONAL
     */
    void FinishConstruction();
    
    /**
     * Check if building is complete
     * @return True if construction finished
     */
    bool IsComplete() const { return m_state == BuildingState::OPERATIONAL; }
    
    // ----- Effects -----
    
    /**
     * Set area of effect radius
     * Used for damage, radar, repair range, etc
     * 
     * @param radius Radius in world units
     */
    void SetAreaOfEffectRadius(float radius) { m_aoe_radius = radius; }
    
    /**
     * Get area of effect radius
     * @return AoE radius in world units
     */
    float GetAreaOfEffectRadius() const { return m_aoe_radius; }
    
    // ----- Lifecycle -----
    
    /**
     * Update building state
     * Handles construction, animations
     * 
     * @param delta_time Time elapsed since last frame (seconds)
     */
    void Update(float delta_time) override;
    
    /**
     * Render building
     * @param camera Rendering camera
     */
    void Render(class Camera* camera) override;
    
protected:
    // Construction
    float m_build_progress;      // [0, 1] construction completion
    float m_build_time;          // Total build time in seconds
    BuildingState m_state;       // Current building state
    
    // Effects
    float m_aoe_radius;          // Area of effect radius
    
    /**
     * Update building construction
     * @param delta_time Time step
     */
    void UpdateConstruction(float delta_time);
    
    /**
     * Called when building construction completes
     */
    virtual void OnConstructionComplete();
};

/**
 * Effect GameObject - Temporary visual effect
 * 
 * Short-lived objects for visual feedback:
 * - Explosions
 * - Smoke
 * - Sparks
 * - Temporary models
 */
class Effect : public GameObject {
public:
    /**
     * Constructor
     * @param id Unique identifier
     * @param name Effect name
     * @param position World position
     * @param duration How long effect lasts (seconds)
     */
    Effect(uint32_t id, const std::string& name, const Vector3& position, float duration = 1.0f);
    
    virtual ~Effect();
    
    /**
     * Get time remaining
     * @return Seconds until effect expires
     */
    float GetTimeRemaining() const;
    
    /**
     * Check if effect has expired
     * @return True if duration elapsed
     */
    bool IsExpired() const { return m_time_remaining <= 0.0f; }
    
    /**
     * Update effect
     * Decrements timer, removes when expired
     * 
     * @param delta_time Time elapsed since last frame (seconds)
     */
    void Update(float delta_time) override;
    
    /**
     * Render effect
     * @param camera Rendering camera
     */
    void Render(class Camera* camera) override;

protected:
    float m_duration;           // Total effect duration (seconds)
    float m_time_remaining;     // Time left until expiration
};

} // namespace GeneralsX
