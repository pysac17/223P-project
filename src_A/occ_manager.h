#ifndef OCC_MANAGER_H
#define OCC_MANAGER_H

#include "../shared/interfaces.h"
#include <memory>
#include <mutex>

class OCCTransaction;

class OCCManager : public CCProtocol {
public:
    explicit OCCManager(StorageLayer* db);

    std::shared_ptr<Transaction> createTransaction(StorageLayer* db,
        const std::vector<std::string>& keys) override;
    std::string name() override { return "OCC"; }

private:
    StorageLayer* storage_;
    std::mutex validationMutex_;
};

#endif