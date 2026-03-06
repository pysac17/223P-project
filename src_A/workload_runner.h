#ifndef WORKLOAD_RUNNER_H
#define WORKLOAD_RUNNER_H

#include "../shared/interfaces.h"
#include "workload_parser.h"
#include "hotset_selector.h"
#include "stats_collector.h"
#include <vector>
#include <memory>

class WorkloadRunner {
public:
    // Run the workload for durationSeconds with numThreads.
    // db: storage; templates: from parser; hotsetSelector: key selection;
    // protocol: OCC or 2PL; statsCollector: records each txn attempt.
    WorkloadStats run(StorageLayer* db,
                      const std::vector<TxnTemplate>& templates,
                      HotsetSelector& hotsetSelector,
                      CCProtocol* protocol,
                      StatsCollector& statsCollector,
                      int numThreads,
                      int durationSeconds);
};

#endif