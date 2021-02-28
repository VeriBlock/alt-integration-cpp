// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VALIDATION_STATE_H
#define VERIBLOCK_POP_CPP_VALIDATION_STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup c-api C interface
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VbkValidationState VbkValidationState;

/**
 * Allocates instance of the VbkValidationState.
 *
 * @return pointer of the VbkValidationState.
 */
VbkValidationState* VBK_NewValidationState();

/**
 * Deallocates resources of the VbkValidationState.
 *
 * @param[out] self VbkValidationState.
 */
void VBK_FreeValidationState(VbkValidationState* self);

/**
 * Get a debug message as a 'C' string.
 *
 * @param[in] self VbkValidationState.
 * @return debug 'C' string debug message.
 */
const char* VBK_ValidationState_getErrorMessage(VbkValidationState* self);

/**
 * Changes this ValidationState into "INVALID" mode.
 *
 * @param[in] self VbkValidationState.
 * @param[in] reject_reason - supply a short, unique message that identifies
 * this class of validation. Example: mandatory-script-verify-flag-failed
 * @param[in] debug_message - supply arbitrary message that will help to debug
 * the error.
 * @return always returns false.
 */
bool VBK_ValidationState_Invalid(VbkValidationState* self,
                                 const char* reject_reason,
                                 const char* debug_message);

/**
 * Returns that VbkValidationState has an errors or not.
 *
 * @param[in] self VbkValidationState.
 * @return true if VbkValidationState HASN`T ERRORS, false otherwise.
 */
bool VBK_ValidationState_isValid(VbkValidationState* self);

/**
 * Returns that VbkValidationState has an errors or not.
 *
 * @param[in] self VbkValidationState.
 * @return true if VbkValidationState HAS ERRORS, false otherwise.
 */
bool VBK_ValidationState_isInvalid(VbkValidationState* self);

/**
 * Reset VbkValidationState
 *
 * @param[in] self VbkValidationState.
 */
void VBK_ValidationState_Reset(VbkValidationState* self);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif