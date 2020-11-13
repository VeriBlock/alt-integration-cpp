// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_POP_CONTEXT_H
#define VERIBLOCK_POP_CPP_POP_CONTEXT_H

/**
 * @defgroup c-api C interface
 */

#include "veriblock/c/bytestream.h"
#include "veriblock/c/config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PopContext PopContext;

PopContext* VBK_NewPopContext(Config_t* config);
void VBK_FreePopContext(PopContext* app);

/**
 * @copybrief altintegration::AltBlockTree::acceptBlockHeader
 * @see altintegration::AltBlockTree::acceptBlockHeader
 * @param[in] self PopContext
 * @param[in] block_bytes altintegration::AltBlock raw represantation
 * @param[in] bytes_size block bytes size
 * @return true if block is valid, and added; false otherwise.
 * @ingroup c-api
 */
bool VBK_AltBlockTree_acceptBlockHeader(PopContext* self,
                                        const uint8_t* block_bytes,
                                        int bytes_size);

/**
 * @copybrief altintegration::AltBlockTree::acceptBlock
 * @see altintegration::AltBlockTree::acceptBlock
 * @param[in] self PopContext
 * @param[in] hash_bytes altintegration::AltBlock hash raw represantation
 * @param[in] hash_bytes_size hash bytes size
 * @param[in] payloads_bytes altintegration::PopData raw represantation of all
 * POP payloads stored in this block
 * @param[in] payloads_bytes_size payloads bytes size
 *
 * @ingroup c-api
 */
void VBK_AltBlockTree_acceptBlock(PopContext* self,
                                  const uint8_t* hash_bytes,
                                  int hash_bytes_size,
                                  const uint8_t* payloads_bytes,
                                  int payloads_bytes_size);

/**
 * @copybrief altintegration::AltBlockTree::addPayloads
 * @see altintegration::AltBlockTree::addPayloads
 * @param[in] self PopContext
 * @param[in] hash_bytes altintegration::AltBlock hash bytes of block where to
 * add the block body
 * @param[in] hash_bytes_size hash bytes size
 * @param[in] payloads_bytes altintegration::PopData raw represantation of all
 * POP payloads stored in this block
 * @param[in] payloads_bytes_size payloads bytes size
 * @return true if altintegration::PopData does not contain duplicates (searched
 * across active chain). However, it is far from certain that it is completely
 * valid.
 * @ingroup c-api
 */
bool VBK_AltBlockTree_addPayloads(PopContext* self,
                                  const uint8_t* hash_bytes,
                                  int hash_bytes_size,
                                  const uint8_t* payloads_bytes,
                                  int payloads_bytes_size);

/**
 * @copybrief altintegration::AltBlockTree::loadTip
 * @see altintegration::AltBlockTree::loadTip
 * @param[in] self PopContext
 * @param[in] hash_bytes altintegration::AltBlock hash bytes of the tip
 * @param[in] hash_bytes_size hash bytes size
 * @return true on success, false otherwise
 * @ingroup c-api
 */
bool VBK_AltBlockTree_loadTip(PopContext* self,
                              const uint8_t* hash_bytes,
                              int hash_bytes_size);

/**
 * @copybrief altintegration::AltBlockTree::comparePopScore
 * @see altintegration::AltBlockTree::comparePopScore
 * @param[in] self PopContext
 * @param[in] A_hash_bytes A altintegration::AltBlock hash bytes of current tip
 * in AltBlockTree. Fails on assert if current tip != A.
 * @param[in] A_hash_bytes_size A block hash bytes size
 * @param[in] B_hash_bytes B altintegration::AltBlock hash bytes. Current tip
 * will be compared against this block. Must exist on chain and have
 * BLOCK_HAS_PAYLOADS.
 * @param[in] B_hash_bytes_size B block hash bytes size
 * @see altintegration::AltBlockTree::comparePopScore
 * @ingroup c-api
 */
int VBK_AltBlockTree_comparePopScore(PopContext* self,
                                     const uint8_t* A_hash_bytes,
                                     int A_hash_bytes_size,
                                     const uint8_t* B_hash_bytes,
                                     int B_hash_bytes_size);

