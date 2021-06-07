// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_POP_CONTEXT_H
#define VERIBLOCK_POP_CPP_C_POP_CONTEXT_H

#include <stdint.h>

#include "veriblock/pop/c/config.h"
#include "veriblock/pop/c/entities/altblock.h"
#include "veriblock/pop/c/entities/block_index.h"
#include "veriblock/pop/c/entities/pop_payouts.h"
#include "veriblock/pop/c/entities/popdata.h"
#include "veriblock/pop/c/storage.h"
#include "veriblock/pop/c/type_helpers.h"
#include "veriblock/pop/c/validation_state.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(pop_context);

POP_ENTITY_NEW_FUNCTION(pop_context,
                        const POP_ENTITY_NAME(config) * config,
                        const POP_ENTITY_NAME(storage) * storage,
                        POP_ARRAY_NAME(string) log_lvl);

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
 * @copybrief altintegration::AltBlockTree::acceptBlock
 * @see altintegration::AltBlockTree::acceptBlock
 * @param[in] self PopContext
 * @param[in] hash POP_ARRAY_NAME(u8) array altintegration::AltBlock hash bytes
 * @param[in] pop_data POP_ENTITY_NAME(pop_data) pointer to the
 * altintegration::PopData
 *
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           void,
                           accept_block,
                           POP_ARRAY_NAME(u8) hash,
                           const POP_ENTITY_NAME(pop_data) * pop_data);

