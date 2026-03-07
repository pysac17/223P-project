#!/bin/bash

mkdir -p exp1 exp2

echo "=== Experiment 1: Vary Threads ==="
for protocol in occ 2pl; do
  for threads in 1 2 4 8 16; do
    echo "Running $protocol with $threads threads..."
    ./txn_system --protocol $protocol --threads $threads --hotset-size 10 --contention 0.5 \
      --workload data/full_workload1.txt --duration 10 --output exp1/${protocol}_t${threads}_workload1.csv
  done
  for threads in 1 2 4 8 16; do
    echo "Running $protocol with $threads threads..."
    ./txn_system --protocol $protocol --threads $threads --hotset-size 10 --contention 0.5 \
      --workload data/full_workload2.txt --duration 10 --output exp1/${protocol}_t${threads}_workload2.csv
  done
done

echo "=== Experiment 2: Vary Contention ==="
for protocol in occ 2pl; do
  for contention in 0.0 0.2 0.4 0.6 0.8 1.0; do
    echo "Running $protocol with contention $contention..."
    ./txn_system --protocol $protocol --threads 8 --hotset-size 10 --contention $contention \
      --workload data/full_workload1.txt --duration 10 --output exp2/${protocol}_c${contention}_workload1.csv
  done
  for contention in 0.0 0.2 0.4 0.6 0.8 1.0; do
    echo "Running $protocol with contention $contention..."
    ./txn_system --protocol $protocol --threads 8 --hotset-size 10 --contention $contention \
      --workload data/full_workload2.txt --duration 10 --output exp2/${protocol}_c${contention}_workload2.csv
  done
done

echo "=== All Experiments Complete ==="