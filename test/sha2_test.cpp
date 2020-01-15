#include <gtest/gtest.h>
#include <vector>
#include <veriblock/sha2.hpp>
#include <util/literals.hpp>

struct TestCase {
    std::vector<uint8_t> binData;
    std::string baseData;
};

class Sha256Test : public testing::TestWithParam<TestCase> {};


//  test data
static std::vector<TestCase> g_Cases = {
    {
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"_unhex,
        "",
    },

    {
        "b5d4045c3f466fa91fe2cc6abe79232a1a57cdf104f7a26e716e0a1e2789df78"_unhex,
        "ABC",
    },

    {
        "b4d5e56e929ba4cda349e9274e3603d0be246b82016bca20f363963c5f2d6845"_unhex,
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
    }
};


//  test while algo sha256
TEST_P(Sha256Test, Hashing) {
    auto tc = GetParam();
    EXPECT_EQ(VeriBlock::sha256(tc.baseData),
        tc.binData);
}

//  test sha256 Init via ctx
TEST(Sha256Test, Init) {
    sha2_context ctx = {};
    VeriBlock::sha256_init(&ctx);
    std::vector<uint8_t> baseCtx = "000000000000000067e6096a85ae67bb72f36e3c3af54fa57f520e518c68059babd9831f19cde05b\
                                    00000000000000000000000000000000000000000000000000000000000000000000000000000000\
                                    00000000000000000000000000000000000000000000000000000000000000000000000000000000\
                                    00000000000000000000000000000000000000000000000000000000000000000000000000000000\
                                    00000000000000000000000000000000000000000000000000000000000000000000000000000000\
                                    000000000000000000000000000000000000000000000000000000000000000000000000"_unhex;
     std::vector<uint8_t> curCtx((char*)&ctx, ((char*)&ctx) + sizeof(ctx));
     EXPECT_EQ(baseCtx, curCtx);
}

//  test sha256 Update via ctx
TEST(Sha256Test, Update) {
    sha2_context ctx = {};
    VeriBlock::sha256_init(&ctx);
    VeriBlock::sha256_update(&ctx, (unsigned char*)"ABC", 3);
    std::vector<uint8_t> baseCtx = "030000000000000067e6096a85ae67bb72f36e3c3af54fa57f520e518c68059babd9831f19cde05b\
                                    41424300000000000000000000000000000000000000000000000000000000000000000000000000\
                                    00000000000000000000000000000000000000000000000000000000000000000000000000000000\
                                    00000000000000000000000000000000000000000000000000000000000000000000000000000000\
                                    00000000000000000000000000000000000000000000000000000000000000000000000000000000\
                                    000000000000000000000000000000000000000000000000000000000000000000000000"_unhex;

    std::vector<uint8_t> curCtx((char*)&ctx, ((char*)&ctx) + sizeof(ctx));
    EXPECT_EQ(baseCtx, curCtx);
}

//  test sha256 Update via Finish
TEST(Sha256Test, Finish) {
    sha2_context ctx = {};
    VeriBlock::sha256_init(&ctx);
    VeriBlock::sha256_update(&ctx, (unsigned char*)"ABC", 3);
    auto result = VeriBlock::sha256_finish(&ctx);
    
    std::vector<uint8_t> baseCtx = "40000000000000005c04d4b5a96f463f6acce21f2a2379bef1cd571a6ea2f7041e0a6e7178df8927\
                                    41424380000000000000000000000000000000000000000000000000000000000000000000000000\
                                    00000000000000000000000000000000000000000000001800000000000000000000000000000000\
                                    00000000000000000000000000000000000000000000000000000000000000000000000000000000\
                                    00000000000000000000000000000000000000000000000000000000000000000000000000000000\
                                    000000000000000000000000000000000000000000000000000000000000000000000000"_unhex;

    std::vector<uint8_t> curCtx((char*)&ctx, ((char*)&ctx) + sizeof(ctx));
    EXPECT_EQ(baseCtx, curCtx);
}

INSTANTIATE_TEST_SUITE_P(Sha256Regression, Sha256Test,
    testing::ValuesIn(g_Cases));