// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_MOCK_MINER_H
#define VERIBLOCK_POP_CPP_MOCK_MINER_H

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
void VBK_FreeConfig(MockMiner_t* miner);

/**
 * Mine new altintegration::BtcBlock on the top of the current btctree.
 *
 * @param[out] block_bytes altintegration::BtcBlock raw bytes, expect
 * ToVbkEncoding format (must be prealocated)
 * @param[out] block_bytes_size size of block bytes
 * @return true if block successfully mined, false otherwise
 * @ingroup c-api
 */
bool VBK_mineBtcBlock(uint8_t* block_bytes, int* block_bytes_size);

/**
 * Mine new altintegration::BtcBlock on the top of the provided block.
 *
 * @param[in] tip_block_bytes provided tip altintegration::BtcBlock raw bytes,
 * expect ToVbkEncoding
 * @param[in] tip_block_bytes_size size of the input block bytes
 * @param[out] block_bytes altintegration::BtcBlock raw bytes, expect
 * ToVbkEncoding format (must be prealocated)
 * @param[out] block_bytes_size size of block bytes
 * @return true if block successfully mined, false otherwise
 * @ingroup c-api
 */
bool VBK_mineBtcBlock(uint8_t* tip_block_bytes,
                      int* tip_block_bytes_size,
                      uint8_t* block_bytes,
                      int* block_bytes_size);

/**
 * Mine new altintegration::VbkBlock on the top of the current vbktree.
 *
 * @param[out] block_bytes altintegration::VbkBlock raw bytes, expect
 * ToVbkEncoding format (must be prealocated)
 * @param[out] block_bytes_size size of block bytes
 * @return true if block successfully mined, false otherwise
 * @ingroup c-api
 */
bool VBK_mineVbkBlock(uint8_t* block_bytes, int* block_bytes_size);

/**
 * Mine new altintegration::VbkBlock on the top of the provided block.
 *
 * @param[in] tip_block_bytes provided tip altintegration::VbkBlock raw bytes,
 * expect ToVbkEncoding
 * @param[in] tip_block_bytes_size size of the input block bytes
 * @param[out] block_bytes altintegration::VbkBlock raw bytes, expect
 * ToVbkEncoding format (must be prealocated)
 * @param[out] block_bytes_size size of block bytes
 * @return true if block successfully mined, false otherwise
 * @ingroup c-api
 */
bool VBK_mineBtcBlock(uint8_t* tip_block_bytes,
                      int* tip_block_bytes_size,
                      uint8_t* block_bytes,
                      int* block_bytes_size);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif
