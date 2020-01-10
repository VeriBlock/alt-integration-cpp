#ifndef _BASE58_H
#define _BASE58_H

#include <string>
#include <vector>

namespace VeriBlock {

std::string EncodeBase58(const unsigned char *pbegin,
                         const unsigned char *pend);

// std::string EncodeBase58(const uint8_t *buf, size_t size);

template <typename T,
          typename = typename std::enable_if<sizeof(typename T::value_type) ==
                                             1>::type>
std::string EncodeBase58(const T &container) {
  const uint8_t *ptr = container.data();
  return EncodeBase58(ptr, ptr + container.size());
}

std::vector<uint8_t> DecodeBase58(const std::string &str);

}  // namespace VeriBlock

#endif
