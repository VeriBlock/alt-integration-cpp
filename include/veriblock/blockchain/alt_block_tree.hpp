#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_BLOCK_TREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_BLOCK_TREE_HPP_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/block_index.hpp"
#include "veriblock/blockchain/chain.hpp"
#include "veriblock/blockchain/pop/fork_resolution.hpp"
#include "veriblock/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/entities/altblock.hpp"
#include "veriblock/entities/payloads.hpp"
#include "veriblock/validation_state.hpp"

namespace altintegration {

template <>
void addContextToBlockIndex(BlockIndex<AltBlock>& index,
                            const typename BlockIndex<AltBlock>::payloads_t& p,
                            const VbkBlockTree& tree);

template <>
void removeContextFromBlockIndex(BlockIndex<AltBlock>& index,
                                 const BlockIndex<AltBlock>::payloads_t& p);

struct AltTree {
  using alt_config_t = AltChainParams;
  using vbk_config_t = VbkChainParams;
  using btc_config_t = BtcChainParams;
  using index_t = BlockIndex<AltBlock>;
  using hash_t = typename AltBlock::hash_t;
  using context_t = typename index_t::block_t::context_t;
  using payloads_t = AltPayloads;
  using block_index_t = std::unordered_map<hash_t, std::shared_ptr<index_t>>;
  using PopForkComparator =
      PopAwareForkResolutionComparator<AltBlock, AltChainParams, VbkBlockTree>;

  virtual ~AltTree() = default;

  AltTree(const alt_config_t& alt_config,
          const vbk_config_t& vbk_config,
          const btc_config_t& btc_config)
      : alt_config_(&alt_config),
        vbk_config_(&vbk_config),
        btc_config_(&btc_config),
        cmp_(VbkBlockTree(vbk_config, btc_config), vbk_config, alt_config) {}

  index_t* getBlockIndex(const std::vector<uint8_t>& hash) const;

  //! before any use, bootstrap the three with ALT bootstrap block.
  //! may return false, if bootstrap block is invalid
  bool bootstrap(ValidationState& state);

  //! add new block to current block tree.
  //! may return false, if block has no connection to blockchain
  bool acceptBlock(const AltBlock& block, ValidationState& state);

  //! add payloads to any of existing blocks in block tree.
  //! may return false, if payloads statelessly, or statefully invalid.
  bool addPayloads(const AltBlock& containingBlock,
                   const std::vector<payloads_t>& payloads,
                   ValidationState& state);

  //! removes given payloads from given block index.
  //! remove ALL payloads from a block, when it has to be removed
  void removePayloads(const AltBlock& containingBlock,
                      const std::vector<payloads_t>& payloads);

  //! set POP state to some known block, for example, when we remove a block or
  bool setState(const AltBlock::hash_t& to, ValidationState& state);

  /**
   * Determine the best chain of the AltBlocks in accordance with the VeriBlock
   * forkresolution rules.
   * @param AltBlock chain1, AltBlock chain2
   * @return '> 0' number if chain1 is better, '< 0' number if chain2 is better,
   * '0' if they are the same
   * @note chain1 and chain2 are being consindered as forks not a full chains
   * from the genesis block, they should start at the common block
   */
  int compareTwoBranches(index_t* chain1, index_t* chain2);
  int compareTwoBranches(const hash_t& chain1, const hash_t& chain2);

  VbkBlockTree& vbk() { return cmp_.getProtectingBlockTree(); }
  const VbkBlockTree& vbk() const { return cmp_.getProtectingBlockTree(); }
  VbkBlockTree::BtcTree& btc() { return cmp_.getProtectingBlockTree().btc(); }
  const VbkBlockTree::BtcTree& btc() const {
    return cmp_.getProtectingBlockTree().btc();
  }

  const AltChainParams& getParams() const { return *alt_config_; }

  const block_index_t& getAllBlocks() const { return block_index_; }

 protected:
  block_index_t block_index_;
  const alt_config_t* alt_config_;
  const vbk_config_t* vbk_config_;
  const btc_config_t* btc_config_;
  PopForkComparator cmp_;

  index_t* insertBlockHeader(const AltBlock& block);

  //! same as unix `touch`: create-and-get if not exists, get otherwise
  index_t* touchBlockIndex(const hash_t& blockHash);
};

template <>
bool AltTree::PopForkComparator::sm_t::applyContext(
    const BlockIndex<AltBlock>& index, ValidationState& state);

template <>
void AltTree::PopForkComparator::sm_t::unapplyContext(
    const BlockIndex<AltBlock>& index);

}  // namespace altintegration

#endif  // !
