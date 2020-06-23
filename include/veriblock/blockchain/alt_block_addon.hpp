// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ALT_BLOCK_ADDON_HPP
#define VERIBLOCK_POP_CPP_ALT_BLOCK_ADDON_HPP

#include <veriblock/arith_uint256.hpp>
#include <veriblock/blockchain/pop/pop_state.hpp>
#include <veriblock/entities/endorsements.hpp>
#include <veriblock/uint.hpp>

namespace altintegration {

struct PopData;
struct ATV;
struct VTB;
struct VbkBlock;

struct AltBlockAddon : public PopState<AltEndorsement> {
  using payloads_t = PopData;

  // TODO: refactor base block tree, and move chainwork to blocktree.hpp, then
  // remove this
  uint8_t chainWork;

  //! list of changes introduced in this block
  // ATV::id_t
  std::vector<uint256> alt_payloadIds;
  // VTB::id_t
  std::vector<uint256> vbk_payloadIds;
  // VbkBlock::id_t
  std::vector<uint96> vbk_blockIds;

  bool payloadsIdsEmpty() const {
    return alt_payloadIds.empty() && vbk_payloadIds.empty() &&
           vbk_blockIds.empty();
  }

  template <typename pop_t, typename pop_id_t>
  std::vector<pop_id_t>& getPayloadIds();

  template <>
  std::vector<uint256>& getPayloadIds<ATV, uint256>() {
    return alt_payloadIds;
  }

  template <>
  std::vector<uint256>& getPayloadIds<VTB, uint256>() {
    return vbk_payloadIds;
  }

  template <>
  std::vector<uint96>& getPayloadIds<VbkBlock, uint96>() {
    return vbk_blockIds;
  }

  template <typename pop_t, typename pop_id_t>
  const std::vector<pop_id_t>& getPayloadIds() const;

  template <>
  const std::vector<uint256>& getPayloadIds<ATV, uint256>() const {
    return alt_payloadIds;
  }

  template <>
  const std::vector<uint256>& getPayloadIds<VTB, uint256>() const {
    return vbk_payloadIds;
  }

  template <>
  const std::vector<uint96>& getPayloadIds<VbkBlock, uint96>() const {
    return vbk_blockIds;
  }

  void setNull() {
    PopState<AltEndorsement>::setNull();
    chainWork = 0;
    alt_payloadIds.clear();
    vbk_payloadIds.clear();
    vbk_blockIds.clear();
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_ALT_BLOCK_ADDON_HPP
