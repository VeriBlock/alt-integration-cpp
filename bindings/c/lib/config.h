// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_CONFIG_H
#define VERIBLOCK_POP_CPP_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Config Config_t;
typedef struct AltParams AltParams_t;

//! allocate new config
extern Config_t* VBK_NewConfig();
//! deallocate config
extern void VBK_FreeConfig(Config_t* config);
//! allocate new alt params
extern AltParams_t* VBK_NewAltParams();
//! deallocate alt params
extern void VBK_FreeAltParams(AltParams_t* params);

extern void VBK_SetAltParams(Config_t* config, AltParams_t* params);

//! set VBK parameters and store them in config
extern bool VBK_SelectVbkParams(Config_t* config,
                                // "main\0", "test\0", "regtest\0", "alpha\0"
                                const char* net,
                                int startHeight,
                                // example: hex,hex,hex
                                const char* blocks);

//! set BTC parameters and store them in config
extern bool VBK_SelectBtcParams(Config_t* config,
                                // "main\0", "test\0", "regtest\0"
                                const char* net,
                                int startHeight,
                                // example: hex,hex,hex
                                const char* blocks);

extern void VBK_SetStartOfSlope(AltParams_t* params, double val);
extern void VBK_SetSlopeNormal(AltParams_t* params, double val);
extern void VBK_SetSlopeKeystone(AltParams_t* params, double val);
extern void VBK_SetKeystoneRound(AltParams_t* params, uint32_t val);
extern void VBK_SetFlatScoreRound(AltParams_t* params, uint32_t val);
extern void VBK_SetUseFlatScoreRound(AltParams_t* params, bool val);
extern void VBK_SetMaxScoreThresholdNormal(AltParams_t* params, double val);
extern void VBK_SetMaxScoreThresholdKeystone(AltParams_t* params, double val);
extern void VBK_SetDifficultyAveragingInterval(AltParams_t* params,
                                               uint32_t val);
extern void VBK_SetRoundRatios(AltParams_t* params,
                               const double* vals,
                               int valslen);
extern void VBK_SetPopRewardsLookupTable(AltParams_t* params,
                                         const double* vals,
                                         int valslen);
extern void VBK_SetMaxFutureBlockTime(AltParams_t* params, uint32_t val);
extern void VBK_SetKeystoneInterval(AltParams_t* params, uint32_t val);
extern void VBK_SetVbkFinalityDelay(AltParams_t* params, uint32_t val);
extern void VBK_SetEndorsementSettlementInterval(AltParams_t* params,
                                                 uint32_t val);
extern void VBK_SetMaxPopDataSize(AltParams_t* params, uint32_t val);
extern void VBK_SetForkResolutionLookupTable(AltParams_t* params,
                                             const double* vals,
                                             int valslen);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif  // VERIBLOCK_POP_CPP_CONFIG_H
