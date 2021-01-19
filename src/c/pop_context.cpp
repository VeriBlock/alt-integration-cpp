// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include "adaptors/block_provider_impl.hpp"
#include "adaptors/payloads_provider_impl.hpp"
#include "bytestream.hpp"
#include "config.hpp"
#include "pop_context.hpp"
#include "validation_state.hpp"
#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/c/extern.h"
#include "veriblock/c/pop_context.h"
#include "veriblock/consts.hpp"
#include "veriblock/pop_context.hpp"

PopContext* VBK_NewPopContext(Config_t* config) {
  VBK_ASSERT(config);
  VBK_ASSERT(config->config);
  auto& c = config->config;

  VBK_ASSERT(c->alt);
  const auto maxPopDataSize = c->getAltParams().getMaxPopDataSize();
  auto* v = new PopContext();
  // maxPopDataSize is the maximum size of payload per block, it is safe
  // to allocate buffer with this size for all operations
  v->payloads_provider =
      std::make_shared<adaptors::PayloadsProviderImpl>(maxPopDataSize);
  v->block_provider = std::make_shared<adaptors::BlockProviderImpl>();

  v->context = altintegration::PopContext::create(
      c, v->payloads_provider, v->block_provider);

  // setup signals
  v->context->mempool->onAccepted<altintegration::ATV>(
      [](const altintegration::ATV& atv) {
        auto bytes = altintegration::SerializeToVbkEncoding(atv);
        VBK_MemPool_onAcceptedATV(bytes.data(), bytes.size());
      });

  v->context->mempool->onAccepted<altintegration::VTB>(
      [](const altintegration::VTB& vtb) {
        auto bytes = altintegration::SerializeToVbkEncoding(vtb);
        VBK_MemPool_onAcceptedVTB(bytes.data(), bytes.size());
      });

  v->context->mempool->onAccepted<altintegration::VbkBlock>(
      [](const altintegration::VbkBlock& vbk) {
        auto bytes = altintegration::SerializeToVbkEncoding(vbk);
        VBK_MemPool_onAcceptedVBK(bytes.data(), bytes.size());
      });

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
  VBK_ASSERT(self->context->altTree);

  altintegration::Slice<const uint8_t> bytes(block_bytes, bytes_size);
  altintegration::ReadStream stream(bytes);
  altintegration::AltBlock blk;

  if (!altintegration::DeserializeFromRaw(stream, blk, state->getState())) {
    return false;
  }

  if (!self->context->altTree->acceptBlockHeader(blk, state->getState())) {
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
  VBK_ASSERT(self->context->altTree);

  altintegration::Slice<const uint8_t> p_bytes(payloads_bytes,
                                               payloads_bytes_size);
  altintegration::ReadStream stream(p_bytes);
  altintegration::PopData popData;
  bool res = altintegration::DeserializeFromVbkEncoding(
      stream, popData, state->getState());
  VBK_ASSERT_MSG(
      res, "can not deserialize PopData, error: %s", state->GetErrorMessage());

  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);

  self->context->altTree->acceptBlock(hash, popData);
}

bool VBK_AltBlockTree_addPayloads(PopContext* self,
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
  VBK_ASSERT(self->context->altTree);

  altintegration::Slice<const uint8_t> p_bytes(payloads_bytes,
                                               payloads_bytes_size);
  altintegration::ReadStream stream(p_bytes);
  altintegration::PopData popData;
  if (!altintegration::DeserializeFromVbkEncoding(
          stream, popData, state->getState())) {
    return false;
  }

  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);

  return self->context->altTree->addPayloads(hash, popData, state->getState());
}

bool VBK_AltBlockTree_loadTip(PopContext* self,
                              const uint8_t* hash_bytes,
                              int hash_bytes_size,
                              VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);

  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);
  return self->context->altTree->loadTip(hash, state->getState());
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
  VBK_ASSERT(self->context->altTree);
  std::vector<uint8_t> A_hash(A_hash_bytes, A_hash_bytes + A_hash_bytes_size);
  std::vector<uint8_t> B_hash(B_hash_bytes, B_hash_bytes + B_hash_bytes_size);
  return self->context->altTree->comparePopScore(A_hash, B_hash);
}

void VBK_AltBlockTree_removeSubtree(PopContext* self,
                                    const uint8_t* hash_bytes,
                                    int hash_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);
  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);
  self->context->altTree->removeSubtree(hash);
}

bool VBK_AltBlockTree_setState(PopContext* self,
                               const uint8_t* hash_bytes,
                               int hash_bytes_size,
                               VbkValidationState* state) {
  VBK_ASSERT(self);
  VBK_ASSERT(state);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);

  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);
  return self->context->altTree->setState(hash, state->getState());
}

