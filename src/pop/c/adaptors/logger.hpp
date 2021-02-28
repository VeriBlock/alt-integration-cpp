// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ADAPTORS_LOGGER_HPP
#define VERIBLOCK_POP_CPP_ADAPTORS_LOGGER_HPP

#include <veriblock/pop/c/extern.h>
#include <veriblock/pop/logger.hpp>

namespace adaptors {

struct Logger : altintegration::Logger {
  ~Logger() override = default;

  void log(altintegration::LogLevel lvl, const std::string& msg) override {
    VBK_Logger_log(altintegration::LevelToString(lvl).c_str(), msg.c_str());
  }
};

}  // namespace adaptors

#endif