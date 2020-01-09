#include "util/literals.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <veriblock/vblake.h>

struct TestCase {
  std::vector<uint8_t> message;
  std::vector<uint8_t> hash;
};

static std::vector<TestCase> cases = {

    { {/*empty*/},
      "235FCE01D9434261188E046AE97ACA9EDC8530AE042B586C"_unhex },

    { "61"_unhex, 
      "A2EEF086BF12745D3938C6B38AEFCB70AECF84AB79213851"_unhex},

    { "68656C6C6F20776F726C64"_unhex,
      "2F7D7C6455C04659F55C58E18A7B5A83301AD21FC28D6174"_unhex},

    { "00"_unhex, 
      "235FCE01D9434261188E046AE97ACA9EDC8530AE042B586C"_unhex},

    { "0000"_unhex, 
      "235FCE01D9434261188E046AE97ACA9EDC8530AE042B586C"_unhex},
    
    {"0001020304050607"_unhex,
     "52556D7B1B55ACF4E3D94724389E2640140E1FF9CC3B68F5"_unhex},
};

class VBlakeTest : public testing::TestWithParam<TestCase> {};

TEST_P(VBlakeTest, Regression) {
  auto tc = GetParam();

  std::vector<uint8_t> actual(VBLAKE_HASH_SIZE, 0);

  int ret = vblake(actual.data(), tc.message.data(), tc.message.size());

  ASSERT_EQ(ret, 0);
  EXPECT_EQ(actual, tc.hash);
}

INSTANTIATE_TEST_SUITE_P(VBlakeRegression, VBlakeTest, testing::ValuesIn(cases));
