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
)

# Add to parent project's source list
list(APPEND GAMEENGINEDEVICE_SRC ${OPTIMIZATION_SOURCES})
set(GAMEENGINEDEVICE_SRC ${GAMEENGINEDEVICE_SRC} PARENT_SCOPE)
