// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/c/config.h"

#include "config.hpp"
#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/btc_chain_params.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/c/extern.h"
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
    VBK_getBlockHeaderHash(bytes.data(), (int)bytes.size(), hash.data(), &size);
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
  if (blocks == nullptr) {
    config->config->SelectVbkParams(net, startHeight, {});
    return true;
  }

  auto b = ParseBlocks(blocks);
  VBK_ASSERT_MSG(
      !b.empty(),
      "VBK 'blocks' does not contain valid comma-separated hexstrings");

  config->config->SelectVbkParams(net, startHeight, b);
  return true;
}

bool VBK_SelectBtcParams(Config_t* config,
                         const char* net,
                         int startHeight,
                         const char* blocks) {
  if (blocks == nullptr) {
    config->config->SelectBtcParams(net, startHeight, {});
    return true;
  }

  auto b = ParseBlocks(blocks);
  VBK_ASSERT_MSG(
      !b.empty(),
      "BTC 'blocks' does not contain valid comma-separated hexstrings");

  config->config->SelectBtcParams(net, startHeight, b);
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
