#ifndef ALT_INTEGRATION_UTIL_HPP
#define ALT_INTEGRATION_UTIL_HPP

#include <string>
#include <vector>

std::vector<uint8_t> HexToBytes(const std::string& hex) {
  std::vector<uint8_t> bytes;

  for (unsigned long i = 0u; i < hex.length(); i += 2) {
    std::string byteString = hex.substr(i, 2);
    char byte = (char)strtol(byteString.c_str(), nullptr, 16);
    bytes.push_back(byte);
  }

  return bytes;
}

#endif // ALT_INTEGRATION_UTIL_HPP
