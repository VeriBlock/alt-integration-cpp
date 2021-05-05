// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_UTILS_H
#define VERIBLOCK_POP_CPP_UTILS_H

#include <stddef.h>
#include <stdint.h>

#include "pop_context.h"
#include "validation_state.h"

#ifdef __cplusplus
extern "C" {
#endif

VBK_ByteStream* VBK_AltBlock_getEndorsedBy(PopContext* self,
                                           const uint8_t* hash,
                                           int hash_size);

/**
 * Calculates an ID of the altintegration::VbkBlock.
 *
 * @param[in] block_bytes altintegration::VbkBlock raw bytes, expect
 * ToVbkEncoding format
 * @param[in] block_bytes_size size of input hash
 * @param[out] id_bytes altintegration::VbkBlock id with
 * size of altintegration::VbkBlock::short_hash_t (must be prealocated)
 * @param[out] id_bytes_size id bytes size, equals to the
 * altintegration::VbkBlock::short_hash_t size
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
 * @param[out] id_bytes altintegration::VTB id with
 * size of altintegration::VTB::id_t (must be prealocated)
 * @param[out] id_bytes_size id bytes size, equals to the
 * altintegration::VTB::id_t size
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
 * @param[out] id_bytes altintegration::ATV id with
 * size of altintegration::ATV::id_t (must be prealocated)
 * @param[out] id_bytes_size id bytes size, equals to the
 * altintegration::ATV::id_t size
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
 * @param[out] hash_bytes altintegration::VbkBlock hash
 * with size of altintegration::VkbBlock::hash_t (must be prealocated)
 * @param[out] hash_bytes_size hash bytes size, equals to the
 * altintegration::VkbBlock::hash_t size
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
 * @param[out] hash_bytes altintegration::BtcBlock hash
 * with size of altintegration::BtcBlock::hash_t (must be prealocated)
 * @param[out] hash_bytes_size hash bytes size, equals to the
 * altintegration::BtcBlock::hash_t size
 */
void VBK_BtcBlock_getHash(const uint8_t* block_bytes,
                          int block_bytes_size,
                          uint8_t* hash_bytes,
                          int* hash_bytes_size);

/**
 * Calculate ContextInfoContainer hash for the provided altintegration::PopData
 * and previous block hash for the current block
 *
 * @param[in] self PopContext
 * @param[in] txRoot, hash of the transactions root
 * @param[in] prev_block_hash, hash of the previous altintegration::AltBlock
 * hash for the current block
 * @param[in] prev_block_hash_size, size of the previous block hash
 * @param[in] pop_data_bytes, bytes of the provided altintegration::PopData in
 * the toVbkEncoding format
 * @param[in] pop_data_bytes_size, size of the provided altintegration::PopData
 * bytes
 * @param[out] out_hash, sha256 hash of the ContextInfoContainer
 * @param[out] out_hash_size, size of the sha256 hash size
 */
void VBK_AltBlock_calculateTopLevelMerkleRoot(PopContext* self,
                                              const uint8_t txRoot[32],
                                              const uint8_t* prev_block_hash,
                                              int prev_block_hash_size,
                                              const uint8_t* pop_data_bytes,
                                              int pop_data_bytes_size,
                                              uint8_t out_hash[32]);

/**
 * Generate altintegration::PublicationData for the provided endorsed block
 *
 * @param[in] self PopContext
 * @param[in] endorsed_block_header, header bytes of the endorsed block
 * @param[in] endorsed_block_header_size, size of the provided block hash
 * @param[in] txRoot, hash of the transactions root
 * @param[in] pop_data_bytes, bytes of the provided altintegration::PopData in
 * the toVbkEncoding format
 * @param[in] payout_info, the payout info the PoP miner
 * @param[in] payout_info_size, size of the provided payout info
 * @return altintgration::PublicationData serialized in the toVbkEncoding format
 * to the stream
 */
VBK_ByteStream* VBK_AltBlock_generatePublicationData(
    PopContext* self,
    const uint8_t* endorsed_block_header,
    int endorsed_block_header_size,
    const uint8_t txRoot[32],
    const uint8_t* pop_data_bytes,
    int pop_data_bytes_size,
    const uint8_t* payout_info,
    int payout_info_size);

/**
 * Stateless validation for the altintegration::ATV.
 *
 * @param[in] self PopContext
 * @param[in] atv_bytes, bytes of the altintegration::ATV raw bytes, expect
 * ToVbkEncoding format.
 * @param[in] atv_bytes_size, size of the atv bytes.
 * @param[out] state VbkValidationState.
 * @return true if atv is valid, false otherwise.
 */
bool VBK_checkATV(PopContext* self,
                  const uint8_t* atv_bytes,
                  int atv_bytes_size,
                  VbkValidationState* state);

/**
 * Stateless validation for the altintegration::VTB.
 *
 * @param[in] self PopContext
 * @param[in] vtb_bytes, bytes of the altintegration::VTB raw bytes, expect
 * ToVbkEncoding format.
 * @param[in] vtb_bytes_size, size of the vtb bytes.
 * @param[out] state VbkValidationState.
 * @return true if vtb is valid, false otherwise.
 */
bool VBK_checkVTB(PopContext* self,
                  const uint8_t* vtb_bytes,
                  int vtb_bytes_size,
                  VbkValidationState* state);

/**
 * Stateless validation for the altintegration::VbkBlock.
 *
 * @param[in] self PopContext
 * @param[in] vbk_bytes, bytes of the altintegration::VbkBlock raw bytes, expect
 * ToVbkEncoding format.
 * @param[in] vbk_bytes_size, size of the vbk bytes.
 * @param[out] state VbkValidationState.
 * @return true if vbk is valid, false otherwise.
 */
bool VBK_checkVbkBlock(PopContext* self,
                       const uint8_t* vbk_bytes,
                       int vbk_bytes_size,
                       VbkValidationState* state);

/**
 * Stateless validation for the altintegration::PopData.
 *
 * @param[in] self PopContext
 * @param[in] pop_data_bytes, bytes of the altintegration::PopData raw bytes,
 * expect ToVbkEncoding format.
 * @param[in] pop_data_bytes_size, size of the vbk bytes.
 * @param[out] state VbkValidationState.
 * @return true if pop_data is valid, false otherwise.
 */
bool VBK_checkPopData(PopContext* self,
                      const uint8_t* pop_data_bytes,
                      int pop_data_bytes_size,
                      VbkValidationState* state);

/**
 * Save library`s trees state into the database.
 *
 * @param[in] self PopContext.
 * @param[out] state VbkValidationState.
 * @return true if successfullty saved, false otherwise.
 */
bool VBK_SaveAllTrees(PopContext* self, VbkValidationState* state);

/**
 * Load library`s trees state from the database.
 *
 * @param[in] self PopContext.
 * @param[out] state VbkValidationState.
 * @return true if successfullty loaded, false loaded.
 */
bool VBK_LoadAllTrees(PopContext* self, VbkValidationState* state);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif