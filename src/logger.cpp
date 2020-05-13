// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <fmt/format.h>

#include <veriblock/logger.hpp>

namespace altintegration {

static std::unique_ptr<Logger> logger = std::unique_ptr<Logger>(new Logger());

Logger& GetLogger() {
  return *logger;
}

std::string LevelToString(LogLevel l) {
  switch (l) {
    case LogLevel::DEBUG:
      return "debug";
    case LogLevel::INFO:
      return "info";
    case LogLevel::WARN:
      return "warn";
    case LogLevel::ERROR:
      return "error";
    default:
      return "";
  }
}

}  // namespace altintegration