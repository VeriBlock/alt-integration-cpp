// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_LOGGER_LOGGER_HPP
#define ALT_INTEGRATION_LOGGER_LOGGER_HPP

#include <memory>
#include <string>

#include "veriblock/fmt.hpp"

/**
 * @file logger.hpp
 *
 * Defines logging helpers.
 *
 * By default, logger is disabled (no-op logger is used).
 * Users can derive from Logger class, and specify their logger instance.
 *
 * Used log levels:
 * - DEBUG - most noisy and verbose, debug level
 * - INFO - default recommended level for logs
 * - WARN - warning level
 * - ERROR - validation error happened
 * - CRITICAL - assert messages are logged here
 * - OFF - disable logs
 */

namespace altintegration {

enum class LogLevel { debug, info, warn, error, critical, off };

/**
 * @struct Logger
 *
 * An interface for logger. Implement it to enable logging in veriblock-pop-cpp.
 *
 * Example:
 * ```C++
 * using namespace altintegration;
 * // set MyLogger as main logger. Default is noop.
 * SetLogger<MyLogger>();
 * GetLogger().level = LogLevel::debug;
 * // see logs
 * ```
 *
 * @ingroup interfaces
 */
struct Logger {
  virtual ~Logger() = default;

  virtual void log(LogLevel lvl, const std::string& msg) {
    (void)lvl;
    (void)msg;
  }

  LogLevel level = LogLevel::off;
};

//! getter for global logger instance
Logger& GetLogger();
//! setter for global logger instance
void SetLogger(std::unique_ptr<Logger> lgr);

//! setter for global logger instance
template <typename L>
void SetLogger() {
  SetLogger(std::unique_ptr<L>(new L()));
}

//! convert loglevel to string
std::string LevelToString(LogLevel l);
//! convert string to loglevel
//! @throws std::invalid_argument
LogLevel StringToLevel(const std::string&);

#ifndef VERIBLOCK_POP_LOGGER_DISABLED

#ifndef VBK_LOG_FORMAT
//! @private
#define VBK_LOG_FORMAT(format, ...) \
  fmt::sprintf(std::string("%s: ") + format, __func__, ##__VA_ARGS__)
#endif

//! @private
#define VBK_LOG(lvl, format, ...)                             \
  do {                                                        \
    auto& logger = altintegration::GetLogger();               \
    if (logger.level <= lvl) {                                \
      logger.log(lvl, VBK_LOG_FORMAT(format, ##__VA_ARGS__)); \
    }                                                         \
  } while (0)

// clang-format off
//! Format and log line in DEBUG
#define VBK_LOG_DEBUG(format, ...) VBK_LOG(altintegration::LogLevel::debug, format, ##__VA_ARGS__)
//! Format and log line in INFO
#define VBK_LOG_INFO(format, ...)  VBK_LOG(altintegration::LogLevel::info, format, ##__VA_ARGS__)
//! Format and log line in WARN
#define VBK_LOG_WARN(format, ...)  VBK_LOG(altintegration::LogLevel::warn, format, ##__VA_ARGS__)
//! Format and log line in ERROR
#define VBK_LOG_ERROR(format, ...) VBK_LOG(altintegration::LogLevel::error, format, ##__VA_ARGS__)
//! Format and log line in CRITICAL
#define VBK_LOG_CRITICAL(format, ...) VBK_LOG(altintegration::LogLevel::critical, format, ##__VA_ARGS__)
// clang-format on

#else  // !VERIBLOCK_POP_LOGGER_DISABLED

#define VBK_LOG_DEBUG(...)
#define VBK_LOG_INFO(...)
#define VBK_LOG_WARN(...)
#define VBK_LOG_ERROR(...)
#define VBK_LOG_CRITICAL(...)

#endif  // VERIBLOCK_POP_LOGGER_DISABLED

}  // namespace altintegration

#endif  // ALT_INTEGRATION_LOGGER_LOGGER_HPP