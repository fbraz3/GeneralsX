# PHASE25: Selection System

## Phase Title

Selection System - Unit Selection and Highlighting

## Objective

Implement unit selection system with multi-select, drag selection, and visual feedback.

## Dependencies

- PHASE21 (GameObject System)
- PHASE24 (Input Handling)
- PHASE07 (Mesh Rendering)

## Key Deliverables

1. Single selection
2. Multi-selection (Ctrl+click)
3. Drag selection (marquee)
4. Selection highlighting
5. Selection state persistence

## Acceptance Criteria

- [ ] Single units select with click
- [ ] Ctrl+click adds to selection
- [ ] Drag selection creates marquee
- [ ] Selected units highlight correctly
- [ ] Deselection works
- [ ] Performance acceptable with 100+ selected

## Technical Details

### Components to Implement

1. **Selection Manager**
   - Track selected objects
   - Selection mode (single/multi)
   - Clear selection

2. **Visual Feedback**
   - Highlight material
   - Outline shader
   - Selection marquee visualization

3. **Marquee Selection**
   - Drag rectangle tracking
   - Hit testing in rectangle
   - Smooth animation

### Code Location

```
GeneralsMD/Code/Game/Selection/
GeneralsMD/Code/Game/SelectionUI/
```

### Selection Manager

```cpp
class SelectionManager {
public:
    void SelectUnit(GameObject* unit, bool addToSelection = false);
    void DeselectUnit(GameObject* unit);
    void DeselectAll();
    void SelectInRect(const Rect& rect);
    
    const std::set<GameObject*>& GetSelection() const;
    void SetHighlightMaterial(Material* mat);
    
private:
    std::set<GameObject*> selectedUnits;
    Material* highlightMaterial;
};
```

## Estimated Scope

**MEDIUM** (2-3 days)

- Selection logic: 1 day
- Marquee rendering: 0.5 day
- Visual feedback: 0.5 day
- Testing: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Test selection with clicking and dragging
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

## Success Criteria

- All selection methods work
- Visual feedback clear
- No lag with large selections
- Selection persists correctly
- Marquee renders smoothly

## Reference Documentation

- RTS selection mechanics
- Material highlighting techniques
- Multi-select UI patterns
