// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/serde.hpp>
#include <veriblock/strutil.hpp>

using namespace altintegration;

// clang-format off
static const std::vector<std::string> cases = {
    "00111d2f00111d2f0002bbf10aa156845fd76020d165945e33e58c0d04839730673f7957946c5111af7a7246d82b58ae71039bbde6b567f05fdba312041e610100034766290000020400000001010220c747a425e719bd1a6ea19b54dc0b0969495fb18051e7662f419e2ec9e29d3097180000000176569bff8e0bd88599a8ef30673f7957946c51111800000000b3a8522db82ae6ed03816080970f4eafd0b16760200000000067005d1488c9c9278d20cc62e39d3cc2c88648848723bee23e49c97c00205b572628f27f699f33c11d51193351360eb113a4908959b3f856ac3c8f262bdd1800000002849f42e35d4ca2ab6584d0945e33e58c0d0483971800000000b3a8522db82ae6ed03816080970f4eafd0b16760200000000067005d1488c9c9278d20cc62e39d3cc2c88648848723bee23e49c97c00010220c747a425e719bd1a6ea19b54dc0b0969495fb18051e7662f419e2ec9e29d3097205b572628f27f699f33c11d51193351360eb113a4908959b3f856ac3c8f262bdd"
};
// clang-format on

struct ParseVbkBlockIndex : public ::testing::TestWithParam<std::string> {};

TEST_P(ParseVbkBlockIndex, DISABLED_Parse) {
  BlockIndex<VbkBlock> index;
  auto data = ParseHex(GetParam());
  ReadStream stream(data);
  ValidationState state;
  ASSERT_TRUE(DeserializeFromVbkEncoding(stream, index, state))
      << state.toString();
}

INSTANTIATE_TEST_SUITE_P(VbkBlockIndex,
                         ParseVbkBlockIndex,
                         testing::ValuesIn(cases));