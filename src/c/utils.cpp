// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>

#include <vector>

#include "pop_context.hpp"
#include "veriblock/alt-util.hpp"
#include "veriblock/c/utils.h"
#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/entities/popdata.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vtb.hpp"
#include "veriblock/stateless_validation.hpp"

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

void VBK_AltBlock_calculateContextInfoContainerHash(
    PopContext* self,
    const uint8_t* prev_block_hash,
    int prev_block_hash_size,
    const uint8_t* pop_data_bytes,
    int pop_data_bytes_size,
    uint8_t* out_hash,
    int* out_hash_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);
  VBK_ASSERT(out_hash);
  VBK_ASSERT(out_hash_size);
  VBK_ASSERT(pop_data_bytes);
  VBK_ASSERT(prev_block_hash);

  using namespace altintegration;
  std::vector<uint8_t> block_hash(prev_block_hash,
                                  prev_block_hash + prev_block_hash_size);
  auto pop_data = AssertDeserializeFromVbkEncoding<PopData>(
      Slice<const uint8_t>(pop_data_bytes, pop_data_bytes_size));
  auto* index = self->context->altTree->getBlockIndex(block_hash);
  auto hash = CalculateContextInfoContainerHash(
      pop_data, index, self->context->altTree->getParams());

  memcpy(out_hash, hash.data(), hash.size());
  *out_hash_size = hash.size();
}

bool VBK_checkATV(PopContext* self,
                  const uint8_t* atv_bytes,
                  int atv_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->config);
  VBK_ASSERT(atv_bytes);

  using namespace altintegration;
  ATV atv = AssertDeserializeFromVbkEncoding<ATV>(
      Slice<const uint8_t>(atv_bytes, atv_bytes_size));
  ValidationState state;
  return checkATV(atv, state, self->context->config->getAltParams());
}

bool VBK_checkVTB(PopContext* self,
                  const uint8_t* vtb_bytes,
                  int vtb_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->config);
  VBK_ASSERT(vtb_bytes);

  using namespace altintegration;
  VTB vtb = AssertDeserializeFromVbkEncoding<VTB>(
      Slice<const uint8_t>(vtb_bytes, vtb_bytes_size));
  ValidationState state;
  return checkVTB(vtb, state, self->context->config->getBtcParams());
}

bool VBK_checkVbkBlock(PopContext* self,
                       const uint8_t* vbk_bytes,
                       int vbk_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->config);
  VBK_ASSERT(vbk_bytes);

  using namespace altintegration;
  VbkBlock block = AssertDeserializeFromVbkEncoding<VbkBlock>(
      Slice<const uint8_t>(vbk_bytes, vbk_bytes_size));
  ValidationState state;
  return checkBlock(block, state, self->context->config->getVbkParams());
}

bool VBK_checkPopData(PopContext* self,
                      const uint8_t* pop_data_bytes,
                      int pop_data_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->config);
  VBK_ASSERT(self->context->popValidator);
  VBK_ASSERT(pop_data_bytes);

  using namespace altintegration;
  PopData pop_data = AssertDeserializeFromVbkEncoding<PopData>(
      Slice<const uint8_t>(pop_data_bytes, pop_data_bytes_size));
  ValidationState state;
  return checkPopData(*self->context->popValidator, pop_data, state);
}
