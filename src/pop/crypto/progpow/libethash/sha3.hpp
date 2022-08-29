#pragma once

#include <cstdint>
#include <cstdlib>
#include <veriblock/pop/uint.hpp>

namespace altintegration {
namespace progpow {

#define decsha3(bits) int sha3_##bits(uint8_t*, size_t, uint8_t const*, size_t);

decsha3(256);
decsha3(512);

inline void SHA3_256(uint256* ret, uint8_t const* data, size_t const size) {
  sha3_256((uint8_t*)ret->data(), 32, data, size);
}

inline void SHA3_512(uint8_t* ret, uint8_t const* data, size_t const size) {
  sha3_512(ret, 64, data, size);
}

}  // namespace progpow
}  // namespace altintegration
