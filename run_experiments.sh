#!/bin/bash

mkdir -p exp1 exp2

echo "=== Building system ==="
make clean && make
if [ $? -ne 0 ]; then
    echo "Build failed! Yay."
    exit 1
fi

echo "=== Experiment 1: Vary Threads ==="
for protocol in occ 2pl; do
  for threads in 1 2 4 8 16; do
    echo "Running $protocol with $threads threads..."
    ./txn_system --input data/input1.txt --workload data/workload1.txt --protocol $protocol --threads $threads --hotset-size 10 --contention 0.5 \
      --duration 10 --output exp1/${protocol}_t${threads}_workload1.csv
  done
  for threads in 1 2 4 8 16; do
    echo "Running $protocol with $threads threads..."
    ./txn_system --input data/input2.txt --workload data/workload2.txt --protocol $protocol --threads $threads --hotset-size 10 --contention 0.5 \
      --duration 10 --output exp1/${protocol}_t${threads}_workload2.csv
  done
done

echo "=== Experiment 2: Vary Contention ==="
for protocol in occ 2pl; do
  for contention in 0.0 0.2 0.4 0.6 0.8 1.0; do
    echo "Running $protocol with contention $contention..."
    ./txn_system --input data/input1.txt --workload data/workload1.txt --protocol $protocol --threads 8 --hotset-size 10 --contention $contention \
      --duration 10 --output exp2/${protocol}_c${contention}_workload1.csv
  done
  for contention in 0.0 0.2 0.4 0.6 0.8 1.0; do
    echo "Running $protocol with contention $contention..."
    ./txn_system --input data/input2.txt --workload data/workload2.txt --protocol $protocol --threads 8 --hotset-size 10 --contention $contention \
      --duration 10 --output exp2/${protocol}_c${contention}_workload2.csv
  done
done

echo "=== All Experiments Complete ==="
echo "Results saved in exp1/ and exp2/ directories"