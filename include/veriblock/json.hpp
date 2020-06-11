// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_JSON_HPP
#define VERIBLOCK_POP_CPP_JSON_HPP

namespace altintegration {

template <typename Value, typename T>
Value ToJSON(const T& t) {
  (void) t;
  static_assert(sizeof(T) == 0, "Undefined function for this type");
}

namespace json {

template <typename Object>
Object makeEmptyObject() {
  static_assert(sizeof(Object) == 0, "Undefined function for this type");
}

template <typename Array>
Array makeEmptyArray() {
  static_assert(sizeof(Array) == 0, "Undefined function for this type");
}

template <typename Value>
void putKV(Value& object, const std::string& key, const Value& val) {
  (void) object;
  (void) key;
  (void) val;
  static_assert(sizeof(Value) == 0, "Undefined function for this type");
}

template <typename Object>
void putStringKV(Object& object,
                 const std::string& key,
                 const std::string& value) {
  (void) object;
  (void) key;
  (void) value;
  static_assert(sizeof(Object) == 0, "Undefined function for this type");
}

template <typename Object>
void putIntKV(Object& object, const std::string& key, int64_t value) {
  (void) object;
  (void) key;
  (void) value;
  static_assert(sizeof(Object) == 0, "Undefined function for this type");
}

template <typename Object>
void putNullKV(Object& object, const std::string& key) {
  (void) object;
  (void) key;
  static_assert(sizeof(Object) == 0, "Undefined function for this type");
}

template <typename Value>
void arrayPushBack(Value& array, const Value& el) {
  (void) array;
  (void) el;
  static_assert(sizeof(Value) == 0, "Undefined function for this type");
}

template <typename Value, typename Iterable>
void putArrayKV(Value& object, const std::string& key, const Iterable& val) {
  auto arr = makeEmptyArray<Value>();
  for (const auto& it : val) {
    arrayPushBack(arr, ToJSON<Value>(it));
  }
  putKV(object, key, arr);
}

template <typename Object>
void putBoolKV(Object& object, const std::string& key, bool value) {
  (void) object;
  (void) key;
  (void) value;
  static_assert(sizeof(Object) == 0, "Undefined function for this type");
}

}  // namespace json
}  // namespace altintegration

// header-only picojson adapter
#include <veriblock/adapters/picojson.hpp>

#endif  // VERIBLOCK_POP_CPP_JSON_HPP
