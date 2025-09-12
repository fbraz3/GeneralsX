#ifndef _WIN32

#include "win32_compat.h"
#include <algorithm>

//=============================================================================
// MultimediaTimerManager Implementation
//=============================================================================

MultimediaTimerManager& MultimediaTimerManager::getInstance()
{
    static MultimediaTimerManager instance;
    return instance;
}

MultimediaTimerManager::~MultimediaTimerManager()
{
    std::lock_guard<std::mutex> lock(m_timersMutex);
    
    // Stop all active timers
    for (auto& pair : m_timers) {
        if (pair.second->active) {
            pair.second->active = false;
            if (pair.second->thread.joinable()) {
                pair.second->thread.join();
            }
        }
    }
    
    m_timers.clear();
}

MMRESULT MultimediaTimerManager::timeSetEvent(UINT delay, UINT resolution, LPTIMECALLBACK callback,
                                             DWORD_PTR user, UINT flags)
{
    if (!callback || delay == 0) {
        return TIMERR_NOCANDO;
    }
    
    std::lock_guard<std::mutex> lock(m_timersMutex);
    
    UINT timerId = m_nextTimerId++;
    
    auto timerInfo = std::make_unique<TimerInfo>();
    timerInfo->active = true;
    timerInfo->delay = delay;
    timerInfo->callback = callback;
    timerInfo->user = user;
    timerInfo->flags = flags;
    
    // Start timer thread
    timerInfo->thread = std::thread(&MultimediaTimerManager::timerThread, this, timerId);
    
    m_timers[timerId] = std::move(timerInfo);
    
    return timerId;
}

MMRESULT MultimediaTimerManager::timeKillEvent(UINT timerId)
{
    std::lock_guard<std::mutex> lock(m_timersMutex);
    
    auto it = m_timers.find(timerId);
    if (it == m_timers.end()) {
        return TIMERR_NOCANDO;
    }
    
    // Stop the timer
    it->second->active = false;
    
    // Wait for thread to finish
    if (it->second->thread.joinable()) {
        it->second->thread.join();
    }
    
    m_timers.erase(it);
    
    return TIMERR_NOERROR;
}

MMRESULT MultimediaTimerManager::timeGetDevCaps(LPTIMECAPS caps, UINT cbcaps)
{
    if (!caps || cbcaps < sizeof(TIMECAPS)) {
        return TIMERR_NOCANDO;
    }
    
    caps->wPeriodMin = 1;    // 1ms minimum resolution
    caps->wPeriodMax = 1000; // 1000ms maximum resolution
    
    return TIMERR_NOERROR;
}

MMRESULT MultimediaTimerManager::timeBeginPeriod(UINT period)
{
    // This would normally set the system timer resolution
    // For our implementation, we just return success
    return TIMERR_NOERROR;
}

MMRESULT MultimediaTimerManager::timeEndPeriod(UINT period)
{
    // This would normally restore the system timer resolution
    // For our implementation, we just return success
    return TIMERR_NOERROR;
}

void MultimediaTimerManager::timerThread(UINT timerId)
{
    TimerInfo* timer = nullptr;
    
    {
        std::lock_guard<std::mutex> lock(m_timersMutex);
        auto it = m_timers.find(timerId);
        if (it != m_timers.end()) {
            timer = it->second.get();
        }
    }
    
    if (!timer) {
        return;
    }
    
    if (timer->flags & TIME_PERIODIC) {
        // Periodic timer
        while (timer->active) {
            std::this_thread::sleep_for(std::chrono::milliseconds(timer->delay));
            
            if (timer->active) {
                // Call the callback function
                timer->callback(timerId, 0, timer->user, 0, 0);
            }
        }
    } else {
        // One-shot timer
        std::this_thread::sleep_for(std::chrono::milliseconds(timer->delay));
        
        if (timer->active) {
            // Call the callback function
            timer->callback(timerId, 0, timer->user, 0, 0);
            
            // Mark as inactive for one-shot timers
            timer->active = false;
        }
    }
}

#endif // !_WIN32