// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VBK_STORAGE_STORED_VBK_BLOCK_ADDON_HPP
#define VERIBLOCK_POP_CPP_VBK_STORAGE_STORED_VBK_BLOCK_ADDON_HPP

#include <veriblock/pop/blockchain/vbk_block_addon.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/uint.hpp>

namespace altintegration {

//! @private
struct StoredVbkBlockAddon {
  using addon_t = VbkBlockAddon;
  using pop_state_t = PopState<VbkEndorsement>;

  StoredVbkBlockAddon() = default;
  StoredVbkBlockAddon(const addon_t& other);

  void toVbkEncoding(WriteStream& w) const;

  void toInmem(addon_t& to) const;

  std::string toPrettyString() const;

  //! reference counter for fork resolution
  uint32_t _refCount = 0;
  // VTB::id_t
  std::vector<uint256> _vtbids;
  // list of endorsement of ALT blocks, whose BlockOfProof is
  // this block ids. Must be a vector, because we can have duplicates here
  std::vector<uint256> blockOfProofEndorsementIds;
  // list of endorsement pointing to this block ids.
  // Must be a vector, because we can have duplicates here
  std::vector<uint256> endorsedByIds;
  // POP endorsements state
  pop_state_t popState;
};

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                StoredVbkBlockAddon& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_VBK_STORAGE_STORED_VBK_BLOCK_ADDON_HPP
