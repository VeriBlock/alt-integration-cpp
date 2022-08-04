#include <veriblock/pop/crypto/progpow.hpp>
#include <veriblock/pop/pop_context.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "veriblock/pop/alt-util.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/blockchain/alt_block_tree.hpp"
#include "veriblock/pop/blockchain/alt_chain_params.hpp"
#include "veriblock/pop/blockchain/pop/vbk_block_tree.hpp"
#include "veriblock/pop/bootstraps.hpp"
#include "veriblock/pop/config.hpp"
#include "veriblock/pop/entities/popdata.hpp"
#include "veriblock/pop/entities/publication_data.hpp"
#include "veriblock/pop/logger.hpp"
#include "veriblock/pop/mempool.hpp"
#include "veriblock/pop/pop_stateless_validator.hpp"
#include "veriblock/pop/rewards/default_poprewards_calculator.hpp"
#include "veriblock/pop/rewards/poprewards_calculator.hpp"
#include "veriblock/pop/stateless_validation.hpp"
#include "veriblock/pop/storage/util.hpp"
#include "veriblock/pop/strutil.hpp"

namespace altintegration {
class ValidationState;
struct ATV;
struct BlockBatch;
struct BlockReader;
struct EthashCache;
struct PayloadsStorage;
struct PopPayouts;
struct ProgpowHeaderCache;
struct VTB;
struct VbkBlock;

std::shared_ptr<PopContext> PopContext::create(
    std::shared_ptr<Config> config,
    std::shared_ptr<PayloadsStorage> payloadsProvider,
    std::shared_ptr<BlockReader> blockProvider,
    const std::shared_ptr<EthashCache>& ethashCache,
    const std::shared_ptr<ProgpowHeaderCache>& progpowHeaderCache,
    size_t validatorWorkers) {
  setEthashCache(ethashCache);
  setProgpowHeaderCache(progpowHeaderCache);

  config->validate();

  // because default constructor is hidden
  auto ctx = std::shared_ptr<PopContext>(new PopContext());
  ctx->config_ = std::move(config);
  ctx->payloadsProvider_ = std::move(payloadsProvider);
  ctx->blockProvider_ = std::move(blockProvider);
  ctx->altTree_ = std::make_shared<AltBlockTree>(*ctx->config_->alt,
                                                 *ctx->config_->vbk.params,
                                                 *ctx->config_->btc.params,
                                                 *ctx->payloadsProvider_,
                                                 *ctx->blockProvider_);
  ctx->popRewardsCalculator_ =
      std::make_shared<DefaultPopRewardsCalculator>(*ctx->altTree_);
  ctx->mempool_ = std::make_shared<MemPool>(*ctx->altTree_);
  ctx->popValidator_ = std::make_shared<PopValidator>(*ctx->config_->vbk.params,
                                                      *ctx->config_->btc.params,
                                                      *ctx->config_->alt,
                                                      validatorWorkers);

  // first, bootstrap BTC
  if (ctx->config_->btc.blocks.size() == 0) {
    ctx->altTree_->btc().bootstrapWithGenesis(GetRegTestBtcBlock());
  } else if (ctx->config_->btc.blocks.size() == 1) {
    ctx->altTree_->btc().bootstrapWithGenesis(ctx->config_->btc.blocks[0]);
  } else {
    ctx->altTree_->btc().bootstrapWithChain(ctx->config_->btc.startHeight,
                                            ctx->config_->btc.blocks);
  }

  // then, bootstrap VBK
  if (ctx->config_->vbk.blocks.size() == 0) {
    ctx->altTree_->vbk().bootstrapWithGenesis(GetRegTestVbkBlock());
  } else if (ctx->config_->vbk.blocks.size() == 1) {
    ctx->altTree_->vbk().bootstrapWithGenesis(ctx->config_->vbk.blocks[0]);
  } else {
    ctx->altTree_->vbk().bootstrapWithChain(ctx->config_->vbk.startHeight,
                                            ctx->config_->vbk.blocks);
  }

  // then, bootstrap ALT
  ctx->altTree_->bootstrap();
  return ctx;
}

PopContext::~PopContext() {
  if (popValidator_) {
    shutdown();
  }
}

void PopContext::shutdown() {
  VBK_ASSERT_MSG(popValidator_ != nullptr, "PopContext is not initialized");
  popValidator_->stop();
}

bool PopContext::check(const ATV& atv, ValidationState& state) {
  return checkATV(
      atv, state, getConfig().getAltParams(), getConfig().getVbkParams());
}

bool PopContext::check(const VTB& vtb, ValidationState& state) {
  return checkVTB(
      vtb, state, getConfig().getBtcParams(), getConfig().getVbkParams());
}

bool PopContext::check(const VbkBlock& v, ValidationState& state) {
  return checkBlock(v, state, getConfig().getVbkParams());
}

bool PopContext::generatePublicationData(
    PublicationData& output,
    const std::vector<uint8_t>& endorsedBlockHeader,
    const std::vector<uint8_t>& txMerkleRoot,
    const PopData& popData,
    const std::vector<uint8_t>& payoutInfo) {
  auto hash = altTree_->getParams().getHash(endorsedBlockHeader);
  auto* index = altTree_->getBlockIndex(hash);
  if (index == nullptr) {
    VBK_LOG_ERROR("Trying to endorse unknown ALT block: %s (header: %s)",
                  HexStr(hash),
                  HexStr(endorsedBlockHeader));
    return false;
  }

  output = GeneratePublicationData(endorsedBlockHeader,
                                   *index,
                                   txMerkleRoot,
                                   popData,
                                   payoutInfo,
                                   getConfig().getAltParams());

  return true;
}

const BtcBlockTree& PopContext::getBtcBlockTree() const {
  return altTree_->btc();
}

const VbkBlockTree& PopContext::getVbkBlockTree() const {
  return altTree_->vbk();
}

AltBlockTree& PopContext::getAltBlockTree() {
  VBK_ASSERT(altTree_);
  return *altTree_;
}

MemPool& PopContext::getMemPool() {
  VBK_ASSERT(mempool_);
  return *mempool_;
}

const Config& PopContext::getConfig() const {
  VBK_ASSERT(config_);
  return *config_;
}

void PopContext::saveAllTrees(BlockBatch& batch) const {
  saveTrees(getAltBlockTree(), batch);
}

bool PopContext::loadAllTrees(bool fast_load, ValidationState& state) {
  return loadTrees(getAltBlockTree(), fast_load, state);
}

bool PopContext::check(const PopData& pd, ValidationState& state) {
  VBK_ASSERT(popValidator_);
  return checkPopData(*popValidator_, pd, state);
}

const AltBlockTree& PopContext::getAltBlockTree() const {
  VBK_ASSERT(altTree_);
  return *altTree_;
}

bool PopContext::getPopPayout(const AltBlockTree::hash_t& prev,
                              PopPayouts& rewards,
                              ValidationState& state) {
  VBK_ASSERT(popRewardsCalculator_);
  return popRewardsCalculator_->getPopPayout(prev, rewards, state);
}

PopData PopContext::generatePopData() { return getMemPool().generatePopData(); }

}  // namespace altintegration
