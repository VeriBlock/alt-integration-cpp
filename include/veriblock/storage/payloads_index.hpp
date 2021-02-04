// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_

#include <veriblock/blockchain/command_group.hpp>
#include <veriblock/command_group_cache.hpp>

#include "payloads_provider.hpp"

namespace altintegration {

struct AltBlockTree;

//! @private
class PayloadsIndex {
  using id_t = typename CommandGroupCache::id_t;

 public:
  virtual ~PayloadsIndex() = default;

  /**
   * Payloads that cause the block validation to fail are not necessarily
   * invalid either globally or within the block. Changing the order of
   * payloads in a block is enough to change the payload validity.
   *
   * Recording the reason why block validation failed using
   * (containingBlock, payloadId)->validity map makes no sense.
   * Block validation aborts on the first payload that fails to validate.
   * Any change to the payloads(addition, removal, reordering) invalidates
   * the cached validity status for all payloads that follow.
   *
   * The validity map should only be used for payloads that will always fail
   * the validation and application of the block they are added to no matter
   * what context is provided. Such a map would be payloadId->validity and
   * not tied to the containing block.
   *
   * Example:
   *   * Block X at height N references block Y as its predecessor.
   *   * Block Y is known to us.
   *   * Block Y is at height N+10.
   * Thus, block X is always invalid and can be added to this map.
   *
   * Example:
   *   * Altchain block A contains VeriBlock block X as a payload.
   *   * Block X references block Y as its predecesor.
   *   * Block Y is unknown to us.
   * Thus, block A is invalid, the validity of block X is unknown and it should
   * NOT be added to this map as invalid despite block A validation failing at
   * this specific payload.
   *
   * Example:
   *   * Altchain block A contains VeriBlock block X as a payload.
   *   * Block X references block Y as its predecesor.
   *   * Block Y is known to us and is a valid predecessor of X.
   *   * Neither block A nor its predecessors contain VeriBlock block Y as a
   *     payload.
   * Thus, block A is invalid, block X is valid and should NOT be added
   * to this map as invalid despite block A validation failing at this specific
   * payload.
   *
   * FIXME: this functionality is not used
   */
  //! getter for cached payload validity
  bool getValidity(Slice<const uint8_t> containingBlockHash,
                   Slice<const uint8_t> payloadId) const;
  //! setter for payload validity
  void setValidity(Slice<const uint8_t> containingBlockHash,
                   Slice<const uint8_t> payloadId,
                   bool validity);

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
  const std::unordered_map<std::vector<uint8_t>, bool>& getValidity() const;
  const std::map<std::vector<uint8_t>, std::set<AltBlock::hash_t>>& getPayloadsInAlt() const;
  const std::map<std::vector<uint8_t>, std::set<VbkBlock::hash_t>>& getPayloadsInVbk() const;
  // clang-format on

 private:
  std::vector<uint8_t> makeGlobalPid(Slice<const uint8_t> a,
                                     Slice<const uint8_t> b) const;

  // reverse index. stores invalid payloads only.
  // key = <containing hash + payload id>
  // value =
  //  true  -> payload is valid in containing block
  //  false -> payload is invalid in containing block
  // if key is missing in this map, assume payload is valid
  std::unordered_map<std::vector<uint8_t>, bool> _cgValidity;

  // reverse index
  // key = id of payload
  // value = set of ALT/VBK blocks containing that payload
  std::map<std::vector<uint8_t>, std::set<AltBlock::hash_t>> payload_in_alt;
  std::map<std::vector<uint8_t>, std::set<VbkBlock::hash_t>> payload_in_vbk;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_
