#ifndef ALTINTEGRATION_FACADE_HPP
#define ALTINTEGRATION_FACADE_HPP

#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/config.hpp>
#include <veriblock/state_manager.hpp>
#include <veriblock/storage/endorsement_repository_inmem.hpp>
#include <veriblock/storage/repository_rocks_manager.hpp>

namespace altintegration {

struct AltIntegration {
  static AltIntegration create(const Config& config) {
    config.validate();

    AltIntegration alt;

    // create state manager
    alt.state_ =
        std::make_shared<StateManager<RepositoryRocksManager>>(config.dbName);

    auto& mgr = alt.state_->getManager();
    alt.btcer_ = mgr.getBtcEndorsementRepo();
    alt.vbker_ = mgr.getVbkEndorsementRepo();
    alt.pr_ = mgr.getAltPayloadsRepo();

    VbkBlockTree::POPComparator vbkcmp(
        config.vbk.params,
        config.btc.params,
        [&](const Chain<BlockIndex<VbkBlock>>& chain) {
          return [&](const BlockIndex<VbkBlock>& index) -> std::vector<BtcEndorsement>
                  {
                    // all endorsements of block 'index'
                    auto allEndorsements = alt.btcer_->get(index.getHash());

                    // efficiently remove endorsements that do not belong to active
                    // protect(ing/ed) chains
                    allEndorsements.erase(std::remove_if(
                        allEndorsements.begin(),
                        allEndorsements.end(),
                        [&](const BtcEndorsement & e) {
                          // 1. check that containing block is on best chain.
                          auto* edindex =
                          protectedBlockTree_.getBlockIndex(e.containingHash);

                          // 2. check that blockOfProof is still on best chain.
                          auto* ingindex = tree_.getBlockIndex(e.blockOfProof);

                          // endorsed block is guaranteed to be ancestor of containing
                          block
                          // therefore, do not do check here
                          return chain.contains(edindex) &&
                                 tree_.getBestChain().contains(ingindex);
                        },
                        allEndorsements.end()));

                    return allEndorsements;
                  };
        },
        [&](const BlockIndex<VbkBlock>& index) {
          // we re-use existing alt-payloads repo for VBK payloads
          return alt.pr_->get(index.getHash().asVector());
        });

    // create alt tree
    alt.altTree_ = std::make_shared<AltTree>(
        *config.altParams, std::move(temp), mgr.getPayloadsRepo());

    ValidationState state;

    auto must = [&state](bool ret) {
      if (!ret) {
        throw std::invalid_argument(state.GetRejectReason() + " | " +
                                    state.GetDebugMessage());
      }
    };

    // bootstrap ALT tree
    must(alt.tree().bootstrapWithGenesis(state));

    auto& pop = alt.tree().getPopManager();

    // bootstrap BTC tree
    must(pop.btc().bootstrapWithChain(
        config.btc.startHeight, config.btc.blocks, state));

    // bootstrap VBK tree
    must(pop.vbk().bootstrapWithChain(
        config.vbk.startHeight, config.vbk.blocks, state));

    return alt;
  }

  AltTree& tree() { return *altTree_; }
  StateManager<RepositoryRocksManager> state() { return *state_; }

 private:
  AltIntegration() = default;

  std::shared_ptr<AltTree> altTree_;
  std::shared_ptr<StateManager<RepositoryRocksManager>> state_;
  std::shared_ptr<EndorsementRepository<BtcEndorsement>> btcer_;
  std::shared_ptr<EndorsementRepository<VbkEndorsement>> vbker_;
  std::shared_ptr<PayloadsRepository<AltBlock, Payloads>> pr_;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_FACADE_HPP
