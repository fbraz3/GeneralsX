// ============================================================================
// Cross-Platform CDManager Implementation
// For non-Windows platforms where CD drives are not used
// ============================================================================

#include "Common/CDManager.h"

class CrossPlatformCDManager : public CDManagerInterface
{
public:
    CrossPlatformCDManager() 
    {
        DEBUG_LOG(("CrossPlatformCDManager: Created (no-op implementation for cross-platform)"));
    }
    
    virtual ~CrossPlatformCDManager() 
    {
        destroyAllDrives();
    }

    // SubsystemInterface implementation
    virtual void init(void) override 
    {
        DEBUG_LOG(("CrossPlatformCDManager::init() - No CD drives on this platform"));
        // On modern systems, games are installed from digital distribution
        // No CD drive enumeration needed
    }
    
    virtual void update(void) override 
    {
        // No-op: No CD drives to monitor
    }
    
    virtual void reset(void) override 
    {
        destroyAllDrives();
    }

    // CDManagerInterface implementation
    virtual Int driveCount(void) override 
    {
        return 0;  // No CD drives
    }
    
    virtual CDDriveInterface* getDrive(Int index) override 
    {
        (void)index;  // Suppress unused parameter warning
        return nullptr;  // No drives available
    }
    
    virtual CDDriveInterface* newDrive(const Char* path) override 
    {
        (void)path;  // Suppress unused parameter warning
        DEBUG_LOG(("CrossPlatformCDManager::newDrive() - CD drives not supported on this platform"));
        return nullptr;
    }
    
    virtual void refreshDrives(void) override 
    {
        // No-op: No CD drives to refresh
    }
    
    virtual void destroyAllDrives(void) override 
    {
        // No-op: No drives were created
    }

protected:
    virtual CDDriveInterface* createDrive(void) override 
    {
        return nullptr;  // Cannot create CD drives on this platform
    }
};

CDManagerInterface* CreateCDManager(void)
{
    DEBUG_LOG(("CreateCDManager: Creating CrossPlatformCDManager"));
    return NEW CrossPlatformCDManager;
}
