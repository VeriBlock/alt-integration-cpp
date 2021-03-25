// Copyright (c) 2019-2020 Xenios SEZC
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

  StoredVbkBlockAddon() = default;
  StoredVbkBlockAddon(const addon_t& other);

  void toVbkEncoding(WriteStream& w) const;

  addon_t toInmem() const;
  void toInmem(addon_t& to) const;

  std::string toPrettyString() const;

  //! reference counter for fork resolution
  uint32_t _refCount = 0;
  // VTB::id_t
  std::vector<uint256> _vtbids;
  // list of endorsements of ALT blocks, whose BlockOfProof is
  // this block. must be a vector, because we can have duplicates here
  std::vector<uint256> blockOfProofEndorsementHashes;
  // list of endorsements pointing to this block.
  // must be a vector, because we can have duplicates here
  std::vector<uint256> endorsedByHashes;
};

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                StoredVbkBlockAddon& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_VBK_STORAGE_STORED_VBK_BLOCK_ADDON_HPP
