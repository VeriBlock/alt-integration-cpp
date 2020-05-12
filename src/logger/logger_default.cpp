// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/third_party/spdlog/spdlog.h>
#include <veriblock/logger/logger_default.hpp>

namespace altintegration {

void LoggerDefault::debug(const std::string& msg) { spdlog::debug(msg); }

void LoggerDefault::info(const std::string& msg) { spdlog::info(msg); }

void LoggerDefault::warn(const std::string& msg) { spdlog::warn(msg); }

void LoggerDefault::error(const std::string& msg) { spdlog::error(msg); }

}  // namespace altintegrati