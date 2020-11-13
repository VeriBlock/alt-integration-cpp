// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_MOCK_MINER_H
#define VERIBLOCK_POP_CPP_MOCK_MINER_H

#include "veriblock/c/bytestream.h"

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
 * @param[in] self MockMiner
 * @return VBK_ByteStream* stream that stores
 * altintegration::BlockIndex<altintegration::BtcBlock> raw bytes
 * @ingroup c-api
 */
VBK_ByteStream* VBK_mineBtcBlockTip(MockMiner_t* self);

/**
 * Mine new altintegration::BtcBlock on the top of the provided block.
 *
 * @param[in] self MockMiner
 * @param[in] tip_block_bytes provided tip
 * altintegration::BlockIndex<altintegration::BtcBlock> raw bytes
 * @param[in] tip_block_bytes_size size of the input block bytes
 * @return VBK_ByteStream* stream that stores
 * altintegration::BlockIndex<altintegration::BtcBlock> raw bytes
 * @ingroup c-api
 */
VBK_ByteStream* VBK_mineBtcBlock(MockMiner_t* self,
                                 const uint8_t* tip_block_bytes,
                                 int tip_block_bytes_size);

/**
 * Mine new altintegration::VbkBlock on the top of the current vbktree.
 *
 * @param[in] self MockMiner
 * @return VBK_ByteStream* stream that stores
 * altintegration::BlockIndex<altintegration::VbkBlock> raw bytes
 * @ingroup c-api
 */
VBK_ByteStream* VBK_mineVbkBlockTip(MockMiner_t* self);

/**
 * Mine new altintegration::VbkBlock on the top of the provided block.
 *
 * @param[in] self MockMiner
 * @param[in] tip_block_bytes provided tip
 * altintegration::BlockIndex<altintegration::VbkBlock> raw bytes
 * @param[in] tip_block_bytes_size size of the input block bytes
 * @return VBK_ByteStream* stream that stores
 * altintegration::BlockIndex<altintegration::VbkBlock> raw bytes
 * @ingroup c-api
 */
VBK_ByteStream* VBK_mineVbkBlock(MockMiner_t* self,
                                 const uint8_t* tip_block_bytes,
                                 int tip_block_bytes_size);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif
