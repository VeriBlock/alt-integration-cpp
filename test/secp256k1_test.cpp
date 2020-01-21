#include <gtest/gtest.h>
#include "veriblock/secp256k1.h"

#include <vector>

#include "util/literals.hpp"

auto defaultPrivateKey =
    "aa4eca67cf7573eaedab283fb5f7c638ec0a9e99583a27288a07e5533b4d6d75"_unhex;
auto defaultMessage =
    "591b075a0e88b3d4449848ead2f69ff39b145747912c1690457455dd1c90ac64"_unhex;

auto defaultSignatureCompact =
    "870170004b938c1362e2a329a44ebbb1e4d24f43428466fcc978f4c40d835a52"
    "731ca13658c176a67d8466b5fb46225299985cf1e95b52c42d93205b943f47a3"_unhex;

TEST(SECP256K1, Verify) {
  secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN |
                                                    SECP256K1_CONTEXT_VERIFY);

  secp256k1_pubkey pubkey;
  secp256k1_ec_pubkey_create(ctx, &pubkey, defaultPrivateKey.data());

  secp256k1_ecdsa_signature signature;
  secp256k1_ecdsa_signature_parse_compact(
      ctx, &signature, defaultSignatureCompact.data());

  int ret = secp256k1_ecdsa_verify(ctx, &signature, defaultMessage.data(), &pubkey);
  secp256k1_context_destroy(ctx);

  EXPECT_EQ(ret, 1);
}
