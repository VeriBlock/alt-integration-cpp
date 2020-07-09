// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP
#define VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP

#include <veriblock/blockchain/btc_block_addon.hpp>
#include <veriblock/blockchain/pop/pop_state.hpp>
#include <veriblock/entities/endorsements.hpp>
#include <veriblock/uint.hpp>

namespace altintegration {

struct VTB;

struct VbkBlockAddon :
    // for endorsement map
    public PopState<VbkEndorsement>,
    // for chainwork + ref
    public BtcBlockAddon {
  using payloads_t = VTB;

  // VTB::id_t
  std::vector<uint256> vtbids;

  bool payloadsIdsEmpty() const { return vtbids.empty(); }

  template <typename pop_t, typename pop_id_t>
  std::vector<pop_id_t>& getPayloadIds();

  template <typename pop_t, typename pop_id_t>
  const std::vector<pop_id_t>& getPayloadIds() const;

  void setNull() {
    BtcBlockAddon::setNull();
    PopState<VbkEndorsement>::setNull();
    vtbids.clear();
  }

  std::string toPrettyString() const {
    return fmt::sprintf("VTB=%d", vtbids.size());
  }

  void toRaw(WriteStream& w) const {
    BtcBlockAddon::toRaw(w);
    PopState<VbkEndorsement>::toRaw(w);
    writeArrayOf<uint256>(w, vtbids, writeSingleByteLenValue);
  }

  void initFromRaw(ReadStream& r) {
    BtcBlockAddon::initFromRaw(r);
    PopState<VbkEndorsement>::initFromRaw(r);

    vtbids = readArrayOf<uint256>(
        r, [](ReadStream& s) -> uint256 { return readSingleByteLenValue(s); });
  }
};

template <>
inline std::vector<uint256>& VbkBlockAddon::getPayloadIds<VTB, uint256>() {
  return vtbids;
}

template <>
inline const std::vector<uint256>& VbkBlockAddon::getPayloadIds<VTB, uint256>()
    const {
  return vtbids;
}

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP
