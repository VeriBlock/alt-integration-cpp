// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include "util/alt_chain_params_regtest.hpp"
#include <veriblock/crypto/progpow.hpp>

using namespace altintegration;

static BtcChainParamsRegTest btc;
static VbkChainParamsRegTest vbk;
static AltChainParamsRegTest alt;

VbkBlock makeBlock() {
  VbkBlock block;
  block.setHeight(vbk.getProgPowForkHeight() + 0);
  block.setTimestamp(vbk.getProgPowStartTimeEpoch());
  return block;
}

TEST(ProgpowHeaderCache, Warmup) {
  auto block = makeBlock();
  progpow::clearHeaderCache();

  uint192 hash = uint192::fromHex("abcde");
  auto serialized = SerializeToRaw(block);

  progpow::insertHeaderCacheEntry(serialized, hash);

  ASSERT_EQ(block.getHash(), hash);
}