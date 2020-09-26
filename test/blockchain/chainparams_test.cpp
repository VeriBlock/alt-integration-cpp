// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <util/pop_test_fixture.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/stateless_validation.hpp>

using namespace altintegration;

TEST(ParamsTestBTC, GetGenesisBlock) {
  BtcChainParamsRegTest param;
  BtcBlock block;
  ASSERT_NO_FATAL_FAILURE({ block = GetRegTestBtcBlock(); });
  ASSERT_TRUE(checkProofOfWork(block, param)) << "bad POW of genesis block";
}

TEST(ParamsTestVBK, GetGenesisBlock) {
  VbkChainParamsRegTest param;
  VbkBlock block;
  ASSERT_NO_FATAL_FAILURE({ block = GetRegTestVbkBlock(); });
  ASSERT_TRUE(checkProofOfWork(block, param)) << "bad POW of genesis block";
}