# Phase 48: Minimal Playable Version - Complete Integration

## Overview

Phase 48 represents the first minimal playable version of GeneralsX. This phase integrates all previous work (Phases 40-47) into a complete, cohesive experience: launching a main menu, loading a map, rendering the game world, and enabling basic unit control.

**Status**: Planning/Final Integration Phase  
**Depends on**: Phase 40-47 (All graphics and game logic systems)  
**Estimated Duration**: 2-3 days  
**Complexity**: High (integration, system coordination)

## Objectives

### Primary Goals

1. **Main Menu System**
   - Menu rendering in UI space
   - Skirmish/Campaign selection
   - Settings options
   - Game mode selection

2. **Map Loading Pipeline**
   - Parse map files from archives
   - Create game world from map data
   - Spawn initial units/buildings
   - Initialize terrain

3. **Core Gameplay Systems**
   - Unit spawning and control
   - Basic unit movement
   - Building placement/destruction
   - Game state management

4. **Complete Integration**
   - Main menu → Map loading → Gameplay
   - All systems working together
   - Stable frame rate maintained
   - Clean shutdown

## Architecture

### Application Lifecycle

```
Application Start
├── Initialize Engine
│   ├── Create window
│   ├── Initialize Vulkan/DXVK
│   ├── Load assets (textures, models)
│   └── Create UI system
├── Main Menu State
│   ├── Render menu
│   ├── Process input
│   └── Wait for selection
├── Map Loading State
│   ├── Load map data
│   ├── Create game world
│   ├── Spawn initial objects
│   └── Initialize camera
├── Gameplay State
│   ├── Game loop
│   │   ├── Update game logic
│   │   ├── Process input
│   │   ├── Render world
│   │   └── Present frame
│   └── Until game ends
└── Cleanup & Exit
    ├── Save game state (optional)
    ├── Destroy game world
    ├── Shutdown Vulkan
    └── Close window
```

### Core Game States

```cpp
enum GameState {
    STATE_INIT,          // Initializing
    STATE_MAIN_MENU,     // Menu display
    STATE_MAP_SELECT,    // Map selection
    STATE_SETTINGS,      // Settings menu
    STATE_LOADING_MAP,   // Loading map
    STATE_GAMEPLAY,      // Active gameplay
    STATE_PAUSED,        // Game paused
    STATE_GAME_OVER,     // Game ended
    STATE_SHUTDOWN       // Closing application
};

class GameApplication {
    GameState current_state;
    
    void Update(float delta_time);
    void Render();
    void ChangeState(GameState new_state);
};
```

## Implementation Strategy

### Stage 1: Core Application Structure (Day 1)

1. Create main application class
2. Implement state machine
3. Create window/context management
4. Implement basic main loop

### Stage 2: Main Menu (Day 1)

1. Create menu UI layout
2. Implement menu rendering
3. Add input handling for menu
4. Connect to game states

### Stage 3: Map Loading (Day 1-2)

1. Implement map parser
2. Create world initialization
3. Spawn starting units/buildings
4. Initialize camera position

### Stage 4: Gameplay Integration (Day 2)

1. Connect all game systems
2. Implement basic unit control
3. Implement basic building system
4. Test complete flow

### Stage 5: Polish & Integration Testing (Day 2-3)

1. Fix integration issues
2. Optimize performance
3. Clean up shutdown
4. Final testing

## Main Menu Design

### Menu Structure

```
Main Menu
├── Play (lead to map select)
├── Settings
│   ├── Graphics
│   │   ├── Resolution
│   │   ├── Graphics Quality
│   │   └── Fullscreen toggle
│   ├── Audio
│   │   ├── Master Volume
│   │   └── Music Volume
│   └── Controls
│       └── Key binding display
├── Credits
└── Exit
```

### UI Rendering

```cpp
class UIRenderer {
    void DrawButton(const Rect& rect, const std::string& label);
    void DrawText(const Vector2& pos, const std::string& text);
    void DrawPanel(const Rect& rect);
};
```

## Map System

### Map Structure

```
Map File (.map extension, in archive)
├── Map Header
│   ├── Dimensions (width x height)
│   ├── Terrain type
│   └── Weather
├── Terrain Data
│   ├── Height map
│   ├── Texture indices
│   └── Detail textures
├── Object Placement
│   ├── Buildings positions
│   ├── Tree/rock positions
│   └── Waypoint positions
└── Team Setup
    ├── Player start positions
    ├── Initial units/buildings
    └── AI opponents (if applicable)
```

### Map Loading Pipeline

```cpp
class MapLoader {
    Map* LoadMap(const std::string& map_path);
    
    // Steps:
    // 1. Open map file from archive
    // 2. Parse map header
    // 3. Load terrain data
    // 4. Create terrain GPU resources
    // 5. Place objects in world
    // 6. Initialize teams/players
};

class GameWorld {
    Terrain* terrain;
    std::vector<GameObject*> objects;
    Camera camera;
    
    void InitializeFromMap(const Map& map);
    void SpawnStartingUnits();
};
```

## Gameplay Systems

### Unit Control System

```cpp
class UnitController {
    Unit* selected_unit;
    
    void SelectUnit(Unit* unit);
    void DeselectAll();
    void IssueMoveCommand(const Vector3& target);
    void IssueAttackCommand(Unit* target);
    
    void Update(float dt);
    void Render(const Camera& camera);
};
```

### Building System (Basic)

```cpp
class Building : public GameObject {
    enum BuildingType { BARRACKS, FACTORY, POWER_PLANT };
    BuildingType type;
    
    float build_time;
    float current_build_progress;
    
    void ProduceUnit(UnitType type);
    void Update(float dt) override;
};
```

### Resource System (Simple)

```cpp
class PlayerResources {
    uint32_t money;
    uint32_t power;
    
    bool CanBuild(const Building& building);
    void SpendResources(uint32_t amount);
    void GainResources(uint32_t amount);
};
```

## Sample Skirmish Map

### "Valley Battle" (1024x1024 units)

```
Team 1 (Player)
├── Start position: (256, 0, 256)
├── Starting buildings:
│   ├── Barracks (produces infantry)
│   ├── War Factory (produces vehicles)
│   └── Power Plant (provides power)
└── Starting units:
    ├── 5 Rangers (basic infantry)
    └── 2 Workers (build units)

Team 2 (Computer - not AI yet, just static)
├── Start position: (768, 0, 768)
├── Mirror setup to Team 1
└── Inactive (future: implement AI)

Terrain:
├── Grassland with paths
├── River down the middle (impassable)
├── Trees and rocks scattered
└── Height variation: ±20 units
```

## Complete Game Loop

```cpp
class GeneralsXGame {
    GameState state;
    GameWorld world;
    UIRenderer ui;
    InputManager input;
    Camera camera;
    
    void Run() {
        while (state != STATE_SHUTDOWN) {
            // Input phase
            input.ProcessInput();
            
            // Update phase
            Update(delta_time);
            
            // Render phase
            BeginScene();
            Render();
            EndScene();
            Present();
        }
    }
    
    void Update(float dt) {
        switch (state) {
            case STATE_MAIN_MENU:
                UpdateMainMenu(dt);
                break;
            case STATE_GAMEPLAY:
                UpdateGameplay(dt);
                break;
            // ... other states ...
        }
    }
    
    void Render() {
        switch (state) {
            case STATE_MAIN_MENU:
                RenderMainMenu();
                break;
            case STATE_GAMEPLAY:
                RenderGameplay();
                break;
            // ... other states ...
        }
    }
};
```

### Gameplay Update Loop

```
Gameplay Update:
├── Update camera position
├── Update selected units
├── Update all game objects
│   ├── Move units toward targets
│   ├── Update animations
│   ├── Check collisions
│   └── Handle interactions
├── Update building production
└── Check win/lose conditions
```

### Gameplay Render Loop

```
Gameplay Render:
├── Clear screen
├── Render terrain
├── Render all game objects
│   ├── Set world transform
│   ├── Bind textures
│   ├── Draw model
│   └── Render effects
├── Render UI overlay
│   ├── Selected unit info
│   ├── Minimap
│   ├── Resource display
│   └── Controls hint
└── Present to screen
```

## Input Handling

### Keyboard Controls

```
In-game Controls:
W/A/S/D       - Move camera
Space         - Center on selected unit
P             - Pause/Unpause
ESC           - Pause menu
1-9           - Unit groups
Ctrl+1-9      - Save unit group
Click + drag  - Select multiple units
Right-click   - Issue move command
```

## Configuration System

### Settings File (INI format)

```ini
[Graphics]
Resolution=1920x1080
Fullscreen=true
TextureQuality=high
ShadowQuality=high
MaxFrameRate=60

[Audio]
MasterVolume=80
MusicVolume=60
SoundEffectsVolume=100

[Gameplay]
UnitGrouping=enabled
CameraSpeed=1.0
ScrollSpeed=1.0
```

## Integration Checklist

### Before Phase 48 Completion

- ✅ All Phase 40-47 systems working
- ✅ Main menu renders and responds to input
- ✅ Map loading works without crashes
- ✅ Game world initializes correctly
- ✅ Units spawn and render correctly
- ✅ Camera follows player input
- ✅ Units move to clicked positions
- ✅ Game runs at 60 FPS consistently
- ✅ Clean exit without memory leaks
- ✅ No Vulkan validation errors

## Success Criteria

### Functional Requirements

- ✅ Main menu displays
- ✅ Can select and load map
- ✅ Game world loads
- ✅ Multiple units render
- ✅ Can select units with mouse
- ✅ Can issue move commands
- ✅ Units move to target positions
- ✅ Can exit to menu gracefully

### Performance Requirements

- ✅ Consistent 60 FPS gameplay
- ✅ Menu responsive to input
- ✅ Map load time < 5 seconds
- ✅ No frame rate drops

### Stability Requirements

- ✅ No crashes during normal gameplay
- ✅ No memory leaks detected
- ✅ Can play for 30 minutes without issue
- ✅ Clean shutdown

## Testing Checklist

### Quick Test (2 minutes)
```bash
1. Launch game
2. Verify main menu appears
3. Select "New Game"
4. Verify map loads
5. Verify units render
6. Move camera (WASD keys)
7. Click to select unit
8. Right-click to move
9. Return to menu (ESC)
10. Quit game (verify clean exit)
```

### Extended Test (30 minutes)
```bash
1. Complete quick test
2. Play full game (build units, expand)
3. Switch between units
4. Verify no performance degradation
5. Check memory usage over time
6. Test pause/unpause
7. Test multiple game sessions
```

## Future Enhancements (Beyond Phase 48)

- Advanced AI opponents
- Multiplayer networking
- Campaign missions
- Unit veterancy system
- Special abilities
- Fog of war
- Advanced UI elements
- Audio system integration
- Advanced particle effects

## Deliverables

### For Phase 48 Completion

```
GeneralsX v0.1.0 - Minimal Playable
├── Main menu with map selection
├── Single skirmish map fully playable
├── Basic unit control and movement
├── Building system (non-functional)
├── UI display of resources
├── Settings menu (read-only)
├── 60 FPS stable gameplay
└── Clean exit to desktop
```

## References

- `docs/PHASE40/` - Graphics backend initialization
- `docs/PHASE41/` - Drawing operations
- `docs/PHASE42/` - Texture system
- `docs/PHASE43/` - Render loop
- `docs/PHASE44/` - Model loading
- `docs/PHASE45/` - Camera system
- `docs/PHASE46/` - Game logic integration
- `docs/PHASE47/` - Testing and optimization
- Generals/Zero Hour game source documentation
