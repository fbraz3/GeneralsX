# PHASE21: GameObject System

## Phase Title

GameObject System - Entity Classes and Components

## Objective

Implement base game object system with component architecture for extensible entity behavior.

## Dependencies

- PHASE07 (Mesh Rendering)
- PHASE08 (Camera System)

## Key Deliverables

1. Base GameObject class
2. Component system
3. Transform component
4. Renderer component
5. Update lifecycle

## Acceptance Criteria

- [ ] GameObjects created and destroyed correctly
- [ ] Components attach and detach properly
- [ ] Update cycle runs reliably
- [ ] Memory properly managed
- [ ] No leaks on object destruction
- [ ] Performance acceptable with 1000+ objects

## Technical Details

### Components to Implement

1. **GameObject**
   - GUID/ID system
   - Active/inactive state
   - Component collection

2. **Component System**
   - Base Component class
   - OnEnable/OnDisable
   - OnUpdate lifecycle

3. **Built-in Components**
   - Transform (position, rotation, scale)
   - Renderer (mesh reference)

### Code Location

```
GeneralsMD/Code/Game/GameObject/
GeneralsMD/Code/Game/Component/
```

### GameObject Architecture

```cpp
class GameObject {
public:
    GameObject(const std::string& name);
    ~GameObject();
    
    template<typename T>
    T* AddComponent();
    
    template<typename T>
    T* GetComponent();
    
    void Update(float deltaTime);
    void SetActive(bool active);
    
private:
    std::vector<std::unique_ptr<Component>> components;
    bool active;
    std::string name;
};

class Component {
public:
    virtual void OnEnable() {}
    virtual void OnDisable() {}
    virtual void OnUpdate(float deltaTime) {}
    
protected:
    GameObject* gameObject;
};
```

## Estimated Scope

**MEDIUM** (3-4 days)

- Architecture design: 1 day
- Base class implementation: 1 day
- Component system: 1 day
- Testing: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Create test GameObjects and verify lifecycle
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

## Success Criteria

- Objects create/destroy cleanly
- Components work reliably
- No memory leaks
- Performance acceptable
- Easy to extend with new components

## Reference Documentation

- Entity Component System (ECS) patterns
- Component-based architecture
- Unity GameObjects (familiar reference)
