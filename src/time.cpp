// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <atomic>
#include <veriblock/assert.hpp>
#include <veriblock/time.hpp>

namespace altintegration {

// for unit testing
static std::atomic<uint32_t> MockTime(0);

void setMockTime(uint32_t mocktime) {
  MockTime.store(mocktime, std::memory_order_relaxed);
}

uint32_t getMockTime() { return MockTime.load(std::memory_order_relaxed); }

uint32_t currentTimestamp4() {
  auto mocktime = MockTime.load(std::memory_order_relaxed);
  if (mocktime != 0) {
    return mocktime;
  }

  time_t now = time(nullptr);
  VBK_ASSERT(now > 0);
  return (uint32_t)now;
}
}  // namespace altintegration