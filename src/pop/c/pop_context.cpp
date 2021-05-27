// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <veriblock/pop/c/extern.h>
#include <veriblock/pop/c/pop_context.h>

#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/consts.hpp>
#include <veriblock/pop/pop_context.hpp>

#include "adaptors/logger.hpp"
#include "bytestream.hpp"
#include "config.hpp"
#include "pop_context.hpp"
#include "storage.hpp"
#include "validation_state.hpp"
#include "veriblock/pop/storage/adaptors/block_provider_impl.hpp"
#include "veriblock/pop/storage/adaptors/payloads_provider_impl.hpp"

PopContext* VBK_NewPopContext(Config_t* config,
                              Storage_t* storage,
                              const char* log_lvl) {
  VBK_ASSERT(config);
  VBK_ASSERT(log_lvl);
  VBK_ASSERT(config->config);
  auto& c = config->config;

  VBK_ASSERT(c->alt);

  // set logger
  altintegration::SetLogger<adaptors::Logger>(
      altintegration::StringToLevel(log_lvl));

  auto* v = new PopContext();

  v->storage = storage->storage;

  v->payloads_storage =
      std::make_shared<altintegration::adaptors::PayloadsStorageImpl>(
          *v->storage);
  v->context = altintegration::PopContext::create(c, v->payloads_storage);

  return v;
}

void VBK_FreePopContext(PopContext* app) {
  if (app != nullptr) {
    delete app;
    app = nullptr;
  }
}

bool VBK_AltBlockTree_acceptBlockHeader(PopContext* self,
                                        const uint8_t* block_bytes,
                                        int bytes_size,
                                        VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(block_bytes);
  VBK_ASSERT(self->context);

  altintegration::Slice<const uint8_t> bytes(block_bytes, bytes_size);
  altintegration::ReadStream stream(bytes);
  altintegration::AltBlock blk;

  if (!altintegration::DeserializeFromRaw(stream, blk, state->getState())) {
    return false;
  }

  if (!self->context->getAltBlockTree().acceptBlockHeader(blk,
                                                          state->getState())) {
    return false;
  }

  return true;
}

void VBK_AltBlockTree_acceptBlock(PopContext* self,
                                  const uint8_t* hash_bytes,
                                  int hash_bytes_size,
                                  const uint8_t* payloads_bytes,
                                  int payloads_bytes_size,
                                  VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(payloads_bytes);
  VBK_ASSERT(self->context);

  altintegration::Slice<const uint8_t> p_bytes(payloads_bytes,
                                               payloads_bytes_size);
  altintegration::ReadStream stream(p_bytes);
  altintegration::PopData popData;
  bool res = altintegration::DeserializeFromVbkEncoding(
      stream, popData, state->getState());
  VBK_ASSERT_MSG(
      res, "can not deserialize PopData, error: %s", state->GetErrorMessage());

  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);

  self->context->getAltBlockTree().acceptBlock(hash, popData);
}

int VBK_AltBlockTree_comparePopScore(PopContext* self,
                                     const uint8_t* A_hash_bytes,
                                     int A_hash_bytes_size,
                                     const uint8_t* B_hash_bytes,
                                     int B_hash_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(A_hash_bytes);
  VBK_ASSERT(B_hash_bytes);
  VBK_ASSERT(self->context);

  std::vector<uint8_t> A_hash(A_hash_bytes, A_hash_bytes + A_hash_bytes_size);
  std::vector<uint8_t> B_hash(B_hash_bytes, B_hash_bytes + B_hash_bytes_size);
  return self->context->getAltBlockTree().comparePopScore(A_hash, B_hash);
}

void VBK_AltBlockTree_removeSubtree(PopContext* self,
                                    const uint8_t* hash_bytes,
                                    int hash_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(self->context);

  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);
  self->context->getAltBlockTree().removeSubtree(hash);
}

bool VBK_AltBlockTree_setState(PopContext* self,
                               const uint8_t* hash_bytes,
                               int hash_bytes_size,
                               VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(self->context);

  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);
  return self->context->getAltBlockTree().setState(hash, state->getState());
}

