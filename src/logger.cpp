// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <fmt/format.h>

#include <veriblock/logger.hpp>

namespace altintegration {

static std::unique_ptr<Logger> logger = std::unique_ptr<Logger>(new Logger());

Logger& GetLogger() {
  assert(logger != nullptr);
  return *logger;
}

void SetLogger(std::unique_ptr<Logger> lgr) {
  assert(lgr != nullptr);
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
    default:
      return "";
  }
}

}  // namespace altintegration