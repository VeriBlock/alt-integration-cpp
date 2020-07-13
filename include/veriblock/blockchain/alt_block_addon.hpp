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

  bool payloadsIdsEmpty() const {
    return _atvids.empty() && _vtbids.empty() && _vbkblockids.empty();
  }

  bool operator==(const AltBlockAddon& o) const {
    return _atvids == o._atvids && _vtbids == o._vtbids &&
           _vbkblockids == o._vbkblockids &&
           PopState<AltEndorsement>::operator==(o);
  }

  /*template <typename pop_t, typename pop_id_t>
  std::vector<pop_id_t>& getPayloadIds();*/

  template <typename pop_t, typename pop_id_t>
  const std::vector<pop_id_t>& getPayloadIds() const;

  std::string toPrettyString() const {
    return fmt::sprintf("ATV=%d, VTB=%d, VBK=%d",
                        _atvids.size(),
                        _vtbids.size(),
                        _vbkblockids.size());
  }

  void toRaw(WriteStream& w) const {
    PopState<AltEndorsement>::toRaw(w);
    writeArrayOf<uint256>(w, _atvids, writeSingleByteLenValue);
    writeArrayOf<uint256>(w, _vtbids, writeSingleByteLenValue);
    writeArrayOf<uint96>(w, _vbkblockids, writeSingleByteLenValue);
  }

 protected:
  //! list of changes introduced in this block
  // ATV::id_t
  std::vector<uint256> _atvids;
  // VTB::id_t
  std::vector<uint256> _vtbids;
  // VbkBlock::id_t
  std::vector<uint96> _vbkblockids;

  void setNull() {
    PopState<AltEndorsement>::setNull();
    chainWork = 0;
    _atvids.clear();
    _vtbids.clear();
    _vbkblockids.clear();
  }

  void initAddonFromRaw(ReadStream& r) {
    PopState<AltEndorsement>::initAddonFromRaw(r);
    _atvids = readArrayOf<uint256>(
        r, [](ReadStream& s) -> uint256 { return readSingleByteLenValue(s); });
    _vtbids = readArrayOf<uint256>(
        r, [](ReadStream& s) -> uint256 { return readSingleByteLenValue(s); });
    _vbkblockids = readArrayOf<uint96>(
        r, [](ReadStream& s) -> uint96 { return readSingleByteLenValue(s); });
  }
};

/*template <>
inline std::vector<uint256>& AltBlockAddon::getPayloadIds<ATV, uint256>() {
  return _atvids;
}

template <>
inline std::vector<uint256>& AltBlockAddon::getPayloadIds<VTB, uint256>() {
  return _vtbids;
}

template <>
inline std::vector<uint96>& AltBlockAddon::getPayloadIds<VbkBlock, uint96>() {
  return _vbkblockids;
}*/

template <>
inline const std::vector<uint256>& AltBlockAddon::getPayloadIds<ATV, uint256>()
    const {
  return _atvids;
}

template <>
inline const std::vector<uint256>& AltBlockAddon::getPayloadIds<VTB, uint256>()
    const {
  return _vtbids;
}

template <>
inline const std::vector<uint96>&
AltBlockAddon::getPayloadIds<VbkBlock, uint96>() const {
  return _vbkblockids;
}

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_ALT_BLOCK_ADDON_HPP
