#include "lock_manager.h"
#include <algorithm>

void LockManager::initKey(const std::string& key) {
    std::lock_guard<std::mutex> guard(globalMutex);
    lockTable[key]; // Initializes the entry
}

bool LockManager::acquireAll(int txnId, const std::vector<std::string>& keys) {
    std::lock_guard<std::mutex> guard(globalMutex);
    
    // Sort keys for consistent ordering (prevents deadlocks)
    std::vector<std::string> sortedKeys = keys;
    std::sort(sortedKeys.begin(), sortedKeys.end());
    
    // Check if ALL keys are available
    for (const auto& key : sortedKeys) {
        LockEntry& entry = lockTable[key];
        if (entry.isLocked && entry.ownerTxnId != txnId) {
            return false; // Key is locked by someone else
        }
    }
    
    // Acquire ALL keys
    for (const auto& key : sortedKeys) {
        LockEntry& entry = lockTable[key];
        entry.isLocked = true;
        entry.ownerTxnId = txnId;
    }
    
    return true;
}

void LockManager::releaseAll(int txnId, const std::vector<std::string>& keys) {
    std::lock_guard<std::mutex> guard(globalMutex);
    
    for (const auto& key : keys) {
        auto it = lockTable.find(key);
        if (it != lockTable.end()) {
            LockEntry& entry = it->second;
            if (entry.ownerTxnId == txnId) {
                entry.isLocked = false;
                entry.ownerTxnId = -1;
            }
        }
    }
}

bool LockManager::isLocked(const std::string& key) {
    std::lock_guard<std::mutex> guard(globalMutex);
    auto it = lockTable.find(key);
    return (it != lockTable.end()) ? it->second.isLocked : false;
}

int LockManager::getLockCount() {
    std::lock_guard<std::mutex> guard(globalMutex);
    int count = 0;
    for (const auto& pair : lockTable) {
        if (pair.second.isLocked) count++;
    }
    return count;
}
