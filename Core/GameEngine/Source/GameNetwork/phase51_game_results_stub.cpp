/**
 * @file phase51_game_results_stub.cpp
 * @brief Phase 51: GameResultsInterface stub implementation for cross-platform compatibility
 * 
 * This file provides a stub implementation of GameResultsInterface that allows
 * the game to initialize without the full GameSpy GameResultsThread (which requires
 * Windows winsock.h).
 * 
 * The original GameResultsThread.cpp is commented out in CMakeLists.txt because
 * it depends on winsock.h which is not available on macOS/Linux.
 * 
 * Future work: Port the full GameResultsThread to use SDL2 networking or
 * cross-platform POSIX sockets.
 */

#include "PreRTS.h"
#include "Common/SubsystemInterface.h"
#include "GameNetwork/GameSpy/GameResultsThread.h"

#include <cstdio>

// Global pointer (declared extern in GameResultsThread.h)
GameResultsInterface *TheGameResultsQueue = nullptr;

/**
 * StubGameResultsQueue - A no-op implementation of GameResultsInterface
 * 
 * This implementation allows the game to proceed through initialization
 * without crashing, while indicating that game results posting is not
 * available on this platform.
 */
class StubGameResultsQueue : public GameResultsInterface
{
public:
    StubGameResultsQueue()
    {
        printf("[Phase51] StubGameResultsQueue created (GameSpy results posting disabled)\n");
    }
    
    ~StubGameResultsQueue() override
    {
        printf("[Phase51] StubGameResultsQueue destroyed\n");
    }
    
    // SubsystemInterface methods
    void init() override
    {
        printf("[Phase51] StubGameResultsQueue::init()\n");
    }
    
    void reset() override
    {
        printf("[Phase51] StubGameResultsQueue::reset()\n");
    }
    
    void update() override
    {
        // No-op: no results to process
    }
    
    // GameResultsInterface methods
    void startThreads(void) override
    {
        printf("[Phase51] StubGameResultsQueue::startThreads() - no-op (cross-platform stub)\n");
        // No threads started - this is a stub
    }
    
    void endThreads(void) override
    {
        printf("[Phase51] StubGameResultsQueue::endThreads() - no-op\n");
    }
    
    Bool areThreadsRunning(void) override
    {
        // Always return false - no threads running
        return FALSE;
    }
    
    void addRequest(const GameResultsRequest& req) override
    {
        printf("[Phase51] StubGameResultsQueue::addRequest() - discarding (no network support)\n");
        // Discard the request - we can't send it anywhere
    }
    
    Bool getRequest(GameResultsRequest& req) override
    {
        // No requests available
        return FALSE;
    }
    
    void addResponse(const GameResultsResponse& resp) override
    {
        // No-op
    }
    
    Bool getResponse(GameResultsResponse& resp) override
    {
        // No responses available
        return FALSE;
    }
    
    Bool areGameResultsBeingSent(void) override
    {
        // We never send results, so nothing is being sent
        return FALSE;
    }
};

/**
 * Factory function implementation
 * 
 * This provides the actual symbol that the linker expects:
 * GameResultsInterface::createNewGameResultsInterface()
 */
GameResultsInterface* GameResultsInterface::createNewGameResultsInterface(void)
{
    printf("[Phase51] createNewGameResultsInterface() - creating StubGameResultsQueue\n");
    return NEW StubGameResultsQueue();
}
