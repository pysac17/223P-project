// A7 — CLI: --input (data/input1.txt) + --workload (data/workload1.txt), no data file edits.

#include "shared/interfaces.h"
#include "src_A/storage_layer.h"
#include "src_A/workload_parser.h"
#include "src_A/hotset_selector.h"
#include "src_A/stats_collector.h"
#include "src_A/workload_runner.h"
#include "src_A/occ_manager.h"

#include <iostream>
#include <string>
#include <cstdlib>

static void usage(const char* prog) {
    std::cerr << "Usage: " << prog
              << " --input <path> --workload <path>"
              << " --protocol <occ|2pl> --threads <N> --hotset-size <N> --contention <0.0-1.0>"
              << " --duration <seconds> --output <path>\n";
}

static std::string getArg(int& i, int argc, char** argv) {
    if (i + 1 >= argc) {
        std::cerr << "Missing value for " << argv[i] << "\n";
        std::exit(1);
    }
    return argv[++i];
}

int main(int argc, char** argv) {
    std::string protocol = "occ";
    int threads = 1;
    int hotsetSize = 10;
    double contention = 0.5;
    std::string inputPath;
    std::string workloadPath;
    int durationSeconds = 30;
    std::string outputPath = "results.csv";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--protocol")   protocol = getArg(i, argc, argv);
        else if (arg == "--threads")       threads = std::atoi(getArg(i, argc, argv).c_str());
        else if (arg == "--hotset-size")   hotsetSize = std::atoi(getArg(i, argc, argv).c_str());
        else if (arg == "--contention")    contention = std::atof(getArg(i, argc, argv).c_str());
        else if (arg == "--input")         inputPath = getArg(i, argc, argv);
        else if (arg == "--workload")      workloadPath = getArg(i, argc, argv);
        else if (arg == "--duration")      durationSeconds = std::atoi(getArg(i, argc, argv).c_str());
        else if (arg == "--output")        outputPath = getArg(i, argc, argv);
        else if (arg == "--help" || arg == "-h") { usage(argv[0]); return 0; }
    }

    if (inputPath.empty() || workloadPath.empty()) {
        std::cerr << "Need both --input and --workload (e.g. data/input1.txt, data/workload1.txt).\n";
        usage(argv[0]);
        return 1;
    }
    if (threads < 1) threads = 1;
    if (durationSeconds < 1) durationSeconds = 1;

    RocksDBStorageLayer db("./db_data");
    WorkloadParser parser;
    ParsedWorkload fromInput = parser.parseInsertOnly(inputPath);
    ParsedWorkload fromWorkload = parser.parseWorkloadOnly(workloadPath);

    ParsedWorkload parsed;
    parsed.initialData = fromInput.initialData;
    parsed.allKeys = fromInput.allKeys;
    parsed.templates = fromWorkload.templates;

    if (parsed.allKeys.empty()) {
        std::cerr << "No keys from --input file.\n";
        return 1;
    }
    if (parsed.templates.empty()) {
        std::cerr << "No transaction templates from --workload file.\n";
        return 1;
    }

    for (const auto& p : parsed.initialData)
        db.put(p.first, p.second);

    HotsetSelector hotsetSelector(parsed.allKeys, hotsetSize, contention);
    StatsCollector statsCollector;

    WorkloadStats w;
    if (protocol == "occ") {
        OCCManager occManager(&db);
        WorkloadRunner runner;
        w = runner.run(&db, parsed.templates, hotsetSelector, &occManager, statsCollector,
                       threads, durationSeconds);
    } else if (protocol == "2pl") {
        std::cerr << "2PL not linked. Use --protocol occ.\n";
        return 1;
    } else {
        std::cerr << "Unknown protocol. Use occ or 2pl.\n";
        return 1;
    }

    std::cout << "totalTimeSeconds=" << w.totalTimeSeconds << "\n"
              << "totalCommitted=" << w.totalCommitted << "\n"
              << "totalAborted=" << w.totalAborted << "\n"
              << "totalRetries=" << w.totalRetries << "\n"
              << "throughput=" << w.throughput << "\n"
              << "avgResponseTimeMs=" << w.avgResponseTimeMs << "\n";

    statsCollector.exportCSV(outputPath);
    std::cout << "CSV written to " << outputPath << "\n";
    return 0;
}
