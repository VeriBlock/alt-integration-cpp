// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_FMTLOGGER_HPP
#define VERIBLOCK_POP_CPP_FMTLOGGER_HPP

#include <veriblock/pop/logger.hpp>

namespace altintegration {

struct FmtLogger : public Logger {
  ~FmtLogger() override = default;

  void log(LogLevel lvl, const std::string& msg) override {
    fmt::printf("[POP] [%s]\t%s \n", LevelToString(lvl), msg);
  }
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_FMTLOGGER_HPP
