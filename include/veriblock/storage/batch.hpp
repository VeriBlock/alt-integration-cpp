// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_BASE_BATCH_HPP
#define VERIBLOCK_POP_CPP_BASE_BATCH_HPP

#include <veriblock/serde.hpp>

#include "serialize.hpp"

namespace altintegration {

struct Batch {
  using key_t = std::vector<uint8_t>;
  using value_t = std::vector<uint8_t>;

  virtual ~Batch() = default;

  /**
   * Write a single KV. If Key exists, it will be overwritten with Value.
   */
  virtual void put(const key_t& key, const value_t& value) = 0;

  template <typename K, typename V>
  void putObject(const K& key, const V& value) {
    WriteStream k, v;
    Serialize(k, key);
    Serialize(v, value);
    this->put(k.data(), v.data());
  }

  /**
   * Efficiently commit given batch. Clears batch. Throws on failure.
   */
  virtual void commit() = 0;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_BASE_BATCH_HPP
