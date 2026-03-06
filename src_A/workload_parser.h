#ifndef WORKLOAD_PARSER_H
#define WORKLOAD_PARSER_H

#include <string>
#include <vector>

// One operation inside a transaction template (e.g. READ, COMPUTE, WRITE).
struct TxnOp {
    enum Type { READ, WRITE, COMPUTE };
    Type type;
    std::string varName;   // variable being assigned or written to (e.g. "value1")
    std::string keySlot;   // which input slot this maps to (e.g. "key1")
    std::string expr;     // for COMPUTE only: expression string (e.g. "value1 + 1")
};

// One transaction template: input slot names + ordered list of ops.
struct TxnTemplate {
    std::vector<std::string> inputSlots;   // e.g. ["key1", "key2"] — slot names, not real keys
    std::vector<TxnOp> operations;
};

// Result of parsing a full workload file.
struct ParsedWorkload {
    std::vector<std::pair<std::string, std::string>> initialData;  // from INSERT block (key, value)
    std::vector<std::string> allKeys;                               // keys from INSERT (keyspace)
    std::vector<TxnTemplate> templates;                             // from WORKLOAD block
};

class WorkloadParser {
public:
    // Parse the file at path; return parsed workload or throw on error.
    ParsedWorkload parse(const std::string& filepath);
};

#endif