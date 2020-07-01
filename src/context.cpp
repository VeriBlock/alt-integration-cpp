// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <atomic>
#include <veriblock/context.hpp>
#include <veriblock/logger.hpp>

namespace altintegration {

// If true, enables time-adjustment algorithm in POP Fork Resolution.
// False is used in tests, as we can not reliably reproduce block times for
// BTC/VBK.
bool EnableTimeAdjustment = true;

// If set to true, we should abandon any further work and exit ASAP.
// POP state may remain invalid if shutdown has been requested in a middle of
// huge POP fork resolution.
//
// atomic is needed, because this variable is likely to be modified from
// different thread than it is being read
static std::atomic_bool fShutdownRequested(false);

void RequestShutdown() {
  VBK_LOG_WARN("Shutdown requested");
  fShutdownRequested.store(true, std::memory_order_relaxed);
}

void AbortShutdown() {
  fShutdownRequested.store(false, std::memory_order_relaxed);
}

bool IsShutdownRequested() {
  return fShutdownRequested.load(std::memory_order_relaxed);
}

}  // namespace altintegration