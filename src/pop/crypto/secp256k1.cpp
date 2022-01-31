// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/crypto/secp256k1.hpp>

#include <utility>
#include <veriblock/pop/assert.hpp>

#include <veriblock/pop/hashutil.hpp>
#include <veriblock/pop/strutil.hpp>
#include <veriblock/pop/third_party/secp256k1.hpp>
#include <veriblock/pop/trace.hpp>

namespace altintegration {
namespace secp256k1 {

// NOLINTNEXTLINE
struct Secp256k1Context {
  secp256k1_context* ctx;

  Secp256k1Context(unsigned int flags) noexcept {
    ctx = secp256k1_context_create(flags);
  }

  ~Secp256k1Context() {
    if (ctx != nullptr) {
      secp256k1_context_destroy(ctx);
      ctx = NULL;
    }
  }

  operator secp256k1_context*() { return ctx; }
  operator const secp256k1_context*() const { return ctx; }
};

// according to Secp256k1 API doc, functions that take
// a const pointer to the context are thread-safe
static const Secp256k1Context ctx(SECP256K1_CONTEXT_VERIFY |
                                  SECP256K1_CONTEXT_SIGN);

// NOLINTNEXTLINE(cert-err58-cpp)
static const std::string ASN1_PREFIX_PRIVKEY =
    "303E020100301006072A8648CE3D020106052B8104000A042730250201010420";
// NOLINTNEXTLINE(cert-err58-cpp)
static const auto ASN1_PREFIX_PRIVKEY_BYTES = ParseHex(ASN1_PREFIX_PRIVKEY);

// NOLINTNEXTLINE(cert-err58-cpp)
static const std::string ASN1_PREFIX_PUBKEY =
    "3056301006072A8648CE3D020106052B8104000A034200";
// NOLINTNEXTLINE(cert-err58-cpp)
static const auto ASN1_PREFIX_PUBKEY_BYTES = ParseHex(ASN1_PREFIX_PUBKEY);

static const size_t PRIVATE_KEY_ASN1_SIZE =
    ASN1_PREFIX_PRIVKEY_BYTES.size() + PRIVATE_KEY_SIZE;

static const size_t PUBLIC_KEY_ASN1_SIZE =
    ASN1_PREFIX_PUBKEY_BYTES.size() + PUBLIC_KEY_UNCOMPRESSED_SIZE;

static void checkLength(size_t num,
                        size_t expected,
                        const std::string& message) {
  if (num != expected) {
    throw std::invalid_argument(message);
  }
}

static PrivateKey getPrivateKeyFromAsn1(Slice<const uint8_t> keyEncoded) {
  VBK_ASSERT(keyEncoded.size() == PRIVATE_KEY_ASN1_SIZE);
  return std::vector<uint8_t>(
      keyEncoded.begin() + ASN1_PREFIX_PRIVKEY_BYTES.size(), keyEncoded.end());
}

static PublicKey publicKeyUncompress(Slice<const uint8_t> publicKey) {
  VBK_ASSERT(publicKey.size() == PUBLIC_KEY_COMPRESSED_SIZE);
  secp256k1_pubkey pubkey;
  if (!(bool)secp256k1_ec_pubkey_parse(
          ctx, &pubkey, publicKey.data(), publicKey.size())) {
    throw std::invalid_argument("publicKeyUncompress(): invalid public key");
  }

  size_t outputlen = PUBLIC_KEY_UNCOMPRESSED_SIZE;
  std::vector<uint8_t> output(outputlen);
  if (!(bool)secp256k1_ec_pubkey_serialize(
          ctx, output.data(), &outputlen, &pubkey, SECP256K1_EC_UNCOMPRESSED)) {
    throw std::invalid_argument(
        "publicKeyUncompress(): public key serialize failed");
  }
  return output;
}

static std::vector<uint8_t> publicKeyUncompressedToAsn1(PublicKey publicKey) {
  VBK_ASSERT(publicKey.size() == PUBLIC_KEY_UNCOMPRESSED_SIZE);
  std::vector<uint8_t> result{};
  result.reserve(ASN1_PREFIX_PUBKEY_BYTES.size() + publicKey.size());
  result.insert(result.end(),
                ASN1_PREFIX_PUBKEY_BYTES.begin(),
                ASN1_PREFIX_PUBKEY_BYTES.end());
  result.insert(result.end(), std::begin(publicKey), std::end(publicKey));
  return result;
}

static PublicKey publicKeyAsn1ToUncompressed(Slice<const uint8_t> publicKey) {
  VBK_ASSERT(publicKey.size() == PUBLIC_KEY_ASN1_SIZE);
  std::vector<uint8_t> result{};
  result.reserve(PUBLIC_KEY_UNCOMPRESSED_SIZE);
  result.insert(result.end(),
                publicKey.end() - PUBLIC_KEY_UNCOMPRESSED_SIZE,
                publicKey.end());
  return result;
}

PrivateKey privateKeyFromVbk(PrivateKeyVbk key) {
  checkLength(key.size(),
              PRIVATE_KEY_ASN1_SIZE,
              "privateKeyFromVbk(): invalid private key");
  return getPrivateKeyFromAsn1(key);
}

PublicKey publicKeyFromVbk(PublicKeyVbk key) {
  if (key.size() == PUBLIC_KEY_ASN1_SIZE) {
    if (key[23] != 0x04) {
      throw std::invalid_argument(
          "publicKeyFromVbk(): unknown public key format");
    }
    return publicKeyAsn1ToUncompressed(key);
  }

  if (key.size() == PUBLIC_KEY_UNCOMPRESSED_SIZE) {
    if (key[0] != 0x04) {
      throw std::invalid_argument(
          "publicKeyFromVbk(): unknown public key format");
    }
    return std::vector<uint8_t>(key.begin(), key.end());
  }

  if (key.size() == PUBLIC_KEY_COMPRESSED_SIZE) {
    if (key[0] != 0x02 && key[0] != 0x03) {
      throw std::invalid_argument(
          "publicKeyFromVbk(): invalid key format, expected 0x02 or 0x03 as "
          "first byte");
    }
    return publicKeyUncompress(key);
  }

  throw std::invalid_argument("publicKeyFromVbk(): invalid public key");
}

PublicKeyVbk publicKeyToVbk(PublicKey key) {
  return publicKeyUncompressedToAsn1(std::move(key));
}

PublicKey derivePublicKey(PrivateKey privateKey) {
  secp256k1_pubkey pubkey;
  int pubCreated = secp256k1_ec_pubkey_create(ctx, &pubkey, privateKey.data());
  // should be always 1
  VBK_ASSERT(pubCreated == 1);

  size_t outputlen = PUBLIC_KEY_UNCOMPRESSED_SIZE;
  std::vector<uint8_t> output(outputlen);
  bool ret = (bool)secp256k1_ec_pubkey_serialize(
      ctx, output.data(), &outputlen, &pubkey, SECP256K1_EC_UNCOMPRESSED);
  VBK_ASSERT_MSG(ret, "can not serialize public key");
  return output;
}


#ifdef VBK_FUZZING_UNSAFE_FOR_PRODUCTION

// implementation for fuzzing

Signature sign(Slice<const uint8_t> message, PrivateKey privateKey) {
  (void)message;
  (void)privateKey;
  return {};
}

bool verify(Slice<const uint8_t> message,
            Signature signature,
            PublicKey publicKey) {
  (void)message;
  (void)signature;
  (void)publicKey;
  return true;
}
#else

// real implementation

Signature sign(Slice<const uint8_t> message, PrivateKey privateKey) {
  VBK_TRACE_ZONE_SCOPED;
  auto messageHash = sha256(message);

  secp256k1_ecdsa_signature signature;
  bool ret = (bool)secp256k1_ecdsa_sign(
      ctx, &signature, messageHash.data(), privateKey.data(), NULL, NULL);
  VBK_ASSERT_MSG(ret, "can not sign message");

  unsigned char sig[100]{};
  size_t outputlen = 100;
  secp256k1_ecdsa_signature_serialize_der(ctx, sig, &outputlen, &signature);
  return {sig, sig + outputlen};
}

bool verify(Slice<const uint8_t> message,
            Signature signature,
            PublicKey publicKey) {
  VBK_TRACE_ZONE_SCOPED;

  secp256k1_pubkey pubkey;
  if (0 == secp256k1_ec_pubkey_parse(
               ctx, &pubkey, publicKey.data(), publicKey.size())) {
    return false;
  }

  secp256k1_ecdsa_signature signatureDecoded;
  secp256k1_ecdsa_signature_parse_der(
      ctx, &signatureDecoded, signature.data(), signature.size());

  // FIXME: Fix this on the other side. We should accept the lower-S form only.
  secp256k1_ecdsa_signature normalizedSignature;
  secp256k1_ecdsa_signature_normalize(
      ctx, &normalizedSignature, &signatureDecoded);

  auto messageHash = sha256(message);
  return 0 != secp256k1_ecdsa_verify(
                  ctx, &normalizedSignature, messageHash.data(), &pubkey);
}
#endif

}  // namespace secp256k1
}  // namespace altintegration
