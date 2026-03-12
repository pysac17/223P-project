#ifndef STATS_COLLECTOR_H
#define STATS_COLLECTOR_H

#include "../shared/interfaces.h"
#include <string>
#include <vector>
#include <mutex>

class StatsCollector {
public:
    // Append one transaction attempt (thread-safe).
    void recordTxn(const TxnStats& stats);

    // Compute aggregates from all recorded TxnStats; totalTimeSeconds = wall-clock run duration.
    WorkloadStats computeWorkloadStats(double totalTimeSeconds) const;

    // Write one row per TxnStats to CSV. Headers: protocol, txnType, responseTimeMs, committed, retryCount, timestampStart, timestampEnd
    void exportCSV(const std::string& filepath) const;

private:
    mutable std::mutex mutex_;
    std::vector<TxnStats> txnStats_;
};

#endif