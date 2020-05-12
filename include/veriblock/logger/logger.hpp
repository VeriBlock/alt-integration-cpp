// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_LOGGER_LOGGER_HPP
#define ALT_INTEGRATION_LOGGER_LOGGER_HPP

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif //FMT_HEADER_ONLY

#include <veriblock/third_party/fmt/printf.h>

#include <memory>
#include <string>

namespace altintegration {

class Logger {
 public:
  virtual ~Logger() = default;
  virtual void debug(const std::string&) {}
  virtual void info(const std::string&) {}
  virtual void warn(const std::string&) {}
  virtual void error(const std::string&) {}
};

enum class LogLevel { LOG_NONE = 0, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR };

#ifdef VERIBLOCK_POP_LOGGER_ENABLED

template <typename LoggerType>
void NewLogger() {
  SetLogger(std::make_shared<LoggerType>());
}

void SetLogger(std::shared_ptr<Logger> logger);

std::shared_ptr<Logger> GetLogger();

std::string LevelToString(LogLevel level);

template <typename... Args>
inline void LogMessage(std::string cat,
                LogLevel level,
                std::string format,
                Args&&... args) {
  auto logger = GetLogger();
  if (logger == nullptr) return;

  std::string formatted = fmt::sprintf(format, std::forward<Args>(args)...);
  std::string msg = fmt::sprintf("(%s) %s", cat, formatted);

  switch (level) {
    case LogLevel::LOG_DEBUG:
      logger->debug(msg);
      return;
    case LogLevel::LOG_INFO:
      logger->info(msg);
      return;
    case LogLevel::LOG_WARN:
      logger->warn(msg);
      return;
    case LogLevel::LOG_ERROR:
      logger->error(msg);
      return;
    default:
      return;
  }
}

#define VBK_LOG_DEBUG(cat, ...) \
  LogMessage(cat, LogLevel::LOG_DEBUG, __VA_ARGS__)
#define VBK_LOG_INFO(cat, ...) \
  LogMessage(cat, LogLevel::LOG_INFO, __VA_ARGS__)
#define VBK_LOG_WARN(cat, ...) \
  LogMessage(cat, LogLevel::LOG_WARN, __VA_ARGS__)
#define VBK_LOG_ERROR(cat, ...) \
  LogMessage(cat, LogLevel::LOG_ERROR, __VA_ARGS__)

#else  // !VERIBLOCK_POP_LOGGER_ENABLED

template <typename LoggerType>
void NewLogger() {}

void SetLogger(std::shared_ptr<Logger>) {}

std::shared_ptr<Logger> GetLogger() { return nullptr; }

std::string LevelToString(LogLevel) { return ""; }

template <typename... Args>
inline void LogMessage(std::string,
                       LogLevel,
                       std::string,
                       Args&&...) {}

#define VBK_LOG_DEBUG(cat, ...)
#define VBK_LOG_INFO(cat, ...)
#define VBK_LOG_WARN(cat, ...)
#define VBK_LOG_ERROR(cat, ...)

#endif  // VERIBLOCK_POP_LOGGER_ENABLED

}  // namespace altintegration

#endif  // ALT_INTEGRATION_LOGGER_LOGGER_HPP