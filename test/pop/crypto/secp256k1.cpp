// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/crypto/secp256k1.hpp>
#include <veriblock/pop/literals.hpp>

using namespace altintegration;
using namespace secp256k1;

static auto defaultPrivateKeyVbk =
    "303e020100301006072a8648ce3d020106052b8104000a0427302502010104203abf83fa470423d4788a760ef6b7aae1dacf98784b0646057a0adca24e522acb"_unhex;

static auto defaultPublicKeyVbk =
    "3056301006072a8648ce3d020106052b8104000a034200042fca63a20cb5208c2a55ff5099ca1966b7f52e687600784d1de062c1dd9c8a5fe55b2ba5d906c703d37cbd02ecd9c97a806110fa05d9014a102a0513dd354ec5"_unhex;

static auto defaultMsg = "Hello world"_v;

static auto defaultSignatureVbk =
    "3044022008d07afee77324d0bced6f3bce19892d0413981e83e68401cd83d1e1ed3bc37c022005273429062dcf623ccd04c8d9c9e601e7fc45b5db32900c9b0ffda2dbc8f452"_unhex;

TEST(SIGN_UTIL, GenPubKey) {
  auto privateKey = privateKeyFromVbk(defaultPrivateKeyVbk);
  auto publicKey = derivePublicKey(privateKey);
  auto publicKeyEncoded = publicKeyToVbk(publicKey);

  auto publicKeyEncodedHex = HexStr(publicKeyEncoded);
  EXPECT_EQ(publicKeyEncodedHex, HexStr(defaultPublicKeyVbk));
}

TEST(SIGN_UTIL, Sign) {
  auto privateKey = privateKeyFromVbk(defaultPrivateKeyVbk);
  auto signature = sign(defaultMsg, privateKey);
  auto signatureEncodedHex = HexStr(signature);

  EXPECT_EQ(signatureEncodedHex, HexStr(defaultSignatureVbk));
}

TEST(SIGN_UTIL, Verify) {
  auto privateKey = privateKeyFromVbk(defaultPrivateKeyVbk);
  auto publicKey = derivePublicKey(privateKey);
  int ret = verify(defaultMsg, defaultSignatureVbk, publicKey);
  EXPECT_EQ(ret, 1);
}

TEST(SIGN_UTIL, Invalid) {
  std::vector<uint8_t> dummy(100, 1);
  EXPECT_THROW(privateKeyFromVbk(dummy), std::invalid_argument);
  ASSERT_DEATH(derivePublicKey(dummy), "");
  ASSERT_DEATH(publicKeyToVbk(dummy), "");
  ASSERT_DEATH(sign(dummy, dummy), "");
  ASSERT_DEATH(verify(dummy, dummy, dummy), "");
}
