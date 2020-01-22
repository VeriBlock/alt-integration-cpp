#include <gtest/gtest.h>

#include <cstdint>
#include <vector>
#include <veriblock/entities/hashes.hpp>

<<<<<<< HEAD
#include "veriblock/hashutil.hpp"

#include "util/literals.hpp"
=======
using namespace VeriBlock;
>>>>>>> fix btc serialization/deserialization. Add checkBtcBlock/checkVbkBlock validation.

struct TestCase {
  std::vector<uint8_t> data;
  std::vector<uint8_t> hashBytes;
  std::vector<uint8_t> hashTwiceBytes;
};

class Sha256Test : public testing::TestWithParam<TestCase> {};

//  test data
static std::vector<TestCase> g_Cases = {
    {""_v,
     "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"_unhex,
     "5df6e0e2761359d30a8275058e299fcc0381534545f55cf43e41983f5d4c9456"_unhex},

    {"ABC"_v,
     "b5d4045c3f466fa91fe2cc6abe79232a1a57cdf104f7a26e716e0a1e2789df78"_unhex,
     "3a00e44e7bb69f11d86866f1adfa75f5eb36c944b22a47fd98868634f05b5235"_unhex},

    {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"_v,
     "b4d5e56e929ba4cda349e9274e3603d0be246b82016bca20f363963c5f2d6845"_unhex,
     "85a3f45b7d73a684a39014cf00cf13a2026c63c7ae852bb67fc992cfe7a2e5b7"_unhex},

    {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"_v,
     "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"_unhex,
     "0cffe17f68954dac3a84fb1458bd5ec99209449749b2b308b7cb55812f9563af"_unhex}};

TEST_P(Sha256Test, OneShot) {
  auto tc = GetParam();
  auto result = VeriBlock::sha256(tc.data);
  EXPECT_EQ(result, tc.hashBytes);
}

TEST_P(Sha256Test, HashTwice) {
  auto tc = GetParam();
  auto result = VeriBlock::sha256twice(tc.data);
  EXPECT_EQ(result, tc.hashTwiceBytes);
}

TEST_P(Sha256Test, IUF) {
  auto tc = GetParam();
  std::vector<uint8_t> result(SHA256_HASH_SIZE, 0);

  VeriBlock::sha256_context ctx{};
  VeriBlock::sha256_init(&ctx);

  // hash of the whole array should equal to the hash of the same string, added
  // byte by byte through sha256_update
  for (auto c : tc.data) {
    std::vector<uint8_t> v{(uint8_t)c};
    VeriBlock::sha256_update(&ctx, v.data(), 1);
  }
  VeriBlock::sha256_finish(&ctx, result.data());
  EXPECT_EQ(result, tc.hashBytes);
}

INSTANTIATE_TEST_SUITE_P(Sha256Regression,
                         Sha256Test,
                         testing::ValuesIn(g_Cases));

TEST(Sha256Test, Sha256Hash_compareTest) {
  VeriBlock::Sha256Hash hash1(
      "00000000000000000000000000000000000000000000000000000000000000ff"_unhex);
  VeriBlock::Sha256Hash hash2(
      "0000000000000000000000000000000000000000000000000000000000000fff"_unhex);

  ASSERT_TRUE(hash2 > hash1);
  ASSERT_TRUE(hash1 < hash2);

  hash1 = VeriBlock::Sha256Hash(
      "00000000000000000000000000000000000000000000000000000000000033ff"_unhex);
  hash2 = VeriBlock::Sha256Hash(
      "00000000000000000000000000000000000000000000000000000000000032ff"_unhex);

  ASSERT_TRUE(hash1 > hash2);
  ASSERT_TRUE(hash2 < hash1);

  hash1 = VeriBlock::Sha256Hash(
      "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff322"_unhex);
  hash2 = VeriBlock::Sha256Hash(
      "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff322"_unhex);

  ASSERT_TRUE(hash1 == hash2);
}

TEST(Sha256Test, Sha256Hash_decodeBits) {
  Sha256Hash target = decodeBits<SHA256_HASH_SIZE>(0x04800000);

  EXPECT_EQ(
      target,
      Sha256Hash(
          "0000000000000000000000000000000000000000000000000000000000000000"_unhex));

  target = decodeBits<SHA256_HASH_SIZE>(0x01123456);

  EXPECT_EQ(
      target,
      Sha256Hash(
          "0000000000000000000000000000000000000000000000000000000000000012"_unhex));

  target = decodeBits<SHA256_HASH_SIZE>(0x02123456);

  EXPECT_EQ(
      target,
      Sha256Hash(
          "0000000000000000000000000000000000000000000000000000000000001234"_unhex));

  target = decodeBits<SHA256_HASH_SIZE>(0x03123456);

  EXPECT_EQ(
      target,
      Sha256Hash(
          "0000000000000000000000000000000000000000000000000000000000123456"_unhex));

  target = decodeBits<SHA256_HASH_SIZE>(0x04123456);

  EXPECT_EQ(
      target,
      Sha256Hash(
          "0000000000000000000000000000000000000000000000000000000012345600"_unhex));

  target = decodeBits<SHA256_HASH_SIZE>(0x05009234);

  EXPECT_EQ(
      target,
      Sha256Hash(
          "0000000000000000000000000000000000000000000000000000000092340000"_unhex));

  target = decodeBits<SHA256_HASH_SIZE>(0x20123456);

  EXPECT_EQ(
      target,
      Sha256Hash(
          "1234560000000000000000000000000000000000000000000000000000000000"_unhex));
}
