#ifndef _BASE59_H
#define _BASE59_H

#include <string>
#include <vector>

namespace VeriBlock {

std::string EncodeBase59(const unsigned char *pbegin,
                         const unsigned char *pend);

std::string EncodeBase59(const uint8_t *buf, size_t size);

template <typename T,
          typename = typename std::enable_if<sizeof(typename T::value_type) ==
                                             1>::type>
std::string EncodeBase59(const T &container) {
  const uint8_t *ptr = container.data();
  return EncodeBase59(ptr, ptr + container.size());
}

std::vector<uint8_t> DecodeBase59(const std::string &str);

}  // namespace VeriBlock

#endif
