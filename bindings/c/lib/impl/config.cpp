// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "../config.h"
#include "../extern.h"
#include "config.hpp"
#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/btc_chain_params.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/config.hpp"

struct AltChainParamsImpl : public altintegration::AltChainParams {
  int64_t getIdentifier() const noexcept { return VBK_getAltchainId(); }

  //! first ALT block used in AltBlockTree. This is first block that can be
  //! endorsed.
  altintegration::AltBlock getBootstrapBlock() const noexcept {
    using namespace altintegration;
    auto v = ParseHex(VBK_getBootstrapBlock());
    VBK_ASSERT_MSG(!v.empty(), "VBK_getBootstrapBlock must not be empty");
    return AltBlock::fromRaw(v);
  }

  /**
   * Calculate hash from block header.
   * @param bytes serialized block header
   * @return hash
   */
  std::vector<uint8_t> getHash(
      const std::vector<uint8_t>& bytes) const noexcept {
    std::vector<uint8_t> hash(1024, 0);
    int size = 0;
    VBK_getBlockHeaderHash(bytes.data(), bytes.size(), hash.data(), &size);
    VBK_ASSERT(size <= 1024);
    hash.resize(size);
    return hash;
  }
};

// Config itself is defined in config.hpp
Config_t* VBK_NewConfig() {
  auto* v = new Config();
  v->config = std::make_shared<altintegration::Config>();
  v->config->alt = std::make_shared<AltChainParamsImpl>();
  return v;
}

void VBK_FreeConfig(Config_t* config) {
  if (config) {
    delete config;
  }
}

static std::shared_ptr<altintegration::VbkChainParams> ParseVbkNetwork(
    const char* net) {
  using namespace altintegration;
  static const std::vector<std::shared_ptr<VbkChainParams>> all{
      std::make_shared<VbkChainParamsMain>(),
      std::make_shared<VbkChainParamsTest>(),
      std::make_shared<VbkChainParamsRegTest>(),
      std::make_shared<VbkChainParamsAlpha>(),
  };

  auto it = std::find_if(
      all.begin(), all.end(), [&](const std::shared_ptr<VbkChainParams>& p) {
        return p->networkName() == net;
      });

  if (it == all.end()) {
    return nullptr;
  }

  return *it;
}

static std::shared_ptr<altintegration::BtcChainParams> ParseBtcNetwork(
    const char* net) {
  using namespace altintegration;
  static const std::vector<std::shared_ptr<BtcChainParams>> all{
      std::make_shared<BtcChainParamsMain>(),
      std::make_shared<BtcChainParamsTest>(),
      std::make_shared<BtcChainParamsRegTest>(),
  };

  auto it = std::find_if(
      all.begin(), all.end(), [&](const std::shared_ptr<BtcChainParams>& p) {
        return p->networkName() == net;
      });

  if (it == all.end()) {
    return nullptr;
  }

  return *it;
}

static std::vector<std::string> ParseBlocks(const char* blocks) {
  std::vector<std::string> ret;
  std::istringstream ss(blocks);
  std::string substr;
  while (std::getline(ss, substr, ',')) {
    ret.push_back(substr);
  }
  return ret;
}

bool VBK_SelectVbkParams(Config_t* config,
                         const char* net,
                         int startHeight,
                         const char* blocks) {
  auto vbknet = ParseVbkNetwork(net);
  VBK_ASSERT_MSG(
      vbknet, "VBK network is invalid, must be one of main/test/regtest/alpha");

  if (blocks == nullptr) {
    // bootstrap with genesis
    config->config->setVBK(startHeight, {}, vbknet);
    return true;
  }

  auto b = ParseBlocks(blocks);
  VBK_ASSERT_MSG(!b.empty(),
                 "'blocks' does not contain valid comma-separated hexstrings");

  config->config->setVBK(startHeight, b, vbknet);
  return true;
}

bool VBK_SelectBtcParams(Config_t* config,
                         const char* net,
                         int startHeight,
                         const char* blocks) {
  auto btcnet = ParseBtcNetwork(net);
  VBK_ASSERT_MSG(btcnet,
                 "BTC network is invalid, must be one of main/test/regtest");

  if (blocks == nullptr) {
    // bootstrap with genesis
    config->config->setBTC(startHeight, {}, btcnet);
    return true;
  }

  auto b = ParseBlocks(blocks);
  VBK_ASSERT_MSG(!b.empty(),
                 "'blocks' does not contain valid comma-separated hexstrings");

  config->config->setBTC(startHeight, b, btcnet);
  return true;
}

void VBK_SetStartOfSlope(Config_t* params, double val) {
  params->config->alt->mPopRewardsParams->mStartOfSlope = val;
}
void VBK_SetSlopeNormal(Config_t* params, double val) {
  params->config->alt->mPopRewardsParams->mSlopeNormal = val;
}
void VBK_SetSlopeKeystone(Config_t* params, double val) {
  params->config->alt->mPopRewardsParams->mSlopeKeystone = val;
}
void VBK_SetKeystoneRound(Config_t* params, uint32_t val) {
  params->config->alt->mPopRewardsParams->mKeystoneRound = val;
}
void VBK_SetFlatScoreRound(Config_t* params, uint32_t val) {
  params->config->alt->mPopRewardsParams->mFlatScoreRound = val;
}
void VBK_SetUseFlatScoreRound(Config_t* params, bool val) {
  params->config->alt->mPopRewardsParams->mUseFlatScoreRound = val;
}
void VBK_SetMaxScoreThresholdNormal(Config_t* params, double val) {
  params->config->alt->mPopRewardsParams->mMaxScoreThresholdNormal = val;
}
void VBK_SetMaxScoreThresholdKeystone(Config_t* params, double val) {
  params->config->alt->mPopRewardsParams->mMaxScoreThresholdKeystone = val;
}
void VBK_SetDifficultyAveragingInterval(Config_t* params, uint32_t val) {
  params->config->alt->mPopRewardsParams->mDifficultyAveragingInterval = val;
}
void VBK_SetRoundRatios(Config_t* params, const double* vals, int valslen) {
  params->config->alt->mPopRewardsParams->mRoundRatios =
      std::vector<double>{vals, vals + valslen};
}
void VBK_SetPopRewardsLookupTable(Config_t* params,
                                  const double* vals,
                                  int valslen) {
  params->config->alt->mPopRewardsParams->mLookupTable =
      std::vector<double>{vals, vals + valslen};
}
void VBK_SetMaxFutureBlockTime(Config_t* params, uint32_t val) {
  params->config->alt->mMaxFutureBlockTime = val;
}
void VBK_SetKeystoneInterval(Config_t* params, uint32_t val) {
  params->config->alt->mKeystoneInterval = val;
}
void VBK_SetVbkFinalityDelay(Config_t* params, uint32_t val) {
  params->config->alt->mFinalityDelay = val;
}
void VBK_SetEndorsementSettlementInterval(Config_t* params, uint32_t val) {
  params->config->alt->mEndorsementSettlementInterval = val;
}
void VBK_SetMaxPopDataSize(Config_t* params, uint32_t val) {
  params->config->alt->mMaxPopDataSize = val;
}
uint32_t VBK_GetMaxPopDataSize(Config_t* params) {
  return params->config->alt->mMaxPopDataSize;
}
void VBK_SetForkResolutionLookupTable(Config_t* params,
                                      const uint32_t* vals,
                                      int valslen) {
  params->config->alt->mForkResolutionLookUpTable =
      std::vector<uint32_t>{vals, vals + valslen};
}
