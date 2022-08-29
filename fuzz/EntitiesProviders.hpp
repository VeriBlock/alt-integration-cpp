// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ENTITIESPROVIDERS_HPP
#define VERIBLOCK_POP_CPP_ENTITIESPROVIDERS_HPP

#include <veriblock/pop/blockchain/block_index.hpp>

#include "FuzzedDataProvider.hpp"

namespace fuzz {

using namespace altintegration;

template <size_t N>
bool ConsumeArray(FuzzedDataProvider& p, Blob<N>& b) {
  if (p.remaining_bytes() < N) {
    return false;
  }

  auto bytes = p.ConsumeBytes<uint8_t>(N);
  b = bytes;
  return true;
}

template <size_t N = 32>
bool ConsumeArray(FuzzedDataProvider& p, std::vector<uint8_t>& out) {
  if (p.remaining_bytes() < N) {
    return false;
  }

  out = p.ConsumeBytes<uint8_t>(N);
  return true;
}

template <typename E>
bool ConsumeEndorsement(FuzzedDataProvider& p, E& e) {
  // clang-format off
  return ConsumeArray(p, e.id) &&
         ConsumeArray(p, e.blockOfProof) &&
         ConsumeArray(p, e.containingHash) &&
         ConsumeArray(p, e.endorsedHash);
  // clang-format on
}

inline bool ConsumeAddon(FuzzedDataProvider& p, BtcBlockAddon& addon) {
  if (!ConsumeArray(p, addon.chainWork)) {
    return false;
  }

  return true;
}

inline bool ConsumeAddon(FuzzedDataProvider& p, VbkBlockAddon& addon) {
  if (!ConsumeArray(p, addon.chainWork)) {
    return false;
  }

  const auto size = p.ConsumeIntegral<uint32_t>();
  for (auto i = 0u; i < size; i++) {
    VbkEndorsement e;
    if (!ConsumeEndorsement(p, e)) {
      return false;
    }

    addon.insertContainingEndorsement(std::make_shared<VbkEndorsement>(e));
  }

  return true;
}

template <typename Block>
bool ConsumeBlockIndex(FuzzedDataProvider& p, BlockIndex<Block>& index) {
  index.setHeight(p.template ConsumeIntegral<int32_t>());
  index.setHeader(Block{});
  index.setStatus(p.template ConsumeIntegral<uint32_t>());

  using addon_t = typename BlockIndex<Block>::addon_t;
  addon_t& addon = index;

  return ConsumeAddon(p, addon);
}

}  // namespace fuzz

#endif  // VERIBLOCK_POP_CPP_ENTITIESPROVIDERS_HPP
