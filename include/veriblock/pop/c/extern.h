// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_EXTERN_H
#define VERIBLOCK_POP_CPP_C_EXTERN_H

#include <stdbool.h>
#include <stdint.h>

#include "veriblock/pop/c/array.h"
#include "veriblock/pop/c/entities/altblock.h"
#include "veriblock/pop/c/type_helpers.h"
#include "veriblock/pop/c/validation_state.h"

#ifdef __cplusplus
extern "C" {
#endif

// AltChain externs which should be defined
//! @returns unique id of each altchain
POP_DECLARE_EXTERN_FUNCTION(get_altchain_id, int64_t);

//! @returns null-terminated hexstring representing serialized
//! altintegration::AltBlock
POP_DECLARE_EXTERN_FUNCTION(get_bootstrap_block, POP_ENTITY_NAME(alt_block) *);

//! @param[in] in input byte array "to be hashed"
POP_DECLARE_EXTERN_FUNCTION(get_block_header_hash,
                            POP_ARRAY_NAME(u8),
                            POP_ARRAY_NAME(u8) bytes);

//! Should return 0 if input bytes are statelessly INVALID block header.
//! Should return non-0 if input bytes are statelessly VALID block header.
POP_DECLARE_EXTERN_FUNCTION(check_block_header,
                            bool,
                            POP_ARRAY_NAME(u8) header_bytes,
                            POP_ARRAY_NAME(u8) root_bytes);

//! Log message.
//! @param[in] log_level, log level altintegration::LogLevel
//! @param[in] msg, log message.
POP_DECLARE_EXTERN_FUNCTION(log,
                            void,
                            POP_ARRAY_NAME(string) log_lvl,
                            POP_ARRAY_NAME(string) msg);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif
