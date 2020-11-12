// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>

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

VBK_ByteStream* VBK_mineBtcBlockTip(MockMiner_t* self) {
  VBK_ASSERT(self);

  auto* btc_block = self->miner->mineBtcBlocks(1);
  return new VbkByteStream(btc_block->toRaw());
}

VBK_ByteStream* VBK_mineBtcBlock(MockMiner_t* self,
                                 const uint8_t* tip_block_bytes,
                                 int tip_block_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(tip_block_bytes);

  altintegration::ReadStream read_stream(altintegration::Slice<const uint8_t>(
      tip_block_bytes, tip_block_bytes_size));
  auto tip = altintegration::BlockIndex<altintegration::BtcBlock>::fromRaw(
      read_stream);
  auto* btc_block = self->miner->mineBtcBlocks(tip, 1);
  return new VbkByteStream(btc_block->toRaw());
}

VBK_ByteStream* VBK_mineVbkBlockTip(MockMiner_t* self) {
  VBK_ASSERT(self);

  auto* vbk_block = self->miner->mineVbkBlocks(1);
  return new VbkByteStream(vbk_block->toRaw());
}

VBK_ByteStream* VBK_mineVbkBlock(MockMiner_t* self,
                                 const uint8_t* tip_block_bytes,
                                 int tip_block_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(tip_block_bytes);

  altintegration::ReadStream read_stream(altintegration::Slice<const uint8_t>(
      tip_block_bytes, tip_block_bytes_size));
  auto tip = altintegration::BlockIndex<altintegration::VbkBlock>::fromRaw(
      read_stream);
  auto vbk_block = self->miner->mineVbkBlocks(tip, 1);
  return new VbkByteStream(vbk_block->toRaw());
}