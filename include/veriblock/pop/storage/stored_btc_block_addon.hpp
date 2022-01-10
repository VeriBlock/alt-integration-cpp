// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VBK_STORAGE_STORED_BTC_BLOCK_ADDON_HPP
#define VERIBLOCK_POP_CPP_VBK_STORAGE_STORED_BTC_BLOCK_ADDON_HPP

#include <veriblock/pop/blockchain/btc_block_addon.hpp>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/uint.hpp>

namespace altintegration {

//! @private
struct StoredBtcBlockAddon {
  using addon_t = BtcBlockAddon;
  using ref_height_t = int32_t;

  StoredBtcBlockAddon() = default;
  StoredBtcBlockAddon(const addon_t& other);

  void toVbkEncoding(WriteStream& w) const;

  void toInmem(addon_t& to) const;

  std::string toPrettyString() const;

  //! reference counter for fork resolution. Stores heights of VBK blocks that
  //! contain VTBs which add this BTC block.
  // Ideally we would want a sorted collection with cheap addition, deletion and
  // lookup. In practice, due to VBK/BTC block time ratio(20x) and multiple APMs
  // running, there's little chance of VTBs shipping non-empty BTC context. The
  // altchain mempool prioritization algo will realistically pick 1 VTB per VBK
  // keystone period(20 blocks), providing us with the BTC block mined during
  // this period. Thus, we can expect to have 1-2 references per block and 2x
  // that during fork resolution making std::vector the fastest storage option
  // in this case.
  // TODO: figure out if this is somehow abusable by spammers/dosers
  std::vector<ref_height_t> refs{};

  //! list of endorsement of VBK blocks, whose BlockOfProof is
  //! this block ids. Must be a vector, because we can have duplicates here
  std::vector<uint256> blockOfProofEndorsementIds;
};

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                StoredBtcBlockAddon& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_VBK_STORAGE_STORED_BTC_BLOCK_ADDON_HPP
