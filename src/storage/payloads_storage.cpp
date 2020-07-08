// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/storage/payloads_storage.hpp>

namespace altintegration {

template <>
PayloadsRepository<ATV>& PayloadsStorage::getRepo() {
  return *_repoAtv;
}
template <>
const PayloadsRepository<ATV>& PayloadsStorage::getRepo() const {
  return *_repoAtv;
}

template <>
PayloadsRepository<VTB>& PayloadsStorage::getRepo() {
  return *_repoVtb;
}
template <>
const PayloadsRepository<VTB>& PayloadsStorage::getRepo() const {
  return *_repoVtb;
}

template <>
PayloadsRepository<VbkBlock>& PayloadsStorage::getRepo() {
  return *_repoBlocks;
}
template <>
const PayloadsRepository<VbkBlock>& PayloadsStorage::getRepo() const {
  return *_repoBlocks;
}

}  // namespace altintegration
