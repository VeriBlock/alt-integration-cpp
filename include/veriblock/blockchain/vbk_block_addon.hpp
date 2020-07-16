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

template <typename BlockIndexT>
struct VbkBlockAddon :
    // for endorsement map
    public PopState<BlockIndexT, VbkEndorsement>,
    // for chainwork + ref
    public BtcBlockAddon<BlockIndexT> {
  using payloads_t = VTB;

  bool payloadsIdsEmpty() const { return _vtbids.empty(); }

  template <typename pop_t>
  const std::vector<typename pop_t::id_t>& getPayloadIds() const;

  template <typename pop_t>
  void removePayloadId(const typename pop_t::id_t& pid) {
    auto it = std::find(_vtbids.begin(), _vtbids.end(), pid);
    VBK_ASSERT(it != _vtbids.end());
    _vtbids.erase(it);
    static_cast<BlockIndexT*>(this)->setDirty();
  }

  template <typename pop_t>
  void insertPayloadId(const typename pop_t::id_t& pid) {
    _vtbids.push_back(pid);
    static_cast<BlockIndexT*>(this)->setDirty();
  }

  template <typename pop_t>
  void insertPayloadIds(const std::vector<typename pop_t::id_t>& pids) {
    _vtbids.insert(_vtbids.end(), pids.begin(), pids.end());
    static_cast<BlockIndexT*>(this)->setDirty();
  }

  bool operator==(const VbkBlockAddon& o) const {
    bool a = _vtbids == o._vtbids;
    bool b = BtcBlockAddon<BlockIndexT>::operator==(o);
    bool c = PopState<BlockIndexT, VbkEndorsement>::operator==(o);
    return a && b && c;
  }

  std::string toPrettyString() const {
    return fmt::sprintf("VTB=%d", _vtbids.size());
  }

  void toRaw(WriteStream& w) const {
    BtcBlockAddon<BlockIndexT>::toRaw(w);
    PopState<BlockIndexT, VbkEndorsement>::toRaw(w);
    writeArrayOf<uint256>(w, _vtbids, writeSingleByteLenValue);
  }

 protected:
  // VTB::id_t
  std::vector<uint256> _vtbids;

  void setNull() {
    BtcBlockAddon<BlockIndexT>::setNull();
    PopState<BlockIndexT, VbkEndorsement>::setNull();
    _vtbids.clear();
  }

  void initAddonFromRaw(ReadStream& r) {
    BtcBlockAddon<BlockIndexT>::initAddonFromRaw(r);
    PopState<BlockIndexT, VbkEndorsement>::initAddonFromRaw(r);

    _vtbids = readArrayOf<uint256>(
        r, [](ReadStream& s) -> uint256 { return readSingleByteLenValue(s); });
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP
