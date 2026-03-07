## Build Instructions
1. Install RocksDB: sudo apt-get install librocksdb-dev  (or brew install rocksdb on Mac)
2. Compile: Using the Makefile (Recommended):
```bash
make
```

Or manually (note the paths to src_A and src_B folders):
```bash
g++ -std=c++17 -O2 -o txn_system main.cpp storage_layer.cpp workload_parser.cpp 
    workload_runner.cpp hotset_selector.cpp stats_collector.cpp occ_transaction.cpp 
    occ_manager.cpp lock_manager.cpp twopl_transaction.cpp twopl_manager.cpp 
    livelock_prevention.cpp -lrocksdb -lpthread
```

## Input Data Preparation (IMPORTANT)
The system requires a single input file containing both the initial data (INSERT block) and the transaction logic (WORKLOAD block).
Since the provided data files are separate, you must combine them before running:

```bash
# Combine Workload 1
cat data/input1.txt data/workload1.txt > data/full_workload1.txt

# Combine Workload 2
cat data/input2.txt data/workload2.txt > data/full_workload2.txt
```

## How to Run
```bash
# Example: Run 2PL on Workload 1
./txn_system --protocol 2pl --threads 8 --hotset-size 10 --contention 0.5 
             --workload data/full_workload1.txt --duration 30 --output results/run1.csv

# Example: Run OCC on Workload 2
./txn_system --protocol occ --threads 8 --hotset-size 10 --contention 0.5 
             --workload data/full_workload2.txt --duration 30 --output results/run2.csv
```

## Directly run the executable
```bash
chmod +x run_experiments.sh
./run_experiments.sh
```

## Parameters
--protocol      : occ or 2pl
--threads       : number of concurrent worker threads (e.g., 1, 2, 4, 8, 16)
--hotset-size   : number of keys in the hotset (e.g., 10)
--contention    : probability of picking a hot key (0.0 = no contention, 1.0 = full contention)
--workload      : path to the workload definition file
--duration      : how many seconds to run the workload
--output        : path to write per-transaction CSV results

## Dependencies
- RocksDB (librocksdb-dev)
- C++17 or later
- pthreads