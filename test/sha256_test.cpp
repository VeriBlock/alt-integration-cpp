#include <gtest/gtest.h>
#include <veriblock/sha256.h>

#include <util/literals.hpp>
#include <vector>

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
    },
    {"248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"_unhex,
     "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"}};

TEST_P(Sha256Test, OneShot) {
  auto tc = GetParam();
  std::vector<uint8_t> result(32, 0);
  VeriBlock::sha256(result.data(),
                    reinterpret_cast<const uint8_t*>(tc.baseData.data()),
                    tc.baseData.size());
  EXPECT_EQ(result, tc.binData);
}

TEST_P(Sha256Test, IUF) {
  auto tc = GetParam();
  std::vector<uint8_t> result(32, 0);

  VeriBlock::sha256_context ctx{};
  VeriBlock::sha256_init(&ctx);

  // hash of the whole array should equal to the hash of the same string, added
  // byte by byte through sha256_update
  for (auto c : tc.baseData) {
    std::vector<uint8_t> v{(uint8_t)c};
    VeriBlock::sha256_update(&ctx, v.data(), 1);
  }
  VeriBlock::sha256_finish(&ctx, result.data());
  EXPECT_EQ(result, tc.binData);
}

INSTANTIATE_TEST_SUITE_P(Sha256Regression,
                         Sha256Test,
                         testing::ValuesIn(g_Cases));