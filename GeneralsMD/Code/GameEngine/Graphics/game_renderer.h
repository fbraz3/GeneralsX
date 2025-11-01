/**
 * Game Renderer - Renders all game objects with proper ordering and culling
 * 
 * Coordinates rendering of:
 * - Game objects (units, buildings, effects)
 * - Terrain and environment
 * - Particle systems
 * - UI overlays
 * 
 * Features:
 * - Frustum culling (camera visibility)
 * - Distance culling (fog of war, far clipping)
 * - Material batching (render efficiency)
 * - Depth sorting (transparency handling)
 * - Shadow rendering
 */

#pragma once

#include "../../Math/math.h"
#include <vector>
#include <memory>
#include <cstdint>

namespace GeneralsX {

class GameWorld;
class Camera;
class GameObject;
class Unit;
class Building;
class Model;
class Texture;

/**
 * Render batch - Groups objects with same material for efficient rendering
 */
struct RenderBatch {
    Material* material;
    std::vector<GameObject*> objects;
    
    RenderBatch(Material* mat = nullptr) : material(mat) {}
};

/**
 * Game renderer - Manages rendering of game world
 * 
 * Responsibilities:
 * - Frustum culling for visibility determination
 * - Material/texture batching for efficiency
 * - Depth sorting for transparency
 * - Render state management
 * - Performance optimization
 */
class GameRenderer {
public:
    /**
     * Constructor
     * @param world GameWorld instance (not owned)
     * @param camera Active camera (not owned)
     */
    GameRenderer(GameWorld* world, Camera* camera);
    
    /**
     * Destructor
     */
    ~GameRenderer();
    
    /**
     * Initialize renderer
     * @return True if successful
     */
    bool Initialize();
    
    /**
     * Shutdown renderer
     */
    void Shutdown();
    
    // ----- Rendering -----
    
    /**
     * Render all visible game objects
     * 
     * Process:
     * 1. Collect visible objects (frustum culling)
     * 2. Sort by material/distance
     * 3. Bind material state
     * 4. Draw batches
     * 5. Render transparency layer
     */
    void RenderGameWorld();
    
    /**
     * Render units only
     */
    void RenderUnits();
    
    /**
     * Render buildings only
     */
    void RenderBuildings();
    
    /**
     * Render effects (explosions, particles, etc)
     */
    void RenderEffects();
    
    /**
     * Render debug overlays (bounding boxes, health bars, selection)
     */
    void RenderDebugOverlays();
    
    /**
     * Render unit selection highlights
     * @param selected_units Units to highlight
     */
    void RenderSelectionHighlights(const std::vector<Unit*>& selected_units);
    
    /**
     * Render health bars above objects
     * @param objects Objects to show health for
     */
    void RenderHealthBars(const std::vector<GameObject*>& objects);
    
    // ----- Culling -----
    
    /**
     * Check if object is visible in camera frustum
     * @param obj Object to test
     * @return True if visible
     */
    bool IsFrustumCulled(const GameObject* obj) const;
    
    /**
     * Check if object is distance culled
     * @param obj Object to test
     * @return True if culled by distance
     */
    bool IsDistanceCulled(const GameObject* obj) const;
    
    /**
     * Get visible objects after culling
     * @return Vector of visible game objects
     */
    std::vector<GameObject*> GetVisibleObjects();
    
    // ----- Settings -----
    
    /**
     * Set maximum draw distance
     * @param distance Objects beyond this are not rendered
     */
    void SetMaxDrawDistance(float distance);
    
    /**
     * Get maximum draw distance
     * @return Current max distance
     */
    float GetMaxDrawDistance() const { return m_max_draw_distance; }
    
    /**
     * Enable/disable frustum culling
     * @param enabled Culling state
     */
    void SetFrustumCullingEnabled(bool enabled) { m_frustum_culling_enabled = enabled; }
    
    /**
     * Enable/disable distance culling
     * @param enabled Culling state
     */
    void SetDistanceCullingEnabled(bool enabled) { m_distance_culling_enabled = enabled; }
    
    /**
     * Enable/disable debug overlays
     * @param enabled Debug state
     */
    void SetDebugOverlaysEnabled(bool enabled) { m_debug_overlays_enabled = enabled; }
    
    // ----- Statistics -----
    
    /**
     * Get objects culled last frame
     * @return Count of culled objects
     */
    uint32_t GetCulledObjectCount() const { return m_culled_count; }
    
    /**
     * Get objects rendered last frame
     * @return Count of rendered objects
     */
    uint32_t GetRenderedObjectCount() const { return m_rendered_count; }
    
    /**
     * Get render batches last frame
     * @return Number of batches rendered
     */
    uint32_t GetRenderBatchCount() const { return m_batch_count; }
    
    /**
     * Get debug statistics
     * @return Formatted string with render stats
     */
    std::string GetDebugStats() const;

private:
    // Systems (not owned)
    GameWorld* m_world;
    Camera* m_camera;
    
    // Rendering state
    bool m_initialized;
    std::vector<RenderBatch> m_batches;
    
    // Culling settings
    float m_max_draw_distance;
    bool m_frustum_culling_enabled;
    bool m_distance_culling_enabled;
    bool m_debug_overlays_enabled;
    
    // Statistics
    uint32_t m_culled_count;
    uint32_t m_rendered_count;
    uint32_t m_batch_count;
    
    /**
     * Collect visible objects and create render batches
     */
    void BuildRenderBatches();
    
    /**
     * Render single render batch
     * @param batch Batch to render
     */
    void RenderBatch(const RenderBatch& batch);
    
    /**
     * Render single object
     * @param obj Object to render
     */
    void RenderObject(GameObject* obj);
    
    /**
     * Sort objects by material and distance for optimal rendering
     */
    void SortRenderBatches();

    // Prevent copying
    GameRenderer(const GameRenderer&) = delete;
    GameRenderer& operator=(const GameRenderer&) = delete;
};

} // namespace GeneralsX
