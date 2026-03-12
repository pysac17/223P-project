#include "workload_parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

using namespace std;

ParsedWorkload WorkloadParser::parse(const std::string& filepath) {
    ifstream f(filepath);
    if (!f.is_open())
        throw runtime_error("WorkloadParser: cannot open " + filepath);

    ParsedWorkload out;
    string line;

    // ----- INSERT block -----
    while (getline(f, line)) {
        // Trim leading/trailing whitespace (you can use a simple loop or skip spaces).
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        string trimmed = (end == string::npos) ? line.substr(start) : line.substr(start, end - start + 1);

        if (trimmed == "INSERT") continue;   // skip the INSERT line
        if (trimmed == "END") break;         // end of INSERT block

        // Format: "KEY: user_001, VALUE: balance=100;age=25"
        if (trimmed.find("KEY:") != 0) continue;
        size_t keyStart = trimmed.find("KEY:") + 4;
        size_t keyEnd = trimmed.find(',', keyStart);
        if (keyEnd == string::npos) continue;
        string key = trimmed.substr(keyStart, keyEnd - keyStart);
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);

        size_t valStart = trimmed.find("VALUE:");
        if (valStart == string::npos) continue;
        valStart += 6;
        string value = trimmed.substr(valStart);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        out.initialData.push_back({key, value});
    }

    // Build allKeys from initialData (keyspace for hotset selection later).
    for (const auto& p : out.initialData)
        out.allKeys.push_back(p.first);
    
            // ----- WORKLOAD block -----
    while (getline(f, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        string trimmed = (end == string::npos) ? line.substr(start) : line.substr(start, end - start + 1);

        if (trimmed.find("TRANSACTION") == 0 && trimmed.find("INPUTS:") != string::npos) {
            TxnTemplate tpl;
            // Parse "TRANSACTION (INPUTS: key1, key2)" or "TRANSACTION (INPUTS: key1, key2, key3)"
            size_t inStart = trimmed.find("INPUTS:") + 7;
            size_t inEnd = trimmed.find(')', inStart);
            if (inEnd == string::npos) continue;
            string inputStr = trimmed.substr(inStart, inEnd - inStart);
            stringstream ss(inputStr);
            string slot;
            while (ss >> slot) {
                if (slot.back() == ',') slot.pop_back();
                if (!slot.empty()) tpl.inputSlots.push_back(slot);
            }

            // Read lines until COMMIT (each line is an op or BEGIN/COMMIT).
            while (getline(f, line)) {
                size_t s = line.find_first_not_of(" \t\r\n");
                if (s == string::npos) continue;
                size_t e = line.find_last_not_of(" \t\r\n");
                string t = (e == string::npos) ? line.substr(s) : line.substr(s, e - s + 1);
                if (t.empty()) continue;

                if (t == "BEGIN") continue;
                if (t == "COMMIT") break;

                // "value1 = READ(key1)" -> READ, varName=value1, keySlot=key1
                if (t.find("READ(") != string::npos) {
                    TxnOp op;
                    op.type = TxnOp::READ;
                    size_t eq = t.find('=');
                    if (eq != string::npos) {
                        op.varName = t.substr(0, eq);
                        op.varName.erase(0, op.varName.find_first_not_of(" \t"));
                        op.varName.erase(op.varName.find_last_not_of(" \t") + 1);
                    }
                    size_t openP = t.find("READ(");
                    size_t closeP = t.find(')', openP);
                    if (closeP != string::npos) {
                        op.keySlot = t.substr(openP + 5, closeP - openP - 5);
                        size_t vs = op.keySlot.find_first_not_of(" \t");
                        size_t ve = op.keySlot.find_last_not_of(" \t");
                        if (vs != string::npos)
                            op.keySlot = (ve == string::npos) ? op.keySlot.substr(vs) : op.keySlot.substr(vs, ve - vs + 1);
                    }
                    op.expr.clear();
                    tpl.operations.push_back(op);
                    continue;
                }

                // "WRITE(key2, value2)" -> WRITE, keySlot=key2, expr unused; varName can hold value source
                if (t.find("WRITE(") != string::npos) {
                    TxnOp op;
                    op.type = TxnOp::WRITE;
                    size_t openP = t.find("WRITE(");
                    size_t comma = t.find(',', openP);
                    size_t closeP = t.find(')', openP);
                    if (closeP != string::npos) {
                        if (comma != string::npos) {
                            op.keySlot = t.substr(openP + 6, comma - openP - 6);
                            op.varName = t.substr(comma + 1, closeP - comma - 1);
                        } else {
                            op.keySlot = t.substr(openP + 6, closeP - openP - 6);
                        }
                        size_t vStart = op.keySlot.find_first_not_of(" \t");
                        size_t vEnd = op.keySlot.find_last_not_of(" \t");
                        if (vStart != string::npos) op.keySlot = op.keySlot.substr(vStart, vEnd - vStart + 1);
                        vStart = op.varName.find_first_not_of(" \t");
                        vEnd = op.varName.find_last_not_of(" \t");
                        if (vStart != string::npos) op.varName = op.varName.substr(vStart, vEnd - vStart + 1);
                    }
                    op.expr.clear();
                    tpl.operations.push_back(op);
                    continue;
                }

                // "value2 = value1 + 1" or "from_acc["balance"] = from_acc["balance"] - 1" -> COMPUTE
                if (t.find('=') != string::npos && t.find("READ(") == string::npos && t.find("WRITE(") == string::npos) {
                    TxnOp op;
                    op.type = TxnOp::COMPUTE;
                    size_t eq = t.find('=');
                    op.varName = t.substr(0, eq);
                    op.varName.erase(0, op.varName.find_first_not_of(" \t"));
                    op.varName.erase(op.varName.find_last_not_of(" \t") + 1);
                    // If left side is var["balance"], use just var as varName.
                    size_t bracket = op.varName.find('[');
                    if (bracket != string::npos)
                        op.varName = op.varName.substr(0, bracket);
                    op.varName.erase(0, op.varName.find_first_not_of(" \t"));
                    op.varName.erase(op.varName.find_last_not_of(" \t") + 1);
                    op.expr = t.substr(eq + 1);
                    op.expr.erase(0, op.expr.find_first_not_of(" \t"));
                    op.expr.erase(op.expr.find_last_not_of(" \t") + 1);
                    op.keySlot.clear();
                    tpl.operations.push_back(op);
                }
            }
            out.templates.push_back(tpl);
        }
        if (trimmed == "END") break;
    }

    return out;
}