VBK_ByteStream* VBK_AltBlockTree_getPopPayout(PopContext* self,
                                              const uint8_t* tip_hash_bytes,
                                              int tip_hash_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);

  std::vector<uint8_t> hash{tip_hash_bytes,
                            tip_hash_bytes + tip_hash_bytes_size};
  auto res = self->context->popRewardsCalculator->getPopPayout(hash);

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
  VBK_ASSERT(self->context->altTree);
  altintegration::BtcBlock::hash_t hash(
      altintegration::Slice<const uint8_t>(hash_bytes, hash_bytes_size));
  auto* blockIndex = self->context->altTree->btc().getBlockIndex(hash);
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
  VBK_ASSERT(self->context->altTree);
  altintegration::VbkBlock::hash_t hash(
      altintegration::Slice<const uint8_t>(hash_bytes, hash_bytes_size));
  auto* blockIndex = self->context->altTree->vbk().getBlockIndex(hash);
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
  VBK_ASSERT(self->context->altTree);
  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);
  auto* blockIndex = self->context->altTree->getBlockIndex(hash);
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
  VBK_ASSERT(self->context->altTree);
  auto* tip = self->context->altTree->getBestChain().tip();
  VBK_ASSERT(tip);
  altintegration::WriteStream stream;
  tip->toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_vbk_BestBlock(PopContext* self) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);
  auto* tip = self->context->altTree->vbk().getBestChain().tip();
  VBK_ASSERT(tip);
  altintegration::WriteStream stream;
  tip->toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_btc_BestBlock(PopContext* self) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);
  auto* tip = self->context->altTree->btc().getBestChain().tip();
  VBK_ASSERT(tip);
  altintegration::WriteStream stream;
  tip->toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_alt_BlockAtActiveChainByHeight(PopContext* self,
                                                   int height) {
  VBK_ASSERT(self);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);
  auto* block = self->context->altTree->getBestChain()[height];
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
  VBK_ASSERT(self->context->altTree);
  auto* block = self->context->altTree->vbk().getBestChain()[height];
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
  VBK_ASSERT(self->context->altTree);
  auto* block = self->context->altTree->btc().getBestChain()[height];
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
  VBK_ASSERT(self->context->altTree);
  std::vector<uint8_t> atv_id{p_id, p_id + p_id_size};
  auto alt_hashes =
      self->context->altTree->getPayloadsIndex().getContainingAltBlocks(atv_id);
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
  VBK_ASSERT(self->context->altTree);
  std::vector<uint8_t> vtb_id{p_id, p_id + p_id_size};
  auto alt_hashes =
      self->context->altTree->getPayloadsIndex().getContainingAltBlocks(vtb_id);
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
  VBK_ASSERT(self->context->altTree);
  std::vector<uint8_t> vbkblock_id{p_id, p_id + p_id_size};
  auto alt_hashes =
      self->context->altTree->getPayloadsIndex().getContainingAltBlocks(
          vbkblock_id);
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
  VBK_ASSERT(self->context->altTree);
  std::vector<uint8_t> vtb_id{p_id, p_id + p_id_size};
  auto vbk_hashes =
      self->context->altTree->getPayloadsIndex().getContainingVbkBlocks(vtb_id);
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
  VBK_ASSERT(self->context->mempool);

  altintegration::Slice<const uint8_t> atv_bytes(bytes, bytes_size);
  auto r = self->context->mempool->submit<altintegration::ATV>(
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
  VBK_ASSERT(self->context->mempool);

  altintegration::Slice<const uint8_t> vtb_bytes(bytes, bytes_size);
  auto r = self->context->mempool->submit<altintegration::VTB>(
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
  VBK_ASSERT(self->context->mempool);

  altintegration::Slice<const uint8_t> vbk_bytes(bytes, bytes_size);
  auto r = self->context->mempool->submit<altintegration::VbkBlock>(
      vbk_bytes, state->getState());
  return handleSubmitResponse(r);
}

void VBK_MemPool_getPop(PopContext* self, uint8_t* out_bytes, int* bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(out_bytes);
  VBK_ASSERT(bytes_size);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->mempool);
  altintegration::PopData popData = self->context->mempool->getPop();
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
  VBK_ASSERT(self->context->mempool);

  altintegration::Slice<const uint8_t> p_bytes(bytes, bytes_size);
  altintegration::ReadStream stream(p_bytes);
  altintegration::PopData popData;
  bool res = altintegration::DeserializeFromVbkEncoding(
      stream, popData, state->getState());
  VBK_ASSERT_MSG(
      res, "can not deserialize PopData, error: %s", state->GetErrorMessage());
  self->context->mempool->removeAll(popData);
}

VBK_ByteStream* VBK_MemPool_GetATV(PopContext* self,
                                   const uint8_t* id_bytes,
                                   int id_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(id_bytes);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->mempool);
  auto atv_id = altintegration::ATV::id_t(
      altintegration::Slice<const uint8_t>(id_bytes, id_bytes_size));
  auto* atv = self->context->mempool->get<altintegration::ATV>(atv_id);
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
  VBK_ASSERT(self->context->mempool);
  auto vtb_id = altintegration::ATV::id_t(
      altintegration::Slice<const uint8_t>(id_bytes, id_bytes_size));
  auto* vtb = self->context->mempool->get<altintegration::VTB>(vtb_id);
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
  VBK_ASSERT(self->context->mempool);
  auto vbk_id = altintegration::VbkBlock::id_t(
      altintegration::Slice<const uint8_t>(id_bytes, id_bytes_size));
  auto* vbk = self->context->mempool->get<altintegration::VbkBlock>(vbk_id);
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
  VBK_ASSERT(self->context->mempool);
  auto atvs = self->context->mempool->getMap<altintegration::ATV>();

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
  VBK_ASSERT(self->context->mempool);
  auto vtbs = self->context->mempool->getMap<altintegration::VTB>();

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
  VBK_ASSERT(self->context->mempool);
  auto vbks = self->context->mempool->getMap<altintegration::VbkBlock>();

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
  VBK_ASSERT(self->context->mempool);
  auto atvs = self->context->mempool->getInFlightMap<altintegration::ATV>();

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
  VBK_ASSERT(self->context->mempool);
  auto vtbs = self->context->mempool->getInFlightMap<altintegration::VTB>();

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
  VBK_ASSERT(self->context->mempool);
  auto vbks =
      self->context->mempool->getInFlightMap<altintegration::VbkBlock>();

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
  VBK_ASSERT(self->context->mempool);
  self->context->mempool->clear();
}
