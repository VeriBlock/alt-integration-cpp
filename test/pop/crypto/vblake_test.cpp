// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <vector>
#include <veriblock/pop/crypto/vblake.hpp>
#include <veriblock/pop/literals.hpp>

using namespace altintegration;

struct TestCase {
  std::vector<uint8_t> message;
  std::vector<uint8_t> hash;
};

static std::vector<TestCase> cases = {

    {{/*empty*/}, "235FCE01D9434261188E046AE97ACA9EDC8530AE042B586C"_unhex},

    {"61"_unhex, "A2EEF086BF12745D3938C6B38AEFCB70AECF84AB79213851"_unhex},

    {"68656C6C6F20776F726C64"_unhex,
     "2F7D7C6455C04659F55C58E18A7B5A83301AD21FC28D6174"_unhex},

    {"00"_unhex, "235FCE01D9434261188E046AE97ACA9EDC8530AE042B586C"_unhex},

    {"0000"_unhex, "235FCE01D9434261188E046AE97ACA9EDC8530AE042B586C"_unhex},

    {"0001020304050607"_unhex,
     "52556D7B1B55ACF4E3D94724389E2640140E1FF9CC3B68F5"_unhex},

    {"00001388000294E7DC3E3BE21A96ECCF0FBDF5F62A3331DC995C36B0935637860679DDD5DB0F135312B2C27867C9A83EF1B99B985C9B949307023AD672BAFD77"_unhex,
     "000000000000480D8196D5B0B41861D032377F5165BB4452"_unhex},
};

class VBlakeTest : public testing::TestWithParam<TestCase> {};

TEST_P(VBlakeTest, Regression) {
  auto tc = GetParam();

  std::vector<uint8_t> actual(VBLAKE_HASH_SIZE, 0);

  int ret = altintegration::vblake(
      actual.data(), tc.message.data(), tc.message.size());

  ASSERT_EQ(ret, 0);
  EXPECT_EQ(actual, tc.hash);
}

INSTANTIATE_TEST_SUITE_P(VBlakeRegression,
                         VBlakeTest,
                         testing::ValuesIn(cases));
