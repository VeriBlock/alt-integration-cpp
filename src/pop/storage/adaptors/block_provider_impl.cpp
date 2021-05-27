// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/pop/storage/adaptors/block_provider_impl.hpp"

namespace altintegration {

namespace adaptors {

template <>
std::vector<uint8_t> block_key<altintegration::AltBlock>(
    const altintegration::AltBlock::hash_t& hash) {
  auto res = hash;
  res.insert(res.begin(), DB_ALT_BLOCK);
  return res;
}

template <>
std::vector<uint8_t> block_key<altintegration::VbkBlock>(
    const altintegration::VbkBlock::hash_t& hash) {
  auto res = hash.asVector();
  res.insert(res.begin(), DB_VBK_BLOCK);
  return res;
}

template <>
std::vector<uint8_t> block_key<altintegration::BtcBlock>(
    const altintegration::BtcBlock::hash_t& hash) {
  auto res = hash.asVector();
  res.insert(res.begin(), DB_BTC_BLOCK);
  return res;
}

template <>
std::vector<uint8_t> tip_key<altintegration::VbkBlock>() {
  return std::vector<uint8_t>{DB_VBK_TIP, 'v', 'b', 'k', 't', 'i', 'p'};
}

template <>
std::vector<uint8_t> tip_key<altintegration::BtcBlock>() {
  return std::vector<uint8_t>{DB_BTC_TIP, 'b', 't', 'c', 't', 'i', 'p'};
}

template <>
std::vector<uint8_t> tip_key<altintegration::AltBlock>() {
  return std::vector<uint8_t>{DB_ALT_TIP, 'a', 'l', 't', 't', 'i', 'p'};
}

}  // namespace adaptors

}  // namespace altintegration