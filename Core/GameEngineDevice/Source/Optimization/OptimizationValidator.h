#pragma once

#include <vector>
#include <string>

namespace GeneralsX::Optimization {

struct ValidationResult {
    bool is_valid;
    double before_time_ms;
    double after_time_ms;
    double actual_gain_ms;
    double gain_percentage;
    std::string validation_notes;
};

class OptimizationValidator {
public:
    OptimizationValidator();
    ~OptimizationValidator();
    
    // Validation engine
    bool Initialize();
    bool Shutdown();
    
    // Validation operations
    bool BeginValidation(const Optimization& optimization);
    bool EndValidation(double actual_frame_time_ms);
    
    // Results
    ValidationResult GetLastValidationResult() const;
    std::vector<ValidationResult> GetValidationHistory() const;
    
    // Analysis
    bool IsOptimizationValid(const Optimization& opt) const;
    double GetExpectedGain(const Optimization& opt) const;
    double GetActualGain() const;
    int GetValidationCount() const;
    
    // Configuration
    void SetValidationFrameCount(int count);
    void SetMinimumGainThreshold(double ms);
    void SetRegressionThreshold(double percentage);

private:
    bool ValidateAgainstBaseline();
    bool MeasureMetrics(double& frame_time);
    
    std::vector<ValidationResult> validation_history_;
    ValidationResult current_validation_;
    
    double baseline_frame_time_;
    std::vector<double> baseline_samples_;
    
    int validation_frame_count_;
    int frames_measured_;
    double minimum_gain_threshold_ms_;
    double regression_threshold_percentage_;
    
    bool validation_in_progress_;
    bool initialized_;
};

}  // namespace GeneralsX::Optimization
