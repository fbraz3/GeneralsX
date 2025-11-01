/**
 * Game renderer implementation
 * 
 * Object rendering with culling and batching
 */

#include "game_renderer.h"
#include "../../GameEngine/Source/GameWorld/gameworld.h"
#include "../../GameEngine/Source/Camera/camera.h"
#include "../../GameEngine/Source/GameObject/gameobject.h"
#include <algorithm>
#include <sstream>
#include <cmath>

namespace GeneralsX {

GameRenderer::GameRenderer(GameWorld* world, Camera* camera)
    : m_world(world)
    , m_camera(camera)
    , m_initialized(false)
    , m_max_draw_distance(1000.0f)
    , m_frustum_culling_enabled(true)
    , m_distance_culling_enabled(true)
    , m_debug_overlays_enabled(false)
    , m_culled_count(0)
    , m_rendered_count(0)
    , m_batch_count(0)
{
}

GameRenderer::~GameRenderer() {
    Shutdown();
}

bool GameRenderer::Initialize() {
    if (m_initialized) return true;
    
    // TODO: Initialize graphics resources if needed
    // - Compile shaders
    // - Create render pipelines
    // - Allocate buffers
    
    m_initialized = true;
    return true;
}

void GameRenderer::Shutdown() {
    if (!m_initialized) return;
    
    // TODO: Clean up graphics resources
    // - Destroy pipelines
    // - Free buffers
    
    m_batches.clear();
    m_initialized = false;
}

// ----- Rendering -----

void GameRenderer::RenderGameWorld() {
    if (m_camera == nullptr || m_world == nullptr || !m_initialized) {
        return;
    }
    
    // Reset statistics
    m_culled_count = 0;
    m_rendered_count = 0;
    m_batch_count = 0;
    
    // Collect visible objects and create batches
    BuildRenderBatches();
    
    // Sort batches by material and distance
    SortRenderBatches();
    
    // Render each batch
    for (const RenderBatch& batch : m_batches) {
        RenderBatch(batch);
        m_batch_count++;
    }
    
    // Render debug overlays if enabled
    if (m_debug_overlays_enabled) {
        RenderDebugOverlays();
    }
}

void GameRenderer::RenderUnits() {
    std::vector<Unit*> units = m_world->GetAllUnits();
    
    for (Unit* unit : units) {
        if (!IsFrustumCulled(unit) && !IsDistanceCulled(unit)) {
            RenderObject(unit);
            m_rendered_count++;
        } else {
            m_culled_count++;
        }
    }
}

void GameRenderer::RenderBuildings() {
    std::vector<Building*> buildings = m_world->GetAllBuildings();
    
    for (Building* building : buildings) {
        if (!IsFrustumCulled(building) && !IsDistanceCulled(building)) {
            RenderObject(building);
            m_rendered_count++;
        } else {
            m_culled_count++;
        }
    }
}

void GameRenderer::RenderEffects() {
    std::vector<Effect*> effects = m_world->GetAllEffects();
    
    for (Effect* effect : effects) {
        if (!IsFrustumCulled(effect) && !IsDistanceCulled(effect)) {
            RenderObject(effect);
            m_rendered_count++;
        } else {
            m_culled_count++;
        }
    }
}

void GameRenderer::RenderDebugOverlays() {
    // TODO: Render debug visualization
    // - Bounding boxes
    // - Coordinate axes
    // - Grid
}

void GameRenderer::RenderSelectionHighlights(const std::vector<Unit*>& selected_units) {
    // TODO: Render selection highlights for units
    // - Green outline
    // - Selection circle
    // - Unit info text
    for (Unit* unit : selected_units) {
        // Draw selection indicator around unit
    }
}

void GameRenderer::RenderHealthBars(const std::vector<GameObject*>& objects) {
    // TODO: Render health bars above objects
    // - Red bar for health
    // - Green text for numbers
    // - Position above unit head
    for (GameObject* obj : objects) {
        float health_percent = obj->GetHealthPercentage();
        // Draw bar at obj->GetPosition() + (0, obj_height, 0)
    }
}

// ----- Culling -----

bool GameRenderer::IsFrustumCulled(const GameObject* obj) const {
    if (!m_frustum_culling_enabled || m_camera == nullptr) {
        return false;
    }
    
    // TODO: Implement frustum culling using camera frustum planes
    // For now, always visible
    return false;
}

bool GameRenderer::IsDistanceCulled(const GameObject* obj) const {
    if (!m_distance_culling_enabled || m_camera == nullptr) {
        return false;
    }
    
    // Check distance from camera
    float distance = (obj->GetPosition() - m_camera->GetPosition()).Length();
    return distance > m_max_draw_distance;
}

std::vector<GameObject*> GameRenderer::GetVisibleObjects() {
    std::vector<GameObject*> visible;
    
    // Get all objects
    std::vector<GameObject*> all_objects = m_world->GetAliveObjects();
    
    // Filter by culling tests
    for (GameObject* obj : all_objects) {
        if (!IsFrustumCulled(obj) && !IsDistanceCulled(obj)) {
            visible.push_back(obj);
        }
    }
    
    return visible;
}

// ----- Settings -----

void GameRenderer::SetMaxDrawDistance(float distance) {
    if (distance > 0.0f) {
        m_max_draw_distance = distance;
    }
}

// ----- Statistics -----

std::string GameRenderer::GetDebugStats() const {
    std::stringstream ss;
    ss << "Game Renderer Statistics:" << std::endl;
    ss << "  Rendered Objects: " << m_rendered_count << std::endl;
    ss << "  Culled Objects: " << m_culled_count << std::endl;
    ss << "  Render Batches: " << m_batch_count << std::endl;
    ss << "  Max Draw Distance: " << m_max_draw_distance << std::endl;
    ss << "  Frustum Culling: " << (m_frustum_culling_enabled ? "Enabled" : "Disabled") << std::endl;
    ss << "  Distance Culling: " << (m_distance_culling_enabled ? "Enabled" : "Disabled") << std::endl;
    return ss.str();
}

// ----- Private Methods -----

void GameRenderer::BuildRenderBatches() {
    m_batches.clear();
    
    // Get all visible objects
    std::vector<GameObject*> visible_objects = GetVisibleObjects();
    
    // Group by material
    std::unordered_map<Material*, std::vector<GameObject*>> material_batches;
    
    for (GameObject* obj : visible_objects) {
        // Get object's material
        Material* material = obj->GetMaterial();
        if (material == nullptr) {
            // Use default material if none specified
            material = nullptr;  // TODO: Get default material
        }
        
        material_batches[material].push_back(obj);
        m_rendered_count++;
    }
    
    // Create render batches
    for (auto& pair : material_batches) {
        RenderBatch batch(pair.first);
        batch.objects = pair.second;
        m_batches.push_back(batch);
    }
    
    m_batch_count = m_batches.size();
}

void GameRenderer::RenderBatch(const RenderBatch& batch) {
    // TODO: Bind material and render all objects in batch
    // 1. Bind material shader
    // 2. Bind material textures
    // 3. For each object:
    //    a. Set world transform
    //    b. Draw model
    
    for (GameObject* obj : batch.objects) {
        RenderObject(obj);
    }
}

void GameRenderer::RenderObject(GameObject* obj) {
    if (obj->GetModel() == nullptr) {
        return;
    }
    
    // TODO: Actually render object
    // 1. Get world transform from GameObject
    // 2. Bind model buffers
    // 3. Draw model with camera view/projection matrices
}

void GameRenderer::SortRenderBatches() {
    // TODO: Sort batches by:
    // 1. Material (to minimize state changes)
    // 2. Distance (front to back for depth prepass)
    
    // For now, just reverse sort by distance to back for transparency
    if (m_camera != nullptr) {
        std::sort(m_batches.begin(), m_batches.end(),
            [this](const RenderBatch& a, const RenderBatch& b) {
                float dist_a = 0, dist_b = 0;
                
                if (!a.objects.empty()) {
                    dist_a = (a.objects[0]->GetPosition() - m_camera->GetPosition()).Length();
                }
                if (!b.objects.empty()) {
                    dist_b = (b.objects[0]->GetPosition() - m_camera->GetPosition()).Length();
                }
                
                return dist_a > dist_b;  // Back to front for transparency
            }
        );
    }
}

} // namespace GeneralsX
