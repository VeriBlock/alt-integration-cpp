#include <veriblock/logger/logger.hpp>

static std::shared_ptr<Logger> logger_;

void SetLogger(std::shared_ptr<Logger> logger) { logger_ = std::move(logger); }

std::shared_ptr<Logger> GetLogger() { return logger_; }

std::string LevelToString(LogLevel level) {
  switch (level) {
    case LogLevel::LOG_NONE:
      return "";
    case LogLevel::LOG_DEBUG:
      return "debug";
    case LogLevel::LOG_INFO:
      return "info";
    case LogLevel::LOG_WARN:
      return "warn";
    case LogLevel::LOG_ERROR:
      return "error";
  }
  return "";
}
