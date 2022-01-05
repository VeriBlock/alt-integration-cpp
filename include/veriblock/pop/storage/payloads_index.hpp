// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_

#include <veriblock/pop/blockchain/command_group.hpp>
#include <veriblock/pop/command_group_cache.hpp>

#include "payloads_provider.hpp"

namespace altintegration {

struct AltBlockTree;

//! @private
class PayloadsIndex {
  using id_t = typename CommandGroupCache::id_t;

 public:
  virtual ~PayloadsIndex() = default;

  void reindex(const AltBlockTree& tree);

  // get a list of ALT containing blocks for given payload
  const std::set<AltBlock::hash_t>& getContainingAltBlocks(
      const std::vector<uint8_t>& payloadId) const;
  // get a list of VBK containing blocks for given payload
  const std::set<VbkBlock::hash_t>& getContainingVbkBlocks(
      const std::vector<uint8_t>& payloadId) const;
  void addBlockToIndex(const BlockIndex<AltBlock>& block);
  void addBlockToIndex(const BlockIndex<VbkBlock>& block);
  // add ALT payload to index
  void addAltPayloadIndex(const AltBlock::hash_t& containing,
                          const std::vector<uint8_t>& payloadId);
  // add VBK payload to index
  void addVbkPayloadIndex(const VbkBlock::hash_t& containing,
                          const std::vector<uint8_t>& payloadId);
  // remove ALT payload from index
  void removeAltPayloadIndex(const AltBlock::hash_t& containing,
                             const std::vector<uint8_t>& payloadId);
  // remove VBK payload from index
  void removeVbkPayloadIndex(const VbkBlock::hash_t& containing,
                             const std::vector<uint8_t>& payloadId);

  void removePayloadsIndex(const BlockIndex<AltBlock>& block);
  void removePayloadsIndex(const BlockIndex<VbkBlock>& block);

  // clang-format off
  // const std::unordered_map<std::vector<uint8_t>, bool>& getValidity() const;
  const std::unordered_map<std::vector<uint8_t>, std::set<AltBlock::hash_t>>& getPayloadsInAlt() const;
  const std::unordered_map<std::vector<uint8_t>, std::set<VbkBlock::hash_t>>& getPayloadsInVbk() const;
  // clang-format on

 private:

  // reverse index
  // key = id of payload
  // value = set of ALT/VBK blocks containing that payload
  std::unordered_map<std::vector<uint8_t>, std::set<AltBlock::hash_t>>
      payload_in_alt;
  std::unordered_map<std::vector<uint8_t>, std::set<VbkBlock::hash_t>>
      payload_in_vbk;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_
