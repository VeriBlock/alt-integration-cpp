// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_IO_HPP
#define VERIBLOCK_POP_CPP_STORAGE_IO_HPP

#include <exception>

namespace altintegration {

struct StorageIOException : public std::domain_error {
  StorageIOException(const std::string& msg) : std::domain_error(msg) {}
};

}  // namespace altintegration

#endif