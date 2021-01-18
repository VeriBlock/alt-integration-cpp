// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "mock_miner.hpp"

#include <cstdio>
#include <fstream>

#include "bytestream.hpp"
#include "validation_state.hpp"
#include "veriblock/c/mock_miner.h"

MockMiner_t* VBK_NewMockMiner() {
  auto* miner = new MockMiner();
  miner->miner = std::make_shared<altintegration::MockMiner2>();
  return miner;
}

void VBK_FreeMockMiner(MockMiner_t* miner) {
  if (miner != nullptr) {
    delete miner;
    miner = nullptr;
  }
}

VBK_ByteStream* VBK_MockMiner_mineBtcBlockTip(MockMiner_t* self) {
  VBK_ASSERT(self);

  auto* new_block = self->miner->mineBtcBlocks(1);
  VBK_ASSERT(new_block);
  return new VbkByteStream(new_block->toVbkEncoding());
}

VBK_ByteStream* VBK_MockMiner_mineBtcBlock(MockMiner_t* self,
                                           const uint8_t* block_hash,
                                           int block_hash_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(block_hash);

  altintegration::BtcBlock::hash_t hash(
      altintegration::Slice<const uint8_t>(block_hash, block_hash_size));

  auto* block = self->miner->btc().getBlockIndex(hash);
  if (block == nullptr) {
    return nullptr;
  }

  auto* new_block = self->miner->mineBtcBlocks(*block, 1);
  VBK_ASSERT(new_block);
  return new VbkByteStream(new_block->toVbkEncoding());
}

VBK_ByteStream* VBK_MockMiner_mineVbkBlockTip(MockMiner_t* self) {
  VBK_ASSERT(self);

  auto* new_block = self->miner->mineVbkBlocks(1);
  VBK_ASSERT(new_block);
  return new VbkByteStream(new_block->toVbkEncoding());
}

VBK_ByteStream* VBK_MockMiner_mineVbkBlock(MockMiner_t* self,
                                           const uint8_t* block_hash,
                                           int block_hash_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(block_hash);

  altintegration::VbkBlock::hash_t hash(
      altintegration::Slice<const uint8_t>(block_hash, block_hash_size));

  auto* block = self->miner->vbk().getBlockIndex(hash);
  if (block == nullptr) {
    return nullptr;
  }

  auto* new_block = self->miner->mineVbkBlocks(*block, 1);
  VBK_ASSERT(new_block);
  return new VbkByteStream(new_block->toVbkEncoding());
}

VBK_ByteStream* VBK_MockMiner_mineATV(MockMiner_t* self,
                                      const uint8_t* publication_data,
                                      int publication_data_size,
                                      VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(publication_data);

  using namespace altintegration;

  Slice<const uint8_t> slice(publication_data, publication_data_size);
  PublicationData pubdata;
  if (!DeserializeFromVbkEncoding(slice, pubdata, state->getState())) {
    // can't deserialize
    return nullptr;
  }

  auto vbktx = self->miner->createVbkTxEndorsingAltBlock(pubdata);
  auto atv = self->miner->applyATV(vbktx, state->getState());
  VBK_ASSERT(state->IsValid());

  WriteStream w_stream;
  atv.toVbkEncoding(w_stream);
  return new VbkByteStream(w_stream.data());
}

VBK_ByteStream* VBK_MockMiner_mineVTB(MockMiner_t* self,
                                      const uint8_t* endorsed_vbk_block,
                                      int endorsed_vbk_block_size,
                                      const uint8_t* last_known_btc_block_hash,
                                      int last_known_btc_block_hash_size,
                                      VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(endorsed_vbk_block);
  VBK_ASSERT(last_known_btc_block_hash);

  using namespace altintegration;
  Slice<const uint8_t> endorsedVbkBlock(endorsed_vbk_block,
                                        endorsed_vbk_block_size);

  Slice<const uint8_t> lastKnownBtcBlockHash(last_known_btc_block_hash,
                                             last_known_btc_block_hash_size);

  VbkBlock vbk_block;
  if (!DeserializeFromVbkEncoding(
          endorsedVbkBlock, vbk_block, state->getState())) {
    // can't deserialize
    return nullptr;
  }

  altintegration::BtcBlock::hash_t hash = lastKnownBtcBlockHash;

  auto tx = self->miner->endorseVbkBlock(vbk_block, hash, state->getState());
  self->miner->vbkmempool.push_back(tx);

  VBK_ASSERT(state->IsValid());
  auto containingBlock = self->miner->mineVbkBlocks(1);
  auto vtbs = self->miner->vbkPayloads[containingBlock->getHash()];
  VBK_ASSERT(vtbs.size() == 1);

  altintegration::WriteStream w_stream;
  vtbs[0].toVbkEncoding(w_stream);
  return new VbkByteStream(w_stream.data());
}