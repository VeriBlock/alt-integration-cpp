// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_ADAPTORS_LOGGER_HPP
#define VERIBLOCK_POP_CPP_C_ADAPTORS_LOGGER_HPP

#include "veriblock/pop/c/extern.h"
#include "veriblock/pop/logger.hpp"

namespace adaptors {

struct Logger : altintegration::Logger {
  ~Logger() override = default;

  void log(altintegration::LogLevel lvl, const std::string& msg) override {
    std::string str_lvl = altintegration::LevelToString(lvl);

    POP_ARRAY_NAME(string) log_lvl_input;
    log_lvl_input.size = str_lvl.size();
    log_lvl_input.data = const_cast<char*>(str_lvl.data());

    POP_ARRAY_NAME(string) msg_input;
    msg_input.size = msg.size();
    msg_input.data = const_cast<char*>(msg.data());

    POP_EXTERN_FUNCTION_NAME(log)(log_lvl_input, msg_input);
  }
};

}  // namespace adaptors

#endif
