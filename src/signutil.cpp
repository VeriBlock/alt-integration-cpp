#include "veriblock/signutil.hpp"

#include "veriblock/secp256k1.h"
#include "veriblock/strutil.hpp"
#include "veriblock/hashutil.hpp"

namespace VeriBlock {

static const std::string ASN1_PREFIX_PUBKEY =
    "3056301006072A8648CE3D020106052B8104000A034200";
static const auto ASN1_PREFIX_PUBKEY_BYTES = ParseHex(ASN1_PREFIX_PUBKEY);

std::vector<uint8_t> getPrivateKeyFromAsn1(Slice<uint8_t> keyEncoded) {
  return std::vector<uint8_t>(keyEncoded.begin() + 32, keyEncoded.end());
}

std::vector<uint8_t> derivePublicKey(Slice<uint8_t> privateKey, bool compressed) {
  secp256k1_context* ctx =
      secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
  secp256k1_pubkey pubkey;
  secp256k1_ec_pubkey_create(ctx, &pubkey, privateKey.data());

  size_t outputlen = compressed ? 33 : 65;
  std::vector<uint8_t> output(outputlen);
  secp256k1_ec_pubkey_serialize(
      ctx,
      output.data(),
      &outputlen,
      &pubkey,
      compressed ? SECP256K1_EC_COMPRESSED : SECP256K1_EC_UNCOMPRESSED);
  secp256k1_context_destroy(ctx);
  return output;
}

std::vector<uint8_t> publicKeyUncompressedToAsn1(Slice<uint8_t> publicKey) {
  std::vector<uint8_t> result{};
  result.reserve(ASN1_PREFIX_PUBKEY_BYTES.size() + publicKey.size());
  result.insert(
      result.end(), ASN1_PREFIX_PUBKEY_BYTES.begin(), ASN1_PREFIX_PUBKEY_BYTES.end());
  result.insert(result.end(), publicKey.begin(), publicKey.end());
  return result;
}

std::vector<uint8_t> sha256EcdsaSign(Slice<uint8_t> message,
                                     Slice<uint8_t> privateKey) {
  secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

  secp256k1_pubkey pubkey;
  secp256k1_ec_pubkey_create(ctx, &pubkey, privateKey.data());

  auto messageHash = sha256(message);

  secp256k1_ecdsa_signature signature;
  int ret = secp256k1_ecdsa_sign(ctx,
                                 &signature,
                                 messageHash.data(),
                                 privateKey.data(),
                                 NULL,
                                 NULL);

  if (!ret) throw std::invalid_argument("sha256EcdsaSign(): cannot sign");

  unsigned char sig[100]{};
  size_t outputlen = 100;
  secp256k1_ecdsa_signature_serialize_der(ctx, sig, &outputlen, &signature);
  secp256k1_context_destroy(ctx);

  std::vector<uint8_t> output(sig, sig + outputlen);
  return output;
}

int sha256EcdsaVerify(Slice<uint8_t> message,
                      Slice<uint8_t> signatureEncoded,
                      Slice<uint8_t> publicKey) {
  secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN |
                                                    SECP256K1_CONTEXT_VERIFY);

  secp256k1_pubkey pubkey;
  secp256k1_ec_pubkey_parse(ctx, &pubkey, publicKey.data(), publicKey.size());

  secp256k1_ecdsa_signature signature;
  secp256k1_ecdsa_signature_parse_der(
      ctx, &signature, signatureEncoded.data(), signatureEncoded.size());

  auto messageHash = sha256(message);

  int ret = secp256k1_ecdsa_verify(ctx, &signature, messageHash.data(), &pubkey);
  secp256k1_context_destroy(ctx);
  return ret;
}

}  // namespace VeriBlock
