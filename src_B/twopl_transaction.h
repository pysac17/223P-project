#ifndef TWOPL_TRANSACTION_H
#define TWOPL_TRANSACTION_H

#include "../shared/interfaces.h"
#include "lock_manager.h"
#include "livelock_prevention.h"
#include <chrono>
#include <memory>

class TwoPLTransaction : public Transaction {
private:
    int txnId;
    StorageLayer* db;
    LockManager* lockManager;
    std::vector<std::string> keys;
    
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;
    bool locksAcquired;

public:
    TwoPLTransaction(int id, StorageLayer* database, LockManager* lm, const std::vector<std::string>& inputKeys)
        : txnId(id), db(database), lockManager(lm), keys(inputKeys), locksAcquired(false) {}

    void begin() override {
        startTime = std::chrono::steady_clock::now();
        // Try to acquire all locks
        if (!lockManager->acquireAll(txnId, keys)) {
            locksAcquired = false;
            // Note: The actual retry logic happens in the WorkloadRunner (Person A's code)
            // But we track state here.
        } else {
            locksAcquired = true;
        }
    }

    std::string read(const std::string& key) override {
        if (!locksAcquired) return ""; // Should not happen if logic is correct
        return db->get(key);
    }

    void write(const std::string& key, const std::string& value) override {
        if (!locksAcquired) return;
        db->put(key, value);
    }

    bool commit() override {
        endTime = std::chrono::steady_clock::now();
        if (locksAcquired) {
            lockManager->releaseAll(txnId);
            return true;
        }
        return false; // Failed to acquire locks initially
    }
    
    double getResponseTimeMs() {
        return std::chrono::duration<double, std::milli>(endTime - startTime).count();
    }
};

#endif