// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "e2e/e2e_utils.hpp"

#include <gtest/gtest.h>

#include "pop/util/pop_test_fixture.hpp"

using namespace altintegration;
using namespace testing_utils;

struct E2E_Utils : public PopTestFixture, public ::testing::Test {};

TEST_F(E2E_Utils, create_alt) {
  E2EState e2e(this->altparam, this->vbkparam, this->btcparam);

  auto fork = GetRandomOption<ForkOption>();

  e2e.createAction(CreateOption::CREATE_ALT, fork, this->alttree);
}