// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_MEMPOOL_H
#define VERIBLOCK_POP_CPP_C_MEMPOOL_H

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/atv.h"
#include "veriblock/pop/c/entities/publication_data.h"
#include "veriblock/pop/c/entities/vbkblock.h"
#include "veriblock/pop/c/entities/vtb.h"
#include "veriblock/pop/c/pop_context2.h"

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

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif