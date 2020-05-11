// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_LOGGER_HPP
#define ALT_INTEGRATION_LOGGER_HPP

#include <memory>
#include <string>
#include <veriblock/third_party/fmt/printf.h>

class Logger {
 public:
  virtual ~Logger() = default;
  virtual void info(std::string msg) = 0;
};

enum class LogLevel {
  LOG_NONE = 0,
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR
};

template <typename LoggerType>
void NewLogger() {
  SetLogger(std::make_shared<LoggerType>());
}

void SetLogger(std::shared_ptr<Logger> logger);

std::shared_ptr<Logger> GetLogger();

std::string LevelToString(LogLevel level);

template <typename... Args>
void LogMessage(std::string cat,
                LogLevel level,
                std::string format,
                const Args &... args) {
  auto logger = GetLogger();
  if (logger == nullptr) return;

  std::string formatted = fmt::sprintf(format, args...);
  std::string msg = fmt::sprintf("(%s) (%s) %s", LevelToString(level), cat, formatted);
  logger->info(msg);
}

#ifdef VERIBLOCK_POP_LOGGER_ENABLED
#define VBK_LOG_DEBUG(cat, format, ...) \
  LogMessage(cat, LogLevel::LOG_DEBUG, format, __VA_ARGS__)
#define VBK_LOG_INFO(cat, format, ...) \
  LogMessage(cat, LogLevel::LOG_INFO, format, __VA_ARGS__)
#define VBK_LOG_WARN(cat, format, ...) \
  LogMessage(cat, LogLevel::LOG_WARN, format, __VA_ARGS__)
#define VBK_LOG_ERROR(cat, format, ...) \
  LogMessage(cat, LogLevel::LOG_ERROR, format, __VA_ARGS__)
#else // !VERIBLOCK_POP_LOGGER_ENABLED
#define VBK_LOG_DEBUG(cat, format, ...) \
  do {                                 \
  } while (0)
#define VBK_LOG_INFO(cat, format, ...) \
  do {                                 \
  } while (0)
#define VBK_LOG_WARN(cat, format, ...) \
  do {                                 \
  } while (0)
#define VBK_LOG_ERROR(cat, format, ...) \
  do {                                 \
  } while (0)
#endif  // VERIBLOCK_POP_LOGGER_ENABLED

#endif  // ALT_INTEGRATION_LOGGER_HPP