#include "stats_collector.h"
#include <fstream>
#include <algorithm>

void StatsCollector::recordTxn(const TxnStats& stats) {
    std::lock_guard<std::mutex> lock(mutex_);
    txnStats_.push_back(stats);
}

WorkloadStats StatsCollector::computeWorkloadStats(double totalTimeSeconds) const {
    std::lock_guard<std::mutex> lock(mutex_);

    WorkloadStats w;
    w.totalTimeSeconds = totalTimeSeconds;
    w.perTxn = txnStats_;

    w.totalCommitted = 0;
    w.totalAborted = 0;
    w.totalRetries = 0;
    double sumResponseTimeMs = 0.0;

    for (const auto& t : txnStats_) {
        if (t.committed) ++w.totalCommitted;
        else ++w.totalAborted;
        w.totalRetries += t.retryCount;
        sumResponseTimeMs += t.responseTimeMs;
    }

    w.throughput = (totalTimeSeconds > 0) ? (w.totalCommitted / totalTimeSeconds) : 0.0;
    w.avgResponseTimeMs = txnStats_.empty() ? 0.0 : (sumResponseTimeMs / static_cast<double>(txnStats_.size()));

    return w;
}

void StatsCollector::exportCSV(const std::string& filepath) const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ofstream out(filepath);
    if (!out.is_open()) return;

    out << "protocol,txnType,responseTimeMs,committed,retryCount,timestampStart,timestampEnd\n";
    for (const auto& t : txnStats_) {
        out << t.protocol << ","
            << t.txnType << ","
            << t.responseTimeMs << ","
            << (t.committed ? 1 : 0) << ","
            << t.retryCount << ","
            << t.timestampStart << ","
            << t.timestampEnd << "\n";
    }
}