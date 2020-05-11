// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_LOGGER_HPP
#define ALT_INTEGRATION_LOGGER_HPP

#include <memory>
#include <string>
#include <veriblock/third_party/fmt/printf.h>

class Logger {
 public:
  virtual ~Logger() = default;
  virtual void info(std::string msg) = 0;
};

template <typename LoggerType>
void NewLogger() {
  SetLogger(std::make_shared<LoggerType>());
}

void SetLogger(std::shared_ptr<Logger> logger);

std::shared_ptr<Logger> GetLogger();

template <typename... Args>
void LogInfo(std::string cat, std::string format, const Args &... args) {
  auto logger = GetLogger();
  if (logger == nullptr) return;

  std::string msg = "(";
  msg.append(cat);
  msg.append(") ");

  std::string formatted = fmt::sprintf(format, args...);
  msg.append(formatted);
  logger->info(msg);
}

#ifdef VERIBLOCK_POP_LOGGER_ENABLED
#define VBK_LOG_INFO(cat, format, ...) LogInfo(cat, format, __VA_ARGS__)
#else // !VERIBLOCK_POP_LOGGER_ENABLED
#define VBK_LOG_INFO(cat, format, ...) \
  do {} while (0)
#endif  // VERIBLOCK_POP_LOGGER_ENABLED

#endif  // ALT_INTEGRATION_LOGGER_HPP