// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_FMT_HPP
#define VERIBLOCK_POP_CPP_FMT_HPP

#include "logger.hpp"

#define FMT_HEADER_ONLY 1

#define VBK_FMT_VERSION 7.1.3
#include "third_party/fmt/format.h"
#include "third_party/fmt/printf.h"

namespace altintegration {

template <typename S, typename... Args>
inline std::string format(const S& format_str, Args&&... args) {
  try {
    return fmt::format(format_str, args...);
  } catch (const std::exception& e) {
    VBK_LOG_INFO("invalid string formatting, str: %s", format_str);
  }
  return "";
}

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_FMT_HPP
