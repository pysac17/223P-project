#ifndef TWOPL_TRANSACTION_H
#define TWOPL_TRANSACTION_H

#include "../shared/interfaces.h"
#include "lock_manager.h"
#include <chrono>
#include <thread>
#include <random>

class TwoPLTransaction : public Transaction {
private:
    int txnId;
    StorageLayer* db;
    LockManager* lockManager;
    std::vector<std::string> keys;
    
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;
    bool locksAcquired;
    int retryCount;
    
    std::mt19937 rng;

public:
    TwoPLTransaction(int id, StorageLayer* database, LockManager* lm, const std::vector<std::string>& inputKeys)
        : txnId(id), db(database), lockManager(lm), keys(inputKeys), 
          locksAcquired(false), retryCount(0), 
          rng(std::random_device{}()) {}

    void begin() override {
        startTime = std::chrono::steady_clock::now();
        
        // Simple retry with exponential backoff
        while (!locksAcquired && retryCount < 20) {
            if (lockManager->acquireAll(txnId, keys)) {
                locksAcquired = true;
                break;
            }
            
            retryCount++;
            // Simple exponential backoff: 1ms, 2ms, 4ms, 8ms, 16ms, 32ms, 64ms, 100ms
            int delay = std::min(100, (1 << std::min(retryCount, 6)));
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
    }

    std::string read(const std::string& key) override {
        if (!locksAcquired) return "";
        return db->get(key);
    }

    void write(const std::string& key, const std::string& value) override {
        if (!locksAcquired) return;
        db->put(key, value);
    }

    bool commit() override {
        endTime = std::chrono::steady_clock::now();
        lockManager->releaseAll(txnId, keys);
        return locksAcquired;
    }
    
    double getResponseTimeMs() {
        return std::chrono::duration<double, std::milli>(endTime - startTime).count();
    }
    
    int getRetryCount() const { return retryCount; }
};

#endif
