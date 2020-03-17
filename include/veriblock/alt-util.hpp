#ifndef ALT_INTEGRATION_UTIL_HPP
#define ALT_INTEGRATION_UTIL_HPP

#include <algorithm>
#include <cstdint>
#include <vector>

namespace VeriBlock {

template <typename BlockTree>
std::vector<std::vector<uint8_t>> getLastKnownBlocks(const BlockTree& tree,
                                                     size_t size) {
  std::vector<std::vector<uint8_t>> ret;
  ret.reserve(size);

  auto* tip = tree.getBestChain().tip();
  while (tip != nullptr) {
    ret.push_back(tip->header.toRaw());
  }

  // reverse them since we add them in reverse order (tip -> genesis)
  std::reverse(ret.begin(), ret.end());
  return ret;
}

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_UTIL_HPP
