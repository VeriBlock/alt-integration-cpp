// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP
#define VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP

#include <veriblock/blockchain/pop/pop_state.hpp>
#include <veriblock/entities/endorsements.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/uint.hpp>

namespace altintegration {

struct VTB;

struct VbkBlockAddon : public PopState<VbkEndorsement> {
  using payloads_t = VTB;

  //! (memory only) total amount of work in the chain up to and including this
  //! block
  ArithUint256 chainWork = 0;

  uint32_t getRefCounter() const { return _refCounter; }

  void incRefCounter() {
    _refCounter++;
    setDirty();
  }

  void decRefCounter() {
    _refCounter--;
    setDirty();
  }

  bool payloadsIdsEmpty() const { return _vtbids.empty(); }

  template <typename pop_t>
  const std::vector<typename pop_t::id_t>& getPayloadIds() const;

  template <typename pop_t>
  void removePayloadId(const typename pop_t::id_t& pid) {
    auto it = std::find(_vtbids.begin(), _vtbids.end(), pid);
    VBK_ASSERT(it != _vtbids.end());
    _vtbids.erase(it);
    setDirty();
  }

  template <typename pop_t>
  void insertPayloadId(const typename pop_t::id_t& pid) {
    _vtbids.push_back(pid);
    setDirty();
  }

  template <typename pop_t>
  void insertPayloadIds(const std::vector<typename pop_t::id_t>& pids) {
    _vtbids.insert(_vtbids.end(), pids.begin(), pids.end());
    setDirty();
  }

  bool operator==(const VbkBlockAddon& o) const {
    bool a = _vtbids == o._vtbids;
    bool b = _refCounter == o._refCounter;
    bool c = chainWork == o.chainWork;
    bool d = PopState<VbkEndorsement>::operator==(o);
    return a && b && c && d;
  }

  std::string toPrettyString() const {
    return fmt::sprintf("VTB=%d", _vtbids.size());
  }

  void toRaw(WriteStream& w) const {
    w.writeBE<uint32_t>(_refCounter);
    PopState<VbkEndorsement>::toRaw(w);
    writeArrayOf<uint256>(w, _vtbids, writeSingleByteLenValue);
  }

 protected:
  //! reference counter for fork resolution
  uint32_t _refCounter = 0;
  // VTB::id_t
  std::vector<uint256> _vtbids;

  void setDirty();

  void setNull() {
    _refCounter = 0;
    chainWork = 0;
    PopState<VbkEndorsement>::setNull();
    _vtbids.clear();
  }

  void initAddonFromRaw(ReadStream& r) {
    _refCounter = r.readBE<uint32_t>();
    PopState<VbkEndorsement>::initAddonFromRaw(r);

    _vtbids = readArrayOf<uint256>(
        r, [](ReadStream& s) -> uint256 { return readSingleByteLenValue(s); });
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP
