// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>

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

void VBK_mineBtcBlockTip(MockMiner_t* self,
                         uint8_t** block_bytes,
                         int* block_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(block_bytes);
  VBK_ASSERT(block_bytes_size);

  auto* btc_block = self->miner->mineBtcBlocks(1);
  std::vector<uint8_t> bytes = btc_block->toRaw();
  *block_bytes = new uint8_t[bytes.size()];
  memcpy(*block_bytes, bytes.data(), bytes.size());
  *block_bytes_size = bytes.size();
}

void VBK_mineBtcBlock(MockMiner_t* self,
                      const uint8_t* tip_block_bytes,
                      int tip_block_bytes_size,
                      uint8_t** block_bytes,
                      int* block_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(tip_block_bytes);
  VBK_ASSERT(block_bytes);
  VBK_ASSERT(block_bytes_size);

  altintegration::ReadStream read_stream(altintegration::Slice<const uint8_t>(
      tip_block_bytes, tip_block_bytes_size));
  auto tip = altintegration::BlockIndex<altintegration::BtcBlock>::fromRaw(
      read_stream);
  auto btc_block = self->miner->mineBtcBlocks(tip, 1);
  std::vector<uint8_t> bytes = btc_block->toRaw();
  *block_bytes = new uint8_t[bytes.size()];
  memcpy(*block_bytes, bytes.data(), bytes.size());
  *block_bytes_size = bytes.size();
}

void VBK_mineVbkBlockTip(MockMiner_t* self,
                         uint8_t** block_bytes,
                         int* block_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(block_bytes);
  VBK_ASSERT(block_bytes_size);

  auto* vbk_block = self->miner->mineVbkBlocks(1);
  std::vector<uint8_t> bytes = vbk_block->toRaw();
  *block_bytes = new uint8_t[bytes.size()];
  memcpy(*block_bytes, bytes.data(), bytes.size());
  *block_bytes_size = bytes.size();
}

void VBK_mineVbkBlock(MockMiner_t* self,
                      const uint8_t* tip_block_bytes,
                      int tip_block_bytes_size,
                      uint8_t** block_bytes,
                      int* block_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(tip_block_bytes);
  VBK_ASSERT(block_bytes);
  VBK_ASSERT(block_bytes_size);

  altintegration::ReadStream read_stream(altintegration::Slice<const uint8_t>(
      tip_block_bytes, tip_block_bytes_size));
  auto tip = altintegration::BlockIndex<altintegration::VbkBlock>::fromRaw(
      read_stream);
  auto vbk_block = self->miner->mineVbkBlocks(tip, 1);
  std::vector<uint8_t> bytes = vbk_block->toRaw();
  *block_bytes = new uint8_t[bytes.size()];
  memcpy(*block_bytes, bytes.data(), bytes.size());
  *block_bytes_size = bytes.size();
}