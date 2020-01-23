#ifndef __SIGNUTIL__HPP__
#define __SIGNUTIL__HPP__

#include <stdexcept>

#include "slice.hpp"

namespace VeriBlock {

std::vector<uint8_t> getPrivateKeyFromAsn1(Slice<uint8_t> keyEncoded);

std::vector<uint8_t> derivePublicKey(Slice<uint8_t> privateKey,
                               bool compressed = true);

std::vector<uint8_t> publicKeyUncompressedToAsn1(Slice<uint8_t> publicKey);

std::vector<uint8_t> sha256EcdsaSign(Slice<uint8_t> message,
                                     Slice<uint8_t> privateKey);

int sha256EcdsaVerify(Slice<uint8_t> message,
                      Slice<uint8_t> signatureEncoded,
                      Slice<uint8_t> publicKey);

}  // namespace VeriBlock

#endif  //__SIGNUTIL__HPP__
