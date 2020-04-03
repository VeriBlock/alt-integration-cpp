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

struct AltTree {
  using alt_config_t = AltChainParams;
  using vbk_config_t = VbkChainParams;
  using btc_config_t = BtcChainParams;
  using index_t = BlockIndex<AltBlock>;
  using hash_t = typename AltBlock::hash_t;
  using block_index_t = std::unordered_map<hash_t, std::shared_ptr<index_t>>;
  using PopForkComparator =
      PopAwareForkResolutionComparator<AltBlock, AltChainParams, VbkBlockTree>;

  virtual ~AltTree() = default;

  AltTree(const alt_config_t& alt_config,
          const vbk_config_t& vbk_config,
          const btc_config_t& btc_config)
      : alt_config_(alt_config),
        vbk_config_(vbk_config),
        btc_config_(btc_config),
        cmp_(VbkBlockTree(vbk_config, btc_config), vbk_config, alt_config) {}

  index_t* getBlockIndex(const std::vector<uint8_t>& hash) const;

  bool bootstrapWithGenesis(ValidationState& state);

  bool acceptBlock(const AltBlock& block,
                   const std::vector<AltPayloads>& contexVbkBlocks,
                   ValidationState& state);

 protected:
  std::vector<index_t*> chainTips_;
  block_index_t block_index_;
  const alt_config_t& alt_config_;
  const vbk_config_t& vbk_config_;
  const btc_config_t& btc_config_;
  PopForkComparator cmp_;

  index_t* insertBlockHeader(const AltBlock& block);

  //! same as unix `touch`: create-and-get if not exists, get otherwise
  index_t* touchBlockIndex(const hash_t& blockHash);

  void addToChains(index_t* block_index);
};

template <>
bool PopStateMachine<VbkBlockTree, BlockIndex<AltBlock>, AltChainParams>::
    applyContext(const BlockIndex<AltBlock>& index, ValidationState& state);

template <>
void PopStateMachine<VbkBlockTree, BlockIndex<AltBlock>, AltChainParams>::
    unapplyContext(const BlockIndex<AltBlock>& index);

template <>
void addContextToBlockIndex(BlockIndex<AltBlock>& index,
                            const typename BlockIndex<AltBlock>::context_t& p,
                            const VbkBlockTree& tree);
/*
template <>
void removeContextFromBlockIndex(BlockIndex<AltBlock>& index,
                                 const BlockIndex<AltBlock>::payloads_t& p);
                                 */

}  // namespace altintegration

#endif  // !
