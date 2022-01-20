// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ALT_BLOCK_ADDON_HPP
#define VERIBLOCK_POP_CPP_ALT_BLOCK_ADDON_HPP

#include <veriblock/pop/blockchain/block_status.hpp>
#include <veriblock/pop/blockchain/pop/pop_state.hpp>
#include <veriblock/pop/entities/endorsements.hpp>
#include <veriblock/pop/uint.hpp>

namespace altintegration {

struct PopData;

//! @private
struct AltBlockAddon : public PopState<AltEndorsement> {
  using payloads_t = PopData;

  void setNullInmemFields();

  static constexpr auto validTipLevel = BLOCK_CONNECTED;

  bool hasPayloads() const;

  void clearPayloads();

  template <typename pop_t>
  const std::vector<typename pop_t::id_t>& getPayloadIds() const;

  // used in `continueOnInvalid` mode
  template <typename pop_t>
  void removePayloadId(const typename pop_t::id_t& pid) {
    auto& payloads = getPayloadIdsInner<pop_t>();
    auto it = std::find(payloads.begin(), payloads.end(), pid);
    VBK_ASSERT(it != payloads.end());
    payloads.erase(it);
    setDirty();
  }

  template <typename pop_t>
  void setPayloads(const std::vector<typename pop_t::id_t>& pids) {
    auto& payloads = getPayloadIdsInner<pop_t>();
    payloads = pids;
    setDirty();
  }

  // used in tests
  template <typename pop_t>
  void insertPayloadIds(const std::vector<typename pop_t::id_t>& pids) {
    auto& payloads = getPayloadIdsInner<pop_t>();
    payloads.insert(payloads.end(), pids.begin(), pids.end());
    setDirty();
  }

  std::string toPrettyString() const;

  void toVbkEncoding(WriteStream& w) const;

 protected:
  //! list of changes introduced in this block
  // ATV::id_t
  std::vector<uint256> _atvids;
  // VTB::id_t
  std::vector<uint256> _vtbids;
  // VbkBlock::id_t
  std::vector<uint96> _vbkblockids;

  void setDirty();

  void setNull();

  template <typename pop_t>
  std::vector<typename pop_t::id_t>& getPayloadIdsInner();

  friend bool DeserializeFromVbkEncoding(ReadStream& stream,
                                         AltBlockAddon& out,
                                         ValidationState& state);
};

//! @overload
bool DeserializeFromVbkEncoding(ReadStream& stream,
                                AltBlockAddon& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_ALT_BLOCK_ADDON_HPP
