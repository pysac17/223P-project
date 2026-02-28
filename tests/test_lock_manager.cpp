#include <iostream>
#include <cassert>
#include "../src_B/lock_manager.h"
#include "../src_B/mock_storage.h"
#include "../src_B/twopl_transaction.h"
#include "../src_B/twopl_manager.h"  

int main() {
    std::cout << "Running LockManager Tests...\n";
    
    LockManager lm;
    
    // Test 1: Basic Acquire
    assert(lm.acquireAll(1, {"keyA", "keyB"}) == true);
    assert(lm.isLocked("keyA"));
    assert(lm.getLockCount() == 2);
    
    // Test 2: Conflict
    assert(lm.acquireAll(2, {"keyB", "keyC"}) == false); 
    assert(lm.getLockCount() == 2); 
    
    // Test 3: Release
    lm.releaseAll(1);
    assert(lm.getLockCount() == 0);
    
    std::cout << "All LockManager Tests Passed!\n";
    
    std::cout << "Running Transaction Logic Test...\n";
    MockStorage storage;
    storage.put("x", "100");
    
    TwoPLManager manager;
    auto txn = manager.createTransaction(&storage, {"x"});
    
    txn->begin();
    std::string val = txn->read("x");
    int intVal = std::stoi(val);
    intVal++;
    txn->write("x", std::to_string(intVal));
    bool success = txn->commit();
    
    assert(success);
    assert(storage.get("x") == "101");
    
    std::cout << "Transaction Logic Test Passed!\n";
    
    return 0;
}