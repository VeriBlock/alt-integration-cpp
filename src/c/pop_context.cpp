// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include "bytestream.hpp"
#include "config.hpp"
#include "pop_context.hpp"
#include "veriblock/blockchain/alt_block_tree.hpp"
#include "veriblock/c/extern.h"
#include "veriblock/c/pop_context.h"
#include "veriblock/pop_context.hpp"

struct PayloadsProviderImpl : public altintegration::PayloadsProvider {
  ~PayloadsProviderImpl() override = default;

  PayloadsProviderImpl(size_t maxSize) { buffer.resize(maxSize); }

  bool getATVs(const std::vector<altintegration::ATV::id_t>& ids,
               std::vector<altintegration::ATV>& out,
               altintegration::ValidationState& state) override {
    for (const auto& id : ids) {
      int size = 0;
      VBK_getATV(id.data(), (int)id.size(), buffer.data(), &size);
      altintegration::Slice<const uint8_t> bytes(buffer.data(), size);
      altintegration::ReadStream stream(bytes);
      altintegration::ATV atv_out;
      if (!altintegration::Deserialize(stream, atv_out, state)) {
        return state.Invalid("get-atv");
      }
      out.push_back(atv_out);
    }

    return true;
  }

  bool getVTBs(const std::vector<altintegration::VTB::id_t>& ids,
               std::vector<altintegration::VTB>& out,
               altintegration::ValidationState& state) override {
    for (const auto& id : ids) {
      int size = 0;
      VBK_getVTB(id.data(), (int)id.size(), buffer.data(), &size);
      altintegration::Slice<const uint8_t> bytes(buffer.data(), size);
      altintegration::ReadStream stream(bytes);
      altintegration::VTB vtb_out;
      if (!altintegration::Deserialize(stream, vtb_out, state)) {
        return state.Invalid("get-atv");
      }
      out.push_back(vtb_out);
    }

    return true;
  }

  bool getVBKs(const std::vector<altintegration::VbkBlock::id_t>& ids,
               std::vector<altintegration::VbkBlock>& out,
               altintegration::ValidationState& state) override {
    for (const auto& id : ids) {
      int size = 0;
      VBK_getVBK(id.data(), (int)id.size(), buffer.data(), &size);
      altintegration::Slice<const uint8_t> bytes(buffer.data(), size);
      altintegration::ReadStream stream(bytes);
      altintegration::VbkBlock vbk_out;
      if (!altintegration::Deserialize(stream, vbk_out, state)) {
        return state.Invalid("get-atv");
      }
      out.push_back(vbk_out);
    }

    return true;
  }

 private:
  std::vector<uint8_t> buffer;
};

PopContext* VBK_NewPopContext(Config_t* config) {
  VBK_ASSERT(config);
  VBK_ASSERT(config->config);
  auto& c = config->config;

  VBK_ASSERT(c->alt);
  auto maxPopDataSize = c->alt->getMaxPopDataSize();
  auto* v = new PopContext();
  // maxPopDataSize is the maximum size of payload per block, it is safe
  // to allocate buffer with this size for all operations
  v->provider = std::make_shared<PayloadsProviderImpl>(maxPopDataSize);
  v->context = altintegration::PopContext::create(c, v->provider);

  return v;
}

void VBK_FreePopContext(PopContext* app) {
  if (app) {
    delete app;
  }
}

bool VBK_AltBlockTree_acceptBlockHeader(PopContext* self,
                                        const uint8_t* block_bytes,
                                        int bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(block_bytes);

  altintegration::ValidationState state;
  altintegration::Slice<const uint8_t> bytes(block_bytes, bytes_size);
  altintegration::ReadStream stream(bytes);
  altintegration::AltBlock blk;

  if (!altintegration::Deserialize(stream, blk, state)) {
    return false;
  }

  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);
  if (!self->context->altTree->acceptBlockHeader(blk, state)) {
    return false;
  }

  return true;
}

void VBK_AltBlockTree_acceptBlock(PopContext* self,
                                  const uint8_t* hash_bytes,
                                  int hash_bytes_size,
                                  const uint8_t* payloads_bytes,
                                  int payloads_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(payloads_bytes);

  altintegration::ValidationState state;
  altintegration::Slice<const uint8_t> p_bytes(payloads_bytes,
                                               payloads_bytes_size);
  altintegration::ReadStream stream(p_bytes);
  altintegration::PopData popData;
  bool res = altintegration::Deserialize(stream, popData, state);
  VBK_ASSERT_MSG(
      res, "can not deserialize PopData, error: %s", state.toString());

  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);

  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);
  self->context->altTree->acceptBlock(hash, popData);
}

bool VBK_AltBlockTree_addPayloads(PopContext* self,
                                  const uint8_t* hash_bytes,
                                  int hash_bytes_size,
                                  const uint8_t* payloads_bytes,
                                  int payloads_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(payloads_bytes);

  altintegration::ValidationState state;
  altintegration::Slice<const uint8_t> p_bytes(payloads_bytes,
                                               payloads_bytes_size);
  altintegration::ReadStream stream(p_bytes);
  altintegration::PopData popData;
  if (!altintegration::Deserialize(stream, popData, state)) {
    return false;
  }

  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);

  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);
  return self->context->altTree->addPayloads(hash, popData, state);
}

bool VBK_AltBlockTree_loadTip(PopContext* self,
                              const uint8_t* hash_bytes,
                              int hash_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash_bytes);
  altintegration::ValidationState state;
  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);
  return self->context->altTree->loadTip(hash, state);
}

int VBK_AltBlockTree_comparePopScore(PopContext* self,
                                     const uint8_t* A_hash_bytes,
                                     int A_hash_bytes_size,
                                     const uint8_t* B_hash_bytes,
                                     int B_hash_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(A_hash_bytes);
  VBK_ASSERT(B_hash_bytes);
  std::vector<uint8_t> A_hash(A_hash_bytes, A_hash_bytes + A_hash_bytes_size);
  std::vector<uint8_t> B_hash(B_hash_bytes, B_hash_bytes + B_hash_bytes_size);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);
  return self->context->altTree->comparePopScore(A_hash, B_hash);
}

void VBK_AltBlockTree_removeSubtree(PopContext* self,
                                    const uint8_t* hash_bytes,
                                    int hash_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash_bytes);
  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);
  self->context->altTree->removeSubtree(hash);
}

bool VBK_AltBlockTree_setState(PopContext* self,
                               const uint8_t* hash_bytes,
                               int hash_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash_bytes);
  altintegration::ValidationState state;
  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);
  return self->context->altTree->setState(hash, state);
}

bool VBK_btc_getBlockIndex(PopContext* self,
                           const uint8_t* hash_bytes,
                           int hash_bytes_size,
                           uint8_t** blockindex,
                           int* blockindex_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(blockindex);
  VBK_ASSERT(blockindex_size);
  altintegration::Slice<const uint8_t> hash(hash_bytes, hash_bytes_size);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);
  auto* blockIndex = self->context->altTree->btc().getBlockIndex(
      altintegration::BtcBlock::hash_t(hash));
  if (blockIndex == nullptr) {
    return false;
  }

  std::vector<uint8_t> bytes = blockIndex->toRaw();
  *blockindex = new uint8_t[bytes.size()];
  memcpy(*blockindex, bytes.data(), bytes.size());
  *blockindex_size = (int)bytes.size();

  return true;
}

bool VBK_vbk_getBlockIndex(PopContext* self,
                           const uint8_t* hash_bytes,
                           int hash_bytes_size,
                           uint8_t** blockindex,
                           int* blockindex_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(blockindex);
  VBK_ASSERT(blockindex_size);
  altintegration::Slice<const uint8_t> hash(hash_bytes, hash_bytes_size);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);
  auto* blockIndex = self->context->altTree->vbk().getBlockIndex(
      altintegration::VbkBlock::hash_t(hash));
  if (blockIndex == nullptr) {
    return false;
  }

  std::vector<uint8_t> bytes = blockIndex->toRaw();
  *blockindex = new uint8_t[bytes.size()];
  memcpy(*blockindex, bytes.data(), bytes.size());
  *blockindex_size = (int)bytes.size();

  return true;
}

bool VBK_alt_getBlockIndex(PopContext* self,
                           const uint8_t* hash_bytes,
                           int hash_bytes_size,
                           uint8_t** blockindex,
                           int* blockindex_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(hash_bytes);
  VBK_ASSERT(blockindex);
  VBK_ASSERT(blockindex_size);
  std::vector<uint8_t> hash(hash_bytes, hash_bytes + hash_bytes_size);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->altTree);
  auto* blockIndex = self->context->altTree->getBlockIndex(hash);
  if (blockIndex == nullptr) {
    return false;
  }

  std::vector<uint8_t> bytes = blockIndex->toRaw();
  *blockindex = new uint8_t[bytes.size()];
  memcpy(*blockindex, bytes.data(), bytes.size());
  *blockindex_size = (int)bytes.size();

  return true;
}

VBK_ByteStream* VBK_alt_BestBlock(PopContext* self) {
  VBK_ASSERT(self);
  auto* tip = self->context->altTree->getBestChain().tip();
  VBK_ASSERT(tip);
  altintegration::WriteStream stream;
  tip->toRaw(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_vbk_BestBlock(PopContext* self) {
  VBK_ASSERT(self);
  auto* tip = self->context->altTree->vbk().getBestChain().tip();
  VBK_ASSERT(tip);
  altintegration::WriteStream stream;
  tip->toRaw(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_btc_BestBlock(PopContext* self) {
  VBK_ASSERT(self);
  auto* tip = self->context->altTree->btc().getBestChain().tip();
  VBK_ASSERT(tip);
  altintegration::WriteStream stream;
  tip->toRaw(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_alt_BlockAtActiveChainByHeight(PopContext* self,
                                                   int height) {
  VBK_ASSERT(self);
  auto* block = self->context->altTree->getBestChain()[height];
  if (block == nullptr) {
    return new VbkByteStream(std::vector<uint8_t>{});
  }
  altintegration::WriteStream stream;
  block->toRaw(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_vbk_BlockAtActiveChainByHeight(PopContext* self,
                                                   int height) {
  VBK_ASSERT(self);
  auto* block = self->context->altTree->vbk().getBestChain()[height];
  if (block == nullptr) {
    return new VbkByteStream(std::vector<uint8_t>{});
  }
  altintegration::WriteStream stream;
  block->toRaw(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_btc_BlockAtActiveChainByHeight(PopContext* self,
                                                   int height) {
  VBK_ASSERT(self);
  auto* block = self->context->altTree->btc().getBestChain()[height];
  if (block == nullptr) {
    return new VbkByteStream(std::vector<uint8_t>{});
  }
  altintegration::WriteStream stream;
  block->toRaw(stream);
  return new VbkByteStream(stream.data());
}

static int handleSubmitResponse(altintegration::MemPool::SubmitResult e) {
  using S = altintegration::MemPool::Status;
  switch (e.status) {
    case S::ACCEPTED:
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
                           int bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(bytes);
  altintegration::ValidationState state;
  altintegration::Slice<const uint8_t> atv_bytes(bytes, bytes_size);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->mempool);
  auto r =
      self->context->mempool->submit<altintegration::ATV>(atv_bytes, state);
  return handleSubmitResponse(r);
}

int VBK_MemPool_submit_vtb(PopContext* self,
                           const uint8_t* bytes,
                           int bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(bytes);
  altintegration::ValidationState state;
  altintegration::Slice<const uint8_t> vtb_bytes(bytes, bytes_size);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->mempool);
  auto r =
      self->context->mempool->submit<altintegration::VTB>(vtb_bytes, state);
  return handleSubmitResponse(r);
}

int VBK_MemPool_submit_vbk(PopContext* self,
                           const uint8_t* bytes,
                           int bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(bytes);
  altintegration::ValidationState state;
  altintegration::Slice<const uint8_t> vbk_bytes(bytes, bytes_size);
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->mempool);
  auto r = self->context->mempool->submit<altintegration::VbkBlock>(vbk_bytes,
                                                                    state);
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
                           int bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(bytes);
  altintegration::ValidationState state;
  altintegration::Slice<const uint8_t> p_bytes(bytes, bytes_size);
  altintegration::ReadStream stream(p_bytes);
  altintegration::PopData popData;
  bool res = altintegration::Deserialize(stream, popData, state);
  VBK_ASSERT_MSG(
      res, "can not deserialize PopData, error: %s", state.toString());
  VBK_ASSERT(self->context);
  VBK_ASSERT(self->context->mempool);
  self->context->mempool->removeAll(popData);
}

VBK_ByteStream* VBK_MemPool_GetATV(PopContext* self,
                                   const uint8_t* id_bytes,
                                   int id_bytes_size) {
  VBK_ASSERT(self);
  VBK_ASSERT(id_bytes);
  auto atv_id = altintegration::ATV::id_t(
      altintegration::Slice<const uint8_t>(id_bytes, id_bytes_size));
  auto* atv = self->context->mempool->get<altintegration::ATV>(atv_id);
  if (atv == nullptr) {
    return new VbkByteStream(std::vector<uint8_t>{});
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
  auto vtb_id = altintegration::ATV::id_t(
      altintegration::Slice<const uint8_t>(id_bytes, id_bytes_size));
  auto* vtb = self->context->mempool->get<altintegration::VTB>(vtb_id);
  if (vtb == nullptr) {
    return new VbkByteStream(std::vector<uint8_t>{});
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
  auto vbk_id = altintegration::VbkBlock::id_t(
      altintegration::Slice<const uint8_t>(id_bytes, id_bytes_size));
  auto* vbk = self->context->mempool->get<altintegration::VbkBlock>(vbk_id);
  if (vbk == nullptr) {
    return new VbkByteStream(std::vector<uint8_t>{});
  }
  altintegration::WriteStream stream;
  vbk->toVbkEncoding(stream);
  return new VbkByteStream(stream.data());
}

VBK_ByteStream* VBK_MemPool_GetATVs(PopContext* self) {
  VBK_ASSERT(self);
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
