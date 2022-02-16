// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "config.hpp"
#include "entities/altblock.hpp"
#include "entities/btcblock.hpp"
#include "entities/vbkblock.hpp"
#include "veriblock/pop/bootstraps.hpp"
#include "veriblock/pop/c/extern.h"
#include "veriblock/pop/config.hpp"

struct AltChainParamsImpl2 : public altintegration::AltChainParams {
  int64_t getIdentifier() const noexcept override {
    return POP_EXTERN_FUNCTION_NAME(get_altchain_id)();
  }

  //! first ALT block used in AltBlockTree. This is first block that can be
  //! endorsed.
  altintegration::AltBlock getBootstrapBlock() const noexcept override {
    auto* bootstrap_block = POP_EXTERN_FUNCTION_NAME(get_bootstrap_block)();
    return bootstrap_block->ref;
  }

  /**
   * Calculate hash from block header.
   * @param bytes serialized block header
   * @return hash
   */
  std::vector<uint8_t> getHash(
      const std::vector<uint8_t>& bytes) const noexcept override {
    VBK_ASSERT(bytes.size() != 0);

    POP_ARRAY_NAME(u8) input;
    input.size = bytes.size();
    input.data = const_cast<uint8_t*>(bytes.data());

    auto hash = POP_EXTERN_FUNCTION_NAME(get_block_header_hash)(input);

    VBK_ASSERT(hash.size <= altintegration::MAX_HEADER_SIZE_PUBLICATION_DATA);
    auto res = std::vector<uint8_t>(hash.data, hash.data + hash.size);
    pop_array_u8_free(&hash);
    return res;
  }

  bool checkBlockHeader(
      const std::vector<uint8_t>& header,
      const std::vector<uint8_t>& root,
      altintegration::ValidationState& state) const noexcept override {
    VBK_ASSERT(header.size() != 0);
    VBK_ASSERT(root.size() != 0);

    POP_ARRAY_NAME(u8) header_input;
    header_input.size = header.size();
    header_input.data = const_cast<uint8_t*>(header.data());

    POP_ARRAY_NAME(u8) root_input;
    root_input.size = root.size();
    root_input.data = const_cast<uint8_t*>(root.data());

    if (!POP_EXTERN_FUNCTION_NAME(check_block_header)(header_input,
                                                      root_input)) {
      return state.Invalid("invalid altchain block header");
    }

    return true;
  }
};

POP_ENTITY_FREE_SIGNATURE(config) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ENTITY_NEW_FUNCTION(config) {
  auto* res = new POP_ENTITY_NAME(config);
  res->ref = std::make_shared<altintegration::Config>();
  res->ref->alt = std::make_shared<AltChainParamsImpl2>();
  return res;
}

static std::vector<std::string> ParseBlocks(const std::string& blocks) {
  std::vector<std::string> ret;
  std::istringstream ss(blocks);
  std::string substr;
  while (std::getline(ss, substr, ',')) {
    ret.push_back(substr);
  }
  return ret;
}

POP_ENTITY_CUSTOM_FUNCTION(config,
                           void,
                           select_vbk_params,
                           POP_ARRAY_NAME(string) net,
                           int start_height,
                           POP_ARRAY_NAME(string) blocks) {
  VBK_ASSERT(self);
  VBK_ASSERT(net.data);
  VBK_ASSERT(blocks.data);

  if (blocks.size == 0) {
    self->ref->SelectVbkParams(
        std::string(net.data, net.data + net.size),
        start_height,
        {SerializeToRawHex(altintegration::GetRegTestVbkBlock())});
    return;
  }

  auto b = ParseBlocks(std::string(blocks.data, blocks.data + blocks.size));
  VBK_ASSERT_MSG(
      !b.empty(),
      "VBK 'blocks' does not contain valid comma-separated hexstrings");

  self->ref->SelectVbkParams(
      std::string(net.data, net.data + net.size), start_height, b);
}

