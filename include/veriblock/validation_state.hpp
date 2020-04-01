#ifndef ALT_INTEGRATION_VERIBLOCK_VALIDATION_STATE_HPP
#define ALT_INTEGRATION_VERIBLOCK_VALIDATION_STATE_HPP

#include <string>
#include <vector>
#include <algorithm>

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

  /**
   * Changes this ValidationState into "INVALID" mode.
   * @param reject_reason - supply a short, unique message that identifies this
   * class of validation. Example: mandatory-script-verify-flag-failed
   * @param debug_message - supply arbitrary message that will help to debug the
   * error.
   * @return always returns false
   */
  ValidationState &Invalid(const std::string &reject_reason,
               const std::string &debug_message) {
    stack_trace.push_back(reject_reason);
    m_debug_message = debug_message;
    if (m_mode != MODE_ERROR) {
      m_mode = MODE_INVALID;
    }
    return *this;
  }

  bool Invalid(const std::string &function_name,
               const std::string &reject_reason,
               const std::string &debug_message) {
    Invalid(reject_reason, debug_message);
    stack_trace.push_back(function_name);
    return false;
  }

  //! during validation of arrays, additional index can be attached, meaning
  //! position of item in this array that is not valid.
  ValidationState &setIndex(size_t index_) {
    stack_trace.push_back(std::to_string(index_));
    return *this;
  }

  ValidationState &setStackFunction(const std::string &function_name) {
    stack_trace.push_back(function_name);
    return *this;
  }

  bool addStackFunction(const std::string &function_name) {
    ValidationState newState = setStackFunction(function_name);
    return newState.IsValid();
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
    std::string out = "";
    if (parts.size() == 0) return out;
    out += parts[0];
    for (size_t i = 1; i < parts.size(); i++) {
      out += "+";
      out += parts[i];
    }
    return out;
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
