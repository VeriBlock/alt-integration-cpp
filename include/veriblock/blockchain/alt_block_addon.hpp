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
  std::vector<uint256> atvids;
  // VTB::id_t
  std::vector<uint256> vtbids;
  // VbkBlock::id_t
  std::vector<uint96> vbkblockids;

  bool payloadsIdsEmpty() const {
    return atvids.empty() && vtbids.empty() && vbkblockids.empty();
  }

  template <typename pop_t, typename pop_id_t>
  std::vector<pop_id_t>& getPayloadIds();

  template <typename pop_t, typename pop_id_t>
  const std::vector<pop_id_t>& getPayloadIds() const;

  void setNull() {
    PopState<AltEndorsement>::setNull();
    chainWork = 0;
    atvids.clear();
    vtbids.clear();
    vbkblockids.clear();
  }

  std::string toPrettyString() const {
    return fmt::sprintf("ATV=%d, VTB=%d, VBK=%d",
                        atvids.size(),
                        vtbids.size(),
                        vbkblockids.size());
  }

  void toRaw(WriteStream& w) const {
    PopState<AltEndorsement>::toRaw(w);
    writeArrayOf<uint256>(w, atvids, writeSingleByteLenValue);
    writeArrayOf<uint256>(w, vtbids, writeSingleByteLenValue);
    writeArrayOf<uint96>(w, vbkblockids, writeSingleByteLenValue);
  }

  void initFromRaw(ReadStream& r) {
    PopState<AltEndorsement>::initFromRaw(r);
    atvids = readArrayOf<uint256>(
        r, [](ReadStream& s) -> uint256 { return readSingleByteLenValue(s); });
    vtbids = readArrayOf<uint256>(
        r, [](ReadStream& s) -> uint256 { return readSingleByteLenValue(s); });
    vbkblockids = readArrayOf<uint96>(
        r, [](ReadStream& s) -> uint96 { return readSingleByteLenValue(s); });
  }
};

template <>
inline std::vector<uint256>& AltBlockAddon::getPayloadIds<ATV, uint256>() {
  return atvids;
}

template <>
inline std::vector<uint256>& AltBlockAddon::getPayloadIds<VTB, uint256>() {
  return vtbids;
}

template <>
inline std::vector<uint96>& AltBlockAddon::getPayloadIds<VbkBlock, uint96>() {
  return vbkblockids;
}

template <>
inline const std::vector<uint256>& AltBlockAddon::getPayloadIds<ATV, uint256>()
    const {
  return atvids;
}

template <>
inline const std::vector<uint256>& AltBlockAddon::getPayloadIds<VTB, uint256>()
    const {
  return vtbids;
}

template <>
inline const std::vector<uint96>&
AltBlockAddon::getPayloadIds<VbkBlock, uint96>() const {
  return vbkblockids;
}

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_ALT_BLOCK_ADDON_HPP
