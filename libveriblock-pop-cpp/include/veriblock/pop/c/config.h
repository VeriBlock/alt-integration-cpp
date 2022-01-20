// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_CONFIG_H
#define VERIBLOCK_POP_CPP_C_CONFIG_H

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/altblock.h"
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

POP_ENTITY_SETTER_FUNCTION(config, double, start_of_slope);
POP_ENTITY_SETTER_FUNCTION(config, double, slope_normal);
POP_ENTITY_SETTER_FUNCTION(config, double, slope_keystone);
POP_ENTITY_SETTER_FUNCTION(config, uint32_t, keystone_round);
POP_ENTITY_SETTER_FUNCTION(config, uint32_t, flat_score_round);
POP_ENTITY_SETTER_FUNCTION(config, bool, use_flat_score_round);
POP_ENTITY_SETTER_FUNCTION(config, double, max_score_threshold_normal);
POP_ENTITY_SETTER_FUNCTION(config, double, max_score_threshold_keystone);
POP_ENTITY_SETTER_FUNCTION(config, uint32_t, difficulty_averaging_interval);
POP_ENTITY_SETTER_FUNCTION(config, POP_ARRAY_NAME(double), round_ratios);
POP_ENTITY_SETTER_FUNCTION(config,
                           POP_ARRAY_NAME(double),
                           pop_rewards_lookup_table);
POP_ENTITY_SETTER_FUNCTION(config, uint32_t, max_future_block_time);
POP_ENTITY_SETTER_FUNCTION(config, uint32_t, keystone_interval);
POP_ENTITY_SETTER_FUNCTION(config, uint32_t, vbk_finality_delay);
POP_ENTITY_SETTER_FUNCTION(config, uint32_t, endorsement_settlement_interval);
POP_ENTITY_SETTER_FUNCTION(config, uint32_t, max_pop_data_size);
POP_ENTITY_SETTER_FUNCTION(config,
                           POP_ARRAY_NAME(u32),
                           fork_resolution_lookup_table);
POP_ENTITY_SETTER_FUNCTION(config, uint32_t, pop_payout_delay);
POP_ENTITY_SETTER_FUNCTION(config, uint32_t, max_reorg_distance);

POP_ENTITY_GETTER_FUNCTION(config, double, start_of_slope);
POP_ENTITY_GETTER_FUNCTION(config, double, slope_normal);
POP_ENTITY_GETTER_FUNCTION(config, double, slope_keystone);
POP_ENTITY_GETTER_FUNCTION(config, uint32_t, keystone_round);
POP_ENTITY_GETTER_FUNCTION(config, uint32_t, flat_score_round);
POP_ENTITY_GETTER_FUNCTION(config, bool, use_flat_score_round);
POP_ENTITY_GETTER_FUNCTION(config, double, max_score_threshold_normal);
POP_ENTITY_GETTER_FUNCTION(config, double, max_score_threshold_keystone);
POP_ENTITY_GETTER_FUNCTION(config, uint32_t, difficulty_averaging_interval);
POP_ENTITY_GETTER_FUNCTION(config, POP_ARRAY_NAME(double), round_ratios);
POP_ENTITY_GETTER_FUNCTION(config,
                           POP_ARRAY_NAME(double),
                           pop_rewards_lookup_table);
POP_ENTITY_GETTER_FUNCTION(config, uint32_t, max_future_block_time);
POP_ENTITY_GETTER_FUNCTION(config, uint32_t, keystone_interval);
POP_ENTITY_GETTER_FUNCTION(config, uint32_t, vbk_finality_delay);
POP_ENTITY_GETTER_FUNCTION(config, uint32_t, endorsement_settlement_interval);
POP_ENTITY_GETTER_FUNCTION(config, uint32_t, max_pop_data_size);
POP_ENTITY_GETTER_FUNCTION(config,
                           POP_ARRAY_NAME(u32),
                           fork_resolution_lookup_table);
POP_ENTITY_GETTER_FUNCTION(config, uint32_t, pop_payout_delay);
POP_ENTITY_GETTER_FUNCTION(config, uint32_t, max_reorg_distance);
POP_ENTITY_GETTER_FUNCTION(config, uint32_t, max_vbk_blocks_in_alt_block);
POP_ENTITY_GETTER_FUNCTION(config, uint32_t, max_vtbs_in_alt_block);
POP_ENTITY_GETTER_FUNCTION(config, uint32_t, max_atvs_in_alt_block);
POP_ENTITY_GETTER_FUNCTION(config, uint32_t, finality_delay);
POP_ENTITY_GETTER_FUNCTION(config, uint32_t, max_altchain_future_block_time);
POP_ENTITY_GETTER_FUNCTION(config,
                           POP_ENTITY_NAME(alt_block) *,
                           alt_bootstrap_block);
POP_ENTITY_GETTER_FUNCTION(config, POP_ARRAY_NAME(string), vbk_network_name);
POP_ENTITY_GETTER_FUNCTION(config, POP_ARRAY_NAME(string), btc_network_name);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif