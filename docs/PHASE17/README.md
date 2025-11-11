# PHASE17: Menu Definitions

## Phase Title

Menu Definitions - Parse MainMenu.ini

## Objective

Parse menu definitions from INI files extracted from .big archives. This establishes the menu structure before rendering.

## Dependencies

- PHASE15 (Button System)
- INI parser (should exist in base engine)

## Key Deliverables

1. MainMenu.ini parsing
2. Menu definition structure
3. Button definition parsing
4. Menu hierarchy establishment
5. Asset references from INI

## Acceptance Criteria

- [ ] MainMenu.ini parses without crashes
- [ ] All menu buttons defined correctly
- [ ] Asset paths resolve correctly
- [ ] Menu hierarchy is correct
- [ ] No orphaned button definitions
- [ ] Parse time acceptable (< 100ms)

## Technical Details

### Components to Implement

1. **INI Parser Integration**
   - Use existing game INI parser
   - Handle menu-specific sections
   - Variable interpolation

2. **Menu Structure**
   - Parse button definitions
   - Parse screen definitions
   - Parse layout information

3. **Validation**
   - Reference checking
   - Required field validation
   - Error reporting

### Code Location

```
GeneralsMD/Code/Menu/MenuDefinition/
GeneralsMD/Code/Menu/Loader/
```

### Menu Definition Structure

```cpp
struct MenuDefinition {
    std::string name;
    std::vector<ButtonDef> buttons;
    std::string backgroundMesh;
    std::vector<TextElement> labels;
};

struct ButtonDef {
    std::string name;
    std::string labelText;
    float x, y, width, height;
    std::string onClick;  // action/callback
};
```

## Estimated Scope

**SMALL** (1-2 days)

- INI parsing setup: 0.5 day
- Definition parsing: 0.5 day
- Testing/validation: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Parse MainMenu.ini and verify structure
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

## Success Criteria

- Menu definitions load without errors
- All asset references valid
- Parse time acceptable
- No validation warnings

## Reference Documentation

- See PHASE22 for INI parsing patterns
- Original game INI format documentation
- Menu structure diagrams (if available)
