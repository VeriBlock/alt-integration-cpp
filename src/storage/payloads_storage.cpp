// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/algorithm.hpp>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/storage/payloads_storage.hpp>

namespace altintegration {

bool PayloadsStorage::getValidity(Slice<const uint8_t> containingBlock,
                                  Slice<const uint8_t> payloadId) {
  auto key = makeGlobalPid(containingBlock, payloadId);
  auto it = _cgValidity.find(key);
  if (it == _cgValidity.end()) {
    // we don't know if this payload is invalid, so assume it is valid
    return true;
  }

  return it->second;
}

Repository& PayloadsStorage::getRepo() { return *repo_; }

const Repository& PayloadsStorage::getRepo() const { return *repo_; }

void PayloadsStorage::setValidity(Slice<const uint8_t> containingBlock,
                                  Slice<const uint8_t> payloadId,
                                  bool validity) {
  auto key = makeGlobalPid(containingBlock, payloadId);
  if (!validity) {
    _cgValidity[key] = validity;
    return;
  }

  auto it = _cgValidity.find(key);
  if (it != _cgValidity.end()) {
    // this saves some memory, because we assume that
    // anything that is not in this map is valid by default
    _cgValidity.erase(it);
  }

  // do nothing. any entry that is not in this map is valid by default
}

const std::set<AltBlock::hash_t>& PayloadsStorage::getContainingAltBlocks(
    const std::vector<uint8_t>& payloadId) {
  static const std::set<AltBlock::hash_t> empty;
  auto it = payload_in_alt.find(payloadId);
  if (it == payload_in_alt.end()) {
    return empty;
  }

  return it->second;
}

const std::set<VbkBlock::hash_t>& PayloadsStorage::getContainingVbkBlocks(
    const std::vector<uint8_t>& payloadId) {
  static const std::set<VbkBlock::hash_t> empty;
  auto it = payload_in_vbk.find(payloadId);
  if (it == payload_in_vbk.end()) {
    return empty;
  }

  return it->second;
}

void PayloadsStorage::addBlockToIndex(const BlockIndex<AltBlock>& block) {
  auto containing = block.getHash();
  for (auto& pid : block.getPayloadIds<VbkBlock>()) {
    this->addAltPayloadIndex(containing, pid.asVector());
  }
  for (auto& pid : block.getPayloadIds<VTB>()) {
    this->addAltPayloadIndex(containing, pid.asVector());
  }
  for (auto& pid : block.getPayloadIds<ATV>()) {
    this->addAltPayloadIndex(containing, pid.asVector());
  }
}

void PayloadsStorage::addBlockToIndex(const BlockIndex<VbkBlock>& block) {
  auto containing = block.getHash();
  for (auto& pid : block.getPayloadIds<VTB>()) {
    this->addVbkPayloadIndex(containing, pid.asVector());
  }
}

void PayloadsStorage::addAltPayloadIndex(
    const AltBlock::hash_t& containing, const std::vector<uint8_t>& payloadId) {
  payload_in_alt[payloadId].insert(containing);
}
void PayloadsStorage::addVbkPayloadIndex(
    const VbkBlock::hash_t& containing, const std::vector<uint8_t>& payloadId) {
  payload_in_vbk[payloadId].insert(containing);
}

void PayloadsStorage::removeAltPayloadIndex(
    const AltBlock::hash_t& containing, const std::vector<uint8_t>& payloadId) {
  payload_in_alt[payloadId].erase(containing);
}

void PayloadsStorage::removeVbkPayloadIndex(
    const VbkBlock::hash_t& containing, const std::vector<uint8_t>& payloadId) {
  payload_in_vbk[payloadId].erase(containing);
}

void PayloadsStorage::removePayloadsIndex(const BlockIndex<AltBlock>& block) {
  auto containingHash = block.getHash();
  for (auto& c : block.getPayloadIds<VbkBlock>()) {
    removeAltPayloadIndex(containingHash, c.asVector());
  }
  for (auto& c : block.getPayloadIds<VTB>()) {
    removeAltPayloadIndex(containingHash, c.asVector());
  }
  for (auto& c : block.getPayloadIds<ATV>()) {
    removeAltPayloadIndex(containingHash, c.asVector());
  }
}

void PayloadsStorage::removePayloadsIndex(const BlockIndex<VbkBlock>& block) {
  auto containingHash = block.getHash();
  for (auto& c : block.getPayloadIds<VTB>()) {
    removeVbkPayloadIndex(containingHash, c.asVector());
  }
}

void PayloadsStorage::reindex(const AltTree& tree) {
  payload_in_alt.clear();
  payload_in_vbk.clear();

  // reindex vbk blocks
  auto& vbkblocks = tree.vbk().getBlocks();
  VBK_LOG_WARN("Reindexing %d VBK blocks...", vbkblocks.size());
  for (auto& b : vbkblocks) {
    addBlockToIndex(*b.second);
  }

  // reindex alt blocks
  auto& altblocks = tree.getBlocks();
  VBK_LOG_WARN("Reindexing %d ALT blocks...", altblocks.size());
  for (auto& b : altblocks) {
    addBlockToIndex(*b.second);
  }
  VBK_LOG_WARN("Reindexing finished");
}

PayloadsStorage::PayloadsStorage(std::shared_ptr<Repository> repo)
    : repo_(std::move(repo)) {}

std::vector<uint8_t> PayloadsStorage::makeGlobalPid(Slice<const uint8_t> a,
                                                    Slice<const uint8_t> b) {
  std::vector<uint8_t> key;
  key.reserve(a.size() + b.size());
  key.insert(key.end(), a.begin(), a.end());
  key.insert(key.end(), b.begin(), b.end());
  VBK_ASSERT(key.size() == a.size() + b.size());
  return key;
}

void PayloadsStorage::savePayloads(const PopData& pop) {
  auto batch = repo_->newBatch();
  for (const auto& b : pop.context) {
    batch->putObject(std::make_pair(DB_VBK_PREFIX, b.getId()), b);
  }
  for (const auto& b : pop.vtbs) {
    batch->putObject(std::make_pair(DB_VTB_PREFIX, b.getId()), b);
  }
  for (const auto& b : pop.atvs) {
    batch->putObject(std::make_pair(DB_ATV_PREFIX, b.getId()), b);
  }
  batch->commit();
}

void PayloadsStorage::savePayloads(const std::vector<VTB>& vtbs) {
  auto batch = repo_->newBatch();
  for (auto& b : vtbs) {
    batch->putObject(std::make_pair(DB_VTB_PREFIX, b.getId()), b);
  }
  batch->commit();
}

}  // namespace altintegration
