set(OPTIMIZATION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/PerformanceOptimizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PerformanceOptimizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ProfilerDataAggregator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ProfilerDataAggregator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BottleneckAnalyzer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/BottleneckAnalyzer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OptimizationRecommender.h
    ${CMAKE_CURRENT_SOURCE_DIR}/OptimizationRecommender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OptimizationValidator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/OptimizationValidator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PerformanceTracker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PerformanceTracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ReportGenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ReportGenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RegressionDetector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RegressionDetector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TrendAnalyzer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TrendAnalyzer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BatchOptimizationApplier.h
    ${CMAKE_CURRENT_SOURCE_DIR}/BatchOptimizationApplier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OptimizationRollback.h
    ${CMAKE_CURRENT_SOURCE_DIR}/OptimizationRollback.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SafeOptimizationValidator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SafeOptimizationValidator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AutomaticMeshBatcher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/AutomaticMeshBatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MemoryCompactor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MemoryCompactor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ShaderOptimizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ShaderOptimizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OptimizationScheduler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/OptimizationScheduler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OptimizationMonitor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/OptimizationMonitor.cpp
)

# Add to parent project's source list
list(APPEND GAMEENGINEDEVICE_SRC ${OPTIMIZATION_SOURCES})
set(GAMEENGINEDEVICE_SRC ${GAMEENGINEDEVICE_SRC} PARENT_SCOPE)
