#ifndef __SIGNUTIL__HPP__
#define __SIGNUTIL__HPP__

#include <stdexcept>
#include <vector>
#include "slice.hpp"

namespace VeriBlock {

static const int PRIVATE_KEY_SIZE = 32;
static const int PUBLIC_KEY_COMPRESSED_SIZE = 33;
static const int PUBLIC_KEY_UNCOMPRESSED_SIZE = 65;

std::vector<uint8_t> privateKeyFromVbk(Slice<uint8_t> key);

std::vector<uint8_t> publicKeyFromVbk(Slice<uint8_t> key);

std::vector<uint8_t> publicKeyToVbk(Slice<uint8_t> key);

std::vector<uint8_t> derivePublicKey(Slice<uint8_t> privateKey,
                                     bool compressed);

std::vector<uint8_t> sha256EcdsaSign(Slice<uint8_t> message,
                                     Slice<uint8_t> privateKey);

int sha256EcdsaVerify(Slice<uint8_t> message,
                      Slice<uint8_t> signatureEncoded,
                      Slice<uint8_t> publicKey);

}  // namespace VeriBlock

#endif  //__SIGNUTIL__HPP__
