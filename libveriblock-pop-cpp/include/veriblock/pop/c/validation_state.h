// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VALIDATION_STATE2_H
#define VERIBLOCK_POP_CPP_VALIDATION_STATE2_H

#include <stdbool.h>

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/serde.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(validation_state);

POP_ENTITY_NEW_FUNCTION(validation_state);

/**
 * Get a debug message.
 *
 * @param[in] self ValidationState.
 * @return debug string debug message.
 */
POP_ENTITY_GETTER_FUNCTION(validation_state,
                           POP_ARRAY_NAME(string),
                           error_message);

/**
 * Returns that ValidationState has an errors or not.
 *
 * @param[in] self ValidationState.
 * @return true if ValidationState HASN`T ERRORS, false otherwise.
 */
POP_ENTITY_GETTER_FUNCTION(validation_state, bool, is_valid);

/**
 * Returns that ValidationState has an errors or not.
 *
 * @param[in] self ValidationState.
 * @return true if ValidationState HAS ERRORS, false otherwise.
 */
POP_ENTITY_GETTER_FUNCTION(validation_state, bool, is_invalid);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif