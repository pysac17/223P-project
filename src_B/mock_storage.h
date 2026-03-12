#include "../shared/interfaces.h"
#include <unordered_map>

class MockStorage : public StorageLayer {
    std::unordered_map<std::string, std::string> data;
public:
    std::string get(const std::string& key) override {
        return data.count(key) ? data[key] : "";
    }
    void put(const std::string& key, const std::string& value) override {
        data[key] = value;
    }
    void remove(const std::string& key) override {
        data.erase(key);
    }
    uint64_t getVersion(const std::string& key) override { return 0; }
    void incrementVersion(const std::string& key) override {}
};