// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_POP_CONTEXT_H
#define VERIBLOCK_POP_CPP_C_POP_CONTEXT_H

#include "veriblock/pop/c/config2.h"
#include "veriblock/pop/c/entities/altblock.h"
#include "veriblock/pop/c/storage2.h"
#include "veriblock/pop/c/type_helpers.h"
#include "veriblock/pop/c/validation_state2.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(pop_context);

POP_ENTITY_NEW_FUNCTION(pop_context,
                        const POP_ENTITY_NAME(config) * config,
                        const POP_ENTITY_NAME(storage) * storage,
                        const char* log_lvl);

/**
 * @copybrief altintegration::AltBlockTree::acceptBlockHeader
 * @see altintegration::AltBlockTree::acceptBlockHeader
 * @param[in] self PopContext
 * @param[in] block POP_ENTITY_NAME(alt_block) pointer to the
 * altintegration::AltBlock
 * @param[out] state POP_ENTITY_NAME(validation_state) pointer to the
 * altintegration::ValidationState
 * @return true if block is valid, and added; false otherwise.
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           bool,
                           accept_block_header,
                           const POP_ENTITY_NAME(alt_block) * block,
                           POP_ENTITY_NAME(validation_state) * state);

/**
 * @copybrief altintegration::AltBlockTree::comparePopScore
 * @see altintegration::AltBlockTree::comparePopScore
 * @param[in] self PopContext
 * @param[in] A_block POP_ENTITY_NAME(alt_block) pointer to the
 * altintegration::AltBlock of current tip in AltBlockTree. Fails on assert if
 * current tip != A_block.
 * @param[in] B_block POP_ENTITY_NAME(alt_block) pointer to the
 * altintegration::AltBlock. Current tip will be compared against this block.
 * Must exist on chain and have BLOCK_HAS_PAYLOADS.
 * @return Returns positive if chain A is better. Returns negative if chain B is
 * better. Returns 0 if blocks are equal in terms of POP. Users should fallback
 * to chain-native Fork Resolution algorithm.
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           int,
                           compare_pop_score,
                           const POP_ENTITY_NAME(alt_block) * A_block,
                           const POP_ENTITY_NAME(alt_block) * B_block);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif