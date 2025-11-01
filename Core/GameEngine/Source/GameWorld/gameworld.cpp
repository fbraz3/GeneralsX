/**
 * GameWorld implementation
 * 
 * Manages object lifecycle, spatial queries, and rendering coordination
 */

#include "gameworld.h"
#include "../Camera/camera.h"
#include <algorithm>
#include <sstream>
#include <cmath>

namespace GeneralsX {

GameWorld::GameWorld(size_t initial_capacity)
    : m_next_object_id(1)
    , m_camera(nullptr)
    , m_terrain(nullptr)
{
    m_objects.reserve(initial_capacity);
    m_units.reserve(initial_capacity / 4);
    m_buildings.reserve(initial_capacity / 10);
    m_effects.reserve(initial_capacity / 2);
}

GameWorld::~GameWorld() {
    Clear();
}

// ----- Object Management -----

Unit* GameWorld::CreateUnit(const std::string& name, const Vector3& position, float speed) {
    auto unit = std::make_unique<Unit>(m_next_object_id++, name, position, speed);
    Unit* unit_ptr = unit.get();
    m_objects[unit_ptr->GetID()] = std::move(unit);
    RegisterObject(unit_ptr);
    return unit_ptr;
}

Building* GameWorld::CreateBuilding(const std::string& name, const Vector3& position, float build_time) {
    auto building = std::make_unique<Building>(m_next_object_id++, name, position, build_time);
    Building* building_ptr = building.get();
    m_objects[building_ptr->GetID()] = std::move(building);
    RegisterObject(building_ptr);
    return building_ptr;
}

Effect* GameWorld::CreateEffect(const std::string& name, const Vector3& position, float duration) {
    auto effect = std::make_unique<Effect>(m_next_object_id++, name, position, duration);
    Effect* effect_ptr = effect.get();
    m_objects[effect_ptr->GetID()] = std::move(effect);
    RegisterObject(effect_ptr);
    return effect_ptr;
}

GameObject* GameWorld::CreateObject(const std::string& name, GameObject::ObjectType type, const Vector3& position) {
    switch (type) {
        case GameObject::ObjectType::UNIT:
            return CreateUnit(name, position);
        case GameObject::ObjectType::BUILDING:
            return CreateBuilding(name, position);
        case GameObject::ObjectType::EFFECT:
            return CreateEffect(name, position);
        default:
            // Generic object
            auto obj = std::make_unique<GameObject>(m_next_object_id++, name, type, position);
            GameObject* obj_ptr = obj.get();
            m_objects[obj_ptr->GetID()] = std::move(obj);
            RegisterObject(obj_ptr);
            return obj_ptr;
    }
}

bool GameWorld::DestroyObject(uint32_t id) {
    auto it = m_objects.find(id);
    if (it != m_objects.end()) {
        m_pending_destruction.push_back(id);
        return true;
    }
    return false;
}

GameObject* GameWorld::GetObjectByID(uint32_t id) {
    auto it = m_objects.find(id);
    if (it != m_objects.end()) {
        return it->second.get();
    }
    return nullptr;
}

const GameObject* GameWorld::GetObjectByID(uint32_t id) const {
    auto it = m_objects.find(id);
    if (it != m_objects.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<GameObject*> GameWorld::GetObjectsByType(GameObject::ObjectType type) {
    std::vector<GameObject*> result;
    
    for (auto& pair : m_objects) {
        if (pair.second->GetType() == type) {
            result.push_back(pair.second.get());
        }
    }
    
    return result;
}

std::vector<GameObject*> GameWorld::GetAliveObjects() {
    std::vector<GameObject*> result;
    
    for (auto& pair : m_objects) {
        if (pair.second->IsAlive()) {
            result.push_back(pair.second.get());
        }
    }
    
    return result;
}

size_t GameWorld::GetObjectCountByType(GameObject::ObjectType type) const {
    switch (type) {
        case GameObject::ObjectType::UNIT:
            return m_units.size();
        case GameObject::ObjectType::BUILDING:
            return m_buildings.size();
        case GameObject::ObjectType::EFFECT:
            return m_effects.size();
        default:
            return 0;
    }
}

void GameWorld::Clear() {
    m_objects.clear();
    m_units.clear();
    m_buildings.clear();
    m_effects.clear();
    m_pending_destruction.clear();
    m_next_object_id = 1;
}

// ----- Spatial Queries -----

GameObject* GameWorld::GetObjectAt(const Vector3& position, float max_distance) {
    GameObject* closest = nullptr;
    float closest_distance = max_distance;
    
    for (auto& pair : m_objects) {
        GameObject* obj = pair.second.get();
        float distance = (obj->GetPosition() - position).Length();
        
        if (distance < closest_distance && distance <= obj->GetBoundingRadius()) {
            closest = obj;
            closest_distance = distance;
        }
    }
    
    return closest;
}

std::vector<GameObject*> GameWorld::GetObjectsInRadius(const Vector3& center, float radius) {
    std::vector<GameObject*> result;
    
    for (auto& pair : m_objects) {
        GameObject* obj = pair.second.get();
        float distance = (obj->GetPosition() - center).Length();
        
        if (distance <= radius + obj->GetBoundingRadius()) {
            result.push_back(obj);
        }
    }
    
    return result;
}

std::vector<GameObject*> GameWorld::GetObjectsInRadius(const Vector3& center, float radius, GameObject::ObjectType type) {
    std::vector<GameObject*> result;
    
    for (auto& pair : m_objects) {
        GameObject* obj = pair.second.get();
        if (obj->GetType() != type) continue;
        
        float distance = (obj->GetPosition() - center).Length();
        if (distance <= radius + obj->GetBoundingRadius()) {
            result.push_back(obj);
        }
    }
    
    return result;
}

std::vector<GameObject*> GameWorld::GetObjectsInFrustum(const Plane frustum_planes[6]) {
    std::vector<GameObject*> result;
    
    for (auto& pair : m_objects) {
        GameObject* obj = pair.second.get();
        
        // Test sphere against all frustum planes
        Vector3 obj_pos = obj->GetPosition();
        float radius = obj->GetBoundingRadius();
        bool in_frustum = true;
        
        for (int i = 0; i < 6; i++) {
            float distance = frustum_planes[i].GetDistanceToPoint(obj_pos);
            if (distance < -radius) {
                in_frustum = false;
                break;
            }
        }
        
        if (in_frustum) {
            result.push_back(obj);
        }
    }
    
    return result;
}

std::vector<GameObject*> GameWorld::GetObjectsInBox(const Vector3& min, const Vector3& max) {
    std::vector<GameObject*> result;
    
    for (auto& pair : m_objects) {
        GameObject* obj = pair.second.get();
        Vector3 obj_pos = obj->GetPosition();
        
        // Simple point-in-box test (expanded by bounding radius)
        float radius = obj->GetBoundingRadius();
        if (obj_pos.x >= min.x - radius && obj_pos.x <= max.x + radius &&
            obj_pos.y >= min.y - radius && obj_pos.y <= max.y + radius &&
            obj_pos.z >= min.z - radius && obj_pos.z <= max.z + radius) {
            result.push_back(obj);
        }
    }
    
    return result;
}

std::vector<GameObject*> GameWorld::LineTrace(const Vector3& start, const Vector3& end) {
    std::vector<GameObject*> result;
    Vector3 ray_dir = (end - start).Normalize();
    float ray_length = (end - start).Length();
    
    for (auto& pair : m_objects) {
        GameObject* obj = pair.second.get();
        Vector3 to_obj = obj->GetPosition() - start;
        
        // Project object onto ray
        float projection = Vector3::Dot(to_obj, ray_dir);
        
        // Check if projection is on the ray
        if (projection >= 0.0f && projection <= ray_length) {
            // Check distance from ray
            Vector3 closest_point = start + ray_dir * projection;
            float distance = (obj->GetPosition() - closest_point).Length();
            
            if (distance <= obj->GetBoundingRadius()) {
                result.push_back(obj);
            }
        }
    }
    
    // Sort by distance from start
    std::sort(result.begin(), result.end(), [start](GameObject* a, GameObject* b) {
        return (a->GetPosition() - start).Length() < (b->GetPosition() - start).Length();
    });
    
    return result;
}

GameObject* GameWorld::GetNearestObject(const Vector3& position, float max_distance, GameObject::ObjectType type) {
    GameObject* nearest = nullptr;
    float nearest_distance = max_distance > 0.0f ? max_distance : 1e6f;
    
    for (auto& pair : m_objects) {
        GameObject* obj = pair.second.get();
        
        // Type filter
        if (type != static_cast<GameObject::ObjectType>(255) && obj->GetType() != type) {
            continue;
        }
        
        float distance = (obj->GetPosition() - position).Length();
        if (distance < nearest_distance) {
            nearest = obj;
            nearest_distance = distance;
        }
    }
    
    return nearest;
}

// ----- Unit-Specific Queries -----

std::vector<Unit*> GameWorld::GetAllUnits() {
    return m_units;
}

std::vector<Building*> GameWorld::GetAllBuildings() {
    return m_buildings;
}

std::vector<Effect*> GameWorld::GetAllEffects() {
    return m_effects;
}

// ----- Update & Render -----

void GameWorld::Update(float delta_time) {
    m_frame_stats.objects_updated = 0;
    
    for (auto& pair : m_objects) {
        GameObject* obj = pair.second.get();
        obj->Update(delta_time);
        m_frame_stats.objects_updated++;
    }
    
    // Process any destruction that occurred during update
    ProcessDestruction();
}

void GameWorld::Render() {
    if (m_camera == nullptr) {
        return;  // Can't render without camera
    }
    
    m_frame_stats.objects_rendered = 0;
    m_frame_stats.objects_culled = 0;
    
    for (auto& pair : m_objects) {
        GameObject* obj = pair.second.get();
        
        if (IsObjectVisible(obj)) {
            obj->Render(m_camera);
            m_frame_stats.objects_rendered++;
        } else {
            m_frame_stats.objects_culled++;
        }
    }
}

void GameWorld::Cleanup() {
    ProcessDestruction();
    
    // Also clean up expired effects
    auto effects_copy = m_effects;
    for (Effect* effect : effects_copy) {
        if (effect->IsExpired()) {
            DestroyObject(effect->GetID());
        }
    }
    
    ProcessDestruction();
}

// ----- Statistics & Debug -----

std::string GameWorld::GetDebugStats() const {
    std::stringstream ss;
    ss << "GameWorld Statistics:" << std::endl;
    ss << "  Total Objects: " << m_objects.size() << std::endl;
    ss << "    Units: " << m_units.size() << std::endl;
    ss << "    Buildings: " << m_buildings.size() << std::endl;
    ss << "    Effects: " << m_effects.size() << std::endl;
    ss << "  Last Frame:" << std::endl;
    ss << "    Updated: " << m_frame_stats.objects_updated << std::endl;
    ss << "    Rendered: " << m_frame_stats.objects_rendered << std::endl;
    ss << "    Culled: " << m_frame_stats.objects_culled << std::endl;
    return ss.str();
}

void GameWorld::DrawDebug() {
    // TODO: Draw debug visualization for all objects
}

// ----- Private Methods -----

void GameWorld::RegisterObject(GameObject* object) {
    switch (object->GetType()) {
        case GameObject::ObjectType::UNIT:
            m_units.push_back(dynamic_cast<Unit*>(object));
            break;
        case GameObject::ObjectType::BUILDING:
            m_buildings.push_back(dynamic_cast<Building*>(object));
            break;
        case GameObject::ObjectType::EFFECT:
            m_effects.push_back(dynamic_cast<Effect*>(object));
            break;
        default:
            break;
    }
}

void GameWorld::UnregisterObject(GameObject* object) {
    switch (object->GetType()) {
        case GameObject::ObjectType::UNIT: {
            auto it = std::find(m_units.begin(), m_units.end(), dynamic_cast<Unit*>(object));
            if (it != m_units.end()) {
                m_units.erase(it);
            }
            break;
        }
        case GameObject::ObjectType::BUILDING: {
            auto it = std::find(m_buildings.begin(), m_buildings.end(), dynamic_cast<Building*>(object));
            if (it != m_buildings.end()) {
                m_buildings.erase(it);
            }
            break;
        }
        case GameObject::ObjectType::EFFECT: {
            auto it = std::find(m_effects.begin(), m_effects.end(), dynamic_cast<Effect*>(object));
            if (it != m_effects.end()) {
                m_effects.erase(it);
            }
            break;
        }
        default:
            break;
    }
}

void GameWorld::ProcessDestruction() {
    for (uint32_t id : m_pending_destruction) {
        auto it = m_objects.find(id);
        if (it != m_objects.end()) {
            UnregisterObject(it->second.get());
            m_objects.erase(it);
        }
    }
    m_pending_destruction.clear();
}

bool GameWorld::IsObjectVisible(const GameObject* object) const {
    if (!object->IsVisible()) {
        return false;
    }
    
    // If we have a camera, do frustum culling
    if (m_camera != nullptr) {
        // TODO: Use camera frustum culling
        // For now, always visible
    }
    
    return true;
}

} // namespace GeneralsX
