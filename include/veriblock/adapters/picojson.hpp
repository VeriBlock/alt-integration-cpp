// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_PICOJSON_HPP
#define VERIBLOCK_POP_CPP_PICOJSON_HPP

#define PICOJSON_USE_INT64

#pragma warning(push, 0)
#include <veriblock/third_party/picojson.h>
#pragma warning(pop)

#include <veriblock/json.hpp>

namespace altintegration {
namespace json {

template <typename T>
inline picojson::value ToJSON(const T& t) {
  return picojson::value(t);
}

template <>
inline void putStringKV<picojson::object>(picojson::object& object,
                                          const std::string& key,
                                          const std::string& value) {
  object[key] = picojson::value(value);
}

template <>
inline void putIntKV<picojson::object>(picojson::object& object,
                                       const std::string& key,
                                       int32_t value) {
  object[key] = picojson::value((int64_t)value);
}

template <>
inline void putObjectKV<picojson::object>(picojson::object& object,
                                          const std::string& key,
                                          const picojson::object& value) {
  object[key] = picojson::value(value);
}

template <>
inline void putNullKV<picojson::object>(picojson::object& object,
                                        const std::string& key) {
  object[key] = picojson::value();
}

template <>
inline void putArrayKV(picojson::object& object,
                       const std::string& key,
                       const picojson::array& array) {
  picojson::array arr;
  for (auto& v : array) {
    arr.push_back(ToJSON(v));
  }

  object[key] = picojson::value(arr);
}

template <>
inline void putBoolKV(picojson::object& object,
                      const std::string& key,
                      bool value) {
  object[key] = picojson::value(value);
}

}  // namespace json
}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_PICOJSON_HPP
