#include "occ_manager.h"
#include "occ_transaction.h"

OCCManager::OCCManager(StorageLayer* db) : storage_(db) {}

std::shared_ptr<Transaction> OCCManager::createTransaction(StorageLayer* db,
    const std::vector<std::string>& keys) {
    (void)keys;
    return std::make_shared<OCCTransaction>(db, &validationMutex_);
}