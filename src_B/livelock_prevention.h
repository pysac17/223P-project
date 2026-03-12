#ifndef LIVELOCK_PREVENTION_H
#define LIVELOCK_PREVENTION_H

#include <random>
#include <cstdint>
#include <algorithm>

class LivelockGuard {
    int retryCount = 0;
    uint64_t txnStartTimestamp;
    
public:
    static const int BASE_DELAY_MS = 1;
    static const int MAX_DELAY_MS = 50;
    static const int MAX_RETRIES = 50;

    LivelockGuard(uint64_t startTs) : txnStartTimestamp(startTs) {}

    // Call this after a failed lock acquire attempt
    int getBackoffMs(std::mt19937& rng) {
        retryCount++;
        
        if (retryCount > MAX_RETRIES) {
            // Forced reset to break livelock cycles - longer random delay
            return 200 + (rng() % 300);
        }
        
        // Exponential backoff with jitter to prevent synchronized retries
        int delay = BASE_DELAY_MS * (1 << std::min(retryCount, 5));
        delay = std::min(delay, MAX_DELAY_MS);
        
        // Add jitter: ±50% of delay
        int jitterRange = delay / 2;
        int jitter = (rng() % (2 * jitterRange + 1)) - jitterRange;
        
        return std::max(1, delay + jitter);
    }

    int getRetryCount() const { return retryCount; }
    
    bool shouldGiveUp() const { return retryCount >= MAX_RETRIES; }
};

#endif