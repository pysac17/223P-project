#ifndef STORAGE_LAYER_H
#define STORAGE_LAYER_H

#include "../shared/interfaces.h"
#include <rocksdb/db.h>
#include <string>
#include <unordered_map>
#include <mutex>

class RocksDBStorageLayer : public StorageLayer {
public:
    explicit RocksDBStorageLayer(const std::string& db_path);
    ~RocksDBStorageLayer() override;

    std::string get(const std::string& key) override;
    void put(const std::string& key, const std::string& value) override;
    void remove(const std::string& key) override;
    uint64_t getVersion(const std::string& key) override;
    void incrementVersion(const std::string& key) override;

    void clear();

private:
    rocksdb::DB* db;
    std::string db_path;
    std::unordered_map<std::string, uint64_t> versionMap;
    std::mutex versionMutex;
};

#endif