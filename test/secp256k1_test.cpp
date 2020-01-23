#include "veriblock/secp256k1.h"

#include <gtest/gtest.h>

#include <vector>

#include "util/literals.hpp"
#include "veriblock/slice.hpp"
#include "veriblock/strutil.hpp"

static auto defaultPrivateKey =
    "aa4eca67cf7573eaedab283fb5f7c638ec0a9e99583a27288a07e5533b4d6d75"_unhex;
static auto defaultMessage =
    "591b075a0e88b3d4449848ead2f69ff39b145747912c1690457455dd1c90ac64"_unhex;

static auto defaultSignatureCompact =
    "870170004b938c1362e2a329a44ebbb1e4d24f43428466fcc978f4c40d835a52"
    "731ca13658c176a67d8466b5fb46225299985cf1e95b52c42d93205b943f47a3"_unhex;

TEST(SECP256K1, Verify) {
  secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN |
                                                    SECP256K1_CONTEXT_VERIFY);

  secp256k1_pubkey pubkey;
  int res = secp256k1_ec_pubkey_create(ctx, &pubkey, defaultPrivateKey.data());
  EXPECT_EQ(res, 1);

  secp256k1_ecdsa_signature signature;
  secp256k1_ecdsa_signature_parse_compact(
      ctx, &signature, defaultSignatureCompact.data());

  int ret =
      secp256k1_ecdsa_verify(ctx, &signature, defaultMessage.data(), &pubkey);
  secp256k1_context_destroy(ctx);

  EXPECT_EQ(ret, 1);
}

TEST(SECP256K1, VerifyInvalid) {
  secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN |
                                                    SECP256K1_CONTEXT_VERIFY);

  secp256k1_pubkey pubkey;
  int res = secp256k1_ec_pubkey_create(ctx, &pubkey, defaultPrivateKey.data());
  EXPECT_EQ(res, 1);

  auto signatureCopy = std::vector<uint8_t>(defaultSignatureCompact);
  signatureCopy[0] = 0xAA;
  secp256k1_ecdsa_signature signature;
  secp256k1_ecdsa_signature_parse_compact(
      ctx, &signature, signatureCopy.data());

  int ret =
      secp256k1_ecdsa_verify(ctx, &signature, defaultMessage.data(), &pubkey);
  secp256k1_context_destroy(ctx);

  EXPECT_EQ(ret, 0);
}

TEST(SECP256K1, Sign) {
  secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

  secp256k1_pubkey pubkey;
  int res = secp256k1_ec_pubkey_create(ctx, &pubkey, defaultPrivateKey.data());
  EXPECT_EQ(res, 1);

  secp256k1_ecdsa_signature signature;
  int ret = secp256k1_ecdsa_sign(ctx,
                       &signature,
                       defaultMessage.data(),
                       defaultPrivateKey.data(),
                       NULL,
                       NULL);

  EXPECT_EQ(ret, 1);

  unsigned char sig[64]{};
  secp256k1_ecdsa_signature_serialize_compact(ctx, sig, &signature);
  secp256k1_context_destroy(ctx);

  VeriBlock::Slice<uint8_t> signatureSlice(sig, 64);
  auto signatureHex = VeriBlock::HexStr(signatureSlice);
  auto defaultSignatureHex = VeriBlock::HexStr(defaultSignatureCompact);

  EXPECT_EQ(signatureHex, defaultSignatureHex);
}
