// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BTC_BLOCK_INDEX_HPP
#define VERIBLOCK_POP_CPP_BTC_BLOCK_INDEX_HPP

#include <veriblock/arith_uint256.hpp>
#include <veriblock/blockchain/block_status.hpp>
#include <veriblock/entities/endorsements.hpp>
#include <veriblock/logger.hpp>
#include <veriblock/serde.hpp>

namespace altintegration {

//! @private
struct BtcBlockAddon {
  using ref_height_t = int32_t;

  //! (memory only) total amount of work in the chain up to and including this
  //! block
  ArithUint256 chainWork = 0;

  //! (memory-only) a list of endorsements of VBK blocks, whose BlockOfProof is
  //! this block. must be a vector, because we can have duplicates here
  std::vector<VbkEndorsement*> blockOfProofEndorsements;

  static constexpr auto validTipLevel = BLOCK_VALID_TREE;

  void setNullInmemFields();

  void setIsBootstrap(bool isBootstrap);

  uint32_t refCount() const;

  const std::vector<ref_height_t>& getRefs() const { return refs; }

  void addRef(ref_height_t referencedAtHeight);

  void removeRef(ref_height_t referencedAtHeight);

  void toVbkEncoding(WriteStream& w) const;

  std::string toPrettyString() const;

 protected:
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

  void setDirty();

  void setNull();

 private:
  friend bool DeserializeFromVbkEncoding(ReadStream& stream,
                                         BtcBlockAddon& out,
                                         ValidationState& state);
};

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                BtcBlockAddon& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_BTC_BLOCK_INDEX_HPP
