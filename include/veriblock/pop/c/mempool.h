// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_MEMPOOL_H
#define VERIBLOCK_POP_CPP_C_MEMPOOL_H

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/atv.h"
#include "veriblock/pop/c/entities/popdata.h"
#include "veriblock/pop/c/entities/publication_data.h"
#include "veriblock/pop/c/entities/vbkblock.h"
#include "veriblock/pop/c/entities/vtb.h"
#include "veriblock/pop/c/pop_context.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @copybrief altintegration::MemPool::submit
 * @see altintegration::MemPool::submit
 * @param[in] self PopContext
 * @param[in] atv POP_ENTITY_NAME(atv) pointer to the altintegration::ATV
 * @param[out] state POP_ENTITY_NAME(validation_state) pointer to the
 * altintegration::ValidationState
 * @return 0 if payload is valid, 1 if statefully invalid, 2 if statelessly
 * invalid
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           int,
                           mempool_submit_atv,
                           const POP_ENTITY_NAME(atv) * atv,
                           POP_ENTITY_NAME(validation_state) * state);

/**
 * @copybrief altintegration::MemPool::submit
 * @see altintegration::MemPool::submit
 * @param[in] self PopContext
 * @param[in] vtb POP_ENTITY_NAME(vtb) pointer to the altintegration::VTB
 * @param[out] state POP_ENTITY_NAME(validation_state) pointer to the
 * altintegration::ValidationState
 * @return 0 if payload is valid, 1 if statefully invalid, 2 if statelessly
 * invalid
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           int,
                           mempool_submit_vtb,
                           const POP_ENTITY_NAME(vtb) * vtb,
                           POP_ENTITY_NAME(validation_state) * state);

/**
 * @copybrief altintegration::MemPool::submit
 * @see altintegration::MemPool::submit
 * @param[in] self PopContext
 * @param[in] vbk_block POP_ENTITY_NAME(vbk_block) pointer to the
 * altintegration::VbkBlock
 * @param[out] state POP_ENTITY_NAME(validation_state) pointer to the
 * altintegration::ValidationState
 * @return 0 if payload is valid, 1 if statefully invalid, 2 if statelessly
 * invalid
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           int,
                           mempool_submit_vbk,
                           const POP_ENTITY_NAME(vbk_block) * vbk_block,
                           POP_ENTITY_NAME(validation_state) * state);

/**
 * @copybrief altintegration::MemPool::generatePopData
 * @see altintegration::MemPool::generatePopData
 * @param[in] self PopContext
 * @return POP_ENTITY_NAME(pop_data) pointer to the altintegration::PopData
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(pop_data) *,
                           mempool_generate_pop_data);

/**
 * @copybrief return altintegration::MemPool known altintegration::ATV by its id
 *
 * @see altintegration::MemPool::get
 * @param[in] self PopContext
 * @param[in] id POP_ARRAY_NAME(u8)
 * @return POP_ENTITY_NAME(atv) pointer to altintegration::ATV
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(atv) *,
                           mempool_get_atv,
                           POP_ARRAY_NAME(u8) id);

/**
 * @copybrief return altintegration::MemPool known VTB`s
 *
 * @see altintegration::MemPool::get
 * @param[in] self PopContext
 * @param[in] id POP_ARRAY_NAME(u8)
 * @return POP_ENTITY_NAME(vtb) pointer to altintegration::VTB
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(vtb) *,
                           mempool_get_vtb,
                           POP_ARRAY_NAME(u8) id);

/**
 * @copybrief return altintegration::MemPool known VbkBlock`s
 *
 * @see altintegration::MemPool::get
 * @param[in] self PopContext
 * @param[in] id POP_ARRAY_NAME(u8)
 * @return POP_ENTITY_NAME(vbk_block) pointer to altintegration::VbkBlock
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(vbk_block) *,
                           mempool_get_vbk_block,
                           POP_ARRAY_NAME(u8) id);

/**
 * @copybrief return altintegration::MemPool known ATV`s
 *
 * This method returns a vector of the payload`s.
 *
 * @see altintegration::MemPool::getMap
 * @param[in] self PopContext
 * @return POP_ARRAY_NAME(atv) array with the ATV`s
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context, POP_ARRAY_NAME(atv), mempool_get_atvs);

/**
 * @copybrief return altintegration::MemPool known VTB`s
 *
 * This method returns a vector of the payload`s.
 *
 * @see altintegration::MemPool::getMap
 * @param[in] self PopContext
 * @return POP_ARRAY_NAME(vtb) array with the VTB`s
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context, POP_ARRAY_NAME(vtb), mempool_get_vtbs);

/**
 * @copybrief return altintegration::MemPool known VbkBlock`s
 *
 * This method returns a vector of the payload`s.
 *
 * @see altintegration::MemPool::getMap
 * @param[in] self PopContext
 * @return POP_ARRAY_NAME(vbk_block) array with the VbkBlock`s
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(vbk_block),
                           mempool_get_vbk_blocks);

/**
 * @copybrief return altintegration::MemPool known ATV`s inFlight
 *
 * This method returns a vector of the payload`s.
 *
 * @see altintegration::MemPool::getInFlightMap
 * @param[in] self PopContext
 * @return POP_ARRAY_NAME(atv) array with the ATV`s
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(atv),
                           mempool_get_in_flight_atvs);

/**
 * @copybrief return altintegration::MemPool known VTB`s inFlight
 *
 * This method returns a vector of the payload`s.
 *
 * @see altintegration::MemPool::getInFlightMap
 * @param[in] self PopContext
 * @return POP_ARRAY_NAME(vtb) array with the VTB`s
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(vtb),
                           mempool_get_in_flight_vtbs);

/**
 * @copybrief return altintegration::MemPool known VbkBlock`s inFlight
 *
 * This method returns a vector of the payload`s.
 *
 * @see altintegration::MemPool::getInFlightMap
 * @param[in] self PopContext
 * @return POP_ARRAY_NAME(vbk_block) array with the VbkBlock`s
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(vbk_block),
                           mempool_get_in_flight_vbk_blocks);

/**
 * @copybrief return altintegration::MemPool known VbkBlock`s inFlight
 *
 * This method returns a vector altintegration::BtcBlock hashes of the missing
 * blocks.
 *
 * @see altintegration::MemPool::getMissingBtcBlocks
 * @param[in] self PopContext
 * @return POP_ARRAY_NAME(array_u8) array of the altintegration::BtcBlock hashes
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(array_u8),
                           mempool_get_missing_btc_blocks);

/**
 * @copybrief altintegration::MemPool::removeAll
 * @see altintegration::MemPool::removeAll
 * @param[in] self PopContext
 * @param[in] pop_data POP_ENTITY_NAME(pop_data) pointer to the
 * altintegration::PopData
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           void,
                           mempool_remove_all,
                           const POP_ENTITY_NAME(pop_data) * pop_data);

/**
 * @copybrief altintegration::MemPool::cleanUp
 * @see altintegration::MemPool::cleanUp
 * @param[in] self PopContext
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context, void, mempool_clean_up);

/**
 * @copybrief altintegration::MemPool::clear
 * @see altintegration::MemPool::clear
 * @param[in] self PopContext
 */
POP_ENTITY_CUSTOM_FUNCTION(pop_context, void, mempool_clear);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif
