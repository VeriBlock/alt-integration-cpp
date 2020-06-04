// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_JSON_HPP
#define VERIBLOCK_POP_CPP_JSON_HPP

namespace altintegration {
namespace json {

template <typename T, typename Value>
Value ToJSON(const T& t);

template <typename Object>
void putStringKV(Object& object, const std::string& key, const std::string& value);

template <typename Object>
void putIntKV(Object& object, const std::string& key, int32_t value);

template <typename Object>
void putDoubleKV(Object& object, const std::string& key, double value);

template <typename Object>
void putObjectKV(Object& object, const std::string& key, const Object& value);

template <typename Object>
void putNullKV(Object& object, const std::string& key);

template <typename Object, typename Array>
void putArrayKV(Object& object, const std::string& key, const Array& array);

template <typename Object>
void putBoolKV(Object& object, const std::string& key, bool value);

}  // namespace json
}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_JSON_HPP
