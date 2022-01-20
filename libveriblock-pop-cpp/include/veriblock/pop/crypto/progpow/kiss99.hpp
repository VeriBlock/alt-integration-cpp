// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_KISS99_HPP
#define VERIBLOCK_POP_CPP_KISS99_HPP

#include <cstdint>

namespace altintegration {

struct kiss99_t;

/**
 * Calculate KISS99.
 *
 * https://en.wikipedia.org/wiki/KISS_(algorithm)
 * http://www.cse.yorku.ca/~oz/marsaglia-rng.html
 * @param st
 * @return
 *
 */
uint32_t kiss99(kiss99_t& st);

//! @private
struct kiss99_t {
  uint32_t z, w, jsr, jcong;

  inline uint32_t operator()() { return kiss99(*this); }

  inline bool operator==(const kiss99_t& o) const {
    return z == o.z && w == o.w && jsr == o.jsr && jcong == o.jcong;
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_KISS99_HPP
