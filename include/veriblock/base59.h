#ifndef _BASE59_H
#define _BASE59_H

#include <string>
#include <vector>

namespace Veriblock {
    std::string base59_encode(const void *buf, size_t size);
    std::vector<uint8_t> base59_decode(const std::string &str);
} // namespace Veriblock


#endif
