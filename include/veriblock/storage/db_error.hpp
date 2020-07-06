// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_DB_ERROR_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_DB_ERROR_HPP_

namespace altintegration {

namespace db {

struct StateCorruptedException : public std::domain_error {
  using base = std::domain_error;
  using base::base;  // to inherit constructors
};

}  // namespace db

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_DB_ERROR_HPP_
