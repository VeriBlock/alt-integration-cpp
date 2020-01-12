#include <gtest/gtest.h>
#include <vector>
#include <veriblock/base59.hpp>

struct TestCase {
    std::vector<uint8_t> binData;
    std::string baseData;
};

class Base59Test : public testing::TestWithParam<TestCase> {};

//  test data
static std::vector<TestCase> g_Cases = {
    {
        {},
        "",
    },

    {
        { 27, 53, 84, 6, 78, 51, 29, 38, 57, 2,  0,  95, 23,
        89, 4,  6,  7, 92, 35, 61, 30, 95, 12, 96, 29 },
        "7ETU3XeGYHRpcxQcVUoZwo6g9pp1PBobct",
    },

    {
        { 2, 9, 5, 7, 9, 3, 4, 5, 7, 3, 8, 9, 4, 6, 7, 3, 7,
        8, 9, 6, 7, 9, 5, 7, 2, 9, 8, 5, 2, 9, 8, 5, 7, 2 },
        "2EyFAa9EevJUFgDY6dP4HPZ7VVqx78LSz5tyXa7eFm79QH",
    },

    {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        "1111111111111111111111111111111111",
    },
};

TEST_P(Base59Test, Encode) {
    auto tc = GetParam();
    EXPECT_EQ(VeriBlock::EncodeBase59(tc.binData),
        tc.baseData);
}

TEST_P(Base59Test, Decode) {
    auto tc = GetParam();
    EXPECT_EQ(VeriBlock::DecodeBase59(tc.baseData), tc.binData);
}

INSTANTIATE_TEST_SUITE_P(Base59Regression, Base59Test,
    testing::ValuesIn(g_Cases));
