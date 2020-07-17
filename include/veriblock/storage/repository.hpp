// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_REPOSITORY_HPP
#define VERIBLOCK_POP_CPP_REPOSITORY_HPP

#include <memory>
#include <veriblock/logger.hpp>

#include "batch.hpp"
#include "cursor.hpp"

namespace altintegration {

struct Repository {
  using key_t = std::vector<uint8_t>;
  using value_t = std::vector<uint8_t>;
  using cursor_t = Cursor<key_t, value_t>;
  using batch_t = Batch;

  virtual ~Repository() = default;

  virtual bool remove(const key_t& id) = 0;

  //! returns true if operation is successful, false otherwise
  virtual bool put(const key_t& key, const value_t& value) = 0;

  //! returns true if key exists, false otherwise/on error
  virtual bool get(const key_t& key, value_t* value = 0) const = 0;

  template <typename K, typename V>
  bool putObject(const K& key, const V& value) {
    WriteStream k, v;
    Serialize(k, key);
    Serialize(v, value);
    return this->put(k.data(), v.data());
  }

  template <typename K, typename V>
  bool getObject(const K& key, V* value) {
    WriteStream w;
    Serialize(w, key);

    value_t val;
    bool ret = get(w.data(), &val);
    // if value==nullptr or ret is false
    if (!value || !ret) {
      return ret;
    }

    ReadStream v(val);
    try {
      Deserialize<V>(v, *value);
      return true;
    } catch (const std::exception& e) {
      VBK_LOG_ERROR("Can not deserialize object with key %s: %s",
                    HexStr(w.data()),
                    e.what());
      return false;
    }
  }

  virtual std::unique_ptr<batch_t> newBatch() = 0;

  virtual std::shared_ptr<cursor_t> newCursor() const = 0;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_REPOSITORY_HPP
