#pragma once

#include <vector>
#include <memory>
#include <string>
#include <ctime>

namespace GeneralsX::Optimization {

// Forward declarations
struct Optimization;

// Validation timing information
struct ValidationTiming {
    double pre_application_time_ms;   // Frame time before optimization
    double post_application_time_ms;  // Frame time after optimization
    double measured_gain_ms;          // Actual measured improvement
    double gain_percentage;           // Percentage improvement
    int sample_count;                 // Number of frames measured
    std::time_t measured_at;
};

// Validation result with confidence score
struct ValidationResult {
    bool is_valid;                    // Is optimization valid/beneficial?
    double confidence_level;          // Validation confidence (0.0 - 1.0)
    double before_time_ms;            // Frame time before
    double after_time_ms;             // Frame time after
    double actual_gain_ms;            // Measured gain
    double gain_percentage;           // Percentage of improvement
    double error_margin;              // Measurement error margin
    int sample_count;                 // Samples used in validation
    std::string validation_notes;     // Human-readable validation summary
    bool meets_minimum_threshold;     // Meets minimum gain threshold
    
    ValidationResult() 
        : is_valid(false), confidence_level(0.0), before_time_ms(0.0), 
          after_time_ms(0.0), actual_gain_ms(0.0), gain_percentage(0.0),
          error_margin(0.0), sample_count(0), meets_minimum_threshold(false) {}
};

// Interface for safe optimization validation
class SafeOptimizationValidator {
public:
    virtual ~SafeOptimizationValidator() = default;
    
    // Begin pre-application validation (before applying optimization)
    virtual bool BeginPreValidation(const Optimization& opt) = 0;
    
    // End pre-application validation and get result
    virtual ValidationResult EndPreValidation() = 0;
    
    // Begin post-application validation (after applying optimization)
    virtual bool BeginPostValidation(const Optimization& opt) = 0;
    
    // End post-application validation and get result
    virtual ValidationResult EndPostValidation() = 0;
    
    // Full validation cycle: pre + application + post
    virtual ValidationResult ValidateOptimizationFull(const Optimization& opt) = 0;
    
    // Check if optimization is valid without applying it (dry-run)
    virtual bool PredictValidation(const Optimization& opt) = 0;
    
    // Get last validation result
    virtual const ValidationResult& GetLastValidationResult() const = 0;
    
    // Get validation history
    virtual const std::vector<ValidationResult>& GetValidationHistory() const = 0;
    
    // Get average validation accuracy (0.0 - 1.0)
    virtual double GetValidationAccuracy() const = 0;
    
    // Get percentage of optimizations that met minimum threshold
    virtual double GetThresholdMeetRate() const = 0;
    
    // Get false positive rate for predictions
    virtual double GetFalsePositiveRate() const = 0;
    
    // Set minimum frame time improvement threshold (in ms)
    virtual void SetMinimumGainThreshold(double gain_ms) = 0;
    
    // Set confidence threshold for validation
    virtual void SetMinimumConfidenceThreshold(double confidence) = 0;
    
    // Get number of validated optimizations
    virtual int GetValidatedOptimizationCount() const = 0;
    
    // Get number of successful validations
    virtual int GetSuccessfulValidationCount() const = 0;
    
    // Clear validation history
    virtual void ResetValidationHistory() = 0;
};

} // namespace GeneralsX::Optimization