/**
 * @copybrief altintegration::AltBlockTree::removeSubtree
 * @see altintegration::AltBlockTree::removeSubtree
 * @param[in] self PopContext
 * @param[in] hash_bytes altintegration::AltBlock hash bytes of the block to be
 * removed
 * @param[in] hash_bytes_size hash bytes size
 * @warning fails on assert if block can not be found in this tree.
 * @ingroup c-api
 */
void VBK_AltBlockTree_removeSubtree(PopContext* self,
                                    const uint8_t* hash_bytes,
                                    int hash_bytes_size);

/**
 * @copybrief altintegration::AltBlockTree::setState
 * @see altintegration::AltBlockTree::setState
 * @param[in] self PopContext
 * @param[in] hash_bytes altintegration::AltBlock hash bytes of the block to
 * tree will be switched to this block
 * @param[in] hash_bytes_size size of input hash
 * @return `false` if intermediate or target block is invalid. In this case
 * tree will rollback into original state. `true` if state change is
 * successful.
 * @invariant atomic - either switches to new state, or does nothing.
 * @warning Expensive operation.
 * @ingroup c-api
 */
bool VBK_AltBlockTree_setState(PopContext* self,
                               const uint8_t* hash_bytes,
                               int hash_bytes_size);

/**
 * Find a BtcBlock index from the BtcTree
 *
 * @param[in] self PopContext
 * @param[in] hash_bytes altintegration::BtcBlock hash bytes
 * @param[in] hash_bytes_size size of input hash
 * @param[out] blockindex pointer to the blockindex bytes (memory will allocated
 * by this method)
 * @param[out] blockindex_size blockindex bytes size
 * @return `false` while block is not found. 'true' while block is found.
 * @ingroup c-api
 */
bool VBK_btc_getBlockIndex(PopContext* self,
                           const uint8_t* hash_bytes,
                           int hash_bytes_size,
                           uint8_t** blockindex,
                           int* blockindex_size);

/**
 * Find a VbkBlock index from the VbkTree
 *
 * @param[in] self PopContext
 * @param[in] hash_bytes altintegration::VbkBlock hash bytes
 * @param[in] hash_bytes_size size of input hash
 * @param[out] blockindex pointer to the blockindex bytes (memory will allocated
 * by this method)
 * @param[out] blockindex_size blockindex bytes size
 * @return `false` while block is not found. 'true' while block is found.
 * @ingroup c-api
 */
bool VBK_vbk_getBlockIndex(PopContext* self,
                           const uint8_t* hash_bytes,
                           int hash_bytes_size,
                           uint8_t** blockindex,
                           int* blockindex_size);

/**
 * Find a AltBlock index from the AltTree
 *
 * @param[in] self PopContext
 * @param[in] hash_bytes altintegration::AltBlock hash bytes
 * @param[in] hash_bytes_size size of input hash
 * @param[out] blockindex pointer to the blockindex bytes (memory will allocated
 * by this method)
 * @param[out] blockindex_size blockindex bytes size
 * @return `false` while block is not found. 'true' while block is found.
 * @ingroup c-api
 */
bool VBK_alt_getBlockIndex(PopContext* self,
                           const uint8_t* hash_bytes,
                           int hash_bytes_size,
                           uint8_t** blockindex,
                           int* blockindex_size);

/**
 * Return best block (tip) of the  AltTree
 *
 * @param[in] self PopContext
 * @return altintegration::BlockIndex<altintegration::AltBlock> serialized block to the stream
 * @ingroup c-api
 */
VBK_ByteStream* VBK_alt_BestBlock(PopContext* self);

/**
 * Return best block (tip) of the  VbkBlockTree
 *
 * @param[in] self PopContext
 * @return altintegration::BlockIndex<altintegration::VbkBlock> serialized block to the stream
 * @ingroup c-api
 */
VBK_ByteStream* VBK_vbk_BestBlock(PopContext* self);

/**
 * Return best block (tip) of the  BtcBlockTree
 *
 * @param[in] self PopContext
 * @return altintegration::BlockIndex<altintegration::BtcBlock> serialized block to the stream
 * @ingroup c-api
 */
VBK_ByteStream* VBK_btc_BestBlock(PopContext* self);

/**
 * Return block on the curent height from the active chain of the AltTree
 *
 * @param[in] self PopContext
 * @return altintegration::BlockIndex<altintegration::AltBlock> serialized block to the stream
 * @ingroup c-api
 */
