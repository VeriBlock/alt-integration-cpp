// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_EXTERN_H
#define VERIBLOCK_POP_CPP_EXTERN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// AltChainConfig externs
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

// PayloadsProvider externs
//! @param[in] in input ATV`s id byte array
//! @param[in] inlen input ATV`s id byte array length
//! @param[out] out output ATV`s payload byte array  (must be pre-allocated)
//! @param[out] output ATV`s payload byte array len
extern void VBK_getATV(const uint8_t* id_bytes,
                       int id_size,
                       uint8_t* atv_bytes_out,
                       int* atv_bytes_len);

//! @param[in] in input VTB`s id byte array
//! @param[in] inlen input VTB`s id byte array length
//! @param[out] out output VTB`s payload byte array  (must be pre-allocated)
//! @param[out] output VTB`s payload byte array len
extern void VBK_getVTB(const uint8_t* id_bytes,
                       int id_size,
                       uint8_t* vtb_bytes_out,
                       int* vtb_bytes_len);

//! @param[in] in input VBK`s id byte array
//! @param[in] inlen input VBK`s id byte array length
//! @param[out] out output VBK`s payload byte array  (must be pre-allocated)
//! @param[out] output VBK`s payload byte array len
extern void VBK_getVBK(const uint8_t* id_bytes,
                       int id_size,
                       uint8_t* vbk_bytes_out,
                       int* vbk_bytes_len);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif  // VERIBLOCK_POP_CPP_EXTERN_HPP
