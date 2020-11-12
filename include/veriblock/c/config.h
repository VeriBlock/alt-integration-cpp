// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_CONFIG_H
#define VERIBLOCK_POP_CPP_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Config Config_t;

//! allocate new config
Config_t* VBK_NewConfig();
//! deallocate config
void VBK_FreeConfig(Config_t* config);

//! set VBK parameters and store them in config
bool VBK_SelectVbkParams(Config_t* config,
                         // "main\0", "test\0", "regtest\0", "alpha\0"
                         const char* net,
                         int startHeight,
                         // example: hex,hex,hex
                         const char* blocks);

//! set BTC parameters and store them in config
bool VBK_SelectBtcParams(Config_t* config,
                         // "main\0", "test\0", "regtest\0"
                         const char* net,
                         int startHeight,
                         // example: hex,hex,hex
                         const char* blocks);

void VBK_SetStartOfSlope(Config_t* params, double val);
void VBK_SetSlopeNormal(Config_t* params, double val);
void VBK_SetSlopeKeystone(Config_t* params, double val);
void VBK_SetKeystoneRound(Config_t* params, uint32_t val);
void VBK_SetFlatScoreRound(Config_t* params, uint32_t val);
void VBK_SetUseFlatScoreRound(Config_t* params, bool val);
void VBK_SetMaxScoreThresholdNormal(Config_t* params, double val);
void VBK_SetMaxScoreThresholdKeystone(Config_t* params, double val);
void VBK_SetDifficultyAveragingInterval(Config_t* params, uint32_t val);
void VBK_SetRoundRatios(Config_t* params, const double* vals, int valslen);
void VBK_SetPopRewardsLookupTable(Config_t* params,
                                  const double* vals,
                                  int valslen);
void VBK_SetMaxFutureBlockTime(Config_t* params, uint32_t val);
void VBK_SetKeystoneInterval(Config_t* params, uint32_t val);
void VBK_SetVbkFinalityDelay(Config_t* params, uint32_t val);
void VBK_SetEndorsementSettlementInterval(Config_t* params, uint32_t val);
void VBK_SetMaxPopDataSize(Config_t* params, uint32_t val);
uint32_t VBK_GetMaxPopDataSize(Config_t* params);
void VBK_SetForkResolutionLookupTable(Config_t* params,
                                      const uint32_t* vals,
                                      int valslen);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif  // VERIBLOCK_POP_CPP_CONFIG_H
