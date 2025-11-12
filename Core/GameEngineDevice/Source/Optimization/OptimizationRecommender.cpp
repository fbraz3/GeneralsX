#include "OptimizationRecommender.h"
#include "BottleneckAnalyzer.h"
#include <algorithm>
#include <iostream>
#include <cmath>

namespace GeneralsX::Optimization {

OptimizationRecommender::OptimizationRecommender()
    : minimum_confidence_(0.7),
      max_recommendations_(10),
      estimation_model_("conservative"),
      initialized_(false) {
}

OptimizationRecommender::~OptimizationRecommender() {
    if (initialized_) {
        Shutdown();
    }
}

bool OptimizationRecommender::Initialize() {
    if (initialized_) {
        return true;
    }
    
    initialized_ = true;
    std::cout << "OptimizationRecommender initialized" << std::endl;
    return true;
}

bool OptimizationRecommender::Shutdown() {
    if (!initialized_) {
        return true;
    }
    
    recommendations_.clear();
    ranked_recommendations_.clear();
    initialized_ = false;
    return true;
}

bool OptimizationRecommender::GenerateRecommendations(const std::vector<Bottleneck>& bottlenecks,
                                                      const PerformanceMetrics& metrics) {
    if (!initialized_) {
        return false;
    }
    
    recommendations_.clear();
    
    for (const auto& bottleneck : bottlenecks) {
        std::vector<Optimization> opts;
        if (!AnalyzeBottleneck(bottleneck, opts)) {
            continue;
        }
        
        for (auto& opt : opts) {
            recommendations_.push_back(opt);
        }
    }
    
    if (!RankRecommendations()) {
        return false;
    }
    
    return true;
}

bool OptimizationRecommender::AnalyzeBottleneck(const Bottleneck& bottleneck,
                                               std::vector<Optimization>& recommendations) {
    OptimizationCategory category = CategorizeBottleneck(bottleneck);
    double estimated_gain = EstimateGain(bottleneck, category);
    double confidence = CalculateConfidence(bottleneck, estimated_gain);
    
    if (confidence >= minimum_confidence_) {
        Optimization opt;
        opt.name = bottleneck.marker_name + "_optimization_" +
                   std::to_string(static_cast<int>(category));
        opt.description = "Optimize " + bottleneck.marker_name + " (Category: " +
                         std::to_string(static_cast<int>(category)) + ")";
        opt.target = bottleneck;
        opt.estimated_gain_ms = estimated_gain;
        opt.confidence_level = confidence;
        opt.priority_rank = 0;
        opt.applied = false;
        opt.actual_gain_ms = 0.0;
        
        recommendations.push_back(opt);
    }
    
    return true;
}

OptimizationCategory OptimizationRecommender::CategorizeBottleneck(const Bottleneck& bottleneck) {
    const std::string& name = bottleneck.marker_name;
    
    if (name.find("Render") != std::string::npos || name.find("Draw") != std::string::npos) {
        return OptimizationCategory::BATCHING;
    } else if (name.find("Memory") != std::string::npos || name.find("Alloc") != std::string::npos) {
        return OptimizationCategory::MEMORY;
    } else if (name.find("Physics") != std::string::npos || name.find("AI") != std::string::npos) {
        return OptimizationCategory::ALGORITHM;
    } else if (name.find("Shader") != std::string::npos) {
        return OptimizationCategory::SHADER;
    } else if (name.find("Data") != std::string::npos || name.find("Cache") != std::string::npos) {
        return OptimizationCategory::DATA_STRUCTURE;
    }
    
    return OptimizationCategory::UNKNOWN;
}

double OptimizationRecommender::EstimateGain(const Bottleneck& bottleneck,
                                            OptimizationCategory category) {
    // Conservative estimation based on category
    double base_gain = bottleneck.time_ms * 0.2;  // 20% baseline
    
    switch (category) {
        case OptimizationCategory::BATCHING:
            return base_gain * 1.5;  // 30% potential gain
        case OptimizationCategory::MEMORY:
            return base_gain * 1.3;  // 26% potential gain
        case OptimizationCategory::ALGORITHM:
            return base_gain * 1.2;  // 24% potential gain
        case OptimizationCategory::SHADER:
            return base_gain * 1.1;  // 22% potential gain
        case OptimizationCategory::DATA_STRUCTURE:
            return base_gain * 1.0;  // 20% potential gain
        default:
            return base_gain;
    }
}

double OptimizationRecommender::CalculateConfidence(const Bottleneck& bottleneck,
                                                   double estimated_gain) {
    // Confidence based on bottleneck characteristics
    double confidence = 0.5;
    
    // Higher confidence for larger bottlenecks
    if (bottleneck.percentage_of_frame > 5.0) {
        confidence += 0.3;
    } else if (bottleneck.percentage_of_frame > 2.0) {
        confidence += 0.2;
    } else {
        confidence += 0.1;
    }
    
    // Higher confidence for frequently called functions
    if (bottleneck.call_count > 100) {
        confidence += 0.1;
    }
    
    // Cap at 1.0
    return std::min(confidence, 1.0);
}

bool OptimizationRecommender::RankRecommendations() {
    ranked_recommendations_ = recommendations_;
    
    std::sort(ranked_recommendations_.begin(), ranked_recommendations_.end(),
        [](const Optimization& a, const Optimization& b) {
            // Sort by confidence and then by estimated gain
            if (a.confidence_level != b.confidence_level) {
                return a.confidence_level > b.confidence_level;
            }
            return a.estimated_gain_ms > b.estimated_gain_ms;
        });
    
    // Assign priority ranks
    for (int i = 0; i < static_cast<int>(ranked_recommendations_.size()); ++i) {
        ranked_recommendations_[i].priority_rank = i + 1;
    }
    
    return true;
}

std::vector<Optimization> OptimizationRecommender::GetRecommendations() const {
    int limit = std::min(max_recommendations_, static_cast<int>(ranked_recommendations_.size()));
    std::vector<Optimization> result(ranked_recommendations_.begin(),
                                     ranked_recommendations_.begin() + limit);
    return result;
}

std::vector<Optimization> OptimizationRecommender::GetRecommendationsByCategory(
    OptimizationCategory category) const {
    std::vector<Optimization> result;
    for (const auto& opt : ranked_recommendations_) {
        if (CategorizeBottleneck(opt.target) == category) {
            result.push_back(opt);
        }
    }
    return result;
}

std::vector<Optimization> OptimizationRecommender::GetHighConfidenceRecommendations() const {
    std::vector<Optimization> result;
    for (const auto& opt : ranked_recommendations_) {
        if (opt.confidence_level >= 0.85) {
            result.push_back(opt);
        }
    }
    return result;
}

Optimization OptimizationRecommender::GetRecommendation(const std::string& opt_name) const {
    for (const auto& opt : recommendations_) {
        if (opt.name == opt_name) {
            return opt;
        }
    }
    return Optimization();
}

int OptimizationRecommender::GetRecommendationCount() const {
    return recommendations_.size();
}

double OptimizationRecommender::GetAverageConfidence() const {
    if (recommendations_.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& opt : recommendations_) {
        sum += opt.confidence_level;
    }
    return sum / recommendations_.size();
}

void OptimizationRecommender::SetMinimumConfidence(double confidence) {
    minimum_confidence_ = confidence;
}

void OptimizationRecommender::SetMaxRecommendations(int count) {
    max_recommendations_ = count;
}

void OptimizationRecommender::SetEstimationModel(const std::string& model_name) {
    estimation_model_ = model_name;
}

}  // namespace GeneralsX::Optimization
