#include "veriblock/signutil.hpp"

#include <gtest/gtest.h>

#include "util/literals.hpp"

static auto defaultPrivateKeyAsn1 =
    "303E020100301006072A8648CE3D020106052B8104000A04273025020101042017"
    "869E398A7ACD18729B8FC6D47DCFE9C1A2B5871334D00471EFC3985762FF8F"_unhex;

static auto defaultPublicKeyAsn1 =
    "3056301006072a8648ce3d020106052b8104000a034200044b649515a30a4361dd"
    "875f8fad16c37142116217e5b8069c444773b59911bcce38782d7ba06c0b9b7713"
    "05d065279ce9f2288c8eab5328d260629085f7653504"_unhex;

static auto defaultMsg =
    "4cb778a158601701c98028b778e583859ef814ba1a57284fadef720a1dd5fbb7"_unhex;

static auto defaultSignatureDer =
    "3045022100a09a4374161134803b05a65e0ec3bb63cebdfbd563436842fb08db"
    "62c1232c2f02201df9472377071bc7a7122c67c45df35f346401f19f66bf93ff"
    "8829b66fbb4bf8"_unhex;

TEST(SIGN_UTIL, GenPubKey) {
  auto privateKey = VeriBlock::privateKeyFromVbk(defaultPrivateKeyAsn1);
  auto publicKey = VeriBlock::derivePublicKey(privateKey);
  auto publicKeyEncoded = VeriBlock::publicKeyToVbk(publicKey);

  auto publicKeyEncodedHex = VeriBlock::HexStr(publicKeyEncoded);
  EXPECT_EQ(publicKeyEncodedHex, VeriBlock::HexStr(defaultPublicKeyAsn1));
}

TEST(SIGN_UTIL, Sign) {
  auto privateKey = VeriBlock::privateKeyFromVbk(defaultPrivateKeyAsn1);
  auto signature = VeriBlock::veriBlockSign(defaultMsg, privateKey);
  auto signatureEncodedHex = VeriBlock::HexStr(signature);

  //EXPECT_EQ(signatureEncodedHex, VeriBlock::HexStr(defaultSignatureDer));
}

TEST(SIGN_UTIL, Verify) {
  auto privateKey = VeriBlock::privateKeyFromVbk(defaultPrivateKeyAsn1);
  auto publicKey = VeriBlock::derivePublicKey(privateKey);
  int ret = VeriBlock::veriBlockVerify(
      defaultMsg, defaultSignatureDer, publicKey);
  EXPECT_EQ(ret, 1);
}
