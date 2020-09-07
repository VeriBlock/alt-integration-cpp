// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "config.h"

#include <veriblock/blockchain/alt_chain_params.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/config.hpp>

#include "extern.h"

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

struct Config {
  altintegration::Config config;
};

Config_t* VBK_NewConfig() { return new Config(); }
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
  std::stringstream ss(blocks);
  while (ss.good()) {
    std::string substr;
    getline(ss, substr, ',');
    ret.push_back(substr);
  }
  return ret;
}

bool VBK_SelectVbkParams(Config_t* config,
                         const char* net,
                         int startHeight,
                         const char* blocks) {
  auto vbknet = ParseVbkNetwork(net);
  if (!vbknet) {
    return false;
  }

  if (blocks == nullptr) {
    // bootstrap with genesis
    config->config.setVBK(startHeight, {}, vbknet);
    return true;
  }

  auto b = ParseBlocks(blocks);
  if (b.empty()) {
    // blocks can not be empty
    return false;
  }

  config->config.setVBK(startHeight, b, vbknet);
  return true;
}

bool VBK_SelectBtcParams(Config_t* config,
                         const char* net,
                         int startHeight,
                         const char* blocks) {
  auto btcnet = ParseBtcNetwork(net);
  if (!btcnet) {
    return false;
  }

  if (blocks == nullptr) {
    // bootstrap with genesis
    config->config.setBTC(startHeight, {}, btcnet);
    return true;
  }

  auto b = ParseBlocks(blocks);
  if (b.empty()) {
    // blocks can not be empty
    return false;
  }

  config->config.setBTC(startHeight, b, btcnet);
  return true;
}

struct AltParams {
  std::shared_ptr<altintegration::AltChainParams> alt =
      std::make_shared<AltChainParamsImpl>();
};

void VBK_SetAltParams(Config_t* config, AltParams_t* params) {
  VBK_ASSERT(config);
  VBK_ASSERT(params);
  config->config.alt = params->alt;
}

void VBK_SetStartOfSlope(AltParams_t* params, double val) {
  params->alt->mPopRewardsParams->mStartOfSlope = val;
}
void VBK_SetSlopeNormal(AltParams_t* params, double val) {
  params->alt->mPopRewardsParams->mSlopeNormal = val;
}
void VBK_SetSlopeKeystone(AltParams_t* params, double val) {
  params->alt->mPopRewardsParams->mSlopeKeystone = val;
}
void VBK_SetKeystoneRound(AltParams_t* params, uint32_t val) {
  params->alt->mPopRewardsParams->mKeystoneRound = val;
}
void VBK_SetFlatScoreRound(AltParams_t* params, uint32_t val) {
  params->alt->mPopRewardsParams->mFlatScoreRound = val;
}
void VBK_SetUseFlatScoreRound(AltParams_t* params, bool val) {
  params->alt->mPopRewardsParams->mUseFlatScoreRound = val;
}
void VBK_SetMaxScoreThresholdNormal(AltParams_t* params, double val) {
  params->alt->mPopRewardsParams->mMaxScoreThresholdNormal = val;
}
void VBK_SetMaxScoreThresholdKeystone(AltParams_t* params, double val) {
  params->alt->mPopRewardsParams->mMaxScoreThresholdKeystone = val;
}
void VBK_SetDifficultyAveragingInterval(AltParams_t* params, uint32_t val) {
  params->alt->mPopRewardsParams->mDifficultyAveragingInterval = val;
}
void VBK_SetRoundRatios(AltParams_t* params, double* vals, int valslen) {
  params->alt->mPopRewardsParams->mRoundRatios =
      std::vector<double>{vals, vals + valslen};
}
void VBK_SetPopRewardsLookupTable(AltParams_t* params,
                                  double* vals,
                                  int valslen) {
  params->alt->mPopRewardsParams->mLookupTable =
      std::vector<double>{vals, vals + valslen};
}
void VBK_SetMaxFutureBlockTime(AltParams_t* params, uint32_t val) {
  params->alt->mMaxFutureBlockTime = val;
}
void VBK_SetKeystoneInterval(AltParams_t* params, uint32_t val) {
  params->alt->mKeystoneInterval = val;
}
void VBK_SetVbkFinalityDelay(AltParams_t* params, uint32_t val) {
  params->alt->mFinalityDelay = val;
}
void VBK_SetEndorsementSettlementInterval(AltParams_t* params, uint32_t val) {
  params->alt->mEndorsementSettlementInterval = val;
}
void VBK_SetMaxPopDataSize(AltParams_t* params, uint32_t val) {
  params->alt->mMaxPopDataSize = val;
}
void SetForkResolutionLookupTable(AltParams_t* params,
                                  uint32_t* vals,
                                  int valslen) {
  params->alt->mForkResolutionLookUpTable =
      std::vector<uint32_t>{vals, vals + valslen};
}