#include "lock_manager.h"

bool LockManager::acquireAll(int txnId, const std::vector<std::string>& keys) {
    std::lock_guard<std::mutex> guard(tableMutex);

    // Phase 1: Check if ALL keys are available
    for (const auto& key : keys) {
        if (lockTable[key].isLocked && lockTable[key].ownerTxnId != txnId) {
            return false; // Conflict detected
        }
    }

    // Phase 2: Acquire all keys
    for (const auto& key : keys) {
        if (!lockTable[key].isLocked) {
            lockTable[key].isLocked = true;
            lockTable[key].ownerTxnId = txnId;
        }
    }
    return true;
}

void LockManager::releaseAll(int txnId) {
    std::lock_guard<std::mutex> guard(tableMutex);
    for (auto& pair : lockTable) {
        if (pair.second.ownerTxnId == txnId) {
            pair.second.isLocked = false;
            pair.second.ownerTxnId = -1;
        }
    }
}

bool LockManager::isLocked(const std::string& key) {
    std::lock_guard<std::mutex> guard(tableMutex);
    return lockTable[key].isLocked;
}

int LockManager::getLockCount() {
    std::lock_guard<std::mutex> guard(tableMutex);
    int count = 0;
    for (const auto& pair : lockTable) {
        if (pair.second.isLocked) count++;
    }
    return count;
}