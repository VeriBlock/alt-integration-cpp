// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_JSON_HPP
#define VERIBLOCK_POP_CPP_JSON_HPP

namespace altintegration {
namespace json {

template <typename Object>
Object makeEmptyObject();

template <typename Array>
Array makeEmptyArray();

template <typename Value, typename T>
Value ToJSON(const T& t);

template <typename Value>
void putKV(Value& object, const std::string& key, const Value& val);

template <typename Object>
void putStringKV(Object& object,
                 const std::string& key,
                 const std::string& value);

template <typename Object>
void putIntKV(Object& object, const std::string& key, int64_t value);

template <typename Object>
void putNullKV(Object& object, const std::string& key);

template <typename Value>
void arrayPushBack(Value& array, const Value& el);

template <typename Value, typename Iterable>
void putArrayKV(Value& object, const std::string& key, const Iterable& val) {
  auto arr = makeEmptyArray<Value>();
  for (const auto& it : val) {
    arrayPushBack(arr, ToJSON<Value>(it));
  }
  putKV(object, key, arr);
}

template <typename Object>
void putBoolKV(Object& object, const std::string& key, bool value);

}  // namespace json
}  // namespace altintegration

// header-only picojson adapter
#include <veriblock/adapters/picojson.hpp>

#endif  // VERIBLOCK_POP_CPP_JSON_HPP
