#pragma once

#include <vector>
#include <memory>
#include <string>
#include <ctime>

namespace GeneralsX::Optimization {

// Shader optimization result
struct ShaderOptimizationResult {
    std::string shader_name;
    int original_instruction_count;
    int optimized_instruction_count;
    int instructions_removed;
    double estimated_gpu_time_gain_ms;
    bool simplification_applied;
    std::string optimization_type;    // "remove_unnecessary_ops", "combine_operations", etc.
    bool success;
};

// Shader optimization statistics
struct ShaderOptimizationStatistics {
    int total_shaders_analyzed;
    int total_shaders_optimized;
    int total_instructions_removed;
    double average_instruction_reduction_percentage;
    double total_estimated_gpu_time_gain_ms;
    int successful_optimizations;
    int failed_optimizations;
};

// Shader complexity metrics
struct ShaderComplexity {
    std::string shader_name;
    int instruction_count;
    int texture_lookups;
    int arithmetic_operations;
    int memory_operations;
    double estimated_gpu_cycles;
    bool is_vertex_shader;
    bool is_fragment_shader;
};

// Interface for shader optimization
class ShaderOptimizer {
public:
    virtual ~ShaderOptimizer() = default;
    
    // Analyze all shaders for optimization opportunities
    virtual int AnalyzeShaderComplexity() = 0;
    
    // Get shader complexity analysis
    virtual const std::vector<ShaderComplexity>& GetShaderComplexityAnalysis() const = 0;
    
    // Optimize a specific shader
    virtual ShaderOptimizationResult OptimizeShader(const std::string& shader_name) = 0;
    
    // Optimize all shaders
    virtual std::vector<ShaderOptimizationResult> OptimizeAllShaders() = 0;
    
    // Get most complex shaders (candidates for optimization)
    virtual std::vector<ShaderComplexity> GetMostComplexShaders(int count = 10) const = 0;
    
    // Revert shader to original (non-optimized) version
    virtual bool RevertShader(const std::string& shader_name) = 0;
    
    // Revert all shaders to original
    virtual int RevertAllShaders() = 0;
    
    // Check if shader optimization would be beneficial
    virtual bool ShouldOptimizeShader(const std::string& shader_name) const = 0;
    
    // Get optimization statistics
    virtual const ShaderOptimizationStatistics& GetStatistics() const = 0;
    
    // Get estimated GPU time reduction from optimizations
    virtual double GetEstimatedGPUTimeReduction() const = 0;
    
    // Set target instruction reduction percentage
    virtual void SetTargetInstructionReductionPercentage(double percentage) = 0;
    
    // Set minimum shader complexity threshold for optimization
    virtual void SetMinimumComplexityThreshold(int instruction_count) = 0;
    
    // Validate optimized shader (compare outputs)
    virtual bool ValidateOptimizedShader(const std::string& shader_name) const = 0;
    
    // Get number of optimized shaders
    virtual int GetOptimizedShaderCount() const = 0;
    
    // Reset all shader optimizations
    virtual void ResetAllOptimizations() = 0;
    
    // Simplify fragment shader by removing unnecessary operations
    virtual bool SimplifyFragmentShader(const std::string& shader_name) = 0;
    
    // Combine similar operations in shader
    virtual bool CombineShaderOperations(const std::string& shader_name) = 0;
};

} // namespace GeneralsX::Optimization
