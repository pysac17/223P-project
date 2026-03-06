#include "workload_runner.h"
#include <thread>
#include <chrono>
#include <random>
#include <map>
#include <string>
#include <stdexcept>

using namespace std;

namespace {

// Evaluate simple expressions: "varName + 1", "varName - 1", or "varName".
// varMap: variable name -> value (string).
string evalExpr(const string& expr, const map<string, string>& varMap) {
    string e = expr;
    size_t start = e.find_first_not_of(" \t");
    if (start == string::npos) return "";
    size_t end = e.find_last_not_of(" \t");
    if (end != string::npos) e = e.substr(start, end - start + 1);

    if (e.size() >= 4 && e.compare(e.size() - 4, 4, " + 1") == 0) {
        string varName = e.substr(0, e.size() - 4);
        size_t vEnd = varName.find_last_not_of(" \t");
        if (vEnd != string::npos) varName = varName.substr(0, vEnd + 1);
        auto it = varMap.find(varName);
        if (it == varMap.end()) return "0";
        int val = 0;
        try { val = stoi(it->second); } catch (...) { return it->second; }
        return to_string(val + 1);
    }
    if (e.size() >= 4 && e.compare(e.size() - 4, 4, " - 1") == 0) {
        string varName = e.substr(0, e.size() - 4);
        size_t vEnd = varName.find_last_not_of(" \t");
        if (vEnd != string::npos) varName = varName.substr(0, vEnd + 1);
        auto it = varMap.find(varName);
        if (it == varMap.end()) return "0";
        int val = 0;
        try { val = stoi(it->second); } catch (...) { return it->second; }
        return to_string(val - 1);
    }
    auto it = varMap.find(e);
    if (it != varMap.end()) return it->second;
    return "";
}

} // namespace
WorkloadStats WorkloadRunner::run(StorageLayer* db,
    const std::vector<TxnTemplate>& templates,
    HotsetSelector& hotsetSelector,
    CCProtocol* protocol,
    StatsCollector& statsCollector,
    int numThreads,
    int durationSeconds) {
if (templates.empty() || !db || !protocol) return WorkloadStats{};

const auto startWall = chrono::steady_clock::now();
const uint64_t startEpochMs = chrono::duration_cast<chrono::milliseconds>(
chrono::system_clock::now().time_since_epoch()).count();

auto worker = [&](int threadId) {
mt19937 rng(12345u + static_cast<unsigned>(threadId));
uniform_int_distribution<size_t> tplDist(0, templates.size() - 1);
uniform_int_distribution<int> backoffMs(1, 10);

while (true) {
auto elapsed = chrono::steady_clock::now() - startWall;
if (chrono::duration_cast<chrono::seconds>(elapsed).count() >= durationSeconds)
break;

size_t tplIndex = tplDist(rng);
const TxnTemplate& tpl = templates[tplIndex];
vector<string> keys = hotsetSelector.selectKeys(static_cast<int>(tpl.inputSlots.size()), rng);
if (keys.size() != tpl.inputSlots.size()) continue;

map<string, string> slotToKey;
for (size_t i = 0; i < tpl.inputSlots.size(); ++i)
slotToKey[tpl.inputSlots[i]] = keys[i];

shared_ptr<Transaction> txn = protocol->createTransaction(db, keys);
int retryCount = 0;
uint64_t txnStartEpoch = chrono::duration_cast<chrono::milliseconds>(
chrono::system_clock::now().time_since_epoch()).count();
auto txnStart = chrono::steady_clock::now();
bool committed = false;

while (true) {
txn->begin();
map<string, string> varMap;

for (const TxnOp& op : tpl.operations) {
if (op.type == TxnOp::READ) {
string realKey = slotToKey[op.keySlot];
string val = txn->read(realKey);
varMap[op.varName] = val;
} else if (op.type == TxnOp::COMPUTE) {
varMap[op.varName] = evalExpr(op.expr, varMap);
} else if (op.type == TxnOp::WRITE) {
string realKey = slotToKey[op.keySlot];
string value = varMap[op.varName];
txn->write(realKey, value);
}
}

committed = txn->commit();
if (committed) break;

retryCount++;
this_thread::sleep_for(chrono::milliseconds(backoffMs(rng)));
}

auto txnEnd = chrono::steady_clock::now();
double responseMs = chrono::duration<double, std::milli>(txnEnd - txnStart).count();
uint64_t txnEndEpoch = chrono::duration_cast<chrono::milliseconds>(
chrono::system_clock::now().time_since_epoch()).count();

TxnStats s;
s.responseTimeMs = responseMs;
s.committed = true;
s.retryCount = retryCount;
s.protocol = protocol->name();
s.txnType = "TXN" + to_string(1 + (int)(&tpl - &templates[0]));  // or use template index
s.timestampStart = txnStartEpoch;
s.timestampEnd = txnEndEpoch;
statsCollector.recordTxn(s);
}
};

vector<thread> threads;
for (int i = 0; i < numThreads; ++i)
threads.emplace_back(worker, i);
for (auto& t : threads) t.join();

double totalSec = chrono::duration<double>(chrono::steady_clock::now() - startWall).count();
return statsCollector.computeWorkloadStats(totalSec);
}