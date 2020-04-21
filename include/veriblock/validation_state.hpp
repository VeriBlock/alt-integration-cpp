#ifndef ALT_INTEGRATION_VERIBLOCK_VALIDATION_STATE_HPP
#define ALT_INTEGRATION_VERIBLOCK_VALIDATION_STATE_HPP

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace altintegration {

/**
 * Class that is used for storing validation state.
 *
 * ValidationState can be in 3 states:
 * - valid (when default initialized)
 * - invalid - validation failed. Example: invalid signature in transaction.
 * - error - internal error during validation. Example: database corrupted.
 */
class ValidationState {
 public:
  ValidationState() : m_mode(MODE_VALID) {}

  std::string toString() const { return GetPath() + ", " + GetDebugMessage(); }

  bool Invalid(const std::string &reject_reason,
               const std::string &debug_message = "") {
    stack_trace.push_back(reject_reason);
    if (!debug_message.empty()) {
      m_debug_message = debug_message;
    }
    if (m_mode != MODE_ERROR) {
      m_mode = MODE_INVALID;
    }
    return false;
  }

  /**
   * Changes this ValidationState into "INVALID" mode.
   * @param reject_reason - supply a short, unique message that identifies this
   * class of validation. Example: mandatory-script-verify-flag-failed
   * @param debug_message - supply arbitrary message that will help to debug the
   * error.
   * @return always returns false
   */
  bool Invalid(const std::string &reject_reason,
               const std::string &debug_message,
               size_t index) {
    stack_trace.push_back(std::to_string(index));
    return Invalid(reject_reason, debug_message);
  }

  bool Invalid(const std::string &reject_reason, size_t index) {
    return Invalid(reject_reason, "", index);
  }

  /**
   * Changes this ValidationState into "ERROR" mode.
   * @param reject_reason - supply a short, unique message that identifies this
   * class of validation. Example: mandatory-script-verify-flag-failed
   * @return always returns false
   */
  bool Error(const std::string &reject_reason) {
    // log first reject reason
    if (m_mode == MODE_VALID) {
      stack_trace.push_back(reject_reason);
    }
    m_mode = MODE_ERROR;
    return false;
  }

  bool IsValid() const { return m_mode == MODE_VALID; }
  bool IsInvalid() const { return m_mode == MODE_INVALID; }
  bool IsError() const { return m_mode == MODE_ERROR; }
  std::string GetDebugMessage() const { return m_debug_message; }

  std::vector<std::string> GetPathParts() const {
    std::vector<std::string> copy(stack_trace.size());
    std::reverse_copy(stack_trace.begin(), stack_trace.end(), copy.begin());
    return copy;
  }

  std::string GetPath() const {
    auto parts = GetPathParts();
    std::ostringstream ss;
    if (parts.empty()) {
      return {};
    }
    ss << parts[0];
    for (size_t i = 1; i < parts.size(); i++) {
      ss << "+";
      ss << parts[i];
    }
    return ss.str();
  }

 private:
  enum mode_state {
    MODE_VALID,    //!< everything ok
    MODE_INVALID,  //!< network rule violation
    MODE_ERROR,    //!< run-time error
  } m_mode;
  std::string m_debug_message;
  std::vector<std::string> stack_trace;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_VALIDATION_STATE_HPP_
