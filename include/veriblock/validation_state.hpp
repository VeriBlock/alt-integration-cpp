#ifndef ALT_INTEGRATION_VERIBLOCK_VALIDATION_STATE_HPP
#define ALT_INTEGRATION_VERIBLOCK_VALIDATION_STATE_HPP

#include <string>
#include <vector>

namespace AltIntegrationLib {

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
  bool Invalid(const std::string &function_name,
               const std::string &reject_reason = "",
               const std::string &debug_message = "") {
    stack_trace.push_back(function_name);
    m_reject_reason = reject_reason;
    m_debug_message = debug_message;
    if (m_mode != MODE_ERROR) {
      m_mode = MODE_INVALID;
    }

    return false;
  }

  bool addStackFunction(const std::string &function_name) {
    stack_trace.push_back(function_name);
    return false;
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
      m_reject_reason = reject_reason;
    }
    m_mode = MODE_ERROR;
    return false;
  }

  bool IsValid() const { return m_mode == MODE_VALID; }
  bool IsInvalid() const { return m_mode == MODE_INVALID; }
  bool IsError() const { return m_mode == MODE_ERROR; }
  std::string GetRejectReason() const { return m_reject_reason; }
  std::string GetDebugMessage() const { return m_debug_message; }

 private:
  enum mode_state {
    MODE_VALID,    //!< everything ok
    MODE_INVALID,  //!< network rule violation
    MODE_ERROR,    //!< run-time error
  } m_mode;
  std::string m_reject_reason;
  std::string m_debug_message;
  std::vector<std::string> stack_trace;
};

}  // namespace AltIntegrationLib

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_VALIDATION_STATE_HPP_
