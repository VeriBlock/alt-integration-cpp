#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_FMT_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_FMT_HPP_

/**
 * A facade for a third party formatting library.
 *
 */

namespace altintegration {

/**
 * A facade for a printf-like formatting.
 * @deprecated
 * TODO: refactor out all format occurrences, or find a way to do more type-safe
 * formatting, as this code is unsafe.
 */
template <typename... Args>
std::string format(const char* fmt, Args&&... args) {
  auto size = std::snprintf(nullptr, 0, fmt, std::forward<Args>(args)...);
  std::string output(size, '\0');
  std::snprintf(&output[0], size + 1, fmt, std::forward<Args>(args)...);
  return output;
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_FMT_HPP_
