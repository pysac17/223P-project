#ifndef LIVELOCK_PREVENTION_H
#define LIVELOCK_PREVENTION_H

#include <random>
#include <cstdint>
#include <algorithm>

class LivelockGuard {
    int retryCount = 0;
    uint64_t txnStartTimestamp;
    static const int BASE_DELAY_MS = 2;
    static const int MAX_DELAY_MS = 100;
    static const int MAX_RETRIES = 50;

public:
    LivelockGuard(uint64_t startTs) : txnStartTimestamp(startTs) {}

    // Call this after a failed lock acquire attempt
    int getBackoffMs(std::mt19937& rng) {
        retryCount++;
        if (retryCount > MAX_RETRIES) {
            // Forced reset to break livelock cycles
            return 100 + (rng() % 200);
        }
        
        // Exponential backoff with jitter
        int delay = BASE_DELAY_MS * (1 << std::min(retryCount, 6));
        delay = std::min(delay, MAX_DELAY_MS);
        int jitter = (rng() % delay) - delay / 2;
        return std::max(1, delay + jitter);
    }

    int getRetryCount() const { return retryCount; }
};

#endif