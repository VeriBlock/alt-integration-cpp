// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP
#define VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP

#include <veriblock/pop/arith_uint256.hpp>
#include <veriblock/pop/entities/endorsements.hpp>
#include <veriblock/pop/logger.hpp>
#include <veriblock/pop/uint.hpp>

#include "block_status.hpp"
#include "pop/pop_state.hpp"

namespace altintegration {

struct VTB;

//! @private
struct VbkBlockAddon : public PopState<VbkEndorsement> {
  using payloads_t = VTB;
  using ref_height_t = int32_t;

  //! (memory only) total amount of work in the chain up to and including this
  //! block
  ArithUint256 chainWork = 0;

  void setNullInmemFields();

  static constexpr auto validTipLevel = BLOCK_VALID_TREE;

  uint32_t refCount() const { return _refCount; }
  void setRef(uint32_t count);

  void addRef(ref_height_t);

  void removeRef(ref_height_t);

  void setIsBootstrap(bool isBootstrap);

  void insertBlockOfProofEndorsement(const AltEndorsement* e);

  bool eraseLastFromBlockOfProofEndorsement(const AltEndorsement* endorsement);

  void clearBlockOfProofEndorsement();

  const std::vector<const AltEndorsement*>& getBlockOfProofEndorsement() const;

  bool hasPayloads() const { return !_vtbids.empty(); }

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
  void insertPayload(const pop_t&) {}

  template <typename pop_t>
  void insertPayloadIds(const std::vector<typename pop_t::id_t>& pids) {
    _vtbids.insert(_vtbids.end(), pids.begin(), pids.end());
    setDirty();
  }

  std::string toPrettyString() const;

  void toVbkEncoding(WriteStream& w) const;

 protected:
  //! reference counter for fork resolution
  uint32_t _refCount = 0;
  // VTB::id_t
  std::vector<uint256> _vtbids;

  //! (memory-only) a list of endorsements of ALT blocks, whose BlockOfProof is
  //! this block. must be a vector, because we can have duplicates here
  std::vector<const AltEndorsement*> _blockOfProofEndorsements;

  void setDirty();

  void setNull();

  friend bool DeserializeFromVbkEncoding(ReadStream& stream,
                                         VbkBlockAddon& out,
                                         ValidationState& state);
};

template <>
void VbkBlockAddon::insertPayload(const VTB&);

//! @overload
bool DeserializeFromVbkEncoding(ReadStream& stream,
                                VbkBlockAddon& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_VBK_BLOCK_INDEX_HPP
