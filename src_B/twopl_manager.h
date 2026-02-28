#ifndef TWOPL_MANAGER_H
#define TWOPL_MANAGER_H

#include "../shared/interfaces.h"
#include "lock_manager.h"
#include "twopl_transaction.h"
#include <atomic>
#include <memory>

class TwoPLManager : public CCProtocol {
private:
    LockManager lockManager;
    std::atomic<int> txnIdCounter{0};

public:
    std::shared_ptr<Transaction> createTransaction(
        StorageLayer* db,
        const std::vector<std::string>& keys
    ) override {
        int id = txnIdCounter.fetch_add(1);
        return std::make_shared<TwoPLTransaction>(id, db, &lockManager, keys);
    }

    std::string name() override {
        return "2PL";
    }
};

#endif