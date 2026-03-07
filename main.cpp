#include <iostream>
#include <string>
#include <cstdlib>
#include "src_A/storage_layer.h"
#include "src_A/workload_parser.h"
#include "src_A/hotset_selector.h"
#include "src_A/workload_runner.h"
#include "src_A/stats_collector.h"
#include "src_A/occ_manager.h"
#include "src_B/twopl_manager.h"

void printUsage() {
    std::cerr << "Usage: ./txn_system --protocol <occ|2pl> --threads <n> --hotset-size <n> --contention <0.0-1.0> --workload <file> --duration <sec> --output <file>\n";
}

int main(int argc, char* argv[]) {
    // 1. Parse Arguments
    std::string protocol = "occ";
    int threads = 1;
    int hotsetSize = 10;
    double contention = 0.5;
    std::string workloadFile = "";
    int duration = 10;
    std::string outputFile = "results.csv";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--protocol" && i + 1 < argc) protocol = argv[++i];
        else if (arg == "--threads" && i + 1 < argc) threads = std::stoi(argv[++i]);
        else if (arg == "--hotset-size" && i + 1 < argc) hotsetSize = std::stoi(argv[++i]);
        else if (arg == "--contention" && i + 1 < argc) contention = std::stod(argv[++i]);
        else if (arg == "--workload" && i + 1 < argc) workloadFile = argv[++i];
        else if (arg == "--duration" && i + 1 < argc) duration = std::stoi(argv[++i]);
        else if (arg == "--output" && i + 1 < argc) outputFile = argv[++i];
    }

    if (workloadFile.empty()) {
        printUsage();
        return 1;
    }

    std::cout << "Config: Protocol=" << protocol << ", Threads=" << threads 
              << ", Contention=" << contention << ", Duration=" << duration << "s\n";

    // 2. Initialize Storage
    RocksDBStorageLayer db("./db_data");

    // 3. Parse Workload
    WorkloadParser parser;
    ParsedWorkload workload;
    try {
        workload = parser.parse(workloadFile);
    } catch (const std::exception& e) {
        std::cerr << "Error parsing workload: " << e.what() << "\n";
        return 1;
    }

    // 4. Load Initial Data
    std::cout << "Loading " << workload.initialData.size() << " initial records...\n";
    for (const auto& pair : workload.initialData) {
        db.put(pair.first, pair.second);
    }

    // 5. Setup Hotset & Stats
    HotsetSelector selector(workload.allKeys, hotsetSize, contention);
    StatsCollector stats;

    // 6. Select Protocol
    CCProtocol* activeProtocol = nullptr;
    OCCManager occManager(&db);
    TwoPLManager twoPLManager;

    if (protocol == "occ") {
        activeProtocol = &occManager;
    } else if (protocol == "2pl") {
        activeProtocol = &twoPLManager;
    } else {
        std::cerr << "Unknown protocol: " << protocol << "\n";
        return 1;
    }

    // 7. Run Workload
    std::cout << "Running workload...\n";
    WorkloadRunner runner;
    WorkloadStats results = runner.run(&db, workload.templates, selector, activeProtocol, stats, threads, duration);

    // 8. Output Results
    std::cout << "Workload Complete.\n";
    std::cout << "Throughput: " << results.throughput << " txns/sec\n";
    std::cout << "Avg Response Time: " << results.avgResponseTimeMs << " ms\n";
    std::cout << "Total Committed: " << results.totalCommitted << "\n";
    std::cout << "Total Retries: " << results.totalRetries << "\n";

    stats.exportCSV(outputFile);
    std::cout << "Results saved to " << outputFile << "\n";

    return 0;
}