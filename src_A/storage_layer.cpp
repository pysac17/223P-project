#include "storage_layer.h"
#include <rocksdb/options.h>
#include <rocksdb/status.h>
#include <rocksdb/write_batch.h>
#include <mutex>

RocksDBStorageLayer::RocksDBStorageLayer(const std::string& db_path) : db(nullptr) {
    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, db_path, &db);
    if (!status.ok()) {
        db = nullptr;
    }
}

RocksDBStorageLayer::~RocksDBStorageLayer() {
    if (db) {
        delete db;
        db = nullptr;
    }
}

std::string RocksDBStorageLayer::get(const std::string& key) {
    if (!db) return "";
    std::string value;
    rocksdb::Status status = db->Get(rocksdb::ReadOptions(), key, &value);
    if (status.IsNotFound()) return "";
    if (!status.ok()) return "";
    return value;
}

void RocksDBStorageLayer::put(const std::string& key, const std::string& value) {
    if (!db) return;
    db->Put(rocksdb::WriteOptions(), key, value);
}

void RocksDBStorageLayer::remove(const std::string& key) {
    if (!db) return;
    db->Delete(rocksdb::WriteOptions(), key);
}

uint64_t RocksDBStorageLayer::getVersion(const std::string& key) {
    std::lock_guard<std::mutex> lock(versionMutex);
    auto it = versionMap.find(key);
    if (it == versionMap.end()) return 0;
    return it->second;
}

void RocksDBStorageLayer::incrementVersion(const std::string& key) {
    std::lock_guard<std::mutex> lock(versionMutex);
    versionMap[key]++;
}

void RocksDBStorageLayer::clear() {
    if (db) {
        delete db;
        db = nullptr;
    }
    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, db_path, &db);
    if (!status.ok()) db = nullptr;
    std::lock_guard<std::mutex> lock(versionMutex);
    versionMap.clear();
}