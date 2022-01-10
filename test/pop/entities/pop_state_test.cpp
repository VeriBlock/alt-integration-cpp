// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

// #include <veriblock/pop/blockchain/pop/pop_state.hpp"
// #include <veriblock/pop/entities/endorsements.hpp"

// #include <gtest/gtest.h>

// #include <veriblock/pop/literals.hpp"

// using namespace altintegration;

// static const PopState<VbkEndorsement> defaultVbkPopState{{
//     "1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"_unhex,
//     {"1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"_unhex,
//     "f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"_unhex,
//     "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1"_unhex,
//     "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1"_unhex,
//     "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"_unhex,
//     "f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"_unhex}}};

// TEST(PopState, Deserialize) {
//   std::vector<uint8_t> bytes = defaultBlock.toVbkEncoding();
//   AltBlock deserializedBlock =
//       AltBlock::fromVbkEncoding(std::string(bytes.begin(), bytes.end()));

//   EXPECT_EQ(deserializedBlock.getHash(), defaultBlock.getHash());
//   EXPECT_EQ(deserializedBlock.height, defaultBlock.height);
//   EXPECT_EQ(deserializedBlock.getBlockTime(), defaultBlock.getBlockTime());
// }
