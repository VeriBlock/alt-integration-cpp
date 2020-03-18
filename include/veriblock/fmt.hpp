#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_FMT_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_FMT_HPP_

// disable MSVS warnings in this file
#pragma warning(push, 0)
#include <veriblock/third_party/tinyfmt.hpp>
#pragma warning(pop)

/**
 * A facade for a third party formatting library.
 *
 * Now it is tinyfmt, but can be any.
 */

namespace AltIntegrationLib {

/**
 * A facade for a printf-like formatting.
 */
template <typename... Args>
std::string format(const char* fmt, const Args&... args) {
  return tinyformat::format(fmt, args...);
}

}  // namespace AltIntegrationLib

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_FMT_HPP_
