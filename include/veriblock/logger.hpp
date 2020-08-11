// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_LOGGER_LOGGER_HPP
#define ALT_INTEGRATION_LOGGER_LOGGER_HPP

#include <memory>
#include <string>

#include "veriblock/fmt.hpp"

namespace altintegration {

enum class LogLevel { debug, info, warn, error, critical, off };

struct Logger {
  virtual ~Logger() = default;

  virtual void log(LogLevel lvl, const std::string& msg) {
    (void)lvl;
    (void)msg;
  }

  LogLevel level = LogLevel::info;
};

Logger& GetLogger();
void SetLogger(std::unique_ptr<Logger> lgr);

template <typename L>
void SetLogger() {
  SetLogger(std::unique_ptr<L>(new L()));
}

std::string LevelToString(LogLevel l);
LogLevel StringToLevel(const std::string&);

#ifndef VERIBLOCK_POP_LOGGER_DISABLED

#ifndef VBK_LOG_FORMAT
#define VBK_LOG_FORMAT(format, ...) \
  fmt::sprintf(std::string("%s: ") + format, __func__, ##__VA_ARGS__)
#endif

#define VBK_LOG(lvl, format, ...)                                             \
  do {                                                                        \
    if (altintegration::GetLogger().level <= lvl) {                           \
      altintegration::GetLogger().log(lvl,                                    \
                                      VBK_LOG_FORMAT(format, ##__VA_ARGS__)); \
    }                                                                         \
  } while (0)

// clang-format off
#define VBK_LOG_DEBUG(format, ...) VBK_LOG(altintegration::LogLevel::debug, format, ##__VA_ARGS__)
#define VBK_LOG_INFO(format, ...)  VBK_LOG(altintegration::LogLevel::info, format, ##__VA_ARGS__)
#define VBK_LOG_WARN(format, ...)  VBK_LOG(altintegration::LogLevel::warn, format, ##__VA_ARGS__)
#define VBK_LOG_ERROR(format, ...) VBK_LOG(altintegration::LogLevel::error, format, ##__VA_ARGS__)
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