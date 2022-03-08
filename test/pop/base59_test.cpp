// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <ostream>
#include <string>
#include <vector>
#include <veriblock/pop/base59.hpp>
#include <veriblock/pop/literals.hpp>

using namespace altintegration;

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
        "1b3554064e331d263902005f17590406075c233d1e5f0c601d"_unhex,
        "7ETU3XeGYHRpcxQcVUoZwo6g9pp1PBobct",
    },

    {
        "02090507090304050703080904060703070809060709050702090805020908050702"_unhex,
        "2EyFAa9EevJUFgDY6dP4HPZ7VVqx78LSz5tyXa7eFm79QH",
    },

    {
        "00000000000000000000000000000000000000000000000000000000000000000000"_unhex,
        "1111111111111111111111111111111111",
    },

    //  monorepo tests
    {"b4df3138d6b98a5a6350e653f4d6cb0c"_unhex, "GcJp9rfcTQfG104YwUGAk9"},

    {"56f1b691c6f17c9339cf01f8aec897ff2d0164b52432fbc2b8a629ba28767f5a8f4ed46e07943c3146efe65cff49404db56a9391b5508c804e208309b1145cf5cbfb19cff5253d7a548a9b8f64c5ffd72c2ce260294f07f41dbc467c76902d3859d5e5820ed9d8d260ae5d8eb842c520584d9a2955a16c79d1466f9ec098d8e8631839b88e90514c8015fa5de6ba1a8714f5f0ac28b841e3e456e587bb515e"_unhex,
     "t3oa01DSPMH9VGy5keycra5MAeZTnzALgvrrXe83smSvNxemVudixefWyVp5ejRRTvKHTthfx"
     "Qsez2QRU0jUqCEWsHKUHKBpBPXcvcX8XKvjJ4G1KZ3eciVxEYwe6QbTuHDajn80UUezFYoBMS"
     "EwwrXSoC3pWjQ3J7f8qphptwmf8GLQAChT2mc4ymDHsvMn5Rtq48ZyvisJxWs19XYHrofo"},

    {"70ec2c867278dc2b15916e5726c473e27f5d4862dcde3c9fff66f57a6a93d4f4574b2205f4c1f63e8b685eb2c2f6f29ec8458453d47270f9270df4f26314181a0ab4507bf5f1a1ae0b1403b3f38a28e41c7ade3eb2f9aa0327ed94a4e71cbb27c01e0926d85ee9a2b7fd002248b8503f31eac7981b8e54a7b8d053f8c530a9d80ac8540e4d335556e793986729537c4afbbf4fe1a7347b55ef68a0738d3d061c3f4e48798ea8a10ce662d6e5df1f42e6f6149cca8e7c6fdf654c"_unhex,
     "N1in4GkMb0EbTUfH5en7rs9ydrGMneJMhV2ATYcv8qgzAojrQLGmD470sqUJRojnG8ADykUpa"
     "Q6popgwRi7s38EyGJp7BGfLHy7tKBGwHBMxggKpKUJKxSgGzpM08KW1pxLH1ssYit5bT3YyYu"
     "XuAdhoZNbfKU3CzHA0wmEWrLzoKJGdC0y9YroHmb9DM5FjfynTDTExKYShDykFaqF4eGtrf91"
     "e4Y2kqawLtUaZH8fEhZZx0mhp3bKTYEDhx"},

    {"9c96bcc1f9ce89b46a90c4596aab871b03b36962719fa6e3b5761175f5af91b5d257556db8f0c391bc9e44ceb91b189071a155dd817f1d6c06019d18c6809b67988d84c1d33cb1ba4deb23dd8945fd0c19137110a8636522cc83898f4c21985ed4b3b15f2b98be6dc29638163a26a9d85136fe41b82f85283c6c1fc0b0a226b5874f4d5001102219"_unhex,
     "WYmwS4xUvCGNGfvMY6tGsaWfZ7VTEhCv53zNKG6MTayAVYLAvEWYhk7EVZrQpR90G37eFN4G4"
     "ZuBziNCRdqXxiyniAgWaUoBcZmGnEK4FnZscVRV5MPRhD6SHDU9WFUbAJjqV0yedvmFEwyTtC"
     "tFjt6bvSAs2Z1DK7DDpPtiQ8n9zy9JFpr0itXVL"},
};

TEST_P(Base59Test, Encode) {
  auto tc = GetParam();
  EXPECT_EQ(altintegration::EncodeBase59(tc.binData), tc.baseData);
}

TEST_P(Base59Test, Decode) {
  auto tc = GetParam();
  std::vector<uint8_t> decoded;
  altintegration::ValidationState state;
  EXPECT_TRUE(altintegration::DecodeBase59(tc.baseData, decoded, state))
      << state.toString();
  EXPECT_EQ(decoded, tc.binData);
}

INSTANTIATE_TEST_SUITE_P(Base59Regression,
                         Base59Test,
                         testing::ValuesIn(g_Cases));
