/**
 * GameWorld System - World object management and spatial queries
 * 
 * Manages all game objects in the world:
 * - Creation and destruction of objects
 * - Tracking active objects by type
 * - Spatial queries (point, radius, frustum)
 * - Update and render loops
 * - Collision detection queries
 * 
 * Architecture:
 * - Object storage: Map by ID + vectors by type
 * - Spatial partitioning: Quadtree for efficient queries (future)
 * - Update phase: All objects get Update() called
 * - Render phase: Visible objects get Render() called
 */

#pragma once

#include "../GameObject/gameobject.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <cstdint>

namespace GeneralsX {

class Camera;
class Terrain;

/**
 * GameWorld class - Manages all game entities
 * 
 * Central container for all game objects in the world. Handles:
 * - Object lifecycle (creation/destruction)
 * - Spatial queries (find objects in radius, frustum, etc)
 * - Update and render coordination
 * - Object type management (units, buildings, effects)
 * 
 * Threading: NOT thread-safe - updates must happen on single thread
 */
class GameWorld {
public:
    /**
     * Constructor
     * @param initial_capacity Hint for pre-allocation (default: 1000)
     */
    explicit GameWorld(size_t initial_capacity = 1000);
    
    /**
     * Destructor - cleans up all objects
     */
    ~GameWorld();
    
    // ----- Object Management -----
    
    /**
     * Create a unit in the world
     * @param name Unit name
     * @param position Spawn position
     * @param speed Movement speed (default: 10.0)
     * @return Newly created Unit* (owned by world)
     */
    Unit* CreateUnit(const std::string& name, const Vector3& position, float speed = 10.0f);
    
    /**
     * Create a building in the world
     * @param name Building name
     * @param position Build position
     * @param build_time Time to construct (default: 10.0)
     * @return Newly created Building* (owned by world)
     */
    Building* CreateBuilding(const std::string& name, const Vector3& position, float build_time = 10.0f);
    
    /**
     * Create an effect in the world
     * @param name Effect name
     * @param position Effect position
     * @param duration How long effect lasts (default: 1.0)
     * @return Newly created Effect* (owned by world)
     */
    Effect* CreateEffect(const std::string& name, const Vector3& position, float duration = 1.0f);
    
    /**
     * Create a generic game object
     * Internal method, use Create* variants above for specific types
     * 
     * @param name Object name
     * @param type Object type
     * @param position Object position
     * @return Newly created GameObject* (owned by world)
     */
    GameObject* CreateObject(const std::string& name, GameObject::ObjectType type, const Vector3& position);
    
    /**
     * Destroy object by ID
     * Object is removed from world, destroyed at end of frame
     * 
     * @param id Object ID to destroy
     * @return True if object was found and marked for destruction
     */
    bool DestroyObject(uint32_t id);
    
    /**
     * Get object by ID
     * @param id Object ID
     * @return GameObject* or nullptr if not found
     */
    GameObject* GetObjectByID(uint32_t id);
    
    /**
     * Get object by ID (const version)
     * @param id Object ID
     * @return const GameObject* or nullptr
     */
    const GameObject* GetObjectByID(uint32_t id) const;
    
    /**
     * Get all objects of a specific type
     * @param type Object type to find
     * @return Vector of GameObjects of that type
     */
    std::vector<GameObject*> GetObjectsByType(GameObject::ObjectType type);
    
    /**
     * Get all alive objects
     * @return Vector of alive GameObjects
     */
    std::vector<GameObject*> GetAliveObjects();
    
    /**
     * Get total number of objects
     * @return Count of all objects in world
     */
    size_t GetObjectCount() const { return m_objects.size(); }
    
    /**
     * Get count by type
     * @param type Object type
     * @return Number of objects of that type
     */
    size_t GetObjectCountByType(GameObject::ObjectType type) const;
    
    /**
     * Clear all objects from world
     * Warning: Destroys all objects immediately
     */
    void Clear();
    
    // ----- Spatial Queries -----
    
    /**
     * Find object at world position (ray query)
     * Returns first object found, prioritizing by render order
     * 
     * @param position Position to test
     * @param max_distance Maximum distance to consider
     * @return GameObject* or nullptr
     */
    GameObject* GetObjectAt(const Vector3& position, float max_distance = 2.0f);
    
    /**
     * Find all objects in radius
     * @param center Center point
     * @param radius Search radius
     * @return Vector of GameObjects within radius
     */
    std::vector<GameObject*> GetObjectsInRadius(const Vector3& center, float radius);
    
    /**
     * Find all objects in radius of specific type
     * @param center Center point
     * @param radius Search radius
     * @param type Filter by object type
     * @return Vector of typed GameObjects within radius
     */
    std::vector<GameObject*> GetObjectsInRadius(const Vector3& center, float radius, GameObject::ObjectType type);
    
