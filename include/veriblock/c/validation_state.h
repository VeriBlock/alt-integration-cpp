// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VALIDATION_STATE_H
#define VERIBLOCK_POP_CPP_VALIDATION_STATE_H

#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup c-api C interface
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VbkValidationState VBK_ValidationState;

/**
 * Get a debug message as a 'C' string
 *
 * @param[in] self VbkValidationState
 * @return debug 'C' string debug message
 */
const char* VBK_ValidationState_getErrorMessage(VBK_ValidationState* self);

/**
 * deallocates resources.
 * @param[out] self VbkValidationState
 */
void VBK_ValidationState_Free(VBK_ValidationState* self);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif