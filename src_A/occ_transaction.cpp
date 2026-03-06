#include "occ_transaction.h"

OCCTransaction::OCCTransaction(StorageLayer* storage, std::mutex* validationMutex)
    : storage_(storage), validationMutex_(validationMutex) {}

void OCCTransaction::begin() {
    readSet_.clear();
    writeBuffer_.clear();
}

std::string OCCTransaction::read(const std::string& key) {
    auto it = writeBuffer_.find(key);
    if (it != writeBuffer_.end())
        return it->second;

    std::string value = storage_->get(key);
    uint64_t version = storage_->getVersion(key);
    readSet_[key] = version;
    return value;
}

void OCCTransaction::write(const std::string& key, const std::string& value) {
    writeBuffer_[key] = value;
}

bool OCCTransaction::commit() {
    std::lock_guard<std::mutex> lock(*validationMutex_);

    for (const auto& p : readSet_) {
        uint64_t currentVersion = storage_->getVersion(p.first);
        if (currentVersion != p.second) {
            readSet_.clear();
            writeBuffer_.clear();
            return false;
        }
    }

    for (const auto& p : writeBuffer_) {
        storage_->put(p.first, p.second);
        storage_->incrementVersion(p.first);
    }
    readSet_.clear();
    writeBuffer_.clear();
    return true;
}