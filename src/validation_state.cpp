// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <veriblock/fmt.hpp>
#include <veriblock/validation_state.hpp>

namespace altintegration {

std::string ValidationState::GetPath() const {
  return fmt::format("{}", fmt::join(GetPathParts(), "+"));
}

std::vector<std::string> ValidationState::GetPathParts() const {
  std::vector<std::string> copy(stack_trace.size());
  std::reverse_copy(stack_trace.begin(), stack_trace.end(), copy.begin());
  return copy;
}

bool ValidationState::Invalid(const std::string& reject_reason,
                              const std::string& debug_message) {
  stack_trace.push_back(reject_reason);
  if (!debug_message.empty()) {
    m_debug_message = debug_message;
  }
  m_mode = MODE_INVALID;
  return false;
}

std::string ValidationState::toString() const {
  return GetPath() + ", " + GetDebugMessage();
}
}  // namespace altintegration