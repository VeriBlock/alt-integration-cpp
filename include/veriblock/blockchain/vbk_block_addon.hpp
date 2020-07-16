// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP
#define VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP

#include <veriblock/blockchain/btc_block_addon.hpp>
#include <veriblock/blockchain/pop/pop_state.hpp>
#include <veriblock/entities/endorsements.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/uint.hpp>

namespace altintegration {

struct VTB;

struct VbkBlockAddon :
    // for endorsement map
    public PopState<VbkEndorsement>,
    // for chainwork + ref
    public BtcBlockAddon {
  using payloads_t = VTB;

  bool payloadsIdsEmpty() const { return _vtbids.empty(); }

  template <typename pop_t>
  const std::vector<typename pop_t::id_t>& getPayloadIds() const;

  template <typename pop_t>
  void removePayloadId(const typename pop_t::id_t& pid) {
    auto it = std::find(_vtbids.begin(), _vtbids.end(), pid);
    VBK_ASSERT(it != _vtbids.end());
    _vtbids.erase(it);
  }

  template <typename pop_t>
  void insertPayloadId(const typename pop_t::id_t& pid) {
    _vtbids.push_back(pid);
  }

  bool operator==(const VbkBlockAddon& o) const {
    bool a = _vtbids == o._vtbids;
    bool b = BtcBlockAddon::operator==(o);
    bool c = PopState<VbkEndorsement>::operator==(o);
    return a && b && c;
  }

  std::string toPrettyString() const {
    return fmt::sprintf("VTB=%d", _vtbids.size());
  }

  void toRaw(WriteStream& w) const {
    BtcBlockAddon::toRaw(w);
    PopState<VbkEndorsement>::toRaw(w);
    writeArrayOf<uint256>(w, _vtbids, writeSingleByteLenValue);
  }

 protected:
  // VTB::id_t
  std::vector<uint256> _vtbids;

  void setNull() {
    BtcBlockAddon::setNull();
    PopState<VbkEndorsement>::setNull();
    _vtbids.clear();
  }

  void initAddonFromRaw(ReadStream& r) {
    BtcBlockAddon::initAddonFromRaw(r);
    PopState<VbkEndorsement>::initAddonFromRaw(r);

    _vtbids = readArrayOf<uint256>(
        r, [](ReadStream& s) -> uint256 { return readSingleByteLenValue(s); });
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP
