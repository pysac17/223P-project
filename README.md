## Build Instructions
1. Install RocksDB: `brew install rocksdb` on Mac or `sudo apt-get install librocksdb-dev` on Ubuntu
2. Compile: Using Makefile (Recommended):
```bash
make
```

The Makefile automatically includes all necessary source files from both src_A and src_B folders.

## System Architecture
This project implements a multi-threaded transaction processing system with two concurrency control protocols:

### Concurrency Control Protocols
- **Optimistic Concurrency Control (OCC)**: Transactions execute without locks, validate at commit time
- **Conservative Two-Phase Locking (2PL)**: Transactions acquire ALL required locks before execution, with exponential backoff retry mechanism

### Key Features
- **Deadlock-free**: Conservative 2PL uses consistent key ordering
- **Livelock prevention**: Exponential backoff with jitter and randomized delays
- **High performance**: Atomic operations and optimized lock management
- **Thread-safe**: All lock operations use atomic variables

## How to Run
```bash
# Example: Run Conservative 2PL on Workload 1
./txn_system --input data/input1.txt --workload data/workload1.txt --protocol 2pl --threads 8 --hotset-size 10 --contention 0.5 \
             --duration 30 --output results/run1.csv

# Example: Run OCC on Workload 2
./txn_system --input data/input2.txt --workload data/workload2.txt --protocol occ --threads 8 --hotset-size 10 --contention 0.5 \
             --duration 30 --output results/run2.csv
```

## Run All Experiments
```bash
chmod +x run_experiments.sh
./run_experiments.sh
```

This script runs comprehensive experiments:
- **Experiment 1**: Vary thread counts (1, 2, 4, 8, 16) at fixed contention (0.5)
- **Experiment 2**: Vary contention levels (0.0, 0.2, 0.4, 0.6, 0.8, 1.0) at fixed threads (8)
- Generates CSV files for both protocols and both workloads

## Parameters
--input         : path to initial data file (e.g., data/input1.txt)
--workload      : path to the workload definition file (e.g., data/workload1.txt)
--protocol      : occ or 2pl
--threads       : number of concurrent worker threads (e.g., 1, 2, 4, 8, 16)
--hotset-size   : number of keys in the hotset (e.g., 10)
--contention    : probability of picking a hot key (0.0 = no contention, 1.0 = full contention)
--duration      : how many seconds to run the workload
--output        : path to write per-transaction CSV results

## Dependencies
- RocksDB (librocksdb-dev)
- C++20 or later (required for atomic operations)
- pthreads
- Standard C++ libraries (random, chrono, algorithm)

## Performance Notes
- **Conservative 2PL**: Uses exponential backoff with jitter to prevent livelocks
- **Maximum retries**: 50 attempts per transaction before giving up
- **Atomic operations**: All lock state changes are thread-safe
- **Fair ordering**: Global timestamps ensure FIFO lock acquisition
- **Zero deadlocks**: Consistent key ordering prevents circular wait conditions