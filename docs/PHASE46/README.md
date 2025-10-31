# Phase 46: Game Logic Integration & Gameplay Connection

## Overview

Phase 46 integrates the graphics system with game logic, connecting rendering to actual game objects, units, and world state. This phase bridges the graphics pipeline created in Phases 40-45 with the game engine from Generals/Zero Hour.

**Status**: Planning/Ready to Implement  
**Depends on**: Phase 41 (Drawing), Phase 42 (Textures), Phase 43 (Render Loop), Phase 44 (Models), Phase 45 (Camera)  
**Estimated Duration**: 3-4 days  
**Complexity**: Very High (game engine integration, state management)

## Objectives

### Primary Goals

1. **Connect Game Objects to Rendering**
   - Map game units/structures to models
   - Handle object transforms
   - Manage lifecycle (creation/destruction)

2. **Implement Game Loop Integration**
   - Connect game update to render updates
   - Synchronize game state with graphics
   - Handle frame timing

3. **Implement Input & Control**
   - Keyboard input for camera control
   - Mouse input for unit selection
   - Basic unit commands

4. **Integrate Game Systems**
   - Map rendering with terrain
   - Weather effects
   - Particle systems
   - UI overlay

## Architecture

### Game Object Rendering Pipeline

```
Game Object (Unit/Structure)
├── Position & Rotation (world transform)
├── Animation State
├── Material Variations
├── Health/Damage State
└── Effects/Particles
    ↓
Graphics Rendering
├── Model Transform
├── Material Binding
├── Texture Coordinate Mapping
├── Lighting Calculations
└── Particle Rendering
    ↓
Screen Display
```

### Core Integration Points

```cpp
class GameObject {
    Vector3 position;
    Quaternion rotation;
    Model* model;
    Vector3 scale;
    
    // Game state
    float health;
    AnimationState animation;
    
    // Rendering
    void Render(const Camera& camera);
    Matrix4x4 GetWorldTransform() const;
};

class GameWorld {
    std::vector<GameObject*> objects;
    Camera* active_camera;
    
    void Update(float delta_time);
    void Render();
};
```

### Game Loop Integration

```
Main Game Loop
├── Update Phase
│   ├── Input handling
│   ├── Game logic update
│   ├── Object physics
│   └── Camera update
├── Render Phase
│   ├── BeginScene()
│   ├── For each game object:
│   │   ├── Get world transform
│   │   ├── Bind textures
│   │   └── Draw model
│   ├── Render UI
│   ├── EndScene()
│   └── Present()
└── Frame Timing
    └── Wait for frame rate
```

## Implementation Strategy

### Stage 1: Game Object Rendering (Day 1)

1. Create GameObject class with transform
2. Implement world transform calculation
3. Connect GameObject to Model system
4. Add simple rendering for objects

### Stage 2: Game Loop Integration (Day 1-2)

1. Integrate graphics render loop into game loop
2. Implement update/render phases
3. Handle frame timing
4. Synchronize game/graphics state

### Stage 3: Camera Integration (Day 2)

1. Implement camera attachment to game objects
2. Add camera orbit/follow logic
3. Handle camera input (mouse/keyboard)
4. Smooth camera transitions

### Stage 4: Input Handling (Day 2-3)

1. Implement keyboard input system
2. Implement mouse input system
3. Connect input to camera control
4. Connect input to unit selection

### Stage 5: Integration Testing (Day 3-4)

1. Load simple map with objects
2. Test rendering of multiple objects
3. Test camera control
4. Test input handling
5. Performance profiling

## GameObject System

```cpp
// Base game object class
class GameObject {
    uint32_t id;
    std::string name;
    
    // Transform
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
    
    // Model reference
    Model* model;
    uint32_t animation_state;
    
    // Game state
    enum ObjectType { UNIT, BUILDING, EFFECT };
    ObjectType type;
    
    // Health/status
    float health;
    float max_health;
    bool is_alive;
    
    // Methods
    virtual void Update(float dt);
    virtual void Render(const Camera& camera);
    virtual Matrix4x4 GetWorldTransform() const;
};

// Unit object (controllable)
class Unit : public GameObject {
    Vector3 move_target;
    float speed;
    
    void MoveTo(const Vector3& target);
    void Update(float dt) override;
};

// Building object (static)
class Building : public GameObject {
    float build_progress;
    
    void Update(float dt) override;
};
```

## Game World Management