ParsedWorkload WorkloadParser::parseInsertOnly(const std::string& filepath) {
    ifstream f(filepath);
    if (!f.is_open()) throw runtime_error("WorkloadParser: cannot open " + filepath);
    ParsedWorkload out;
    string line;
    bool seenInsert = false;
    while (getline(f, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        string trimmed = (end == string::npos) ? line.substr(start) : line.substr(start, end - start + 1);
        if (trimmed == "INSERT") { seenInsert = true; continue; }
        if (!seenInsert) continue;
        if (trimmed == "END") break;
        if (trimmed.find("KEY:") != 0) continue;
        size_t keyStart = trimmed.find("KEY:") + 4;
        size_t keyEnd = trimmed.find(',', keyStart);
        if (keyEnd == string::npos) continue;
        string key = trimmed.substr(keyStart, keyEnd - keyStart);
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        size_t valStart = trimmed.find("VALUE:");
        if (valStart == string::npos) continue;
        string value = trimmed.substr(valStart + 6);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        out.initialData.push_back({key, value});
    }
    for (const auto& p : out.initialData) out.allKeys.push_back(p.first);
    return out;
}

ParsedWorkload WorkloadParser::parseWorkloadOnly(const std::string& filepath) {
    ifstream f(filepath);
    if (!f.is_open()) throw runtime_error("WorkloadParser: cannot open " + filepath);
    ParsedWorkload out;
    string line;
    bool seenWorkload = false;
    while (getline(f, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        string trimmed = (end == string::npos) ? line.substr(start) : line.substr(start, end - start + 1);
        if (trimmed == "WORKLOAD") { seenWorkload = true; continue; }
        if (!seenWorkload) continue;
        if (trimmed == "END") break;
        if (trimmed.find("TRANSACTION") == 0 && trimmed.find("INPUTS:") != string::npos) {
            TxnTemplate tpl;
            size_t inStart = trimmed.find("INPUTS:") + 7;
            size_t inEnd = trimmed.find(')', inStart);
            if (inEnd != string::npos) {
                string inputStr = trimmed.substr(inStart, inEnd - inStart);
                stringstream ss(inputStr);
                string part;
                while (getline(ss, part, ',')) {
                    size_t vStart = part.find_first_not_of(" \t");
                    if (vStart == string::npos) continue;
                    size_t vEnd = part.find_last_not_of(" \t");
                    string slot = (vEnd == string::npos) ? part.substr(vStart) : part.substr(vStart, vEnd - vStart + 1);
                    if (!slot.empty()) tpl.inputSlots.push_back(slot);
                }
            }
            while (getline(f, line)) {
                size_t s = line.find_first_not_of(" \t\r\n");
                if (s == string::npos) continue;
                size_t e = line.find_last_not_of(" \t\r\n");
                string t = (e == string::npos) ? line.substr(s) : line.substr(s, e - s + 1);
                if (t.empty()) continue;
                if (t == "BEGIN") continue;
                if (t == "COMMIT") break;
                if (t.find("READ(") != string::npos) {
                    TxnOp op;
                    op.type = TxnOp::READ;
                    size_t eq = t.find('=');
                    if (eq != string::npos) {
                        op.varName = t.substr(0, eq);
                        op.varName.erase(0, op.varName.find_first_not_of(" \t"));
                        op.varName.erase(op.varName.find_last_not_of(" \t") + 1);
                    }
                    size_t openP = t.find("READ(");
                    size_t closeP = t.find(')', openP);
                    if (closeP != string::npos) {
                        op.keySlot = t.substr(openP + 5, closeP - openP - 5);
                        size_t vs = op.keySlot.find_first_not_of(" \t");
                        size_t ve = op.keySlot.find_last_not_of(" \t");
                        if (vs != string::npos) op.keySlot = (ve == string::npos) ? op.keySlot.substr(vs) : op.keySlot.substr(vs, ve - vs + 1);
                    }
                    op.expr.clear();
                    tpl.operations.push_back(op);
                    continue;
                }
                if (t.find("WRITE(") != string::npos) {
                    TxnOp op;
                    op.type = TxnOp::WRITE;
                    size_t openP = t.find("WRITE(");
                    size_t comma = t.find(',', openP);
                    size_t closeP = t.find(')', openP);
                    if (closeP != string::npos) {
                        if (comma != string::npos) {
                            op.keySlot = t.substr(openP + 6, comma - openP - 6);
                            op.varName = t.substr(comma + 1, closeP - comma - 1);
                        } else {
                            op.keySlot = t.substr(openP + 6, closeP - openP - 6);
                        }
                        size_t vs = op.keySlot.find_first_not_of(" \t");
                        size_t ve = op.keySlot.find_last_not_of(" \t");
                        if (vs != string::npos) op.keySlot = op.keySlot.substr(vs, ve - vs + 1);
                        vs = op.varName.find_first_not_of(" \t");
                        ve = op.varName.find_last_not_of(" \t");
                        if (vs != string::npos) op.varName = op.varName.substr(vs, ve - vs + 1);
                    }
                    op.expr.clear();
                    tpl.operations.push_back(op);
                    continue;
                }
                if (t.find('=') != string::npos && t.find("READ(") == string::npos && t.find("WRITE(") == string::npos) {
                    TxnOp op;
                    op.type = TxnOp::COMPUTE;
                    size_t eq = t.find('=');
                    op.varName = t.substr(0, eq);
                    op.varName.erase(0, op.varName.find_first_not_of(" \t"));
                    op.varName.erase(op.varName.find_last_not_of(" \t") + 1);
                    size_t bracket = op.varName.find('[');
                    if (bracket != string::npos) op.varName = op.varName.substr(0, bracket);
                    op.varName.erase(0, op.varName.find_first_not_of(" \t"));
                    op.varName.erase(op.varName.find_last_not_of(" \t") + 1);
                    op.expr = t.substr(eq + 1);
                    op.expr.erase(0, op.expr.find_first_not_of(" \t"));
                    op.expr.erase(op.expr.find_last_not_of(" \t") + 1);
                    op.keySlot.clear();
                    tpl.operations.push_back(op);
                }
            }
            out.templates.push_back(tpl);
        }
    }
    return out;
}