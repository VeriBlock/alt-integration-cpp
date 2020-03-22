#ifndef ALTINTEGRATION_ALT_BLOCK_INDEX_HPP
#define ALTINTEGRATION_ALT_BLOCK_INDEX_HPP

#include <vector>
#include <veriblock/entities/altblock.hpp>

namespace altintegration {

struct AltBlockIndex {
  using block_t = AltBlock;
  using hash_t = AltBlock::hash_t;

  AltBlock header{};
  AltBlockIndex* pprev{};
};

}

#endif  // ALTINTEGRATION_ALT_BLOCK_INDEX_HPP
