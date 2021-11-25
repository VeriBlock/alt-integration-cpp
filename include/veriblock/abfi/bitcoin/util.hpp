#ifndef D843188C_779F_49BB_91EA_5EFE1E569F7D
#define D843188C_779F_49BB_91EA_5EFE1E569F7D

#include <vector>

#include "sip_hash.hpp"

namespace altintegration {
namespace abfi {
namespace btc {
CSipHasher GetDeterministicRandomizer(uint64_t seed0,
                                      uint64_t seed1,
                                      uint64_t id);

std::vector<uint8_t> Serialize(const char* command,
                               const std::vector<uint8_t>& body);
}  // namespace btc
}  // namespace abfi
}  // namespace altintegration

#endif /* D843188C_779F_49BB_91EA_5EFE1E569F7D */
