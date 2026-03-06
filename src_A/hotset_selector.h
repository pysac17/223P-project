#ifndef HOTSET_SELECTOR_H
#define HOTSET_SELECTOR_H

#include <string>
#include <vector>
#include <random>

class HotsetSelector {
public:
    // allKeys: full keyspace from workload parser
    // hotsetSize: how many keys are in the "hot" set
    // contentionProb: probability (0.0 to 1.0) of picking from the hotset
    HotsetSelector(const std::vector<std::string>& allKeys,
                   int hotsetSize,
                   double contentionProb);

    // With probability contentionProb return a key from hotset; else from full keyspace.
    // rng: per-thread generator (caller owns it).
    std::string selectKey(std::mt19937& rng) const;

    // Call selectKey n times; duplicates allowed.
    std::vector<std::string> selectKeys(int n, std::mt19937& rng) const;

private:
    std::vector<std::string> allKeys_;
    std::vector<std::string> hotset_;
    double contentionProb_;
};

#endif