// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/c/utils.h>

#include <cstdio>
#include <vector>
#include <veriblock/pop/alt-util.hpp>
#include <veriblock/pop/entities/atv.hpp>
#include <veriblock/pop/entities/btcblock.hpp>
#include <veriblock/pop/entities/popdata.hpp>
#include <veriblock/pop/entities/vbkblock.hpp>
#include <veriblock/pop/entities/vtb.hpp>
#include <veriblock/pop/exceptions/storage_io.hpp>
#include <veriblock/pop/stateless_validation.hpp>
#include <veriblock/pop/storage/util.hpp>

#include "adaptors/block_provider_impl.hpp"
#include "bytestream.hpp"
#include "pop_context.hpp"
#include "validation_state.hpp"

VBK_ByteStream* VBK_AltBlock_getEndorsedBy(PopContext* self,
                                           const uint8_t* hash,
                                           int hash_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash);

  using namespace altintegration;
  std::vector<uint8_t> v_hash(hash, hash + hash_size);

  auto* index = self->context->getAltBlockTree().getBlockIndex(v_hash);
  if (index == nullptr) {
    return nullptr;
  }

  WriteStream stream;
  writeArrayOf<const AltEndorsement*>(
      stream,
      index->getEndorsedBy(),
      [](WriteStream& stream, const AltEndorsement* v) {
        v->toVbkEncoding(stream);
      });

  return nullptr;
}

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

void VBK_AltBlock_calculateTopLevelMerkleRoot(PopContext* self,
                                              const uint8_t txRoot[32],
                                              const uint8_t* prev_block_hash,
                                              int prev_block_hash_size,
                                              const uint8_t* pop_data_bytes,
                                              int pop_data_bytes_size,
                                              uint8_t out_hash[32]) {
  VBK_ASSERT(self);
  VBK_ASSERT(txRoot);
  VBK_ASSERT(self->context);
  VBK_ASSERT(out_hash);
  VBK_ASSERT(pop_data_bytes);
  VBK_ASSERT(prev_block_hash);

  using namespace altintegration;
  std::vector<uint8_t> txmroot(txRoot, txRoot + 32);

  std::vector<uint8_t> prev_hash(prev_block_hash,
                                 prev_block_hash + prev_block_hash_size);
  auto pop_data = AssertDeserializeFromVbkEncoding<PopData>(
      Slice<const uint8_t>(pop_data_bytes, pop_data_bytes_size));

  auto* index = self->context->getAltBlockTree().getBlockIndex(prev_hash);
  VBK_ASSERT_MSG(index != nullptr,
                 "Can't find a block with hash %s. Did you forget to make "
                 "TopLevelMerkleRoot calculation stateful?",
                 HexStr(prev_hash));

  auto hash = CalculateTopLevelMerkleRoot(
      txmroot, pop_data, index, self->context->getAltBlockTree().getParams());

  std::copy(hash.begin(), hash.end(), out_hash);
}

VBK_ByteStream* VBK_AltBlock_generatePublicationData(
    PopContext* self,
    const uint8_t* endorsed_block_header,
    int endorsed_block_header_size,
    const uint8_t txRoot[32],
    const uint8_t* pop_data_bytes,
    int pop_data_bytes_size,
    const uint8_t* payout_info,
    int payout_info_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);
  VBK_ASSERT(payout_info);
  VBK_ASSERT(endorsed_block_header);
  VBK_ASSERT(txRoot);
  VBK_ASSERT(pop_data_bytes);

  using namespace altintegration;

  std::vector<uint8_t> txmroot(txRoot, txRoot + 32);

  auto pop_data = AssertDeserializeFromVbkEncoding<PopData>(
      Slice<const uint8_t>(pop_data_bytes, pop_data_bytes_size));

  std::vector<uint8_t> header(
      endorsed_block_header,
      endorsed_block_header + endorsed_block_header_size);
  std::vector<uint8_t> payout(payout_info, payout_info + payout_info_size);

  PublicationData res;
  if (!self->context->generatePublicationData(
          res, header, txmroot, pop_data, payout)) {
    return nullptr;
  }

  return new VbkByteStream(altintegration::SerializeToVbkEncoding(res));
}

bool VBK_checkATV(PopContext* self,
                  const uint8_t* atv_bytes,
                  int atv_bytes_size,
                  VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(self->context);
  VBK_ASSERT(atv_bytes);

  using namespace altintegration;
  ATV atv = AssertDeserializeFromVbkEncoding<ATV>(
      Slice<const uint8_t>(atv_bytes, atv_bytes_size));
  return self->context->check(atv, state->getState());
}

bool VBK_checkVTB(PopContext* self,
                  const uint8_t* vtb_bytes,
                  int vtb_bytes_size,
                  VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(self->context);
  VBK_ASSERT(vtb_bytes);

  using namespace altintegration;
  VTB vtb = AssertDeserializeFromVbkEncoding<VTB>(
      Slice<const uint8_t>(vtb_bytes, vtb_bytes_size));
  return self->context->check(vtb, state->getState());
}

bool VBK_checkVbkBlock(PopContext* self,
                       const uint8_t* vbk_bytes,
                       int vbk_bytes_size,
                       VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(self->context);
  VBK_ASSERT(vbk_bytes);

  using namespace altintegration;
  VbkBlock block = AssertDeserializeFromVbkEncoding<VbkBlock>(
      Slice<const uint8_t>(vbk_bytes, vbk_bytes_size));
  return self->context->check(block, state->getState());
}

bool VBK_checkPopData(PopContext* self,
                      const uint8_t* pop_data_bytes,
                      int pop_data_bytes_size,
                      VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(self->context);
  VBK_ASSERT(pop_data_bytes);

  using namespace altintegration;
  PopData pop_data = AssertDeserializeFromVbkEncoding<PopData>(
      Slice<const uint8_t>(pop_data_bytes, pop_data_bytes_size));
  return self->context->check(pop_data, state->getState());
}

bool VBK_SaveAllTrees(PopContext* self, VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(self->storage);
  VBK_ASSERT(self->context);

  using namespace altintegration;
  auto write_batch = self->storage->generateWriteBatch();
  adaptors::BlockBatchImpl block_batch(*write_batch);
  try {
    self->context->saveAllTrees(block_batch);
    write_batch->writeBatch();
  } catch (const StorageIOException& e) {
    state->getState().Invalid("failed-save-trees", e.what());
    return false;
  } catch (...) {
    VBK_ASSERT_MSG(false, "catched unexpected exception");
  }

  return true;
}

bool VBK_LoadAllTrees(PopContext* self, VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(self->storage);
  VBK_ASSERT(self->context);

  using namespace altintegration;

  adaptors::BlockReaderImpl block_reader(*self->storage);
  return loadTrees(*self->context, block_reader, state->getState());
}