VBK_ByteStream* VBK_alt_BlockAtActiveChainByHeight(PopContext* self,
                                                   int height);

/**
 * Return block on the curent height from the active chain of the VbkBlockTree
 *
 * @param[in] self PopContext
 * @return altintegration::BlockIndex<altintegration::VbkBlock> serialized block to the stream
 * @ingroup c-api
 */
VBK_ByteStream* VBK_vbk_BlockAtActiveChainByHeight(PopContext* self,
                                                   int height);

/**
 * Return block on the curent height from the active chain of the BtcBlockTree
 *
 * @param[in] self PopContext
 * @return altintegration::BlockIndex<altintegration::BtcBlock> serialized block to the stream
 * @ingroup c-api
 */
VBK_ByteStream* VBK_btc_BlockAtActiveChainByHeight(PopContext* self,
                                                   int height);

/**
 * @copybrief altintegration::MemPool::submit
 * @see altintegration::MemPool::submit
 * @param[in] self PopContext
 * @param[in] bytes altintegration::ATV raw representation
 * @param[in] bytes_size bytes size
 * @return 0 if payload is valid, 1 if statefully invalid, 2 if statelessly
 * invalid
 * @ingroup c-api
 */
int VBK_MemPool_submit_atv(PopContext* self,
                           const uint8_t* bytes,
                           int bytes_size);

/**
 * @copybrief altintegration::MemPool::submit
 * @see altintegration::MemPool::submit
 * @param[in] self PopContext
 * @param[in] bytes altintegration::VTB raw representation
 * @param[in] bytes_size bytes size
 * @return 0 if payload is valid, 1 if statefully invalid, 2 if statelessly
 * invalid
 * @ingroup c-api
 */
int VBK_MemPool_submit_vtb(PopContext* self,
                           const uint8_t* bytes,
                           int bytes_size);

/**
 * @copybrief altintegration::MemPool::submit
 * @see altintegration::MemPool::submit
 * @param[in] self PopContext
 * @param[in] bytes altintegration::VbkBlock raw representation
 * @param[in] bytes_size bytes size
 * @return 0 if payload is valid, 1 if statefully invalid, 2 if statelessly
 * invalid
 * @ingroup c-api
 */
int VBK_MemPool_submit_vbk(PopContext* self,
                           const uint8_t* bytes,
                           int bytes_size);

/**
 * @copybrief altintegration::MemPool::getPop
 * @see altintegration::MemPool::getPop
 * @param[in] self PopContext
 * @param[out] out_bytes output byte array of the altintegration::PopData (must
 * be pre-allocated). Statefully valid altintegration::PopData in the raw
 * representation that can be connected to current tip.
 * @param[out] bytes_size bytes size
 * @ingroup c-api
 */
void VBK_MemPool_getPop(PopContext* self, uint8_t* out_bytes, int* bytes_size);

/**
 * @copybrief altintegration::MemPool::removeAll
 * @see altintegration::MemPool::removeAll
 * @param[in] self PopContext
 * @param[in] bytes altintegration::PopData raw representation
 * @param[in] bytes_size bytes size
 * @ingroup c-api
 */
void VBK_MemPool_removeAll(PopContext* self,
                           const uint8_t* bytes,
                           int bytes_size);

/**
 * @copybrief return altintegration::MemPool known altintegration::ATV by its id
 *
 * @see altintegration::MemPool::get
 * @param[in] self PopContext
 * @param[in] id_bytes altintegration::ATV::id_t
 * @param[in] id_bytes_size size of the altintegration::ATV::id_t
 * @return return altintegration::ATV in the toVbkEncoding format, if cannot
 * find payloads stream will empty
 * @ingroup c-api
 */
VBK_ByteStream* VBK_MemPool_GetATV(PopContext* self,
                                   const uint8_t* id_bytes,
                                   int id_bytes_size);

/**
 * @copybrief return altintegration::MemPool known altintegration::VTB by its id
 *
 * @see altintegration::MemPool::get
 * @param[in] self PopContext
 * @param[in] id_bytes altintegration::VTB::id_t
 * @param[in] id_bytes_size size of the altintegration::VTB::id_t
 * @return return altintegration::VTB in the toVbkEncoding format, if cannot
 * find payloads stream will empty
 * @ingroup c-api
 */
