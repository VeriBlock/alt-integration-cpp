// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include "veriblock/c/pop_context.h"

struct PopContextFixture : public ::testing::Test {
  ~PopContextFixture() {
    VBK_FreePopContext(pop_context);
    VBK_FreeConfig(config);
  }

  PopContextFixture() {
    config = VBK_NewConfig();

    std::string vbk_params = "regtest";
    std::string btc_params = "regtest";

    VBK_SelectVbkParams(config, vbk_params.data(), 0, nullptr);
    VBK_SelectBtcParams(config, btc_params.data(), 0, nullptr);
  }

 private:
  Config_t* config;
  PopContext* pop_context;
};

TEST_F(PopContextFixture, basic_test) {}