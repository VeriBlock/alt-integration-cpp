#ifndef _VBCRYPTO
#define _VBCRYPTO

#include <vector>
#include <string>

namespace Veriblock
{
    std::string          base58_encode(const void* buf, size_t size);
    std::vector<uint8_t> base58_decode(const std::string& str);
}

#endif 
