# PHASE14: UI Layout

## Phase Title

UI Layout - Positioning and Anchoring System

## Objective

Implement layout system for positioning UI elements with anchoring, margins, and responsive sizing.

## Dependencies

- PHASE13 (UI Rendering)
- Screen resolution change handling

## Key Deliverables

1. Anchor points (top-left, center, etc.)
2. Margin/padding system
3. Relative positioning
4. Screen resolution change handling
5. Layout recalculation

## Acceptance Criteria

- [ ] Elements position correctly with anchors
- [ ] Margins apply as expected
- [ ] Resolution changes reflow layout
- [ ] No overlapping unless intended
- [ ] Performance acceptable with many elements

## Technical Details

### Components to Implement

1. **Anchor System**
   - 9 anchor points (corners, edges, center)
   - Pivot point offsets

2. **Layout Calculation**
   - Rect calculation from anchor + margins
   - Hierarchy traversal
   - Caching for performance

3. **Responsiveness**
   - Screen size change callbacks
   - Layout invalidation/recalculation

### Code Location

```
GeneralsMD/Code/UI/Layout/
GeneralsMD/Code/UI/RectTransform/
```

### Layout Classes

```cpp
class RectTransform {
public:
    void SetAnchor(Anchor anchor);
    void SetMargins(float left, float top, float right, float bottom);
    void SetSize(float width, float height);
    Rect GetRect() const;
    
private:
    Anchor anchor;
    glm::vec4 margins;  // l, t, r, b
    glm::vec2 size;
    Rect cachedRect;
};
```

## Estimated Scope

**MEDIUM** (2-3 days)

- Anchor system: 1 day
- Layout calculation: 0.5 day
- Responsiveness: 0.5 day
- Testing: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Test UI layout with different resolutions
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

## Success Criteria

- All UI elements position correctly
- Layout responsive to resolution changes
- No performance degradation
- Layout matches game design

## Reference Documentation

- Anchor-based layout systems
- UI hierarchy and transforms
- Responsive design patterns
