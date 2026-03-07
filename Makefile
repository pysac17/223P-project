# Project root and RocksDB
PROJECT_ROOT := .
ROCKSDB_INC ?= /opt/homebrew/include
ROCKSDB_LIB ?= /opt/homebrew/lib

# Compiler Flags
CXXFLAGS := -std=c++20 -I$(PROJECT_ROOT) -I$(ROCKSDB_INC) -I src_A
LDFLAGS  := -L$(ROCKSDB_LIB) -lrocksdb -lpthread -lsnappy -lz -lbz2 -llz4 -lzstd

# Source Files (Person A + Person B)
SRCS := main.cpp \
        src_A/storage_layer.cpp \
        src_A/workload_parser.cpp \
        src_A/hotset_selector.cpp \
        src_A/stats_collector.cpp \
        src_A/workload_runner.cpp \
        src_A/occ_transaction.cpp \
        src_A/occ_manager.cpp \
        src_B/lock_manager.cpp

.PHONY: all clean

all: txn_system

txn_system: $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f txn_system
