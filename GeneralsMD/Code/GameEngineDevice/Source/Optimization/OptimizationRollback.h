#pragma once

#include <vector>
#include <memory>
#include <string>
#include <ctime>
#include <cstdint>

namespace GeneralsX::Optimization {

// Forward declarations
struct Optimization;

// Minimal state backup for rollback
struct RollbackPoint {
    std::time_t created_at;
    std::string optimization_name;
    std::vector<uint8_t> state_backup;  // Minimal state for rollback
    double baseline_frame_time_ms;
    
    RollbackPoint() : created_at(0), baseline_frame_time_ms(0.0) {}
};

// Rollback status enumeration
enum class RollbackStatus {
    PENDING,      // Rollback not needed yet
    IN_PROGRESS,  // Currently rolling back
    COMPLETE,     // Rollback completed successfully
    FAILED        // Rollback failed
};

// Result of a rollback operation
struct RollbackResult {
    bool success;
    RollbackStatus status;
    double rollback_latency_ms;  // Should be <10ms
    std::string error_message;
    std::time_t completed_at;
    
    RollbackResult() 
        : success(false), status(RollbackStatus::PENDING), 
          rollback_latency_ms(0.0), completed_at(0) {}
};

// Interface for optimization rollback capability
class OptimizationRollback {
public:
    virtual ~OptimizationRollback() = default;
    
    // Create a rollback point before applying optimization
    virtual RollbackPoint CreateRollbackPoint(const Optimization& optimization) = 0;
    
    // Perform rollback to a specific point
    // Returns success and latency info
    virtual RollbackResult Rollback(const RollbackPoint& point) = 0;
    
    // Rollback last applied optimization
    virtual RollbackResult RollbackLast() = 0;
    
    // Rollback all optimizations in reverse order
    virtual RollbackResult RollbackAll() = 0;
    
    // Check if rollback is possible for an optimization
    virtual bool CanRollback(const Optimization& opt) const = 0;
    
    // Get number of available rollback points
    virtual int GetRollbackPointCount() const = 0;
    
    // Get average rollback latency
    virtual double GetAverageRollbackLatency() const = 0;
    
    // Get rollback success rate (0.0 - 1.0)
    virtual double GetRollbackSuccessRate() const = 0;
    
    // Check if state corruption detected during rollback
    virtual bool HasStateCorruption() const = 0;
    
    // Verify integrity of rollback point
    virtual bool VerifyRollbackPointIntegrity(const RollbackPoint& point) const = 0;
    
    // Clear old rollback points (keep only recent ones)
    virtual void PruneOldRollbackPoints(int max_age_seconds = 3600) = 0;
    
    // Get total memory used by rollback points
    virtual size_t GetRollbackPointsMemoryUsage() const = 0;
};

} // namespace GeneralsX::Optimization
