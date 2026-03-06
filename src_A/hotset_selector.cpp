#include "hotset_selector.h"
#include <algorithm>
#include <random>

HotsetSelector::HotsetSelector(const std::vector<std::string>& allKeys,
                               int hotsetSize,
                               double contentionProb)
    : allKeys_(allKeys), contentionProb_(contentionProb) {
    if (allKeys_.empty()) return;

    // Fixed seed for reproducible hotset across runs.
    std::mt19937 rng(42);
    std::vector<size_t> indices(allKeys_.size());
    for (size_t i = 0; i < indices.size(); ++i) indices[i] = i;
    std::shuffle(indices.begin(), indices.end(), rng);

    size_t n = static_cast<size_t>(std::min(hotsetSize, static_cast<int>(allKeys_.size())));
    hotset_.reserve(n);
    for (size_t i = 0; i < n; ++i)
        hotset_.push_back(allKeys_[indices[i]]);
}

std::string HotsetSelector::selectKey(std::mt19937& rng) const {
    if (allKeys_.empty()) return "";

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    if (dist(rng) < contentionProb_ && !hotset_.empty()) {
        std::uniform_int_distribution<size_t> hotDist(0, hotset_.size() - 1);
        return hotset_[hotDist(rng)];
    }
    std::uniform_int_distribution<size_t> fullDist(0, allKeys_.size() - 1);
    return allKeys_[fullDist(rng)];
}

std::vector<std::string> HotsetSelector::selectKeys(int n, std::mt19937& rng) const {
    std::vector<std::string> result;
    result.reserve(n);
    for (int i = 0; i < n; ++i)
        result.push_back(selectKey(rng));
    return result;
}