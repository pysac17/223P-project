#!/bin/bash
# A8 — OCC experiments. Run from project root. Saves CSVs to experiments/

set -e
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_ROOT"

INPUT=data/input1.txt
WORKLOAD=data/workload1.txt
DURATION=30
HOTSET=10
BIN=./txn_system

mkdir -p experiments

# Reset DB so each run starts from same initial data
reset_db() { rm -rf db_data; }

echo "=== Set 1: Throughput vs Thread Count (contention=0.5) ==="
for THREADS in 1 2 4 8 16; do
  reset_db
  echo "  threads=$THREADS ..."
  $BIN --input "$INPUT" --workload "$WORKLOAD" --protocol occ \
    --threads $THREADS --hotset-size $HOTSET --contention 0.5 \
    --duration $DURATION --output "experiments/occ_workload1_threads${THREADS}_contention0.5.csv"
done

echo "=== Set 2: Throughput vs Contention (threads=8) ==="
for CONT in 0.0 0.2 0.4 0.6 0.8 1.0; do
  reset_db
  echo "  contention=$CONT ..."
  $BIN --input "$INPUT" --workload "$WORKLOAD" --protocol occ \
    --threads 8 --hotset-size $HOTSET --contention $CONT \
    --duration $DURATION --output "experiments/occ_workload1_threads8_contention${CONT}.csv"
done

echo "=== Done. CSVs in experiments/ ==="
ls -la experiments/