POP_ENTITY_CUSTOM_FUNCTION(config,
                           void,
                           select_btc_params,
                           POP_ARRAY_NAME(string) net,
                           int start_height,
                           POP_ARRAY_NAME(string) blocks) {
  VBK_ASSERT(self);
  VBK_ASSERT(net.data);
  VBK_ASSERT(blocks.data);

  if (blocks.size == 0) {
    self->ref->SelectBtcParams(
        std::string(net.data, net.data + net.size),
        start_height,
        {SerializeToRawHex(altintegration::GetRegTestBtcBlock())});
    return;
  }

  auto b = ParseBlocks(std::string(blocks.data, blocks.data + blocks.size));
  VBK_ASSERT_MSG(
      !b.empty(),
      "VBK 'blocks' does not contain valid comma-separated hexstrings");

  self->ref->SelectBtcParams(
      std::string(net.data, net.data + net.size), start_height, b);
}

POP_ENTITY_SETTER_FUNCTION(config, double, start_of_slope) {
  VBK_ASSERT(self);
  self->ref->alt->mPopPayoutsParams->mStartOfSlope = val;
}

POP_ENTITY_SETTER_FUNCTION(config, double, slope_normal) {
  VBK_ASSERT(self);
  self->ref->alt->mPopPayoutsParams->mSlopeNormal = val;
}

POP_ENTITY_SETTER_FUNCTION(config, double, slope_keystone) {
  VBK_ASSERT(self);
  self->ref->alt->mPopPayoutsParams->mSlopeKeystone = val;
}

POP_ENTITY_SETTER_FUNCTION(config, uint32_t, keystone_round) {
  VBK_ASSERT(self);
  self->ref->alt->mPopPayoutsParams->mKeystoneRound = val;
}

POP_ENTITY_SETTER_FUNCTION(config, uint32_t, flat_score_round) {
  VBK_ASSERT(self);
  self->ref->alt->mPopPayoutsParams->mFlatScoreRound = val;
}

POP_ENTITY_SETTER_FUNCTION(config, bool, use_flat_score_round) {
  VBK_ASSERT(self);
  self->ref->alt->mPopPayoutsParams->mUseFlatScoreRound = val;
}

POP_ENTITY_SETTER_FUNCTION(config, double, max_score_threshold_normal) {
  VBK_ASSERT(self);
  self->ref->alt->mPopPayoutsParams->mMaxScoreThresholdNormal = val;
}

POP_ENTITY_SETTER_FUNCTION(config, double, max_score_threshold_keystone) {
  VBK_ASSERT(self);
  self->ref->alt->mPopPayoutsParams->mMaxScoreThresholdKeystone = val;
}

POP_ENTITY_SETTER_FUNCTION(config, uint32_t, difficulty_averaging_interval) {
  VBK_ASSERT(self);
  self->ref->alt->mPopPayoutsParams->mDifficultyAveragingInterval = val;
}

POP_ENTITY_SETTER_FUNCTION(config, POP_ARRAY_NAME(double), round_ratios) {
  VBK_ASSERT(self);
  self->ref->alt->mPopPayoutsParams->mRoundRatios =
      std::vector<double>{val.data, val.data + val.size};
}

POP_ENTITY_SETTER_FUNCTION(config,
                           POP_ARRAY_NAME(double),
                           pop_rewards_lookup_table) {
  VBK_ASSERT(self);
  self->ref->alt->mPopPayoutsParams->mLookupTable =
      std::vector<double>{val.data, val.data + val.size};
}

POP_ENTITY_SETTER_FUNCTION(config, uint32_t, max_future_block_time) {
  VBK_ASSERT(self);
  self->ref->alt->mMaxAltchainFutureBlockTime = val;
}

POP_ENTITY_SETTER_FUNCTION(config, uint32_t, keystone_interval) {
  VBK_ASSERT(self);
  self->ref->alt->mKeystoneInterval = val;
}

POP_ENTITY_SETTER_FUNCTION(config, uint32_t, vbk_finality_delay) {
  VBK_ASSERT(self);
  self->ref->alt->mFinalityDelay = val;
}

POP_ENTITY_SETTER_FUNCTION(config, uint32_t, endorsement_settlement_interval) {
  VBK_ASSERT(self);
  self->ref->alt->mEndorsementSettlementInterval = val;
}

