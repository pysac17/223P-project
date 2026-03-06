#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include "../shared/interfaces.h"
#include "../src_B/mock_storage.h"
#include "../src_B/twopl_manager.h"
#include "../src_B/livelock_prevention.h"

// Global stats
std::atomic<int> successCount(0);
std::atomic<int> failCount(0);

void workerThread(int threadId, TwoPLManager* manager, MockStorage* db, int numTxns) {
    // Each thread needs its own RNG for livelock prevention
    std::mt19937 rng(threadId + 42);
    
    // Simulate keys (like input1.txt bank accounts)
    std::vector<std::string> allKeys;
    for(int i=1; i<=10; i++) { 
        allKeys.push_back("A_" + std::to_string(i)); 
    }

    for (int i = 0; i < numTxns; i++) {
        // 1. Pick random keys (Simulating Hotset Selection)
        std::string key1 = allKeys[rng() % allKeys.size()];
        std::string key2 = allKeys[rng() % allKeys.size()];
        
        // 2. Create Transaction
        auto txn = manager->createTransaction(db, {key1, key2});
        
        // 3. Run Transaction Logic (Simulating Workload 1: Transfer)
        txn->begin();
        
        if (txn->commit()) {
            successCount++;
        } else {
            failCount++;
            // Here is where you would retry, but for this test we just count the fail
        }
    }
}

int main() {
    std::cout << "Starting High-Contention Stress Test...\n";

    // Setup Mock Database with some data
    MockStorage db;
    for(int i=1; i<=10; i++) {
        db.put("A_" + std::to_string(i), "1000");
    }

    TwoPLManager manager;

    // Run 8 threads, each doing 1000 transactions
    int numThreads = 8;
    int txnsPerThread = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(workerThread, i, &manager, &db, txnsPerThread);
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    // Results
    std::cout << "Test Complete!\n";
    std::cout << "Total Time: " << duration.count() << " seconds\n";
    std::cout << "Throughput: " << (successCount.load() / duration.count()) << " txns/sec\n";
    std::cout << "Success: " << successCount.load() << "\n";
    std::cout << "Failures (Lock Contentions): " << failCount.load() << "\n";
    
    double failRate = (double)failCount.load() / (successCount.load() + failCount.load()) * 100.0;
    std::cout << "Failure Rate: " << failRate << "%\n";

    return 0;
}