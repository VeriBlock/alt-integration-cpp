// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP
#define VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP

#include <veriblock/arith_uint256.hpp>
#include <veriblock/blockchain/pop/pop_state.hpp>
#include <veriblock/entities/endorsements.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/uint.hpp>

namespace altintegration {

struct VTB;

struct VbkBlockAddon : public PopState<VbkEndorsement> {
  using payloads_t = VTB;
  using ref_height_t = int32_t;

  //! (memory only) total amount of work in the chain up to and including this
  //! block
  ArithUint256 chainWork = 0;

  uint32_t refCount() const { return _refCount; }

  void addRef(ref_height_t) { ++_refCount; setDirty(); }

  void removeRef(ref_height_t) {
    VBK_ASSERT(refCount_ > 0 &&
               "state corruption: attempted to remove a nonexitent reference "
               "to a VBK block");
    --_refCount;
    setDirty();
  }

  void setIsBootstrap(bool isBootstrap) {
    if (isBootstrap) {
      // pretend this block is referenced by the genesis block of the SI chain
      addRef(0);
    } else {
      VBK_ASSERT(false && "not supported");
    }
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
    // comparing reference counts does not seem like a good idea
    // as the only situation where they would be different is
    // comparing blocks across different trees eg mock miner vs
    // the test tree and in this situation the references and counts
    // are likely to differ
    bool b = true;  // _refCount == o._refCount;
    bool c = chainWork == o.chainWork;
    bool d = PopState<VbkEndorsement>::operator==(o);
    return a && b && c && d;


  std::string toPrettyString() const {
    return fmt::sprintf("VTB=%d", _vtbids.size());
  }

  void toRaw(WriteStream& w) const {
    w.writeBE<uint32_t>(_refCount);
    PopState<VbkEndorsement>::toRaw(w);
    writeArrayOf<uint256>(w, _vtbids, writeSingleByteLenValue);
  }

 protected:
  //! reference counter for fork resolution
  uint32_t _refCount = 0;
  // VTB::id_t
  std::vector<uint256> _vtbids;

  void setDirty();

  void setNull() {
    _refCount = 0;
    chainWork = 0;
    PopState<VbkEndorsement>::setNull();
    _vtbids.clear();
  }

  void initAddonFromRaw(ReadStream& r) {
    _refCount = r.readBE<uint32_t>();
    PopState<VbkEndorsement>::initAddonFromRaw(r);

    _vtbids = readArrayOf<uint256>(
        r, [](ReadStream& s) -> uint256 { return readSingleByteLenValue(s); });
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP
