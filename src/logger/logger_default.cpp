// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/logger/logger_default.hpp>
#include <veriblock/third_party/spdlog/spdlog.h>

namespace altintegration {

void LoggerDefault::debug(std::string msg) {
  spdlog::debug(msg);
}

void LoggerDefault::info(std::string msg) { spdlog::info(msg); }

void LoggerDefault::warn(std::string msg) { spdlog::warn(msg); }

void LoggerDefault::error(std::string msg) {
  spdlog::error(msg);
}

}  // namespace altintegration
