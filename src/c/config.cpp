// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "config.hpp"

#include "veriblock/blockchain/alt_chain_params.hpp"
#include "veriblock/blockchain/btc_chain_params.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"
#include "veriblock/bootstraps.hpp"
#include "veriblock/c/config.h"
#include "veriblock/c/extern.h"
#include "veriblock/config.hpp"

struct AltChainParamsImpl : public altintegration::AltChainParams {
  int64_t getIdentifier() const noexcept override {
    return VBK_getAltchainId();
  }

  //! first ALT block used in AltBlockTree. This is first block that can be
  //! endorsed.
  altintegration::AltBlock getBootstrapBlock() const noexcept override {
    return altintegration::AssertDeserializeFromRawHex<
        altintegration::AltBlock>(VBK_getBootstrapBlock());
  }

  /**
   * Calculate hash from block header.
   * @param bytes serialized block header
   * @return hash
   */
  std::vector<uint8_t> getHash(
      const std::vector<uint8_t>& bytes) const noexcept override {
    VBK_ASSERT(bytes.size() != 0);
    std::vector<uint8_t> hash(altintegration::MAX_HEADER_SIZE_PUBLICATION_DATA,
                              0);
    int size = 0;
    VBK_getBlockHeaderHash(bytes.data(), (int)bytes.size(), hash.data(), &size);
    VBK_ASSERT(size <= altintegration::MAX_HEADER_SIZE_PUBLICATION_DATA);
    hash.resize(size);
    return hash;
  }

  bool checkBlockHeader(
      const std::vector<uint8_t>& bytes,
      const std::vector<uint8_t>& root) const noexcept override {
    return VBK_checkBlockHeader(
        bytes.data(), (int)bytes.size(), root.data(), (int)root.size());
  };
};

// Config itself is defined in config.hpp
Config_t* VBK_NewConfig() {
  auto* v = new Config();
  v->config = std::make_shared<altintegration::Config>();
  v->config->alt = std::make_shared<AltChainParamsImpl>();
  return v;
}

void VBK_FreeConfig(Config_t* config) {
  if (config != nullptr) {
    delete config;
    config = nullptr;
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
    config->config->SelectVbkParams(
        net,
        startHeight,
        {SerializeToRawHex(altintegration::GetRegTestVbkBlock())});
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
    config->config->SelectBtcParams(
        net,
        startHeight,
        {SerializeToRawHex(altintegration::GetRegTestBtcBlock())});
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
  params->config->alt->mPopPayoutsParams->mStartOfSlope = val;
}
void VBK_SetSlopeNormal(Config_t* params, double val) {
  params->config->alt->mPopPayoutsParams->mSlopeNormal = val;
}
void VBK_SetSlopeKeystone(Config_t* params, double val) {
  params->config->alt->mPopPayoutsParams->mSlopeKeystone = val;
}
void VBK_SetKeystoneRound(Config_t* params, uint32_t val) {
  params->config->alt->mPopPayoutsParams->mKeystoneRound = val;
}
void VBK_SetFlatScoreRound(Config_t* params, uint32_t val) {
  params->config->alt->mPopPayoutsParams->mFlatScoreRound = val;
}
void VBK_SetUseFlatScoreRound(Config_t* params, bool val) {
  params->config->alt->mPopPayoutsParams->mUseFlatScoreRound = val;
}
void VBK_SetMaxScoreThresholdNormal(Config_t* params, double val) {
  params->config->alt->mPopPayoutsParams->mMaxScoreThresholdNormal = val;
}
void VBK_SetMaxScoreThresholdKeystone(Config_t* params, double val) {
  params->config->alt->mPopPayoutsParams->mMaxScoreThresholdKeystone = val;
}
void VBK_SetDifficultyAveragingInterval(Config_t* params, uint32_t val) {
  params->config->alt->mPopPayoutsParams->mDifficultyAveragingInterval = val;
}
void VBK_SetRoundRatios(Config_t* params, const double* vals, int valslen) {
  params->config->alt->mPopPayoutsParams->mRoundRatios =
      std::vector<double>{vals, vals + valslen};
}
void VBK_SetPopRewardsLookupTable(Config_t* params,
                                  const double* vals,
                                  int valslen) {
  params->config->alt->mPopPayoutsParams->mLookupTable =
      std::vector<double>{vals, vals + valslen};
}
void VBK_SetMaxFutureBlockTime(Config_t* params, uint32_t val) {
  params->config->alt->mMaxAltchainFutureBlockTime = val;
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
void VBK_SetForkResolutionLookupTable(Config_t* params,
                                      const uint32_t* vals,
                                      int valslen) {
  params->config->alt->mForkResolutionLookUpTable =
      std::vector<uint32_t>{vals, vals + valslen};
}
void VBK_SetPopPayoutDelay(Config_t* params, int32_t val) {
  params->config->alt->mPopPayoutsParams->mPopPayoutDelay = val;
}

uint32_t VBK_GetMaxPopDataSize(Config_t* params) {
  return params->config->alt->mMaxPopDataSize;
}