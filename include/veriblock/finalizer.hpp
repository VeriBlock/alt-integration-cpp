#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_FINALIZER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_FINALIZER_HPP_

#include <functional>

namespace VeriBlock {

/**
 * Finalizer holds a function that is executed on
 * Finalizer destruction.
 *
 */
struct Finalizer {
  explicit Finalizer(std::function<void()> func) : onDestroy(std::move(func)) {}
  ~Finalizer() { onDestroy(); }

 private:
  std::function<void()> onDestroy;
};

inline bool tryValidateWithResources(const std::function<bool()>& action,
                                     const std::function<void()>& finally) {
  try {
    // try
    if (!action()) {
      // invalid...
      finally();
      return false;
    }
    // valid
    return true;
  } catch (...) {
    // got exception... revert
    finally();
    throw;
  }
}

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_FINALIZER_HPP_
