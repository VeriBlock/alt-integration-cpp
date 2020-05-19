// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/entities/test_case_entity.hpp>

#include <gtest/gtest.h>

#include "util/literals.hpp"
#include "util/test_utils.hpp"

using namespace altintegration;

TEST(TestCase_entity, deserialize_test) {
    const auto atvBytes = ParseHex(defaultAtvEncoded);
    auto stream = ReadStream(atvBytes);
    ATV atv = ATV::fromVbkEncoding(stream);

    const auto vtbBytes = ParseHex(defaultVtbEncoded);
    stream = ReadStream(vtbBytes);
    VTB vtb = VTB::fromVbkEncoding(stream);



    AltBlock block1;
    block1.hash = generateRandomBytesVector(32);
    block1.height = 1;
    block1.previousBlock = generateRandomBytesVector(32);
    block1.timestamp = 124;

    AltBlock block2;
    block2.hash = generateRandomBytesVector(32);
    block2.height = 2;
    block2.previousBlock = generateRandomBytesVector(32);
    block2.timestamp = 256;

    AltPayloads payloads;
    payloads.containingBlock.hash = generateRandomBytesVector(32);
    payloads.containingBlock.height = 123;
    payloads.containingBlock.previousBlock = generateRandomBytesVector(32);
    payloads.containingBlock.timestamp = 1234;
    payloads.endorsed.hash = generateRandomBytesVector(32);
    payloads.endorsed.height = 142;
    payloads.endorsed.previousBlock = generateRandomBytesVector(32);
    payloads.endorsed.timestamp = 3214;
    payloads.popData.hasAtv = true;
    payloads.popData.atv = atv;
    payloads.popData.version = 2;
    payloads.popData.vtbs = { vtb };

    std::vector<AltPayloads> vec_payloads1(4, payloads);
    std::vector<AltPayloads> vec_payloads2(8, payloads);
    

    TestCase expected;
    expected.alt_tree = { std::make_pair(block1,vec_payloads1), std::make_pair(block2, vec_payloads2) };
    
    std::vector<uint8_t> serialized_bytes = expected.toRaw();

    TestCase deserialized = TestCase::fromRaw(serialized_bytes);

    EXPECT_EQ(deserialized.alt_tree.size(), expected.alt_tree.size());

    for (size_t i = 0; i < expected.alt_tree.size(); ++i) {
        EXPECT_EQ(deserialized.alt_tree[i].first, expected.alt_tree[i].first);

        EXPECT_EQ(deserialized.alt_tree[i].second.size(), expected.alt_tree[i].second.size());
        for (const auto& p : deserialized.alt_tree[i].second) {
            EXPECT_EQ(p, payloads);
        }
    }
}