```cpp
class GameWorld {
    std::unordered_map<uint32_t, GameObject*> objects;
    Camera* camera;
    Terrain* terrain;
    
    GameObject* CreateObject(ObjectType type, const Vector3& pos);
    void DestroyObject(uint32_t id);
    
    void Update(float dt);
    void Render();
    
    GameObject* GetObjectAt(const Vector3& pos);
    std::vector<GameObject*> GetObjectsInRadius(const Vector3& center, float radius);
};
```

## Input System

```cpp
class InputManager {
    // Keyboard state
    bool keys_pressed[256];
    
    // Mouse state
    Vector2 mouse_position;
    Vector2 mouse_delta;
    bool mouse_buttons[3];  // left, middle, right
    
    void ProcessInput();
    
    bool IsKeyPressed(int key_code);
    bool IsMouseButtonPressed(int button);
    Vector2 GetMouseDelta();
};
```

### Camera Control Input

```
Keyboard:
W/A/S/D - Move camera (WASD)
Q/E - Rotate camera
Space - Reset camera
Mouse:
Right drag - Rotate camera
Scroll - Zoom in/out
```

### Unit Selection Input

```
Mouse:
Left click - Select unit
Left drag - Multi-select (box)
Right click - Move/command
```

## Frame Timing

```cpp
struct FrameTiming {
    double frame_time;      // Total frame time
    double delta_time;      // Time since last frame
    double render_time;     // Time spent rendering
    uint32_t frame_number;
    
    double target_fps;      // Usually 60 FPS
    double target_frame_time = 1.0 / target_fps;
};

void UpdateFrameTiming(FrameTiming& timing) {
    // Cap frame rate
    if (frame_time < target_frame_time) {
        sleep(target_frame_time - frame_time);
    }
}
```

## Integration Points

### With Phase 41-42 (Drawing & Textures)
- Render calls routed through GameObject system
- Material/texture binding happens per-object

### With Phase 43 (Render Loop)
- Game loop wraps render loop
- BeginScene/EndScene called by game loop

### With Phase 44-45 (Models & Camera)
- Models stored in GameObjects
- Camera follows game objects

## World Coordinate System

```
Generals/Zero Hour uses:
- X-axis: East (right)
- Y-axis: Up
- Z-axis: North (away from player, typically)
- Units: 1 game unit ≈ 1 meter

Typical map size: 2000x2000 units (2km x 2km)
Object types:
- Buildings: Static at map start
- Units: Mobile, under player control
- Effects/Particles: Dynamic temporary
- Terrain: Heightmap + detail textures
```

## Testing Strategy

### Unit Tests

```cpp
// tests/test_gameobject.cpp
- Test GameObject creation/destruction
- Test world transform calculation
- Test object lifecycle

// tests/test_gameloop.cpp
- Test game update/render cycle
- Test frame timing
- Test input processing
```

### Integration Tests

```bash
# Load sample map with objects
./GeneralsXZH --map-test skirmish_map_01
# Verify:
# - Units render correctly
# - Camera follows units
# - Input controls work
# - Framerate stable at 60 FPS
```

### Gameplay Tests

1. Spawn 10 units, verify all render
2. Move camera, verify objects update
3. Select unit with mouse, verify selection
4. Command unit to move, verify animation plays
5. Destroy unit, verify removal from scene

## Performance Targets

```
Game world with 100 units:
- Update time: < 5ms
- Render time: < 10ms
- Total frame time: < 16.67ms (60 FPS)

Culling:
- Only render objects visible to camera
- Use camera frustum culling
- Distance culling for far objects
```

## Simple Test Map Structure

```
Test Map (2000x2000 units):
├── Barracks (Team 1) at (500, 0, 500)
├── Factory (Team 1) at (500, 0, 700)
├── War Factory (Team 1) at (500, 0, 900)
├── Barracks (Team 2) at (1500, 0, 500)
├── Factory (Team 2) at (1500, 0, 700)
├── Units spawned from Barracks
└── Terrain: Flat grassland with detail textures
```

## Next Phases

**Phase 47**: Testing & Stabilization - Comprehensive testing and profiling  
**Phase 48**: Minimal Playable Version - Main menu, map loading, complete integration

## References

- `docs/PHASE41/` - Drawing operations
- `docs/PHASE42/` - Texture system
- `docs/PHASE43/` - Render loop
- `docs/PHASE44/` - Model loading
- `docs/PHASE45/` - Camera system
- Game Engine Architecture (object management, game loops)
- Generals/Zero Hour game source documentation
