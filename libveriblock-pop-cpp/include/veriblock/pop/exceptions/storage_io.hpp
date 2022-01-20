// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_IO_HPP
#define VERIBLOCK_POP_CPP_STORAGE_IO_HPP

#include <exception>

namespace altintegration {

//! exception which is thrown on any IO errors.
//! @warning This is FATAL error, and can not be recovered. If Altchain caught this exception, then POP state MAY BE INVALID.
struct StorageIOException : public std::domain_error {
  StorageIOException(const std::string& msg) : std::domain_error(msg) {}
};

}  // namespace altintegration

#endif