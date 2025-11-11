# PHASE26: Command System

## Phase Title

Command System - Move, Attack, and Game Commands

## Objective

Implement command system for unit control with command queuing and execution.

## Dependencies

- PHASE21 (GameObject System)
- PHASE24 (Input Handling)
- PHASE25 (Selection System)

## Key Deliverables

1. Command types (Move, Attack, Build, etc.)
2. Command queue per unit
3. Command execution
4. Command feedback (animations, effects)
5. Command validation

## Acceptance Criteria

- [ ] Units execute commands in order
- [ ] Multiple commands queue properly
- [ ] Commands validate correctly
- [ ] Animations play during command execution
- [ ] Performance acceptable with many queued commands

## Technical Details

### Components to Implement

1. **Command System**
   - Base Command class
   - Specific command types
   - Command queue

2. **Execution**
   - Update-based execution
   - Animation/effect triggers
   - Completion detection

3. **Validation**
   - Range checking (can attack target?)
   - Path validity (can move to location?)

### Code Location

```
GeneralsMD/Code/Game/Command/
GeneralsMD/Code/Game/Unit/
```

### Command Architecture

```cpp
class Command {
public:
    virtual bool Execute(float deltaTime) = 0;
    virtual bool IsComplete() const = 0;
    virtual void OnStart() {}
    virtual void OnComplete() {}
};

class MoveCommand : public Command {
public:
    MoveCommand(const glm::vec3& target, float speed);
    bool Execute(float deltaTime) override;
    bool IsComplete() const override;
};

class Unit : public GameObject {
public:
    void QueueCommand(std::unique_ptr<Command> cmd);
    void Update(float deltaTime) override;
};
```

## Estimated Scope

**LARGE** (3-4 days)

- Command base classes: 0.5 day
- Move command: 1 day
- Attack command: 1 day
- Queuing/execution: 0.5 day
- Testing: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Queue commands on units and verify execution
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

## Success Criteria

- Commands execute in order
- No animation conflicts
- Proper feedback on execution
- Validation prevents invalid commands
- Performance acceptable

## Reference Documentation

- Command pattern design
- RTS command system
- Animation state machines