POP_ENTITY_SETTER_FUNCTION(config, uint32_t, max_pop_data_size) {
  VBK_ASSERT(self);
  self->ref->alt->mMaxPopDataSize = val;
}

POP_ENTITY_SETTER_FUNCTION(config,
                           POP_ARRAY_NAME(u32),
                           fork_resolution_lookup_table) {
  VBK_ASSERT(self);
  self->ref->alt->mForkResolutionLookUpTable =
      std::vector<uint32_t>{val.data, val.data + val.size};
}

POP_ENTITY_SETTER_FUNCTION(config, uint32_t, pop_payout_delay) {
  VBK_ASSERT(self);
  self->ref->alt->mPopPayoutsParams->mPopPayoutDelay = val;
}

POP_ENTITY_SETTER_FUNCTION(config, uint32_t, max_reorg_distance) {
  VBK_ASSERT(self);
  self->ref->alt->mMaxReorgBlocks = val;
}

POP_ENTITY_GETTER_FUNCTION(config, double, start_of_slope) {
  VBK_ASSERT(self);
  return self->ref->alt->mPopPayoutsParams->mStartOfSlope;
}

POP_ENTITY_GETTER_FUNCTION(config, double, slope_normal) {
  VBK_ASSERT(self);
  return self->ref->alt->mPopPayoutsParams->mSlopeNormal;
}

POP_ENTITY_GETTER_FUNCTION(config, double, slope_keystone) {
  VBK_ASSERT(self);
  return self->ref->alt->mPopPayoutsParams->mSlopeKeystone;
}

POP_ENTITY_GETTER_FUNCTION(config, uint32_t, keystone_round) {
  VBK_ASSERT(self);
  return self->ref->alt->mPopPayoutsParams->mKeystoneRound;
}

POP_ENTITY_GETTER_FUNCTION(config, uint32_t, flat_score_round) {
  VBK_ASSERT(self);
  return self->ref->alt->mPopPayoutsParams->mFlatScoreRound;
}

POP_ENTITY_GETTER_FUNCTION(config, bool, use_flat_score_round) {
  VBK_ASSERT(self);
  return self->ref->alt->mPopPayoutsParams->mUseFlatScoreRound;
}

POP_ENTITY_GETTER_FUNCTION(config, double, max_score_threshold_normal) {
  VBK_ASSERT(self);
  return self->ref->alt->mPopPayoutsParams->mMaxScoreThresholdNormal;
}

POP_ENTITY_GETTER_FUNCTION(config, double, max_score_threshold_keystone) {
  VBK_ASSERT(self);
  return self->ref->alt->mPopPayoutsParams->mMaxScoreThresholdKeystone;
}

POP_ENTITY_GETTER_FUNCTION(config, uint32_t, difficulty_averaging_interval) {
  VBK_ASSERT(self);
  return self->ref->alt->mPopPayoutsParams->mDifficultyAveragingInterval;
}

POP_ENTITY_GETTER_FUNCTION(config, POP_ARRAY_NAME(double), round_ratios) {
  VBK_ASSERT(self);

  auto v = self->ref->alt->mPopPayoutsParams->mRoundRatios;

  POP_ARRAY_NAME(double) res;
  res.size = v.size();
  res.data = new double[res.size];
  for (size_t i = 0; i < v.size(); ++i) {
    res.data[i] = v[i];
  }

  return res;
}

POP_ENTITY_GETTER_FUNCTION(config,
                           POP_ARRAY_NAME(double),
                           pop_rewards_lookup_table) {
  VBK_ASSERT(self);

  auto v = self->ref->alt->mPopPayoutsParams->mLookupTable;

  POP_ARRAY_NAME(double) res;
  res.size = v.size();
  res.data = new double[res.size];
  for (size_t i = 0; i < v.size(); ++i) {
    res.data[i] = v[i];
  }

  return res;
}

POP_ENTITY_GETTER_FUNCTION(config, uint32_t, max_future_block_time) {
  VBK_ASSERT(self);
  return self->ref->alt->mMaxAltchainFutureBlockTime;
}

POP_ENTITY_GETTER_FUNCTION(config, uint32_t, keystone_interval) {
  VBK_ASSERT(self);
  return self->ref->alt->mKeystoneInterval;
}

