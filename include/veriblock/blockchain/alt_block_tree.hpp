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
  using config_t = AltChainParams;
  using index_t = BlockIndex<AltBlock>;
  using hash_t = typename AltBlock::hash_t;
  using block_index_t = std::unordered_map<hash_t, std::unique_ptr<index_t>>;
  using VbkTree = VbkBlockTree;

  virtual ~AltTree() = default;

  AltTree(const config_t& config,
          PopManager pop,
          std::shared_ptr<PayloadsRepository<AltBlock, Payloads>> prepo)
      : config_(config), pop_(std::move(pop)), prepo_(std::move(prepo)) {}

  index_t* getBlockIndex(const std::vector<uint8_t>& hash) const;

  bool bootstrapWithGenesis(ValidationState& state);

  // accept block with a single payload from this block
  bool acceptBlock(const AltBlock& block,
                   const Payloads* payloads,
                   ValidationState& state,
                   StateChange* change = nullptr);

  bool setState(const AltBlock::hash_t& hash, ValidationState& state);

  PopManager& currentPopManager() { return pop_; }
  index_t* currentPopState() { return popState_; }

  const PopManager& currentPopManager() const { return pop_; }
  const index_t* currentPopState() const { return popState_; }

  /**
   * Determine the best chain of the AltBlocks in accordance with the VeriBlock
   * forkresolution rules
   * @param AltBlock chain1, AltBlock chain2
   * @return '> 0' number if chain1 is better, '< 0' number if chain2 is better,
   * '0' if they are the same
   * @note chain1 and chain2 are being consindered as forks not a full chains
   * from the genesis block, they should start at the common block
   */
  int compareTwoBranches(const Chain<index_t>& chain1,
                         const Chain<index_t>& chain2);

 protected:
  block_index_t block_index_;
  const config_t& config_;

  index_t* popState_{};
  PopManager pop_;

  ComparePopScore<AltTree, VbkTree, VbkEndorsement> compare_;

  std::shared_ptr<PayloadsRepository<AltBlock, Payloads>> prepo_;

  index_t* insertBlockHeader(const AltBlock& block);

  //! same as unix `touch`: create-and-get if not exists, get otherwise
  index_t* touchBlockIndex(const hash_t& blockHash);

  void unapply(PopManager& pop, index_t** popState, index_t& to);

  bool apply(PopManager& pop,
             index_t** popState,
             index_t& to,
             ValidationState& state);

  bool unapplyAndApply(PopManager& pop,
                       index_t** popState,
                       index_t& to,
                       ValidationState& state);
};

}  // namespace altintegration

#endif  // !
