#include "veriblock/signutil.hpp"

#include "veriblock/checks.hpp"
#include "veriblock/secp256k1.h"
#include "veriblock/strutil.hpp"
#include "veriblock/hashutil.hpp"

namespace VeriBlock {

struct Secp256k1Context {
  secp256k1_context* ctx{};

  ~Secp256k1Context() {
    if (ctx) {
      secp256k1_context_destroy(ctx);
      ctx = NULL;
    }
  }

  constexpr secp256k1_context* operator()() const { return ctx; }
};

static const std::string ASN1_PREFIX_PRIVKEY =
    "303E020100301006072A8648CE3D020106052B8104000A042730250201010420";
static const auto ASN1_PREFIX_PRIVKEY_BYTES = ParseHex(ASN1_PREFIX_PRIVKEY);

static const std::string ASN1_PREFIX_PUBKEY =
    "3056301006072A8648CE3D020106052B8104000A034200";
static const auto ASN1_PREFIX_PUBKEY_BYTES = ParseHex(ASN1_PREFIX_PUBKEY);

static const int PRIVATE_KEY_ASN1_SIZE =
    (int)ASN1_PREFIX_PRIVKEY_BYTES.size() + PRIVATE_KEY_SIZE;

static const int PUBLIC_KEY_ASN1_SIZE =
    (int)ASN1_PREFIX_PUBKEY_BYTES.size() + PUBLIC_KEY_UNCOMPRESSED_SIZE;

static std::vector<uint8_t> getPrivateKeyFromAsn1(Slice<uint8_t> keyEncoded) {
  checkRange(keyEncoded.size(), PRIVATE_KEY_ASN1_SIZE, PRIVATE_KEY_ASN1_SIZE);
  return std::vector<uint8_t>(
      keyEncoded.begin() + ASN1_PREFIX_PRIVKEY_BYTES.size(),
      keyEncoded.end());
}

static std::vector<uint8_t> publicKeyUncompress(Slice<uint8_t> publicKey) {
  checkRange(publicKey.size(),
             PUBLIC_KEY_COMPRESSED_SIZE, PUBLIC_KEY_COMPRESSED_SIZE);

  Secp256k1Context ctx{secp256k1_context_create(SECP256K1_CONTEXT_NONE)};

  secp256k1_pubkey pubkey;
  int res = secp256k1_ec_pubkey_parse(
      ctx(), &pubkey, publicKey.data(), publicKey.size());
  if (!res) {
    throw std::invalid_argument("publicKeyUncompress(): invalid public key");
  }

  size_t outputlen = PUBLIC_KEY_UNCOMPRESSED_SIZE;
  std::vector<uint8_t> output(outputlen);
  res = secp256k1_ec_pubkey_serialize(
      ctx(),
      output.data(),
      &outputlen,
      &pubkey,
      SECP256K1_EC_UNCOMPRESSED);

  if (!res) {
    throw std::invalid_argument("publicKeyUncompress(): public key serialize failed");
  }
  return output;
}

static std::vector<uint8_t> publicKeyUncompressedToAsn1(Slice<uint8_t> publicKey) {
  checkRange(publicKey.size(),
             PUBLIC_KEY_UNCOMPRESSED_SIZE, PUBLIC_KEY_UNCOMPRESSED_SIZE);
  std::vector<uint8_t> result{};
  result.reserve(ASN1_PREFIX_PUBKEY_BYTES.size() + publicKey.size());
  result.insert(
      result.end(), ASN1_PREFIX_PUBKEY_BYTES.begin(), ASN1_PREFIX_PUBKEY_BYTES.end());
  result.insert(result.end(), publicKey.begin(), publicKey.end());
  return result;
}

static std::vector<uint8_t> publicKeyAsn1ToUncompressed(Slice<uint8_t> publicKey) {
  checkRange(publicKey.size(), PUBLIC_KEY_ASN1_SIZE, PUBLIC_KEY_ASN1_SIZE);
  std::vector<uint8_t> result{};
  result.reserve(PUBLIC_KEY_UNCOMPRESSED_SIZE);
  result.insert(result.end(),
                publicKey.end() - PUBLIC_KEY_UNCOMPRESSED_SIZE,
                publicKey.end());
  return result;
}

std::vector<uint8_t> privateKeyFromVbk(Slice<uint8_t> key) {
  return getPrivateKeyFromAsn1(key);
}

std::vector<uint8_t> publicKeyFromVbk(Slice<uint8_t> key) {
  if (key.size() == PUBLIC_KEY_ASN1_SIZE) {
    return publicKeyAsn1ToUncompressed(key);
  }

  if (key.size() == PUBLIC_KEY_UNCOMPRESSED_SIZE) {
    return std::vector<uint8_t>(key.begin(), key.end());
  }

  if (key.size() == PUBLIC_KEY_COMPRESSED_SIZE) {
    return publicKeyUncompress(key);
  }

  throw std::invalid_argument("publicKeyFromVbk(): invalid public key");
}

std::vector<uint8_t> publicKeyToVbk(Slice<uint8_t> key) {
  return publicKeyUncompressedToAsn1(key);
}

std::vector<uint8_t> derivePublicKey(Slice<uint8_t> privateKey,
                                     bool compressed) {
  checkRange(privateKey.size(), PRIVATE_KEY_SIZE, PRIVATE_KEY_SIZE);
  Secp256k1Context ctx{secp256k1_context_create(SECP256K1_CONTEXT_NONE)};
  secp256k1_pubkey pubkey;
  secp256k1_ec_pubkey_create(ctx(), &pubkey, privateKey.data());

  size_t outputlen =
      compressed ? PUBLIC_KEY_COMPRESSED_SIZE : PUBLIC_KEY_UNCOMPRESSED_SIZE;
  std::vector<uint8_t> output(outputlen);
  int res = secp256k1_ec_pubkey_serialize(
      ctx(),
      output.data(),
      &outputlen,
      &pubkey,
      compressed ? SECP256K1_EC_COMPRESSED : SECP256K1_EC_UNCOMPRESSED);
  if (!res) {
    throw std::invalid_argument(
        "derivePublicKey(): public key serialize failed");
  }
  return output;
}

std::vector<uint8_t> sha256EcdsaSign(Slice<uint8_t> message,
                                     Slice<uint8_t> privateKey) {
  Secp256k1Context ctx{secp256k1_context_create(SECP256K1_CONTEXT_SIGN)};
  secp256k1_pubkey pubkey;
  secp256k1_ec_pubkey_create(ctx(), &pubkey, privateKey.data());

  auto messageHash = sha256(message);

  secp256k1_ecdsa_signature signature;
  int ret = secp256k1_ecdsa_sign(ctx(),
                                 &signature,
                                 messageHash.data(),
                                 privateKey.data(),
                                 NULL,
                                 NULL);

  if (!ret) {
    throw std::invalid_argument("sha256EcdsaSign(): cannot sign");
  }

  unsigned char sig[100]{};
  size_t outputlen = 100;
  secp256k1_ecdsa_signature_serialize_der(ctx(), sig, &outputlen, &signature);
  std::vector<uint8_t> output(sig, sig + outputlen);
  return output;
}

int sha256EcdsaVerify(Slice<uint8_t> message,
                      Slice<uint8_t> signatureEncoded,
                      Slice<uint8_t> publicKey) {
  Secp256k1Context ctx{secp256k1_context_create(SECP256K1_CONTEXT_VERIFY)};
  secp256k1_pubkey pubkey;
  secp256k1_ec_pubkey_parse(ctx(), &pubkey, publicKey.data(), publicKey.size());

  secp256k1_ecdsa_signature signature;
  secp256k1_ecdsa_signature_parse_der(
      ctx(), &signature, signatureEncoded.data(), signatureEncoded.size());

  auto messageHash = sha256(message);

  int ret = secp256k1_ecdsa_verify(ctx(), &signature, messageHash.data(), &pubkey);
  return ret;
}

}  // namespace VeriBlock
