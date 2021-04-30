// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_CONFIG_H
#define VERIBLOCK_POP_CPP_C_CONFIG_H

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/btcblock.h"
#include "veriblock/pop/c/entities/vbkblock.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(config);

POP_ENTITY_NEW_FUNCTION(config);

//! set VBK parameters and store them in config
POP_ENTITY_CUSTOM_FUNCTION(config,
                           void,
                           select_vbk_params,
                           // "main\0", "test\0", "regtest\0", "alpha\0"
                           POP_ARRAY_NAME(string) net,
                           int start_height,
                           // example: hex,hex,hex
                           POP_ARRAY_NAME(string) blocks);

//! set BTC parameters and store them in config
POP_ENTITY_CUSTOM_FUNCTION(config,
                           void,
                           select_btc_params,
                           // "main\0", "test\0", "regtest\0", "alpha\0"
                           POP_ARRAY_NAME(string) net,
                           int start_height,
                           // example: hex,hex,hex
                           POP_ARRAY_NAME(string) blocks);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif