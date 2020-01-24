#include <time.h>
#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_TIME_HPP
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_TIME_HPP

namespace VeriBlock {

uint32_t currentTimestamp4() { return (uint32_t)time(0); }

}  // namespace VeriBlock

#endif  // ! ALT_INTEGRATION_INCLUDE_VERIBLOCK
