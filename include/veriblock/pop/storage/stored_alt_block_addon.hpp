// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VBK_STORAGE_STORED_ALT_BLOCK_ADDON_HPP
#define VERIBLOCK_POP_CPP_VBK_STORAGE_STORED_ALT_BLOCK_ADDON_HPP

#include <veriblock/pop/blockchain/alt_block_addon.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/uint.hpp>

namespace altintegration {

//! @private
struct StoredAltBlockAddon {
  using addon_t = AltBlockAddon;
  using pop_state_t = PopState<AltEndorsement>;

  StoredAltBlockAddon() = default;
  StoredAltBlockAddon(const addon_t& other);

  void toVbkEncoding(WriteStream& w) const;

  void toInmem(addon_t& to) const;

  std::string toPrettyString() const;

  //! list of changes introduced in this block
  // ATV::id_t
  std::vector<uint256> _atvids;
  // VTB::id_t
  std::vector<uint256> _vtbids;
  // VbkBlock::id_t
  std::vector<uint96> _vbkblockids;
  // list of endorsement pointing to this block ids.
  // Must be a vector, because we can have duplicates here
  std::vector<uint256> endorsedByIds;
  // POP endorsements state
  pop_state_t popState;
};

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                StoredAltBlockAddon& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_VBK_STORAGE_STORED_ALT_BLOCK_ADDON_HPP