VBK_ByteStream* VBK_MemPool_GetVTB(PopContext* self,
                                   const uint8_t* id_bytes,
                                   int id_bytes_size);

/**
 * @copybrief return altintegration::MemPool known altintegration::VbkBlock by
 * its id
 *
 * @see altintegration::MemPool::get
 * @param[in] self PopContext
 * @param[in] id_bytes altintegration::VbkBlock::id_t
 * @param[in] id_bytes_size size of the altintegration::VbkBlock::id_t
 * @return return altintegration::VbkBlock in the toVbkEncoding format, if
 * cannot find payloads stream will empty
 * @ingroup c-api
 */
VBK_ByteStream* VBK_MemPool_GetVbkBlock(PopContext* self,
                                        const uint8_t* id_bytes,
                                        int id_bytes_size);

/**
 * @copybrief return altintegration::MemPool known ATV`s ids
 *
 * This method returns a vector of the payload`s ids serialized to the stream.
 * Each id has been serialized using the function
 * altintegration::writeSingleByteLenValue().
 *
 * @see altintegration::MemPool::getMap
 * @param[in] self PopContext
 * @return VbkByteStream with the serialized vector of the ATV`s ids
 * @ingroup c-api
 */
VBK_ByteStream* VBK_MemPool_GetATVs(PopContext* self);

/**
 * @copybrief return altintegration::MemPool known VTB`s ids
 *
 * This method returns a vector of the payload`s ids serialized to the stream.
 * Each id has been serialized using the function
 * altintegration::writeSingleByteLenValue().
 *
 * @see altintegration::MemPool::getMap
 * @param[in] self PopContext
 * @return VbkByteStream with the serialized vector of the VTB`s ids
 * @ingroup c-api
 */
VBK_ByteStream* VBK_MemPool_GetVTBs(PopContext* self);

/**
 * @copybrief return altintegration::MemPool known VbkBlock`s ids
 *
 * This method returns a vector of the payload`s ids serialized to the stream.
 * Each id has been serialized using the function
 * altintegration::writeSingleByteLenValue().
 *
 * @see altintegration::MemPool::getMap
 * @param[in] self PopContext
 * @return VbkByteStream with the serialized vector of the VbkBlock`s ids
 * @ingroup c-api
 */
VBK_ByteStream* VBK_MemPool_GetVbkBlocks(PopContext* self);

/**
 * @copybrief return altintegration::MemPool known ATV`s inFlight ids
 *
 * This method returns a vector of the payload`s ids serialized to the stream.
 * Each id has been serialized using the function
 * altintegration::writeSingleByteLenValue().
 *
 * @see altintegration::MemPool::getInFlightMap
 * @param[in] self PopContext
 * @return VbkByteStream with the serialized vector of the ATV`s ids
 * @ingroup c-api
 */
VBK_ByteStream* VBK_MemPool_GetATVsInFlight(PopContext* self);

/**
 * @copybrief return altintegration::MemPool known VTB`s inFlight ids
 *
 * This method returns a vector of the payload`s ids serialized to the stream.
 * Each id has been serialized using the function
 * altintegration::writeSingleByteLenValue().
 *
 * @see altintegration::MemPool::getInFlightMap
 * @param[in] self PopContext
 * @return VbkByteStream with the serialized vector of the VTB`s ids
 * @ingroup c-api
 */
VBK_ByteStream* VBK_MemPool_GetVTBsInFlight(PopContext* self);

/**
 * @copybrief return altintegration::MemPool known VbkBlock`s inFlight ids
 *
 * This method returns a vector of the payload`s ids serialized to the stream.
 * Each id has been serialized using the function
 * altintegration::writeSingleByteLenValue().
 *
 * @see altintegration::MemPool::getInFlightMap
 * @param[in] self PopContext
 * @return VbkByteStream with the serialized vector of the VbkBlock`s ids
 * @ingroup c-api
 */
VBK_ByteStream* VBK_MemPool_GetVbkBlocksInFlight(PopContext* self);

/**
 * @copybrief altintegration::MemPool::clear
 * @see altintegration::MemPool::clear
 * @param[in] self PopContext
 * @ingroup c-api
 */
void VBK_MemPool_clear(PopContext* self);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif  // VERIBLOCK_POP_CPP_POP_CONTEXT_H
