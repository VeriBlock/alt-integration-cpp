// Copyright (c) 2019-2022 Xenios SEZC
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

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_FINALIZER_HPP_
