// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_UTILS_H
#define VERIBLOCK_POP_CPP_C_UTILS_H

#include <stdbool.h>

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/altblock.h"
#include "veriblock/pop/c/entities/popdata.h"
#include "veriblock/pop/c/entities/publication_data.h"
#include "veriblock/pop/c/pop_context.h"
#include "veriblock/pop/c/validation_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generate altintegration::PublicationData for the provided endorsed
 * altintegration::AltBlock
 *
 * @param[in] self PopContext
 * @param[in] endorsed_block_header bytes
 * @param[in] tx_root, hash of the transactions root
 * @param[in] pop_data POP_ENTITY_NAME(pop_data) pointer to the
 * altintegration::PopData
 * @param[in] tx_root, the payout info the PoP miner
 * @return POP_ENTITY_NAME(publication_data)  pointer to the
 * altintgration::PublicationData
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(publication_data) *,
                           generate_publication_data,
                           POP_ARRAY_NAME(u8) endorsed_block_header,
                           POP_ARRAY_NAME(u8) tx_root,
                           POP_ARRAY_NAME(u8) payout_info,
                           const POP_ENTITY_NAME(pop_data) * pop_data);

/**
 * Calculate AuthenticatedContextInfoContainer hash of the top level merkle root
 * for the provided altintegration::PopData and previous block for the current
 * block
 *
 * @param[in] self PopContext
 * @param[in] tx_root, hash of the transactions root
 * @param[in] prev_block_hash POP_ARRAY_NAME(u8) hash of the
 * altintegration::AltBlock
 * @param[in] pop_data POP_ENTITY_NAME(pop_data) pointer to the
 * altintegration::PopData
 * @return POP_ARRAY_NAME(u8) hash of the top level merkle root
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(u8),
                           calculate_top_level_merkle_root,
                           POP_ARRAY_NAME(u8) tx_root,
                           POP_ARRAY_NAME(u8) prev_block_hash,
                           const POP_ENTITY_NAME(pop_data) * pop_data);

/**
 * Save library`s trees state into the database.
 *
 * @param[in] self PopContext.
 * @param[out] state POP_ENTITY_NAME(validation_state) pointer to the
 * altintegration::ValidationState
 * @return true if successfullty saved, false otherwise.
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           bool,
                           save_all_trees,
                           POP_ENTITY_NAME(validation_state) * state);

/**
 * Load library`s trees state from the database.
 *
 * @param[in] self PopContext.
 * @param[out] state POP_ENTITY_NAME(validation_state) pointer to the
 * altintegration::ValidationState
 * @return true if successfullty loaded, false loaded.
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           bool,
                           load_all_trees,
                           POP_ENTITY_NAME(validation_state) * state);

/**
 * Stateless validation for the altintegration::ATV.
 *
 * @param[in] self PopContext
 * @param[in] atv POP_ENTITY_NAME(atv) pointer to the altintegration::ATV
 * @param[out] state POP_ENTITY_NAME(validation_state) pointer to the
 * altintegration::ValidationState
 * @return true if ATV is valid, false otherwise.
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           bool,
                           check_atv,
                           const POP_ENTITY_NAME(atv) * atv,
                           POP_ENTITY_NAME(validation_state) * state);

/**
 * Stateless validation for the altintegration::VTB.
 *
 * @param[in] self PopContext
 * @param[in] vtb POP_ENTITY_NAME(vtb) pointer to the altintegration::VTB
 * @param[out] state POP_ENTITY_NAME(validation_state) pointer to the
 * altintegration::ValidationState
 * @return true if VTB is valid, false otherwise.
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           bool,
                           check_vtb,
                           const POP_ENTITY_NAME(vtb) * vtb,
                           POP_ENTITY_NAME(validation_state) * state);

/**
 * Stateless validation for the altintegration::VbkBlock.
 *
 * @param[in] self PopContext
 * @param[in] vbk_block POP_ENTITY_NAME(vbk_block) pointer to the
 * altintegration::VbkBlock
 * @param[out] state POP_ENTITY_NAME(validation_state) pointer to the
 * altintegration::ValidationState
 * @return true if VbkBlock is valid, false otherwise.
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           bool,
                           check_vbk_block,
                           const POP_ENTITY_NAME(vbk_block) * vbk_block,
                           POP_ENTITY_NAME(validation_state) * state);

/**
 * Stateless validation for the altintegration::PopData.
 *
 * @param[in] self PopContext
 * @param[in] vbk_block POP_ENTITY_NAME(pop_data) pointer to the
 * altintegration::PopData
 * @param[out] state POP_ENTITY_NAME(validation_state) pointer to the
 * altintegration::ValidationState
 * @return true if PopData is valid, false otherwise.
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           bool,
                           check_pop_data,
                           const POP_ENTITY_NAME(pop_data) * pop_data,
                           POP_ENTITY_NAME(validation_state) * state);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif