// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/logger/logger.hpp>

namespace altintegration {

static std::shared_ptr<Logger> logger_;

void SetLogger(std::shared_ptr<Logger> logger) { logger_ = std::move(logger); }

std::shared_ptr<Logger> GetLogger() { return logger_; }

std::string LevelToString(LogLevel level) {
  switch (level) {
    case LogLevel::LOG_DEBUG:
      return "debug";
    case LogLevel::LOG_INFO:
      return "info";
    case LogLevel::LOG_WARN:
      return "warn";
    case LogLevel::LOG_ERROR:
      return "error";
    default:
      break;
  }
  return "";
}

}  // namespace altintegration
