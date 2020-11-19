// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>

#include <fstream>

#include "bytestream.hpp"
#include "mock_miner.hpp"
#include "veriblock/c/mock_miner.h"

MockMiner_t* VBK_NewMockMiner() {
  auto* miner = new MockMiner();
  miner->miner = std::make_shared<altintegration::MockMiner>();
  return miner;
}

void VBK_FreeMockMiner(MockMiner_t* miner) {
  if (miner) {
    delete miner;
  }
}

VBK_ByteStream* VBK_MockMiner_mineBtcBlockTip(MockMiner_t* self) {
  VBK_ASSERT(self);

  auto* new_block = self->miner->mineBtcBlocks(1);
  VBK_ASSERT(new_block);
  return new VbkByteStream(new_block->toRaw());
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
  return new VbkByteStream(new_block->toRaw());
}

VBK_ByteStream* VBK_MockMiner_mineVbkBlockTip(MockMiner_t* self) {
  VBK_ASSERT(self);

  auto* new_block = self->miner->mineVbkBlocks(1);
  VBK_ASSERT(new_block);
  return new VbkByteStream(new_block->toRaw());
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
  return new VbkByteStream(new_block->toRaw());
}

VBK_ByteStream* VBK_MockMiner_mineATV(MockMiner_t* self,
                                      const uint8_t* publication_data,
                                      int publication_data_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(publication_data);

  altintegration::ReadStream r_stream(altintegration::Slice<const uint8_t>(
      publication_data, publication_data_size));
  auto vbktx = self->miner->createVbkTxEndorsingAltBlock(
      altintegration::PublicationData::fromRaw(r_stream));

  altintegration::ValidationState state;
  auto atv = self->miner->applyATV(vbktx, state);
  VBK_ASSERT(state.IsValid());

  altintegration::WriteStream w_stream;
  atv.toVbkEncoding(w_stream);
  return new VbkByteStream(w_stream.data());
}

VBK_ByteStream* VBK_MockMiner_mineVTB(MockMiner_t* self,
                                      const uint8_t* endorsed_vbk_block,
                                      int endorsed_vbk_block_size,
                                      const uint8_t* last_known_btc_block_hash,
                                      int last_known_btc_block_hash_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(endorsed_vbk_block);
  VBK_ASSERT(last_known_btc_block_hash);

  altintegration::ReadStream r_stream(altintegration::Slice<const uint8_t>(
      endorsed_vbk_block, endorsed_vbk_block_size));
  auto vbk_block = altintegration::VbkBlock::fromVbkEncoding(r_stream);

  altintegration::BtcBlock::hash_t hash(altintegration::Slice<const uint8_t>(
      last_known_btc_block_hash, last_known_btc_block_hash_size));

  altintegration::ValidationState state;
  auto tx = self->miner->endorseVbkBlock(vbk_block, hash, state);
  self->miner->vbkmempool.push_back(tx);

  VBK_ASSERT(state.IsValid());
  auto containingBlock = self->miner->mineVbkBlocks(1);
  auto vtbs = self->miner->vbkPayloads[containingBlock->getHash()];
  VBK_ASSERT(vtbs.size() == 1);

  altintegration::WriteStream w_stream;
  vtbs[0].toVbkEncoding(w_stream);
  return new VbkByteStream(w_stream.data());
}