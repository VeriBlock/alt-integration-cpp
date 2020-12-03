// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <fmt/format.h>

#include <veriblock/assert.hpp>
#include <veriblock/logger.hpp>

namespace altintegration {

// NOLINTNEXTLINE(cert-err58-cpp)
static std::unique_ptr<Logger> logger = std::unique_ptr<Logger>(new Logger());

Logger& GetLogger() {
  VBK_ASSERT(logger != nullptr);
  return *logger;
}

void SetLogger(std::unique_ptr<Logger> lgr) {
  VBK_ASSERT(lgr != nullptr);
  logger = std::move(lgr);
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
  } else if (str == "info") {
    return LogLevel::info;
  } else if (str == "warn") {
    return LogLevel::warn;
  } else if (str == "error") {
    return LogLevel::error;
  } else if (str == "critical") {
    return LogLevel::critical;
  } else if (str == "off") {
    return LogLevel::off;
  } else {
    throw std::invalid_argument(
        fmt::sprintf("%s is not valid log level. Expected one of "
                     "debug/info/warn/error/critical/off"));
  }
}

}  // namespace altintegration