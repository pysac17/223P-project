#ifndef OCC_TRANSACTION_H
#define OCC_TRANSACTION_H

#include "../shared/interfaces.h"
#include <unordered_map>
#include <string>
#include <mutex>

class OCCManager;

class OCCTransaction : public Transaction {
public:
    OCCTransaction(StorageLayer* storage, std::mutex* validationMutex);

    void begin() override;
    std::string read(const std::string& key) override;
    void write(const std::string& key, const std::string& value) override;
    bool commit() override;

private:
    StorageLayer* storage_;
    std::mutex* validationMutex_;
    std::unordered_map<std::string, uint64_t> readSet_;
    std::unordered_map<std::string, std::string> writeBuffer_;
};

#endif