// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_UTILS_H
#define VERIBLOCK_POP_CPP_C_UTILS_H

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/altblock.h"
#include "veriblock/pop/c/entities/popdata.h"
#include "veriblock/pop/c/entities/publication_data.h"
#include "veriblock/pop/c/pop_context2.h"

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

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif