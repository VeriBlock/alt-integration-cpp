// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/algorithm.hpp>
#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/storage/payloads_index.hpp>

namespace altintegration {

// bool PayloadsIndex::getValidity(Slice<const uint8_t> containingBlockHash,
//                                 Slice<const uint8_t> payloadId) const {
//   auto key = makeGlobalPid(containingBlockHash, payloadId);
//   auto it = _cgValidity.find(key);
//   if (it == _cgValidity.end()) {
//     // we don't know if this payload is invalid, so assume it is valid
//     return true;
//   }

//   return it->second;
// }

// void PayloadsIndex::setValidity(Slice<const uint8_t> containingBlockHash,
//                                 Slice<const uint8_t> payloadId,
//                                 bool validity) {
//   auto key = makeGlobalPid(containingBlockHash, payloadId);
//   if (!validity) {
//     _cgValidity[key] = validity;
//     return;
//   }

//   auto it = _cgValidity.find(key);
//   if (it != _cgValidity.end()) {
//     // this saves some memory, because we assume that
//     // anything that is not in this map is valid by default
//     _cgValidity.erase(it);
//   }

//   // do nothing. any entry that is not in this map is valid by default
// }

const std::set<AltBlock::hash_t>& PayloadsIndex::getContainingAltBlocks(
    const std::vector<uint8_t>& payloadId) const {
  static const std::set<AltBlock::hash_t> empty;
  auto it = payload_in_alt.find(payloadId);
  if (it == payload_in_alt.end()) {
    return empty;
  }

  return it->second;
}

const std::set<VbkBlock::hash_t>& PayloadsIndex::getContainingVbkBlocks(
    const std::vector<uint8_t>& payloadId) const {
  static const std::set<VbkBlock::hash_t> empty;
  auto it = payload_in_vbk.find(payloadId);
  if (it == payload_in_vbk.end()) {
    return empty;
  }

  return it->second;
}

void PayloadsIndex::addBlockToIndex(const BlockIndex<AltBlock>& block) {
  const auto& containing = block.getHash();
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

void PayloadsIndex::addBlockToIndex(const BlockIndex<VbkBlock>& block) {
  const auto& containing = block.getHash();
  for (auto& pid : block.getPayloadIds<VTB>()) {
    this->addVbkPayloadIndex(containing, pid.asVector());
  }
}

void PayloadsIndex::addAltPayloadIndex(const AltBlock::hash_t& containing,
                                       const std::vector<uint8_t>& payloadId) {
  payload_in_alt[payloadId].insert(containing);
}
void PayloadsIndex::addVbkPayloadIndex(const VbkBlock::hash_t& containing,
                                       const std::vector<uint8_t>& payloadId) {
  payload_in_vbk[payloadId].insert(containing);
}

void PayloadsIndex::removeAltPayloadIndex(
    const AltBlock::hash_t& containing, const std::vector<uint8_t>& payloadId) {
  payload_in_alt[payloadId].erase(containing);
}

void PayloadsIndex::removeVbkPayloadIndex(
    const VbkBlock::hash_t& containing, const std::vector<uint8_t>& payloadId) {
  payload_in_vbk[payloadId].erase(containing);
}

void PayloadsIndex::removePayloadsIndex(const BlockIndex<AltBlock>& block) {
  const auto& containingHash = block.getHash();
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

void PayloadsIndex::removePayloadsIndex(const BlockIndex<VbkBlock>& block) {
  const auto& containingHash = block.getHash();
  for (auto& c : block.getPayloadIds<VTB>()) {
    removeVbkPayloadIndex(containingHash, c.asVector());
  }
}

void PayloadsIndex::reindex(const AltBlockTree& tree) {
  payload_in_alt.clear();
  payload_in_vbk.clear();

  // reindex vbk blocks
  auto vbkblocks = tree.vbk().getBlocks();
  VBK_LOG_WARN("Reindexing %d VBK blocks...", vbkblocks.size());
  for (auto& b : vbkblocks) {
    addBlockToIndex(*b);
  }

  // reindex alt blocks
  auto altblocks = tree.getBlocks();
  VBK_LOG_WARN("Reindexing %d ALT blocks...", altblocks.size());
  for (auto& b : altblocks) {
    addBlockToIndex(*b);
  }
  VBK_LOG_WARN("Reindexing finished");
}

const std::unordered_map<std::vector<uint8_t>, std::set<AltBlock::hash_t>>&
PayloadsIndex::getPayloadsInAlt() const {
  return payload_in_alt;
}

const std::unordered_map<std::vector<uint8_t>, std::set<VbkBlock::hash_t>>&
PayloadsIndex::getPayloadsInVbk() const {
  return payload_in_vbk;
}

// const std::unordered_map<std::vector<uint8_t>, bool>&
// PayloadsIndex::getValidity() const {
//   return _cgValidity;
// }

}  // namespace altintegration
