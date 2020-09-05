// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_EXTERN_HPP
#define VERIBLOCK_POP_CPP_EXTERN_HPP

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//! @returns unique id of each altchain
extern int64_t VBK_getAltchainId();
//! @returns null-terminated hexstring representing serialized AltBlock
extern const char* VBK_getBootstrapBlock();
//! @param[in] in input byte array "to be hashed"
//! @param[in] inlen input byte array length
//! @param[out] out output byte array (must be pre-allocated)
//! @param[out] output hash size
extern void VBK_getBlockHeaderHash(const uint8_t* in,
                                   int inlen,
                                   uint8_t* out,
                                   int* outlen);
#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif  // VERIBLOCK_POP_CPP_EXTERN_HPP
