// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>

#include <vector>

#include "veriblock/c/utils.h"
#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vtb.hpp"

void VBK_VbkBlock_getId(const uint8_t* block_bytes,
                        int block_bytes_size,
                        uint8_t* id_bytes,
                        int* id_bytes_size) {
  altintegration::Slice<const uint8_t> bytes(block_bytes, block_bytes_size);
  altintegration::ReadStream stream(bytes);
  auto block = altintegration::VbkBlock::fromVbkEncoding(stream);
  auto id = block.getId();
  memcpy(id_bytes, id.data(), id.size());
  *id_bytes_size = id.size();
}

void VBK_VTB_getId(const uint8_t* vtb_bytes,
                   int vtb_bytes_size,
                   uint8_t* id_bytes,
                   int* id_bytes_size) {
  altintegration::Slice<const uint8_t> bytes(vtb_bytes, vtb_bytes_size);
  altintegration::ReadStream stream(bytes);
  auto vtb = altintegration::VTB::fromVbkEncoding(stream);
  auto id = vtb.getId();
  memcpy(id_bytes, id.data(), id.size());
  *id_bytes_size = id.size();
}

void VBK_ATV_getId(const uint8_t* atv_bytes,
                   int atv_bytes_size,
                   uint8_t* id_bytes,
                   int* id_bytes_size) {
  altintegration::Slice<const uint8_t> bytes(atv_bytes, atv_bytes_size);
  altintegration::ReadStream stream(bytes);
  auto atv = altintegration::ATV::fromVbkEncoding(stream);
  auto id = atv.getId();
  memcpy(id_bytes, id.data(), id.size());
  *id_bytes_size = id.size();
}

void VBK_VbkBlock_getHash(const uint8_t* block_bytes,
                          int block_bytes_size,
                          uint8_t* hash_bytes,
                          int* hash_bytes_size) {
  altintegration::Slice<const uint8_t> bytes(block_bytes, block_bytes_size);
  altintegration::ReadStream stream(bytes);
  auto block = altintegration::VbkBlock::fromVbkEncoding(stream);
  auto hash = block.getHash();
  memcpy(hash_bytes, hash.data(), hash.size());
  *hash_bytes_size = hash.size();
}

void VBK_BtcBlock_getHash(const uint8_t* block_bytes,
                          int block_bytes_size,
                          uint8_t* hash_bytes,
                          int* hash_bytes_size) {
  altintegration::Slice<const uint8_t> bytes(block_bytes, block_bytes_size);
  altintegration::ReadStream stream(bytes);
  auto block = altintegration::BtcBlock::fromVbkEncoding(stream);
  auto hash = block.getHash();
  memcpy(hash_bytes, hash.data(), hash.size());
  *hash_bytes_size = hash.size();
}