POP_ENTITY_GETTER_FUNCTION(config, uint32_t, vbk_finality_delay) {
  VBK_ASSERT(self);
  return self->ref->alt->mFinalityDelay;
}

POP_ENTITY_GETTER_FUNCTION(config, uint32_t, endorsement_settlement_interval) {
  VBK_ASSERT(self);
  return self->ref->alt->mEndorsementSettlementInterval;
}

POP_ENTITY_GETTER_FUNCTION(config, uint32_t, max_pop_data_size) {
  VBK_ASSERT(self);
  return self->ref->alt->mMaxPopDataSize;
}

POP_ENTITY_GETTER_FUNCTION(config,
                           POP_ARRAY_NAME(u32),
                           fork_resolution_lookup_table) {
  VBK_ASSERT(self);

  auto v = self->ref->alt->mForkResolutionLookUpTable;

  POP_ARRAY_NAME(u32) res;
  res.size = v.size();
  res.data = new uint32_t[res.size];
  for (size_t i = 0; i < v.size(); ++i) {
    res.data[i] = v[i];
  }

  return res;
}

POP_ENTITY_GETTER_FUNCTION(config, uint32_t, pop_payout_delay) {
  VBK_ASSERT(self);
  return self->ref->alt->mPopPayoutsParams->mPopPayoutDelay;
}

POP_ENTITY_GETTER_FUNCTION(config, uint32_t, max_vbk_blocks_in_alt_block) {
  VBK_ASSERT(self);
  return self->ref->alt->mMaxVbkBlocksInAltBlock;
}

POP_ENTITY_GETTER_FUNCTION(config, uint32_t, max_vtbs_in_alt_block) {
  VBK_ASSERT(self);
  return self->ref->alt->mMaxVTBsInAltBlock;
}

POP_ENTITY_GETTER_FUNCTION(config, uint32_t, max_atvs_in_alt_block) {
  VBK_ASSERT(self);
  return self->ref->alt->mMaxATVsInAltBlock;
}

POP_ENTITY_GETTER_FUNCTION(config, uint32_t, finality_delay) {
  VBK_ASSERT(self);
  return self->ref->alt->mFinalityDelay;
}

POP_ENTITY_GETTER_FUNCTION(config, uint32_t, max_altchain_future_block_time) {
  VBK_ASSERT(self);
  return self->ref->alt->mMaxAltchainFutureBlockTime;
}

POP_ENTITY_GETTER_FUNCTION(config, uint32_t, max_reorg_distance) {
  VBK_ASSERT(self);
  return self->ref->alt->mMaxReorgBlocks;
}

POP_ENTITY_GETTER_FUNCTION(config,
                           POP_ENTITY_NAME(alt_block) *,
                           alt_bootstrap_block) {
  VBK_ASSERT(self);
  auto* v = new POP_ENTITY_NAME(alt_block);
  v->ref = self->ref->alt->getBootstrapBlock();
  return v;
}

POP_ENTITY_GETTER_FUNCTION(config, POP_ARRAY_NAME(string), vbk_network_name) {
  VBK_ASSERT(self);
  VBK_ASSERT_MSG(self->ref->vbk.params,
                 "vbk config has not been initialized. Select vbk config.");

  auto name = self->ref->getVbkParams().networkName();

  POP_ARRAY_NAME(string) res;
  res.size = strlen(name);
  // add some space for string null terminator. It is unused but keeps compiler
  // happy.
  res.data = new char[res.size + 1];
  strncpy(res.data, name, res.size + 1);

  return res;
}

POP_ENTITY_GETTER_FUNCTION(config, POP_ARRAY_NAME(string), btc_network_name) {
  VBK_ASSERT(self);
  VBK_ASSERT_MSG(self->ref->btc.params,
                 "btc config has not been initialized. Select btc config.");

  auto name = self->ref->getBtcParams().networkName();

  POP_ARRAY_NAME(string) res;
  res.size = strlen(name);
  res.data = new char[res.size + 1];
  strncpy(res.data, name, res.size + 1);

  return res;
}
