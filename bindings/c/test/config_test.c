// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <assert.h>
#include <string.h>

#include "../lib/config.h"

void VBK_SetStartOfSlope_test() {
  Config_t* config = VBK_NewConfig();

  double val = 4.06;
  VBK_SetStartOfSlope(config, val);

  VBK_FreeConfig(config);
}

void VBK_SetSlopeNormal_test() {
  Config_t* config = VBK_NewConfig();

  double val = 5.03;
  VBK_SetSlopeNormal(config, val);

  VBK_FreeConfig(config);
}

void VBK_SetSlopeKeystone_test() {
  Config_t* config = VBK_NewConfig();

  double val = 5.24;
  VBK_SetSlopeKeystone(config, val);

  VBK_FreeConfig(config);
}

void VBK_SetKeystoneRound_test() {
  Config_t* config = VBK_NewConfig();

  uint32_t val = 145;
  VBK_SetKeystoneRound(config, val);

  VBK_FreeConfig(config);
}

void VBK_SetFlatScoreRound_test() {
  Config_t* config = VBK_NewConfig();

  uint32_t val = 145;
  VBK_SetFlatScoreRound(config, val);

  VBK_FreeConfig(config);
}

void VBK_SetUseFlatScoreRound_test() {
  Config_t* config = VBK_NewConfig();

  bool val = false;
  VBK_SetUseFlatScoreRound(config, val);

  VBK_FreeConfig(config);
}

void VBK_SetMaxScoreThresholdNormal_test() {
  Config_t* config = VBK_NewConfig();

  double val = 45.23;
  VBK_SetMaxScoreThresholdNormal(config, val);

  VBK_FreeConfig(config);
}

void VBK_SetMaxScoreThresholdKeystone_test() {
  Config_t* config = VBK_NewConfig();

  double val = 45.23;
  VBK_SetMaxScoreThresholdKeystone(config, val);

  VBK_FreeConfig(config);
}

void VBK_SetDifficultyAveragingInterval_test() {
  Config_t* config = VBK_NewConfig();

  uint32_t val = 1235;
  VBK_SetDifficultyAveragingInterval(config, val);

  VBK_FreeConfig(config);
}

void VBK_SetRoundRatios_test() {
  Config_t* config = VBK_NewConfig();

  double val[] = {45.23, 36.41, 12.23};
  VBK_SetRoundRatios(config, val, 3);

  VBK_FreeConfig(config);
}

void VBK_SetPopRewardsLookupTable_test() {
  Config_t* config = VBK_NewConfig();

  double val[] = {45.23, 36.41, 12.23};
  VBK_SetPopRewardsLookupTable(config, val, 3);

  VBK_FreeConfig(config);
}

void VBK_SetMaxFutureBlockTime_test() {
  Config_t* config = VBK_NewConfig();

  uint32_t val = 4513;
  VBK_SetMaxFutureBlockTime(config, val);

  VBK_FreeConfig(config);
}

void VBK_SetKeystoneInterval_test() {
  Config_t* config = VBK_NewConfig();

  uint32_t val = 4513;
  VBK_SetKeystoneInterval(config, val);

  VBK_FreeConfig(config);
}

void VBK_SetVbkFinalityDelay_test() {
  Config_t* config = VBK_NewConfig();

  uint32_t val = 4513;
  VBK_SetVbkFinalityDelay(config, val);

  VBK_FreeConfig(config);
}

void VBK_SetEndorsementSettlementInterval_test() {
  Config_t* config = VBK_NewConfig();

  int32_t val = 4513;
  VBK_SetEndorsementSettlementInterval(config, val);

  VBK_FreeConfig(config);
}

void VBK_SetMaxPopDataSize_test() {
  Config_t* config = VBK_NewConfig();

  uint32_t val = 4513;
  VBK_SetMaxPopDataSize(config, val);

  VBK_FreeConfig(config);
}

void VBK_SetForkResolutionLookupTable_test() {
  Config_t* config = VBK_NewConfig();

  uint32_t val[] = {4513, 213, 654};
  VBK_SetForkResolutionLookupTable(config, val, 3);

  VBK_FreeConfig(config);
}

void config_test_suite() {
  VBK_SetStartOfSlope_test();
  VBK_SetSlopeNormal_test();
  VBK_SetSlopeKeystone_test();
  VBK_SetKeystoneRound_test();
  VBK_SetFlatScoreRound_test();
  VBK_SetUseFlatScoreRound_test();
  VBK_SetMaxScoreThresholdNormal_test();
  VBK_SetMaxScoreThresholdKeystone_test();
  VBK_SetDifficultyAveragingInterval_test();
  VBK_SetRoundRatios_test();
  VBK_SetPopRewardsLookupTable_test();
  VBK_SetMaxFutureBlockTime_test();
  VBK_SetKeystoneInterval_test();
  VBK_SetVbkFinalityDelay_test();
  VBK_SetEndorsementSettlementInterval_test();
  VBK_SetMaxPopDataSize_test();
  VBK_SetForkResolutionLookupTable_test();
}
