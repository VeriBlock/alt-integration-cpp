#pragma once

#include <stdint.h>
#include <stdlib.h>

namespace altintegration {
namespace progpow {

struct ethash_h256;

#define decsha3(bits) int sha3_##bits(uint8_t*, size_t, uint8_t const*, size_t);

decsha3(256);
decsha3(512);

inline void SHA3_256(ethash_h256* ret, uint8_t const* data, size_t const size) {
  sha3_256((uint8_t*)ret, 32, data, size);
}

inline void SHA3_512(uint8_t* ret, uint8_t const* data, size_t const size) {
  sha3_512(ret, 64, data, size);
}

}  // namespace progpow
}  // namespace altintegration