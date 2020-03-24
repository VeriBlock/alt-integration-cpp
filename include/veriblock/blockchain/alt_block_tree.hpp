#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_BLOCK_TREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_BLOCK_TREE_HPP_

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/blockchain/btc_chain_params.hpp"
#include "veriblock/blockchain/chain.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/entities/altblock.hpp"
#include "veriblock/entities/endorsement.hpp"
#include "veriblock/entities/payloads.hpp"
#include "veriblock/popmanager.hpp"
#include "veriblock/storage/endorsement_repository.hpp"
#include "veriblock/storage/payloads_repository.hpp"
#include "veriblock/validation_state.hpp"

namespace altintegration {

struct AltTree {
  using block_t = AltBlock;
  using params_t = AltChainParams;
  using index_t = BlockIndex<AltBlock>;
  using hash_t = typename AltBlock::hash_t;
  using block_index_t = std::unordered_map<hash_t, std::shared_ptr<index_t>>;
  using VbkTree = VbkBlockTree;
  using BtcTree = VbkTree::BtcTree;
  using POPForkResolution = PopAwareForkResolutionComparator<AltBlock,
                                                             AltChainParams,
                                                             VbkTree,
                                                             VbkEndorsement>;

  virtual ~AltTree() = default;

  AltTree(const params_t& config, POPForkResolution cmp)
      : config_(config), cmp_(std::move(cmp)) {}

  BtcTree& btc() { return cmp_.getProtectingBlockTree().btc(); }
  const BtcTree& btc() const { return cmp_.getProtectingBlockTree().btc(); }
  VbkTree& vbk() { return cmp_.getProtectingBlockTree(); }
  const VbkTree& vbk() const { return cmp_.getProtectingBlockTree(); }

  index_t* getBlockIndex(const std::vector<uint8_t>& hash) const;

  //! before use, AltTree must be bootstrapped
  bool bootstrapWithGenesis(ValidationState& state);

  //! call acceptBlock on every new altchain block received. if block does not
  //! contain payloads, pass nullptr.
  bool acceptBlock(const AltBlock& block,
                   const Payloads* payloads,
                   ValidationState& state);

  //! call setState every time TIP of best chain changed.
  bool setState(const AltBlock::hash_t& hash, ValidationState& state);

  //! call setState every time TIP of best chain changed.
  bool setState(const index_t& index, ValidationState& state);

  /**
   * Determine the best chain of the AltBlocks in accordance with the VeriBlock
   * forkresolution rules
   * @param AltBlock chain1, AltBlock chain2
   * @return '> 0' number if chain1 is better, '< 0' number if chain2 is better,
   * '0' if they are the same
   * @note chain1 and chain2 are being consindered as forks not a full chains
   * from the genesis block, they should start at the common block
   */
  int compareTwoBranches(index_t* chain1,
                         index_t* chain2);

 protected:
  block_index_t block_index_;
  const params_t& config_;
  POPForkResolution cmp_;

  index_t* insertBlockHeader(const AltBlock& block);

  //! same as unix `touch`: create-and-get if not exists, get otherwise
  index_t* touchBlockIndex(const hash_t& blockHash);
};

template <>
bool addPayloads(AltTree& tree,
                 const Payloads& payloads,
                 ValidationState& state) {
  auto& vbk = tree.vbk();

  // does checkVTB internally
  if (!addPayloads(vbk, payloads, state)) {
    return state.addStackFunction("AltTree::addPayloads");
  }

  /// update vbk context
  for (const auto& b : payloads.vbkcontext) {
    if (!vbk.acceptBlock(b, state)) {
      return state.addStackFunction("AltTree::addPayloads");
    }
  }

  /// ADD ALL VTBs
  for (const auto& vtb : payloads.vtbs) {
    // no need to checkVTB, because first addPayloads does it

    // add vbk context blocks
    for (const auto& block : vtb.context) {
      if (!vbk.acceptBlock(block, state)) {
        return state.addStackFunction("AltTree::addPayloads");
      }
    }
  }

  /// ADD ATV
  if (payloads.alt.hasAtv) {
    // check if atv is statelessly valid
    if (!checkATV(payloads.alt.atv, state, vbk.getParams())) {
      return state.addStackFunction("AltTree::addPayloads");
    }

    /// apply atv context
    for (const auto& block : payloads.alt.atv.context) {
      if (!vbk.acceptBlock(block, state)) {
        return state.addStackFunction("AltTree::addPayloads");
      }
    }
  }

  return true;
}

template <>
bool removePayloads(AltTree& tree, const Payloads& payloads) {
  auto& vbk = tree.vbk();
  removePayloads(tree.vbk(), payloads);

  /// first, remove ATV context
  if (payloads.alt.hasAtv) {
    for (const auto& b : payloads.alt.atv.context) {
      vbk.invalidateBlockByHash(b.getHash());
    }
  }

  /// second, remove VTBs context
  for (const auto& vtb : payloads.vtbs) {
    for (const auto& b : vtb.context) {
      vbk.invalidateBlockByHash(b.getHash());
    }
  }

  /// third, remove vbk context
  for (const auto& b : payloads.vbkcontext) {
    vbk.invalidateBlockByHash(b.getHash());
  }

  return true;
}

}  // namespace altintegration

#endif  // !
