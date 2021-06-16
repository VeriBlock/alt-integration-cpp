// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/assert.hpp>
#include <veriblock/pop/logger.hpp>

namespace altintegration {

static std::unique_ptr<Logger>& getLoggerInner() {
  // NOLINTNEXTLINE(cert-err58-cpp)
  static std::unique_ptr<Logger> logger = std::unique_ptr<Logger>(new Logger());
  return logger;
}

Logger& GetLogger() { return *getLoggerInner(); }

void SetLogger(std::unique_ptr<Logger> lgr, LogLevel log_lvl) {
  VBK_ASSERT(lgr != nullptr);

  getLoggerInner() = std::move(lgr);
  getLoggerInner()->level = log_lvl;
}

std::string LevelToString(LogLevel l) {
  switch (l) {
    case LogLevel::debug:
      return "debug";
    case LogLevel::info:
      return "info";
    case LogLevel::warn:
      return "warn";
    case LogLevel::error:
      return "error";
    case LogLevel::critical:
      return "critical";
    case LogLevel::off:
      return "off";
    default:
      return "";
  }
}

LogLevel StringToLevel(const std::string& str) {
  if (str == "debug") {
    return LogLevel::debug;
  }
  if (str == "info") {
    return LogLevel::info;
  }
  if (str == "warn") {
    return LogLevel::warn;
  }
  if (str == "error") {
    return LogLevel::error;
  }
  if (str == "critical") {
    return LogLevel::critical;
  }
  if (str == "off") {
    return LogLevel::off;
  }
  // default
  if (str == "") {
    return LogLevel::warn;
  }
  throw std::invalid_argument(
      fmt::format("{} is not valid log level. Expected one of "
                  "debug/info/warn/error/critical/off",
                  str));
}

}  // namespace altintegration