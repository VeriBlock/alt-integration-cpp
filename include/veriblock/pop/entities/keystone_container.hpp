// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_KEYSTONE_CONTAINER_HPP
#define VERIBLOCK_POP_CPP_KEYSTONE_CONTAINER_HPP

#include <cstddef>
#include <cstdint>
#include <vector>
#include <veriblock/pop/blockchain/block_index.hpp>
#include <veriblock/pop/entities/altblock.hpp>

#include "veriblock/pop/json.hpp"
#include "veriblock/pop/strutil.hpp"

namespace altintegration {
class ValidationState;
class WriteStream;
struct AltBlock;
struct ReadStream;
template <typename Block>
struct BlockIndex;

/**
 * A container for two previous keystones of endorsed block.
 *
 * A keystone is a block which satisfies a condition `isKeystone(block.height)
 * == true`.
 */
struct KeystoneContainer {
  std::vector<uint8_t> firstPreviousKeystone;
  std::vector<uint8_t> secondPreviousKeystone;

  static KeystoneContainer createFromPrevious(const BlockIndex<AltBlock>* prev,
                                              const uint32_t keystoneInterval);

  void toVbkEncoding(WriteStream& stream) const;

  size_t estimateSize() const;

  bool operator==(const KeystoneContainer& o) const {
    return firstPreviousKeystone == o.firstPreviousKeystone &&
           secondPreviousKeystone == o.secondPreviousKeystone;
  }
};

//! @overload
bool DeserializeFromVbkEncoding(ReadStream& stream,
                                KeystoneContainer& container,
                                ValidationState& state);

//! @overload
template <typename JsonValue>
JsonValue ToJSON(const KeystoneContainer& c) {
  auto obj = json::makeEmptyObject<JsonValue>();
  json::putStringKV(
      obj, "firstPreviousKeystone", HexStr(c.firstPreviousKeystone));
  json::putStringKV(
      obj, "secondPreviousKeystone", HexStr(c.secondPreviousKeystone));
  return obj;
}

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_KEYSTONE_CONTAINER_HPP
