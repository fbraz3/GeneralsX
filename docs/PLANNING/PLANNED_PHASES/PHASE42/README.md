# Phase 42: Performance Visualization Dashboard

**Phase**: 42  
**Title**: Performance Visualization Dashboard  
**Area**: Optimization & Profiling  
**Scope**: MEDIUM  
**Status**: not-started  
**Difficulty**: MEDIUM  
**Estimated Duration**: 2 weeks  
**Dependencies**: Phase 39 (Profiling), Phase 40 (Data Analysis)

---

## Quick Reminders

- Use `Fail fast` approach when testing new changes
- Focus on GeneralsXZH first, then backport to GeneralsX
- See `.github/instructions/project.instructions.md` for project guidelines
- Check for integration issues from previous phases before proceeding
- Always include rollback capability for applied fixes

---

## Overview

Phase 42 creates a **real-time performance visualization dashboard** that displays profiling data from Phase 39 in an intuitive graphical format. This enables developers to monitor performance metrics visually and quickly identify issues.

## Objectives

1. **Real-Time Metrics Display**: Show FPS, CPU/GPU time, memory usage
2. **Interactive Graphs**: Zoom, pan, and filter performance data
3. **Alerting System**: Visual alerts for performance regressions
4. **Export Capabilities**: Export graphs and reports as images
5. **Custom Dashboards**: Allow users to customize visible metrics
6. **Cross-Platform UI**: Work on macOS, Linux, Windows

## Key Deliverables

- [ ] DashboardUI (main UI framework)
- [ ] PerformanceGraphs (frame time, CPU/GPU, memory graphs)
- [ ] MetricsPanel (displays key statistics)
- [ ] AlertingSystem (visual regression alerts)
- [ ] ExportEngine (export to PNG/PDF)
- [ ] CustomizationEngine (save/load dashboard configs)
- [ ] Real-time UpdateLoop (refresh at 60Hz)
- [ ] ImGui integration (lightweight cross-platform UI)

## Key Files

- `Core/GameEngineDevice/Source/UI/PerformanceDashboard.h/cpp`
- `Core/GameEngineDevice/Source/UI/PerformanceGraphs.h/cpp`
- `Core/GameEngineDevice/Source/UI/DashboardMetrics.h/cpp`

## Technical Details

### Dashboard Layout

```
┌─────────────────────────────────────────────┐
│ GeneralsX Performance Dashboard              │
├─────────────────────────────────────────────┤
│ FPS: 61.2  Avg: 60.8  Min: 58.2  Max: 63.5 │
│                                             │
│ ┌──── Frame Time (ms) ─────────────────┐   │
│ │ 20│                                 │   │
│ │ 16│   ╭╮  ╭╮  ╭╮  ╭╮               │   │
│ │ 12│   ││╭─╯╰╮ │╰─ ││   ╭╮         │   │
│ │  8│   ││   │╰─╯   ││  ╭╯╰─┐       │   │
│ │ 0└───┴┴───┴───────┴┴──┴────┴──────→│   │
│ │ Target: 16.67ms                    │   │
│ └────────────────────────────────────┘   │
│                                             │
│ CPU: 8.5ms (51%)  │ GPU: 6.2ms (37%)       │
│ Memory: 456MB / 1234MB Peak (37%)          │
│ Draw Calls: 12,041  │ Batches: 823         │
│                                             │
│ [Export] [Save] [Settings] [Clear]         │
└─────────────────────────────────────────────┘
```

### Graphs Supported

1. **Frame Time Graph**: Line graph with target threshold
2. **CPU/GPU Time Breakdown**: Stacked area chart
3. **Memory Usage Over Time**: Area chart with peak marker
4. **Draw Call Count**: Bar chart
5. **FPS Histogram**: Distribution of frame rates
6. **GPU Utilization**: Gauge or line chart

### Data Structures

```cpp
struct DashboardMetrics {
    float current_fps;
    float avg_fps;
    float min_fps;
    float max_fps;
    double frame_time_ms;
    double cpu_time_ms;
    double gpu_time_ms;
    uint64_t gpu_memory;
    uint64_t cpu_memory;
    uint32_t draw_calls;
    int active_batches;
};

struct DashboardConfig {
    vector<string> visible_metrics;
    vector<Graph> active_graphs;
    bool show_alerts;
    bool show_targets;
    float graph_update_rate;
};
```

## Acceptance Criteria

### Build & Compilation

- [ ] Compiles without errors
- [ ] All platforms build successfully
- [ ] ImGui integration working

### Functionality

- [ ] Real-time metrics display (60Hz refresh)
- [ ] All graphs rendering correctly
- [ ] Zoom/pan functionality working
- [ ] Alert system triggering on regression
- [ ] Export to PNG/PDF working
- [ ] Dashboard config save/load working

### Performance

- [ ] Dashboard rendering <2ms overhead
- [ ] No frame rate impact from dashboard
- [ ] Memory overhead <50MB

### Usability

- [ ] Intuitive layout and controls
- [ ] Clear visual indicators for problems
- [ ] Easy to understand graphs

## Testing Strategy

### Unit Tests

- [ ] Metrics calculation accuracy
- [ ] Graph data aggregation
- [ ] Alert triggering logic

### Visual Tests

- [ ] Graph rendering correctness
- [ ] Alert visibility
- [ ] Export image quality

### Performance Tests

- [ ] Rendering overhead measurement
- [ ] Memory usage under load

## Success Criteria

- Dashboard displays metrics accurately
- Real-time updates at 60Hz
- <2ms rendering overhead
- <50MB memory usage
- Cross-platform compatibility

## Implementation Plan

### Week 1: Core Dashboard Framework
- [ ] ImGui integration
- [ ] Metrics panel layout
- [ ] Basic graph rendering

### Week 2: Advanced Features
- [ ] Interactive graph controls
- [ ] Alert system
- [ ] Export functionality
- [ ] Configuration system

## Related Phases

- **Phase 39**: Provides profiling data
- **Phase 40**: Data analysis results
- **Phase 41**: Optimization impact visualization
- **Phase 43**: Cross-build comparison visualization

---

**Last Updated**: November 12, 2025  
**Status**: Ready for Implementation
