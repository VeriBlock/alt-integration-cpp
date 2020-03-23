#ifndef ALTINTEGRATION_FACADE_HPP
#define ALTINTEGRATION_FACADE_HPP

#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/config.hpp>
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

    // create current pop state
    PopManager temp(*config.altParams,
                   *config.btc.params,
                   *config.vbk.params,
                   mgr.getBtcEndorsementRepo(),
                   mgr.getVbkEndorsementRepo());

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

    auto& pop = alt.tree().currentPopManager();

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
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_FACADE_HPP
