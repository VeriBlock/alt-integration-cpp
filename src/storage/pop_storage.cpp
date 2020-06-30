// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/storage/pop_storage.hpp>

namespace altintegration {

template <>
BlockRepository<BlockIndex<BtcBlock>>& PopStorage::getBlockRepo() {
  return *_brepoBtc;
}
template <>
const BlockRepository<BlockIndex<BtcBlock>>& PopStorage::getBlockRepo() const {
  return *_brepoBtc;
}

template <>
BlockRepository<BlockIndex<VbkBlock>>& PopStorage::getBlockRepo() {
  return *_brepoVbk;
}
template <>
const BlockRepository<BlockIndex<VbkBlock>>& PopStorage::getBlockRepo() const {
  return *_brepoVbk;
}

template <>
BlockRepository<BlockIndex<AltBlock>>& PopStorage::getBlockRepo() {
  return *_brepoAlt;
}
template <>
const BlockRepository<BlockIndex<AltBlock>>& PopStorage::getBlockRepo() const {
  return *_brepoAlt;
}

template <>
TipsRepository<BlockIndex<BtcBlock>>& PopStorage::getTipsRepo() {
  return *_trepoBtc;
}
template <>
const TipsRepository<BlockIndex<BtcBlock>>& PopStorage::getTipsRepo() const {
  return *_trepoBtc;
}

template <>
TipsRepository<BlockIndex<VbkBlock>>& PopStorage::getTipsRepo() {
  return *_trepoVbk;
}
template <>
const TipsRepository<BlockIndex<VbkBlock>>& PopStorage::getTipsRepo() const {
  return *_trepoVbk;
}

template <>
TipsRepository<BlockIndex<AltBlock>>& PopStorage::getTipsRepo() {
  return *_trepoAlt;
}
template <>
const TipsRepository<BlockIndex<AltBlock>>& PopStorage::getTipsRepo() const {
  return *_trepoAlt;
}

template <>
PayloadsRepository<VbkEndorsement>& PopStorage::getEndorsementsRepo() {
  return *_erepoVbk;
}
template <>
const PayloadsRepository<VbkEndorsement>& PopStorage::getEndorsementsRepo()
    const {
  return *_erepoVbk;
}

template <>
PayloadsRepository<AltEndorsement>& PopStorage::getEndorsementsRepo() {
  return *_erepoAlt;
}
template <>
const PayloadsRepository<AltEndorsement>& PopStorage::getEndorsementsRepo()
    const {
  return *_erepoAlt;
}

template <>
void PopStorage::processSaveBlock(const BlockIndex<BtcBlock>&) {}

template <>
void PopStorage::processSaveBlock(const BlockIndex<VbkBlock>& b) {
  for (const auto& e : b.containingEndorsements) {
    saveEndorsements<typename BlockIndex<VbkBlock>::endorsement_t>(*e.second);
  }
}

template <>
void PopStorage::processSaveBlock(const BlockIndex<AltBlock>& b) {
  for (const auto& e : b.containingEndorsements) {
    saveEndorsements<typename BlockIndex<AltBlock>::endorsement_t>(*e.second);
  }
}

}  // namespace altintegration
