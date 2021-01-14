// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ALTINTEGRATION_HPP
#define ALTINTEGRATION_ALTINTEGRATION_HPP

#include <utility>
#include <veriblock/alt-util.hpp>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/bootstraps.hpp>
#include <veriblock/config.hpp>
#include <veriblock/mempool.hpp>
#include <veriblock/pop_stateless_validator.hpp>
#include <veriblock/rewards/default_poprewards_calculator.hpp>
#include <veriblock/storage/block_provider.hpp>
#include <veriblock/storage/payloads_index.hpp>

/**
 * @defgroup api Public API
 *
 * This is public API of veriblock-pop-cpp library, which is meant to be used by
 * Altchains.
 */

namespace altintegration {

/**
 * @struct Altintegration
 *
 * Main entrypoint into the veriblock-pop library.
 *
 * @ingroup api
 */
struct PopContext {
  ~PopContext() {
    if (popValidator) {
      shutdown();
    }
  }

  static std::shared_ptr<PopContext> create(
      const Config& config,
      std::shared_ptr<PayloadsProvider> payloadsProvider,
      std::shared_ptr<BlockProvider> blockProvider,
      size_t validatorWorkers = 0) {
    return create(std::make_shared<Config>(config),
                  std::move(payloadsProvider),
                  std::move(blockProvider),
                  validatorWorkers);
  }

  static std::shared_ptr<PopContext> create(
      std::shared_ptr<Config> config,
      std::shared_ptr<PayloadsProvider> payloadsProvider,
      std::shared_ptr<BlockProvider> blockProvider,
      size_t validatorWorkers = 0) {
    config->validate();

    // because default constructor is hidden
    auto ctx = std::shared_ptr<PopContext>(new PopContext());
    ctx->config = std::move(config);
    ctx->payloadsProvider = std::move(payloadsProvider);
    ctx->blockProvider = std::move(blockProvider);
    ctx->altTree = std::make_shared<AltBlockTree>(*ctx->config->alt,
                                                  *ctx->config->vbk.params,
                                                  *ctx->config->btc.params,
                                                  *ctx->payloadsProvider);
    ctx->popRewardsCalculator =
        std::make_shared<DefaultPopRewardsCalculator>(*ctx->altTree);
    ctx->mempool = std::make_shared<MemPool>(*ctx->altTree);
    ctx->popValidator = std::make_shared<PopValidator>(*ctx->config->vbk.params,
                                                       *ctx->config->btc.params,
                                                       *ctx->config->alt,
                                                       validatorWorkers);
    ValidationState state;

    // first, bootstrap BTC
    if (ctx->config->btc.blocks.size() == 0) {
      ctx->altTree->btc().bootstrapWithGenesis(GetRegTestBtcBlock(), state);
    } else if (ctx->config->btc.blocks.size() == 1) {
      ctx->altTree->btc().bootstrapWithGenesis(ctx->config->btc.blocks[0],
                                               state);
    } else {
      ctx->altTree->btc().bootstrapWithChain(
          ctx->config->btc.startHeight, ctx->config->btc.blocks, state);
    }
    VBK_ASSERT_MSG(state.IsValid(),
                   "BTC bootstrap block is invalid: %s",
                   state.toString());

    // then, bootstrap VBK
    if (ctx->config->vbk.blocks.size() == 0) {
      ctx->altTree->vbk().bootstrapWithGenesis(GetRegTestVbkBlock(), state);
    } else if (ctx->config->vbk.blocks.size() == 1) {
      ctx->altTree->vbk().bootstrapWithGenesis(ctx->config->vbk.blocks[0],
                                               state);
    } else {
      ctx->altTree->vbk().bootstrapWithChain(
          ctx->config->vbk.startHeight, ctx->config->vbk.blocks, state);
    }
    VBK_ASSERT_MSG(state.IsValid(),
                   "VBK bootstrap block is invalid: %s",
                   state.toString());

    // then, bootstrap ALT
    bool bootstrapped = ctx->altTree->bootstrap(state);
    VBK_ASSERT_MSG(
        bootstrapped, "Can not bootstrap Alt Tree: %s", state.toString());
    VBK_ASSERT_MSG(state.IsValid(),
                   "ALT bootstrap block is invalid: %s",
                   state.toString());
    return ctx;
  }

  void shutdown() {
    VBK_ASSERT_MSG(popValidator != nullptr, "PopContext is not initialized");
    popValidator->stop();
  }

  std::shared_ptr<Config> config;
  std::shared_ptr<MemPool> mempool;
  std::shared_ptr<AltBlockTree> altTree;
  std::shared_ptr<PopValidator> popValidator;
  std::shared_ptr<PopRewardsCalculator> popRewardsCalculator;
  std::shared_ptr<PayloadsProvider> payloadsProvider;
  std::shared_ptr<BlockProvider> blockProvider;

 private:
  PopContext() = default;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ALTINTEGRATION_HPP
