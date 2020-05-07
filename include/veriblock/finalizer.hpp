// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_FINALIZER_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_FINALIZER_HPP_

#include <functional>

namespace altintegration {

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

/// similar construction to try-with-resources in java but for "validation"
/// functions
inline bool tryValidateWithResources(const std::function<bool()>& action,
                                     const std::function<void()>& finally) {
  try {
    // try
    if (!action()) {
      // invalid...
      try {
        // if we get exception here, just rethrow.
        // do not invoke finally() again.
        finally();
      } catch (...) {
        throw;
      }
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

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_FINALIZER_HPP_
