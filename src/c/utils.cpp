// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/c/utils.h"

#include <stdio.h>

#include <vector>

#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vtb.hpp"

void VBK_VbkBlock_getId(const uint8_t* block_bytes,
                        int block_bytes_size,
                        uint8_t* id_bytes,
                        int* id_bytes_size) {
  VBK_ASSERT(block_bytes);
  VBK_ASSERT(id_bytes);
  VBK_ASSERT(id_bytes_size);

  using namespace altintegration;
  Slice<const uint8_t> bytes(block_bytes, block_bytes_size);

  auto block = AssertDeserializeFromVbkEncoding<VbkBlock>(bytes);
  auto id = block.getId();

  std::copy(id.begin(), id.end(), id_bytes);
  *id_bytes_size = id.size();
}

void VBK_VTB_getId(const uint8_t* vtb_bytes,
                   int vtb_bytes_size,
                   uint8_t* id_bytes,
                   int* id_bytes_size) {
  VBK_ASSERT(vtb_bytes);
  VBK_ASSERT(id_bytes);
  VBK_ASSERT(id_bytes_size);
  using namespace altintegration;

  Slice<const uint8_t> bytes(vtb_bytes, vtb_bytes_size);
  auto vtb = AssertDeserializeFromVbkEncoding<VTB>(bytes);
  auto id = vtb.getId();
  std::copy(id.begin(), id.end(), id_bytes);
  *id_bytes_size = id.size();
}

void VBK_ATV_getId(const uint8_t* atv_bytes,
                   int atv_bytes_size,
                   uint8_t* id_bytes,
                   int* id_bytes_size) {
  VBK_ASSERT(atv_bytes);
  VBK_ASSERT(id_bytes);
  VBK_ASSERT(id_bytes_size);
  using namespace altintegration;

  Slice<const uint8_t> bytes(atv_bytes, atv_bytes_size);
  auto atv = AssertDeserializeFromVbkEncoding<ATV>(bytes);
  auto id = atv.getId();
  std::copy(id.begin(), id.end(), id_bytes);
  *id_bytes_size = id.size();
}

void VBK_VbkBlock_getHash(const uint8_t* block_bytes,
                          int block_bytes_size,
                          uint8_t* hash_bytes,
                          int* hash_bytes_size) {
  VBK_ASSERT(block_bytes);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(hash_bytes_size);

  using namespace altintegration;
  Slice<const uint8_t> bytes(block_bytes, block_bytes_size);

  auto block = AssertDeserializeFromVbkEncoding<VbkBlock>(bytes);
  auto id = block.getHash();

  std::copy(id.begin(), id.end(), hash_bytes);
  *hash_bytes_size = id.size();
}

void VBK_BtcBlock_getHash(const uint8_t* block_bytes,
                          int block_bytes_size,
                          uint8_t* hash_bytes,
                          int* hash_bytes_size) {
  VBK_ASSERT(block_bytes);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(hash_bytes_size);

  using namespace altintegration;
  Slice<const uint8_t> bytes(block_bytes, block_bytes_size);

  auto block = AssertDeserializeFromVbkEncoding<BtcBlock>(bytes);
  auto id = block.getHash();

  std::copy(id.begin(), id.end(), hash_bytes);
  *hash_bytes_size = id.size();
}