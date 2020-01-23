#include "veriblock/signutil.hpp"

#include <gtest/gtest.h>

#include "util/literals.hpp"

static auto defaultPrivateKeyVbk =
    "303E020100301006072A8648CE3D020106052B8104000A04273025020101042017"
    "869E398A7ACD18729B8FC6D47DCFE9C1A2B5871334D00471EFC3985762FF8F"_unhex;

static auto defaultPublicKeyVbk =
    "3056301006072a8648ce3d020106052b8104000a034200044b649515a30a4361dd"
    "875f8fad16c37142116217e5b8069c444773b59911bcce38782d7ba06c0b9b7713"
    "05d065279ce9f2288c8eab5328d260629085f7653504"_unhex;

static auto defaultMsg =
    "4cb778a158601701c98028b778e583859ef814ba1a57284fadef720a1dd5fbb7"_unhex;

static auto defaultSignatureVbk =
    "304402204d0e47f42007c6c1fd3cd6248b7a65d6c892147666f4a2e817b83d80db661f48022079307e54b1b841c0b531dd6270a485429be61800decae6b936741edbe3cd7800"_unhex;

TEST(SIGN_UTIL, GenPubKey) {
  auto privateKey = VeriBlock::privateKeyFromVbk(defaultPrivateKeyVbk);
  auto publicKey = VeriBlock::derivePublicKey(privateKey);
  auto publicKeyEncoded = VeriBlock::publicKeyToVbk(publicKey);

  auto publicKeyEncodedHex = VeriBlock::HexStr(publicKeyEncoded);
  EXPECT_EQ(publicKeyEncodedHex, VeriBlock::HexStr(defaultPublicKeyVbk));
}

/*TEST(SIGN_UTIL, Sign) {
  auto privateKey = VeriBlock::privateKeyFromVbk(defaultPrivateKeyVbk);
  auto signature = VeriBlock::veriBlockSign(defaultMsg, privateKey);
  auto signatureEncodedHex = VeriBlock::HexStr(signature);

  EXPECT_EQ(signatureEncodedHex, VeriBlock::HexStr(defaultSignatureVbk));
}*/

TEST(SIGN_UTIL, Verify) {
  auto privateKey = VeriBlock::privateKeyFromVbk(defaultPrivateKeyVbk);
  auto publicKey = VeriBlock::derivePublicKey(privateKey);
  int ret = VeriBlock::veriBlockVerify(defaultMsg, defaultSignatureVbk, publicKey);
  EXPECT_EQ(ret, 1);
}