/**
 * @copybrief altintegration::AltBlockTree::setState
 * @see altintegration::AltBlockTree::setState
 * @param[in] self PopContext
 * @param[in] hash POP_ARRAY_NAME(u8) array altintegration::AltBlock hash bytes
 * @param[out] state POP_ENTITY_NAME(validation_state) pointer to the
 * altintegration::ValidationState
 * @return `false` if intermediate or target block is invalid. In this case
 * tree will rollback into original state. `true` if state change is
 * successful.
 * @invariant atomic - either switches to new state, or does nothing.
 * @warning Expensive operation.
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           bool,
                           set_state,
                           POP_ARRAY_NAME(u8) hash,
                           POP_ENTITY_NAME(validation_state) * state);

/**
 * @copybrief altintegration::AltBlockTree::comparePopScore
 * @see altintegration::AltBlockTree::comparePopScore
 * @param[in] self PopContext
 * @param[in] A_block_hash POP_ARRAY_NAME(u8) array altintegration::AltBlock
 * hash bytes of current tip in AltBlockTree. Fails on assert if current tip !=
 * A_block.
 * @param[in] B_block_hash POP_ARRAY_NAME(u8) array altintegration::AltBlock
 * hash bytes. Current tip will be compared against this block. Must exist on
 * chain and have BLOCK_HAS_PAYLOADS.
 * @return Returns positive if chain A is better. Returns negative if chain B is
 * better. Returns 0 if blocks are equal in terms of POP. Users should fallback
 * to chain-native Fork Resolution algorithm.
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           int,
                           compare_pop_score,
                           POP_ARRAY_NAME(u8) A_block_hash,
                           POP_ARRAY_NAME(u8) B_block_hash);

/**
 * @copybrief altintegration::AltBlockTree::getPopPayout
 * @see altintegration::AltBlockTree::getPopPayout
 *
 * Before executing this method the state should be switched to the provided
 * block, should execute set_state for the provided block
 * before executing this function.
 *
 * @param[in] self PopContext
 * @param[in] hash POP_ARRAY_NAME(u8) array altintegration::AltBlock hash bytes
 * @return POP_ARRAY_NAME(pop_payout) array of the POP_ENTITY_NAME(pop_payout)
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(pop_payout),
                           get_pop_payouts,
                           POP_ARRAY_NAME(u8) hash);

/**
 * @copybrief altintegration::AltBlockTree::removeSubtree
 * @see altintegration::AltBlockTree::removeSubtree
 * @param[in] self PopContext
 * @param[in] hash POP_ARRAY_NAME(u8) array altintegration::AltBlock hash bytes
 * @warning fails on assert if block can not be found in this tree.
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           void,
                           remove_subtree,
                           POP_ARRAY_NAME(u8) hash);

/**
 * Find a AltBlock index from the AltTree
 *
 * @param[in] self PopContext
 * @param[in] hash POP_ARRAY_NAME(u8) array altintegration::AltBlock hash bytes
 * @return POP_ENTITY_NAME(alt_block_index) pointer to the
 * altintegration::BlockIndex<altintegration::AltBlock>, if cannot find return
 * nullptr
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(alt_block_index) *,
                           alt_get_block_index,
                           POP_ARRAY_NAME(u8) hash);

/**
 * Find a VbkBlock index from the VbkTree
 *
 * @param[in] self PopContext
 * @param[in] hash POP_ARRAY_NAME(u8) array altintegration::VbkBlock hash bytes
 * @return POP_ENTITY_NAME(vbk_block_index) pointer to the
 * altintegration::BlockIndex<altintegration::VbkBlock>, if cannot find return
 * nullptr
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(vbk_block_index) *,
                           vbk_get_block_index,
                           POP_ARRAY_NAME(u8) hash);

/**
 * Find a BtcBlock index from the BtcTree
 *
 * @param[in] self PopContext
 * @param[in] hash POP_ARRAY_NAME(u8) array altintegration::BtcBlock hash bytes
 * @return POP_ENTITY_NAME(btc_block_index) pointer to the
 * altintegration::BlockIndex<altintegration::BtcBlock>, if cannot find return
 * nullptr
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(btc_block_index) *,
                           btc_get_block_index,
                           POP_ARRAY_NAME(u8) hash);

/**
 * Return best block (tip) of the AltTree
 *
 * @param[in] self PopContext
 * @return POP_ENTITY_NAME(alt_block_index) pointer to the
 * altintegration::BlockIndex<altintegration::AltBlock>
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(alt_block_index) *,
                           alt_get_best_block);

/**
 * Return best block (tip) of the VbkTree
 *
 * @param[in] self PopContext
 * @return POP_ENTITY_NAME(vbk_block_index) pointer to the
 * altintegration::BlockIndex<altintegration::VbkBlock>
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(vbk_block_index) *,
                           vbk_get_best_block);

/**
 * Return best block (tip) of the BtcTree
 *
 * @param[in] self PopContext
 * @return POP_ENTITY_NAME(btc_block_index) pointer to the
 * altintegration::BlockIndex<altintegration::BtcBlock>
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(btc_block_index) *,
                           btc_get_best_block);

/**
 * Return bootsrap block (first) of the AltTree
 * @param[in] self PopContext
 * @return POP_ENTITY_NAME(alt_block_index) pointer to the
 * altintegration::BlockIndex<altintegration::AltBlock>
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(alt_block_index) *,
                           alt_get_bootstrap_block);

/**
 * Return bootsrap block (first) of the VbkTree
 * @param[in] self PopContext
 * @return POP_ENTITY_NAME(vbk_block_index) pointer to the
 * altintegration::BlockIndex<altintegration::VbkBlock>
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(vbk_block_index) *,
                           vbk_get_bootstrap_block);

/**
 * Return bootsrap block (first) of the BtcTree
 * @param[in] self PopContext
 * @return POP_ENTITY_NAME(btc_block_index) pointer to the
 * altintegration::BlockIndex<altintegration::BtcBlock>
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(btc_block_index) *,
                           btc_get_bootstrap_block);

/**
 * Return block on the curent height from the active chain of the AltTree
 *
 * @param[in] self PopContext
 * @param[in] height uint32_t
 * @return POP_ENTITY_NAME(alt_block_index) pointer to the
 * altintegration::BlockIndex<altintegration::AltBlock>, if cannot find return
 * nullptr
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(alt_block_index) *,
                           alt_get_block_at_active_chain,
                           uint32_t height);

/**
 * Return block on the curent height from the active chain of the VbkTree
 *
 * @param[in] self PopContext
 * @param[in] height uint32_t
 * @return POP_ENTITY_NAME(vbk_block_index) pointer to the
 * altintegration::BlockIndex<altintegration::VbkBlock>, if cannot find return
 * nullptr
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(vbk_block_index) *,
                           vbk_get_block_at_active_chain,
                           uint32_t height);

/**
 * Return block on the curent height from the active chain of the BtcTree
 *
 * @param[in] self PopContext
 * @param[in] height uint32_t
 * @return POP_ENTITY_NAME(btc_block_index) pointer to the
 * altintegration::BlockIndex<altintegration::BtcBlock>, if cannot find return
 * nullptr
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(btc_block_index) *,
                           btc_get_block_at_active_chain,
                           uint32_t height);

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(array_u8),
                           get_payload_containing_blocks,
                           POP_ARRAY_NAME(u8) id);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif