#ifndef LOCK_MANAGER_H
#define LOCK_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

class LockManager {
private:
    struct LockEntry {
        bool isLocked = false;
        int ownerTxnId = -1;
        int waiters = 0;
    };

    std::unordered_map<std::string, LockEntry> lockTable;
    std::mutex tableMutex; // Global mutex for simplicity

public:
    // Attempts to acquire all locks atomically. 
    // Returns false if ANY key is already locked by someone else.
    bool acquireAll(int txnId, const std::vector<std::string>& keys);

    // Releases all locks held by this transaction
    void releaseAll(int txnId);

    // Helper for debugging/testing
    bool isLocked(const std::string& key);
    int getLockCount();
};

#endif