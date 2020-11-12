// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ENTITIES_UTILS_H
#define VERIBLOCK_POP_CPP_ENTITIES_UTILS_H

/**
 * @defgroup c-api C interface
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Calculates an ID of the altintegration::VbkBlock.
 *
 * @param[in] block_bytes altintegration::VbkBlock raw bytes, expect
 * ToVbkEncoding format
 * @param[in] block_bytes_size size of input hash
 * @param[out] id_bytes altintegration::VbkBlock id, should be prealocated, with
 * size of altintegration::VbkBlock::short_hash_t
 * @param[out] id_bytes_size id bytes size, equals to the
 * altintegration::VbkBlock::short_hash_t size
 * @ingroup c-api
 */
void VBK_VbkBlock_getId(const uint8_t* block_bytes,
                        int block_bytes_size,
                        uint8_t* id_bytes,
                        int* id_bytes_size);

/**
 * Calculates an ID of the altintegration::VTB.
 *
 * @param[in] vtb_bytes altintegration::VTB raw bytes, expect ToVbkEncoding
 * format
 * @param[in] vtb_bytes_size size of input hash
 * @param[out] id_bytes altintegration::VTB id, should be prealocated, with
 * size of altintegration::VTB::id_t
 * @param[out] id_bytes_size id bytes size, equals to the
 * altintegration::VTB::id_t size
 * @ingroup c-api
 */
void VBK_VTB_getId(const uint8_t* vtb_bytes,
                   int vtb_bytes_size,
                   uint8_t* id_bytes,
                   int* id_bytes_size);

/**
 * Calculates an ID of the altintegration::ATV.
 *
 * @param[in] atv_bytes altintegration::ATV raw bytes, expect ToVbkEncoding
 * format
 * @param[in] atv_bytes_size size of input hash
 * @param[out] id_bytes altintegration::ATV id, should be prealocated, with
 * size of altintegration::ATV::id_t
 * @param[out] id_bytes_size id bytes size, equals to the
 * altintegration::ATV::id_t size
 * @ingroup c-api
 */
void VBK_ATV_getId(const uint8_t* atv_bytes,
                   int atv_bytes_size,
                   uint8_t* id_bytes,
                   int* id_bytes_size);

/**
 * Calculates an hash of the altintegration::VbkBlock.
 *
 * @param[in] block_bytes altintegration::VbkBlock raw bytes, expect
 * ToVbkEncoding format
 * @param[in] block_bytes_size size of input hash
 * @param[out] hash_bytes altintegration::VbkBlock hash, should be prealocated,
 * with size of altintegration::VkbBlock::hash_t
 * @param[out] hash_bytes_size hash bytes size, equals to the
 * altintegration::VkbBlock::hash_t size
 * @ingroup c-api
 */
void VBK_VbkBlock_getHash(const uint8_t* block_bytes,
                          int block_bytes_size,
                          uint8_t* hash_bytes,
                          int* hash_bytes_size);

/**
 * Calculates an hash of the altintegration::BtcBlock.
 *
 * @param[in] block_bytes altintegration::BtcBlock raw bytes, expect
 * ToVbkEncoding format
 * @param[in] block_bytes_size size of input hash
 * @param[out] hash_bytes altintegration::BtcBlock hash, should be prealocated,
 * with size of altintegration::BtcBlock::hash_t
 * @param[out] hash_bytes_size hash bytes size, equals to the
 * altintegration::BtcBlock::hash_t size
 * @ingroup c-api
 */
void VBK_BtcBlock_getHash(const uint8_t* block_bytes,
                          int block_bytes_size,
                          uint8_t* hash_bytes,
                          int* hash_bytes_size);

typedef struct VbkByteStream VbkByteStream;

/**
 * Read from VbkByteStream
 * @param[in] stream input byte stream
 * @param[out] buffer output buffer
 * @param[in] toRead number of bytes to read
 * @return number of bytes actually read.
 */
size_t VbkByteStream_Read(VbkByteStream* stream,
                          uint8_t* buffer,
                          size_t toRead);

/**
 * Closes stream and deallocates resources.
 * @param[out] stream
 */
void VbkByteStream_Free(VbkByteStream* stream);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif