// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "e2e/e2e_utils.hpp"

#include <gtest/gtest.h>

#include "pop/util/pop_test_fixture.hpp"

using namespace altintegration;
using namespace testing_utils;

struct E2E_Utils : public PopTestFixture, public ::testing::Test {
  E2EState e2e;

  E2E_Utils()
      : PopTestFixture(), e2e(this->altparam, this->vbkparam, this->btcparam) {}
};

TEST_F(E2E_Utils, submit_vbk) {
  auto fork = GetRandomOption<ForkOption>();

  this->e2e.createAction(CreateOption::CREATE_VBK, fork, this->alttree);

  EXPECT_EQ(this->mempool.getMap<VbkBlock>().size() +
                this->mempool.getInFlightMap<VbkBlock>().size(),
            0);

  this->e2e.submitAction(
      SubmitOption::SUBMIT_VBK, this->mempool, this->alttree);

  EXPECT_EQ(this->mempool.getMap<VbkBlock>().size() +
                this->mempool.getInFlightMap<VbkBlock>().size(),
            1);

  EXPECT_EQ(this->e2e.getStats().created_vbk, (uint32_t)1);
}

TEST_F(E2E_Utils, submit_vtb) {
  auto fork = GetRandomOption<ForkOption>();

  this->e2e.createAction(CreateOption::CREATE_BTC_TX, fork, this->alttree);
  this->e2e.createAction(CreateOption::CREATE_BTC, fork, this->alttree);
  this->e2e.createAction(CreateOption::CREATE_VBK_POP_TX, fork, this->alttree);
  this->e2e.createAction(CreateOption::CREATE_VBK, fork, this->alttree);

  EXPECT_EQ(this->mempool.getMap<VTB>().size() +
                this->mempool.getInFlightMap<VTB>().size(),
            0);

  this->e2e.submitAction(
      SubmitOption::SUBMIT_VTB, this->mempool, this->alttree);

  EXPECT_EQ(this->mempool.getMap<VTB>().size() +
                this->mempool.getInFlightMap<VTB>().size(),
            1);

  EXPECT_EQ(this->e2e.getStats().created_vbk, (uint32_t)1);
  EXPECT_EQ(this->e2e.getStats().created_vbk_pop_tx, (uint32_t)1);
  EXPECT_EQ(this->e2e.getStats().created_btc, (uint32_t)1);
  EXPECT_EQ(this->e2e.getStats().created_btc_tx, (uint32_t)1);
}

TEST_F(E2E_Utils, submit_atv) {
  auto fork = GetRandomOption<ForkOption>();

  this->e2e.createAction(CreateOption::CREATE_VBK_TX, fork, this->alttree);
  this->e2e.createAction(CreateOption::CREATE_VBK, fork, this->alttree);

  EXPECT_EQ(this->mempool.getMap<ATV>().size() +
                this->mempool.getInFlightMap<ATV>().size(),
            0);

  this->e2e.submitAction(
      SubmitOption::SUBMIT_ATV, this->mempool, this->alttree);

  EXPECT_EQ(this->mempool.getMap<ATV>().size() +
                this->mempool.getInFlightMap<ATV>().size(),
            1);

  EXPECT_EQ(this->e2e.getStats().created_vbk, (uint32_t)1);
  EXPECT_EQ(this->e2e.getStats().created_vbk_tx, (uint32_t)1);
}

TEST_F(E2E_Utils, submit_alt) {
  auto fork = GetRandomOption<ForkOption>();

  this->e2e.createAction(CreateOption::CREATE_ALT, fork, this->alttree);

  EXPECT_EQ(this->alttree.getAllBlocks().size(), (uint32_t)1);

  this->e2e.submitAction(
      SubmitOption::SUBMIT_ALT, this->mempool, this->alttree);

  EXPECT_EQ(this->alttree.getAllBlocks().size(), (size_t)2);

  EXPECT_EQ(this->e2e.getStats().created_alt, (uint32_t)1);
}