// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/consts.hpp>
#include <veriblock/crypto/progpow.hpp>
#include <veriblock/crypto/veriblock.hpp>

namespace altintegration {

VbkBlock::hash_t getVbkBlockHash(Slice<const uint8_t> vbkblock) {
  VBK_ASSERT(vbkblock.size() == VBK_HEADER_SIZE_PROGPOW);
  return {};
}

}