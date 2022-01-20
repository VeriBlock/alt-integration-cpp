// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_JSON_HPP
#define VERIBLOCK_POP_CPP_JSON_HPP

#include <string>

namespace altintegration {

/**
 * Encode given entity into JSON.
 * @tparam Value JSON Value type, which represents a "union" of JSON types.
 * @tparam T Entity type to encode.
 * @return JSON Value.
 */
template <typename Value, typename T>
Value ToJSON(const T& /*entity*/) {
  static_assert(sizeof(T) == 0, "Undefined function for this type");
}

/**
 * Package that contains adaptors to simplify serialization of entities to JSON.
 *
 * Depending on altchain, different JSON libraries can be used. Provide
 * specializations of methods from this namespace to use `ToJSON<T>(...)`.
 */
namespace json {

//! @cond Doxygen_Suppress

/**
 * Creates an empty JSON object.
 * @tparam Object type of JSON object.
 * @return empty JSON object.
 */
template <typename Object>
Object makeEmptyObject() {
  static_assert(sizeof(Object) == 0, "Undefined function for this type");
}

/**
 * Creates an empty JSON array.
 * @tparam Array type of JSON array.
 * @return empty JSON array.
 */
template <typename Array>
Array makeEmptyArray() {
  static_assert(sizeof(Array) == 0, "Undefined function for this type");
}

/**
 * Adds key-value pair to JSON Object.
 * @tparam Value union type for JSON types
 */
template <typename Value>
void putKV(Value& /*object*/,
           const std::string& /*key*/,
           const Value& /*value*/) {
  static_assert(sizeof(Value) == 0, "Undefined function for this type");
}

/**
 * Adds a key-value pair where `value` is `std::string`.
 * @tparam Object
 */
template <typename Object>
void putStringKV(Object& /*object to modify*/,
                 const std::string& /*key*/,
                 const std::string& /*value*/) {
  static_assert(sizeof(Object) == 0, "Undefined function for this type");
}

/**
 * Sets a key-value pair where `value` is `int64_t`.
 * @tparam Object
 */
template <typename Object>
void putIntKV(Object& /*object to modify*/,
              const std::string& /*key*/,
              int64_t /*value*/) {
  static_assert(sizeof(Object) == 0, "Undefined function for this type");
}

/**
 * Sets a key-value pair where `value` is `double`.
 * @tparam Object
 */
template <typename Object>
void putDoubleKV(Object& object, const std::string& key, double value) {
  json::putKV(object, key, ToJSON<Object>(value));
}

/**
 * Sets `object[key] = null`
 * @tparam Object
 */
template <typename Object>
void putNullKV(Object& /*object to modify*/, const std::string& /*key*/) {
  static_assert(sizeof(Object) == 0, "Undefined function for this type");
}

/**
 * Same as `std::vector::push_back` but for JSON array.
 * @tparam Value union type for JSON.
 */
template <typename Value>
void arrayPushBack(Value& /*array to modify*/, const Value& /*element*/) {
  static_assert(sizeof(Value) == 0, "Undefined function for this type");
}

/**
 * Sets `object[key] = value` for bools.
 * @tparam Object
 */
template <typename Object>
void putBoolKV(Object& /*object to modify*/,
               const std::string& /*key*/,
               bool /*value*/) {
  static_assert(sizeof(Object) == 0, "Undefined function for this type");
}

//! @endcond

template <typename Value, typename Iterable>
void putArrayKV(Value& object, const std::string& key, const Iterable& val) {
  auto arr = makeEmptyArray<Value>();
  for (const auto& it : val) {
    arrayPushBack(arr, ToJSON<Value>(it));
  }
  putKV(object, key, arr);
}

}  // namespace json
}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_JSON_HPP
