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
    };

    std::unordered_map<std::string, LockEntry> lockTable;
    std::mutex globalMutex; // Single mutex for simplicity and correctness

public:
    void initKey(const std::string& key);
    
    // Simple Conservative 2PL: Acquire all or none
    bool acquireAll(int txnId, const std::vector<std::string>& keys);
    
    // Release all locks for this transaction
    void releaseAll(int txnId, const std::vector<std::string>& keys);

    // Helpers
    bool isLocked(const std::string& key);
    int getLockCount();
};

#endif