VBK_ByteStream* VBK_AltBlockTree_getPopPayout(PopContext* self,
                                              const uint8_t* tip_hash_bytes,
                                              int tip_hash_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);

  std::vector<uint8_t> hash{tip_hash_bytes,
                            tip_hash_bytes + tip_hash_bytes_size};
  auto res = self->context->getPopPayout(hash);

  altintegration::WriteStream stream;
  res.toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_btc_getBlockIndex(PopContext* self,
                                      const uint8_t* hash_bytes,
                                      int hash_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(self->context);

  altintegration::BtcBlock::hash_t hash(
      altintegration::Slice<const uint8_t>(hash_bytes, hash_bytes_size));
  auto* blockIndex = self->context->getAltBlockTree().btc().getBlockIndex(hash);
  if (blockIndex == nullptr) {
    return nullptr;
  }
  altintegration::WriteStream stream;
  blockIndex->toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_vbk_getBlockIndex(PopContext* self,
                                      const uint8_t* hash_bytes,
                                      int hash_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(self->context);

  altintegration::VbkBlock::hash_t hash(
      altintegration::Slice<const uint8_t>(hash_bytes, hash_bytes_size));
  auto* blockIndex = self->context->getAltBlockTree().vbk().getBlockIndex(hash);
  if (blockIndex == nullptr) {
    return nullptr;
  }
  altintegration::WriteStream stream;
  blockIndex->toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_alt_getBlockIndex(PopContext* self,
                                      const uint8_t* hash_bytes,
                                      int hash_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(self->context);
  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);
  auto* blockIndex = self->context->getAltBlockTree().getBlockIndex(hash);
  if (blockIndex == nullptr) {
    return nullptr;
  }
  altintegration::WriteStream stream;
  blockIndex->toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_alt_BestBlock(PopContext* self) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);
  auto* tip = self->context->getAltBlockTree().getBestChain().tip();
  VBK_ASSERT(tip);
  altintegration::WriteStream stream;
  tip->toVbkEncoding(stream);
  VBK_LOG_DEBUG("tip: %s, hex bytes: %s",
                tip->toPrettyString(),
                altintegration::HexStr(stream.data()));
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_vbk_BestBlock(PopContext* self) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);
  auto* tip = self->context->getAltBlockTree().vbk().getBestChain().tip();
  VBK_ASSERT(tip);
  altintegration::WriteStream stream;
  tip->toVbkEncoding(stream);
  VBK_LOG_DEBUG("tip: %s, hex bytes: %s",
                tip->toPrettyString(),
                altintegration::HexStr(stream.data()));
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_btc_BestBlock(PopContext* self) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);

  auto* tip = self->context->getAltBlockTree().btc().getBestChain().tip();
  VBK_ASSERT(tip);
  altintegration::WriteStream stream;
  tip->toVbkEncoding(stream);
  VBK_LOG_DEBUG("tip: %s, hex bytes: %s",
                tip->toPrettyString(),
                altintegration::HexStr(stream.data()));
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_vbk_BootstrapBlock(PopContext* self) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);

  auto* first = self->context->getAltBlockTree().vbk().getBestChain().first();
  VBK_ASSERT(first);
  altintegration::WriteStream stream;
  first->toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_btc_BootstrapBlock(PopContext* self) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);

  auto* first = self->context->getAltBlockTree().btc().getBestChain().first();
  VBK_ASSERT(first);
  altintegration::WriteStream stream;
  first->toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_alt_BlockAtActiveChainByHeight(PopContext* self,
                                                   int height) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);

  auto* block = self->context->getAltBlockTree().getBestChain()[height];
  if (block == nullptr) {
    return nullptr;
  }
  altintegration::WriteStream stream;
  block->toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_vbk_BlockAtActiveChainByHeight(PopContext* self,
                                                   int height) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);

  auto* block = self->context->getAltBlockTree().vbk().getBestChain()[height];
  if (block == nullptr) {
    return nullptr;
  }
  altintegration::WriteStream stream;
  block->toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_btc_BlockAtActiveChainByHeight(PopContext* self,
                                                   int height) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);

  auto* block = self->context->getAltBlockTree().btc().getBestChain()[height];
  if (block == nullptr) {
    return nullptr;
  }
  altintegration::WriteStream stream;
  block->toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_alt_getATVContainingBlock(PopContext* self,
                                              const uint8_t* p_id,
                                              int p_id_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(p_id);
  VBK_ASSERT(self->context);

  std::vector<uint8_t> atv_id{p_id, p_id + p_id_size};
  auto alt_hashes = self->context->getAltBlockTree()
                        .getPayloadsIndex()
                        .getContainingAltBlocks(atv_id);
  if (alt_hashes.empty()) {
    return nullptr;
  }
  altintegration::WriteStream stream;
  altintegration::writeContainer(
      stream, alt_hashes, altintegration::writeSingleByteLenValue);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_alt_getVTBContainingBlock(PopContext* self,
                                              const uint8_t* p_id,
                                              int p_id_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(p_id);
  VBK_ASSERT(self->context);

  std::vector<uint8_t> vtb_id{p_id, p_id + p_id_size};
  auto alt_hashes = self->context->getAltBlockTree()
                        .getPayloadsIndex()
                        .getContainingAltBlocks(vtb_id);
  if (alt_hashes.empty()) {
    return nullptr;
  }
  altintegration::WriteStream stream;
  altintegration::writeContainer(
      stream, alt_hashes, altintegration::writeSingleByteLenValue);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_alt_getVbkBlockContainingBlock(PopContext* self,
                                                   const uint8_t* p_id,
                                                   int p_id_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(p_id);
  VBK_ASSERT(self->context);

  std::vector<uint8_t> vbkblock_id{p_id, p_id + p_id_size};
  auto alt_hashes = self->context->getAltBlockTree()
                        .getPayloadsIndex()
                        .getContainingAltBlocks(vbkblock_id);
  if (alt_hashes.empty()) {
    return nullptr;
  }
  altintegration::WriteStream stream;
  altintegration::writeContainer(
      stream, alt_hashes, altintegration::writeSingleByteLenValue);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_vbk_getVTBContainingBlock(PopContext* self,
                                              const uint8_t* p_id,
                                              int p_id_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(p_id);
  VBK_ASSERT(self->context);

  std::vector<uint8_t> vtb_id{p_id, p_id + p_id_size};
  auto vbk_hashes = self->context->getAltBlockTree()
                        .getPayloadsIndex()
                        .getContainingVbkBlocks(vtb_id);
  if (vbk_hashes.empty()) {
    return nullptr;
  }
  altintegration::WriteStream stream;
  altintegration::writeContainer(
      stream, vbk_hashes, altintegration::writeSingleByteLenValue);
  return new VbkByteStream(stream.data());
}

static int handleSubmitResponse(altintegration::MemPool::SubmitResult e) {
  using S = altintegration::MemPool::Status;
  switch (e.status) {
    case S::VALID:
      return 0;
    case S::FAILED_STATEFUL:
      return 1;
    case S::FAILED_STATELESS:
      return 2;
  }

  VBK_ASSERT_MSG(false, "Unhandled case");
}

int VBK_MemPool_submit_atv(PopContext* self,
                           const uint8_t* bytes,
                           int bytes_size,
                           VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(bytes);
  VBK_ASSERT(self->context);

  altintegration::Slice<const uint8_t> atv_bytes(bytes, bytes_size);
  auto r = self->context->getMemPool().submit<altintegration::ATV>(
      atv_bytes, state->getState());
  return handleSubmitResponse(r);
}

int VBK_MemPool_submit_vtb(PopContext* self,
                           const uint8_t* bytes,
                           int bytes_size,
                           VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(bytes);
  VBK_ASSERT(self->context);

  altintegration::Slice<const uint8_t> vtb_bytes(bytes, bytes_size);
  auto r = self->context->getMemPool().submit<altintegration::VTB>(
      vtb_bytes, state->getState());
  return handleSubmitResponse(r);
}

int VBK_MemPool_submit_vbk(PopContext* self,
                           const uint8_t* bytes,
                           int bytes_size,
                           VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(bytes);
  VBK_ASSERT(self->context);

  altintegration::Slice<const uint8_t> vbk_bytes(bytes, bytes_size);
  auto r = self->context->getMemPool().submit<altintegration::VbkBlock>(
      vbk_bytes, state->getState());
  return handleSubmitResponse(r);
}

void VBK_MemPool_getPop(PopContext* self, uint8_t* out_bytes, int* bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(out_bytes);
  VBK_ASSERT(bytes_size);
  VBK_ASSERT(self->context);

  altintegration::PopData popData =
      self->context->getMemPool().generatePopData();
  std::vector<uint8_t> bytes = popData.toVbkEncoding();

  memcpy(out_bytes, bytes.data(), bytes.size());
  *bytes_size = (int)bytes.size();
}

void VBK_MemPool_removeAll(PopContext* self,
                           const uint8_t* bytes,
                           int bytes_size,
                           VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(bytes);
  VBK_ASSERT(self->context);

  altintegration::Slice<const uint8_t> p_bytes(bytes, bytes_size);
  altintegration::ReadStream stream(p_bytes);
  altintegration::PopData popData;
  bool res = altintegration::DeserializeFromVbkEncoding(
      stream, popData, state->getState());
  VBK_ASSERT_MSG(
      res, "can not deserialize PopData, error: %s", state->GetErrorMessage());
  self->context->getMemPool().removeAll(popData);
}

VBK_ByteStream* VBK_MemPool_GetATV(PopContext* self,
                                   const uint8_t* id_bytes,
                                   int id_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(id_bytes);
  VBK_ASSERT(self->context);

  auto atv_id = altintegration::ATV::id_t(
      altintegration::Slice<const uint8_t>(id_bytes, id_bytes_size));
  auto* atv = self->context->getMemPool().get<altintegration::ATV>(atv_id);
  if (atv == nullptr) {
    return nullptr;
  }
  altintegration::WriteStream stream;
  atv->toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_MemPool_GetVTB(PopContext* self,
                                   const uint8_t* id_bytes,
                                   int id_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(id_bytes);
  VBK_ASSERT(self->context);

  auto vtb_id = altintegration::ATV::id_t(
      altintegration::Slice<const uint8_t>(id_bytes, id_bytes_size));
  auto* vtb = self->context->getMemPool().get<altintegration::VTB>(vtb_id);
  if (vtb == nullptr) {
    return nullptr;
  }
  altintegration::WriteStream stream;
  vtb->toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_MemPool_GetVbkBlock(PopContext* self,
                                        const uint8_t* id_bytes,
                                        int id_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(id_bytes);
  VBK_ASSERT(self->context);

  auto vbk_id = altintegration::VbkBlock::id_t(
      altintegration::Slice<const uint8_t>(id_bytes, id_bytes_size));
  auto* vbk = self->context->getMemPool().get<altintegration::VbkBlock>(vbk_id);
  if (vbk == nullptr) {
    return nullptr;
  }
  altintegration::WriteStream stream;
  vbk->toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_MemPool_GetATVs(PopContext* self) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);

  auto atvs = self->context->getMemPool().getMap<altintegration::ATV>();

  std::vector<altintegration::ATV::id_t> atv_ids;
  atv_ids.reserve(atvs.size());
  for (const auto& el : atvs) {
    atv_ids.push_back(el.first);
  }

  altintegration::WriteStream stream;
  altintegration::writeArrayOf<altintegration::ATV::id_t>(
      stream, atv_ids, altintegration::writeSingleByteLenValue);

  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_MemPool_GetVTBs(PopContext* self) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);

  auto vtbs = self->context->getMemPool().getMap<altintegration::VTB>();

  std::vector<altintegration::VTB::id_t> vtb_ids;
  vtb_ids.reserve(vtbs.size());
  for (const auto& el : vtbs) {
    vtb_ids.push_back(el.first);
  }

  altintegration::WriteStream stream;
  altintegration::writeArrayOf<altintegration::VTB::id_t>(
      stream, vtb_ids, altintegration::writeSingleByteLenValue);

  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_MemPool_GetVbkBlocks(PopContext* self) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);

  auto vbks = self->context->getMemPool().getMap<altintegration::VbkBlock>();

  std::vector<altintegration::VbkBlock::id_t> vbk_ids;
  vbk_ids.reserve(vbks.size());
  for (const auto& el : vbks) {
    vbk_ids.push_back(el.first);
  }

  altintegration::WriteStream stream;
  altintegration::writeArrayOf<altintegration::VbkBlock::id_t>(
      stream, vbk_ids, altintegration::writeSingleByteLenValue);

  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_MemPool_GetATVsInFlight(PopContext* self) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);

  auto atvs = self->context->getMemPool().getInFlightMap<altintegration::ATV>();

  std::vector<altintegration::ATV::id_t> atv_ids;
  atv_ids.reserve(atvs.size());
  for (const auto& el : atvs) {
    atv_ids.push_back(el.first);
  }

  altintegration::WriteStream stream;
  altintegration::writeArrayOf<altintegration::ATV::id_t>(
      stream, atv_ids, altintegration::writeSingleByteLenValue);

  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_MemPool_GetVTBsInFlight(PopContext* self) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context)

  auto vtbs = self->context->getMemPool().getInFlightMap<altintegration::VTB>();

  std::vector<altintegration::VTB::id_t> vtb_ids;
  vtb_ids.reserve(vtbs.size());
  for (const auto& el : vtbs) {
    vtb_ids.push_back(el.first);
  }

  altintegration::WriteStream stream;
  altintegration::writeArrayOf<altintegration::VTB::id_t>(
      stream, vtb_ids, altintegration::writeSingleByteLenValue);

  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_MemPool_GetVbkBlocksInFlight(PopContext* self) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);

  auto vbks =
      self->context->getMemPool().getInFlightMap<altintegration::VbkBlock>();

  std::vector<altintegration::VbkBlock::id_t> vbk_ids;
  vbk_ids.reserve(vbks.size());
  for (const auto& el : vbks) {
    vbk_ids.push_back(el.first);
  }

  altintegration::WriteStream stream;
  altintegration::writeArrayOf<altintegration::VbkBlock::id_t>(
      stream, vbk_ids, altintegration::writeSingleByteLenValue);

  return new VbkByteStream(stream.data());
}

void VBK_MemPool_clear(PopContext* self) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);

  self->context->getMemPool().clear();
}