    /**
     * Find objects in frustum (camera frustum culling)
     * @param frustum_planes Array of 6 frustum planes
     * @return Vector of GameObjects in frustum
     */
    std::vector<GameObject*> GetObjectsInFrustum(const Plane frustum_planes[6]);
    
    /**
     * Find objects in AABB (axis-aligned bounding box)
     * @param min Minimum corner
     * @param max Maximum corner
     * @return Vector of GameObjects in box
     */
    std::vector<GameObject*> GetObjectsInBox(const Vector3& min, const Vector3& max);
    
    /**
     * Line trace - find objects along a ray
     * @param start Ray start position
     * @param end Ray end position
     * @return Vector of GameObjects hit by ray, sorted by distance
     */
    std::vector<GameObject*> LineTrace(const Vector3& start, const Vector3& end);
    
    /**
     * Get nearest object to position
     * @param position Reference position
     * @param max_distance Maximum search distance (0 = unlimited)
     * @param type Filter by type (UNIT = any type)
     * @return Nearest GameObject or nullptr
     */
    GameObject* GetNearestObject(const Vector3& position, float max_distance = 0.0f, 
                                 GameObject::ObjectType type = static_cast<GameObject::ObjectType>(255));
    
    // ----- Unit-Specific Queries -----
    
    /**
     * Get all units (convenience method)
     * @return Vector of Unit pointers
     */
    std::vector<Unit*> GetAllUnits();
    
    /**
     * Get all buildings (convenience method)
     * @return Vector of Building pointers
     */
    std::vector<Building*> GetAllBuildings();
    
    /**
     * Get all effects (convenience method)
     * @return Vector of Effect pointers
     */
    std::vector<Effect*> GetAllEffects();
    
    // ----- Camera Management -----
    
    /**
     * Set active camera
     * @param camera Camera to use for rendering (not owned)
     */
    void SetActiveCamera(Camera* camera) { m_camera = camera; }
    
    /**
     * Get active camera
     * @return Current camera or nullptr
     */
    Camera* GetActiveCamera() const { return m_camera; }
    
    // ----- Terrain Management (Future) -----
    
    /**
     * Set terrain
     * @param terrain Terrain reference (not owned)
     */
    void SetTerrain(Terrain* terrain) { m_terrain = terrain; }
    
    /**
     * Get terrain
     * @return Current terrain or nullptr
     */
    Terrain* GetTerrain() const { return m_terrain; }
    
    // ----- Update & Render -----
    
    /**
     * Update all objects
     * Called once per frame before rendering
     * 
     * Updates:
     * - All alive objects via their Update() method
     * - Animation states
     * - Movement and physics
     * 
     * @param delta_time Time since last frame (seconds)
     */
    void Update(float delta_time);
    
    /**
     * Render all visible objects
     * Called once per frame after update
     * 
     * Renders:
     * - Objects within camera frustum
     * - Sorted by material/texture (for efficiency)
     * - Culled by distance if configured
     */
    void Render();
    
    /**
     * Perform cleanup on objects
     * Removes dead units, expired effects, etc
     * Called at end of frame or explicitly
     */
    void Cleanup();
    
    // ----- Statistics & Debug -----
    
    /**
     * Get debug statistics string
     * @return Formatted string with world stats
     */
    std::string GetDebugStats() const;
    
    /**
     * Draw debug visualization
     * Shows bounding volumes, object outlines, etc
     */
    void DrawDebug();

private:
    // Object storage
    uint32_t m_next_object_id;  // Counter for unique IDs
    std::unordered_map<uint32_t, std::unique_ptr<GameObject>> m_objects;  // All objects by ID
    
    // Type-specific storage (for efficient queries)
    std::vector<Unit*> m_units;
    std::vector<Building*> m_buildings;
    std::vector<Effect*> m_effects;
    
    // Objects pending destruction
    std::vector<uint32_t> m_pending_destruction;
    
    // References (not owned)
    Camera* m_camera;
    Terrain* m_terrain;
    
    // Optimization: Track frame stats
    struct FrameStats {
        size_t objects_updated = 0;
        size_t objects_rendered = 0;
        size_t objects_culled = 0;
    } m_frame_stats;
    
    /**
     * Internal method to add object to type-specific vectors
     * @param object Object to register
     */
    void RegisterObject(GameObject* object);
    
    /**
     * Internal method to remove object from type-specific vectors
     * @param object Object to unregister
     */
    void UnregisterObject(GameObject* object);
    
    /**
     * Internal method to process pending destruction
     */
    void ProcessDestruction();
    
    /**
     * Check if object is visible in camera frustum
     * @param object Object to check
     * @return True if visible
     */
    bool IsObjectVisible(const GameObject* object) const;

    // Prevent copying
    GameWorld(const GameWorld&) = delete;
    GameWorld& operator=(const GameWorld&) = delete;
};

} // namespace GeneralsX
