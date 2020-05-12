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
  void debug(const std::string& msg);
  void info(const std::string& msg);
  void warn(const std::string& msg);
  void error(const std::string& msg);
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_LOGGER_LOGGER_DEFAULT_HPP