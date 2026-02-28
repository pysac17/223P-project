#ifndef SHARED_INTERFACES_H
#define SHARED_INTERFACES_H

#include <string>
#include <map>
#include <vector>
#include <cstdint>

// ==========================================
// 1. DATA TYPES (Shared by both A and B)
// ==========================================

// Represents a record (key-value map) in the database
using Record = std::map<std::string, std::string>;

// Simple serialization: "field1=val1;field2=val2"
inline std::string serializeRecord(const Record& r) {
    std::string s;
    for (const auto& pair : r) {
        s += pair.first + "=" + pair.second + ";";
    }
    return s;
}

// Simple deserialization
inline Record deserializeRecord(const std::string& s) {
    Record r;
    size_t start = 0;
    size_t end = s.find(';');
    while (end != std::string::npos) {
        std::string token = s.substr(start, end - start);
        size_t eq = token.find('=');
        if (eq != std::string::npos) {
            r[token.substr(0, eq)] = token.substr(eq + 1);
        }
        start = end + 1;
        end = s.find(';', start);
    }
    return r;
}

// ==========================================
// 2. TRANSACTION BASE CLASS (Person B implements this)
// ==========================================

class Transaction {
public:
    virtual ~Transaction() = default;
    
    // Called by WorkloadRunner (Person A's code)
    virtual void begin() = 0;
    virtual std::string read(const std::string& key) = 0;
    virtual void write(const std::string& key, const std::string& value) = 0;
    virtual bool commit() = 0; // true = success, false = abort/retry needed
};

// ==========================================
// 3. STORAGE LAYER INTERFACE (Person A implements, Person B uses)
// ==========================================

class StorageLayer {
public:
    virtual ~StorageLayer() = default;
    
    // Basic CRUD
    virtual std::string get(const std::string& key) = 0;
    virtual void put(const std::string& key, const std::string& value) = 0;
    virtual void remove(const std::string& key) = 0;
    
    // Versioning for OCC (Person A implements, Person B can ignore or use for debugging)
    virtual uint64_t getVersion(const std::string& key) = 0;
    virtual void incrementVersion(const std::string& key) = 0;
};

// ==========================================
// 4. PROTOCOL FACTORY (Person B implements for 2PL)
// ==========================================

class CCProtocol {
public:
    virtual ~CCProtocol() = default;
    
    // Called by WorkloadRunner to create a new transaction instance for a thread
    virtual std::shared_ptr<Transaction> createTransaction(
        StorageLayer* db,
        const std::vector<std::string>& keys
    ) = 0;
    
    virtual std::string name() = 0; // Returns "OCC" or "2PL"
};

#endif // SHARED_INTERFACES_H