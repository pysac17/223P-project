# Project root and RocksDB (override if RocksDB is elsewhere)
PROJECT_ROOT := .
ROCKSDB_INC ?= /opt/homebrew/include
ROCKSDB_LIB ?= /opt/homebrew/lib

# Include path so we can use #include "shared/..." and #include <rocksdb/...>
CXXFLAGS := -std=c++20 -I$(PROJECT_ROOT) -I$(ROCKSDB_INC)
LDFLAGS := -L$(ROCKSDB_LIB) -lrocksdb -lpthread -lsnappy -lz -lbz2 -llz4 -lzstd

# Add more .cpp files here as you create them
SRCS := src_A/storage_layer.cpp main.cpp

.PHONY: all clean
all: txn_system

txn_system: $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f txn_system
