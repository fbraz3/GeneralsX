#pragma once

#include <vector>
#include <string>
#include <memory>

namespace GeneralsX::Optimization {

enum class OptimizationCategory {
    BATCHING,
    MEMORY,
    ALGORITHM,
    SHADER,
    DATA_STRUCTURE,
    UNKNOWN
};

class OptimizationRecommender {
public:
    OptimizationRecommender();
    ~OptimizationRecommender();
    
    // Engine
    bool Initialize();
    bool Shutdown();
    
    // Recommendation generation
    bool GenerateRecommendations(const std::vector<Bottleneck>& bottlenecks,
                                 const PerformanceMetrics& metrics);
    
    // Results
    std::vector<Optimization> GetRecommendations() const;
    std::vector<Optimization> GetRecommendationsByCategory(OptimizationCategory category) const;
    std::vector<Optimization> GetHighConfidenceRecommendations() const;
    
    // Analysis
    Optimization GetRecommendation(const std::string& opt_name) const;
    int GetRecommendationCount() const;
    double GetAverageConfidence() const;
    
    // Configuration
    void SetMinimumConfidence(double confidence);
    void SetMaxRecommendations(int count);
    void SetEstimationModel(const std::string& model_name);

private:
    bool AnalyzeBottleneck(const Bottleneck& bottleneck, std::vector<Optimization>& recommendations);
    OptimizationCategory CategorizeBottleneck(const Bottleneck& bottleneck);
    double EstimateGain(const Bottleneck& bottleneck, OptimizationCategory category);
    double CalculateConfidence(const Bottleneck& bottleneck, double estimated_gain);
    bool RankRecommendations();
    
    std::vector<Optimization> recommendations_;
    std::vector<Optimization> ranked_recommendations_;
    
    double minimum_confidence_;
    int max_recommendations_;
    std::string estimation_model_;
    
    bool initialized_;
};

}  // namespace GeneralsX::Optimization
