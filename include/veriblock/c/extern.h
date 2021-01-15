// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_EXTERN_H
#define VERIBLOCK_POP_CPP_EXTERN_H

#include <stdbool.h>
#include <stdint.h>

#include "validation_state.h"

#ifdef __cplusplus
extern "C" {
#endif

// AltChainConfig externs
//! @returns unique id of each altchain
int64_t VBK_getAltchainId();

//! @returns null-terminated hexstring representing serialized AltBlock
const char* VBK_getBootstrapBlock();

//! @param[in] in input byte array "to be hashed"
//! @param[in] inlen input byte array length
//! @param[out] out output byte array (must be pre-allocated)
//! @param[out] output hash size
void VBK_getBlockHeaderHash(const uint8_t* in,
                            int inlen,
                            uint8_t* out,
                            int* outlen);

//! Accepts arbitrary bytes.
//! Should return 0 if input bytes are statelessly INVALID block header.
//! Should return non-0 if input bytes are statelessly VALID block header.
bool VBK_checkBlockHeader(const uint8_t* in, int inlen, const uint8_t* root, int root_size);

bool VBK_MemPool_onAcceptedATV(const uint8_t* bytes, int size);

bool VBK_MemPool_onAcceptedVTB(const uint8_t* bytes, int size);

bool VBK_MemPool_onAcceptedVBK(const uint8_t* bytes, int size);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif  // VERIBLOCK_POP_CPP_EXTERN_HPP
