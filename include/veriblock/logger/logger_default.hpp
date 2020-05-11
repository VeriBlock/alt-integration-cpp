// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_LOGGER_LOGGER_DEFAULT_HPP
#define ALT_INTEGRATION_LOGGER_LOGGER_DEFAULT_HPP

#include <veriblock/logger/logger.hpp>

namespace altintegration {

class LoggerDefault : public Logger {
 public:
  void debug(std::string msg);
  void info(std::string msg);
  void warn(std::string msg);
  void error(std::string msg);
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_LOGGER_LOGGER_DEFAULT_HPP