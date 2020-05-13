// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_LOGGER_LOGGER_HPP
#define ALT_INTEGRATION_LOGGER_LOGGER_HPP

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/printf.h>

#include <memory>
#include <string>

namespace altintegration {

enum class LogLevel { DEBUG = 0, INFO = 1, WARN = 2, ERROR = 3, OFF = 4 };

struct Logger {
  virtual ~Logger() = default;

  virtual void log(LogLevel lvl, const std::string& msg) {
    (void)lvl;
    (void)msg;
  }

  LogLevel level = LogLevel::INFO;
};

Logger& GetLogger();

std::string LevelToString(LogLevel l);

#ifdef VERIBLOCK_POP_LOGGER_ENABLED

#define VBK_LOG_FORMAT(format, ...) \
  fmt::sprintf(std::string("[POP] %s:") + format, __func__, ##__VA_ARGS__)

#define VBK_LOG(level, format, ...)                                \
  do {                                                             \
    if (GetLogger().level >= level) {                              \
      GetLogger().log(level, VBK_LOG_FORMAT(format, __VA_ARGS__)); \
    }                                                              \
  } while (0)

#define VBK_LOG_DEBUG(...) VBK_LOG(LogLevel::DEBUG, ##__VA_ARGS__)
#define VBK_LOG_INFO(...) VBK_LOG(LogLevel::INFO, ##__VA_ARGS__)
#define VBK_LOG_WARN(...) VBK_LOG(LogLevel::WARN, ##__VA_ARGS__)
#define VBK_LOG_ERROR(...) VBK_LOG(LogLevel::ERROR, ##__VA_ARGS__)

#else  // !VERIBLOCK_POP_LOGGER_ENABLED

#define VBK_LOG_DEBUG(...)
#define VBK_LOG_INFO(...)
#define VBK_LOG_WARN(...)
#define VBK_LOG_ERROR(...)

#endif  // VERIBLOCK_POP_LOGGER_ENABLED

}  // namespace altintegration

#endif  // ALT_INTEGRATION_LOGGER_LOGGER_HPP