// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>

#include <veriblock/pop/entities/vtb.hpp>
#include <veriblock/pop/stateless_validation.hpp>

#include "stateless_vtb_test_data.hpp"

using namespace altintegration;

struct StatelessVtbTest : public ::testing::TestWithParam<std::string> {};

TEST_P(StatelessVtbTest, validateVtbStatelessly) {
  VTB vtb = AssertDeserializeFromHex<VTB>(GetParam());
  ValidationState state;
  BtcChainParamsTest param;
  VbkChainParamsTest vbk;
  bool result = checkVTB(vtb, state, param, vbk);
  ASSERT_TRUE(result) << state.toString();
}

INSTANTIATE_TEST_SUITE_P(StatelessVtbTestRegression,
                         StatelessVtbTest,
                         testing::ValuesIn(VTBS));
