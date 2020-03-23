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

  virtual ~AltTree() = default;

  AltTree(std::shared_ptr<config_t> config,
          std::shared_ptr<BtcChainParams> btcParams,
          std::shared_ptr<VbkChainParams> vbkParams,
          std::shared_ptr<EndorsementRepository<BtcEndorsement>> btce,
          std::shared_ptr<EndorsementRepository<VbkEndorsement>> vbke)
      : config_(config), pop_(btcParams, vbkParams, btce, vbke, config) {}

  index_t* getBlockIndex(const std::vector<uint8_t>& hash) const;

  bool bootstrapWithGenesis(ValidationState& state);

  // accept block with a single payload from this block
  bool acceptBlock(const AltBlock& block,
                   const Payloads& payloads,
                   ValidationState& state,
                   StateChange* change = nullptr);

  bool acceptBlock(const AltBlock& block,
                   const std::vector<Payloads>& payloads,
                   ValidationState& state,
                   StateChange* change = nullptr);

  bool setState(const AltBlock::hash_t& hash,
                ValidationState& state,
                StateChange* change = nullptr);

  // void invalidateBlockByHash(const hash_t& hash);

  int compareThisToOtherChain(index_t* other);

  index_t* currentPopState() { return popState_; }

 protected:
  block_index_t block_index_;
  std::shared_ptr<config_t> config_;
  std::shared_ptr<PayloadsRepository<AltBlock, Payloads>> prepo_;

  index_t* popState_;
  PopManager pop_;

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
