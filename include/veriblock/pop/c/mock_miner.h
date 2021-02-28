// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_MOCK_MINER_H
#define VERIBLOCK_POP_CPP_MOCK_MINER_H

#include "bytestream.h"
#include "validation_state.h"

/**
 * @defgroup c-api C interface
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MockMiner MockMiner_t;

//! allocate new mock miner
MockMiner_t* VBK_NewMockMiner();
//! deallocate mock miner
void VBK_FreeMockMiner(MockMiner_t* miner);

/**
 * Mine new altintegration::BtcBlock on the top of the current btctree.
 *
 * @param[in] self MockMiner.
 * @return VBK_ByteStream* stream that stores.
 * altintegration::BlockIndex<altintegration::BtcBlock> raw bytes.
 * @ingroup c-api
 */
VBK_ByteStream* VBK_MockMiner_mineBtcBlockTip(MockMiner_t* self);

/**
 * Mine new altintegration::BtcBlock on the top of the provided block.
 *
 * @param[in] self MockMiner.
 * @param[in] block_hash hash of the previous altintegration::BtcBlock.
 * @param[in] block_hash_size size of the input block hash.
 * @return VBK_ByteStream* stream that stores
 * altintegration::BlockIndex<altintegration::BtcBlock> raw bytes.
 * @ingroup c-api
 */
VBK_ByteStream* VBK_MockMiner_mineBtcBlock(MockMiner_t* self,
                                           const uint8_t* block_hash,
                                           int block_hash_size);

/**
 * Mine new altintegration::VbkBlock on the top of the current vbktree.
 *
 * @param[in] self MockMiner.
 * @return VBK_ByteStream* stream that stores.
 * altintegration::BlockIndex<altintegration::VbkBlock> raw bytes.
 * @ingroup c-api
 */
VBK_ByteStream* VBK_MockMiner_mineVbkBlockTip(MockMiner_t* self);

/**
 * Mine new altintegration::VbkBlock on the top of the provided block.
 *
 * @param[in] self MockMiner.
 * @param[in] block_hash hash of the altintegration::VbkBlock.
 * @param[in] block_hash_size size of the input block hash.
 * @return VBK_ByteStream* stream that stores.
 * altintegration::BlockIndex<altintegration::VbkBlock> raw bytes.
 * @ingroup c-api
 */
VBK_ByteStream* VBK_MockMiner_mineVbkBlock(MockMiner_t* self,
                                           const uint8_t* block_hash,
                                           int block_hash_size);

/**
 * Mine new altintegration::ATV.
 *
 * @param[in] self MockMiner.
 * @param[in] publication_data altintegration::PublicationData of the
 * altintegration::AltBlock.
 * @param[in] publication_data_size size of the publication data.
 * @param[out] state VbkValidationState.
 * @return VBK_ByteStream* stream that stores altintgration::ATV in the
 * VbkEncoding format.
 * @ingroup c-api
 */
VBK_ByteStream* VBK_MockMiner_mineATV(MockMiner_t* self,
                                      const uint8_t* publication_data,
                                      int publication_data_size,
                                      VbkValidationState* state);

/**
 * Mine new altintegration::VTB.
 *
 * @param[in] self MockMiner.
 * @param[in] endorsed_vbk_block endorsed altintegration::VbkBlock in the
 * VbkEncoding format.
 * @param[in] endorsed_vbk_block_size size of endorsed block.
 * @param[in] last_known_btc_block_hash altintegration::BtcBlock hash of the
 * last known block.
 * @param[in] last_known_btc_block_hash_size size of the last known
 * altintegration::BtcBlock hash.
 * @param[out] state VbkValidationState.
 * @return VBK_ByteStream* stream that stores altintgration::VTB in the
 * VbkEncoding format.
 * @ingroup c-api
 */
VBK_ByteStream* VBK_MockMiner_mineVTB(MockMiner_t* self,
                                      const uint8_t* endorsed_vbk_block,
                                      int endorsed_vbk_block_size,
                                      const uint8_t* last_known_btc_block_hash,
                                      int last_known_btc_block_hash_size,
                                      VbkValidationState* state);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif
