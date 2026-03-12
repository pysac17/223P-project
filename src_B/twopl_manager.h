#ifndef TWOPL_MANAGER_H
#define TWOPL_MANAGER_H

#include "../shared/interfaces.h"
#include "lock_manager.h"
#include "twopl_transaction.h"
#include <memory>
#include <vector>

class TwoPLManager : public CCProtocol {
private:
    LockManager lockManager;

public:
    TwoPLManager() {}
    
    std::shared_ptr<Transaction> createTransaction(
        StorageLayer* db,
        const std::vector<std::string>& keys
    ) override {
        return std::make_shared<TwoPLTransaction>(0, db, &lockManager, keys);
    }
    
    std::string name() override {
        return "2PL";
    }
};

#endif
