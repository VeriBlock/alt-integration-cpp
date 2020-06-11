// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_PICOJSON_HPP
#define VERIBLOCK_POP_CPP_PICOJSON_HPP

#define PICOJSON_USE_INT64

#ifdef WIN32
#pragma warning(push, 0)
#endif

#include <veriblock/third_party/picojson.h>

#ifdef WIN32
#pragma warning(pop)
#endif

#include <veriblock/json.hpp>

namespace altintegration {

template <>
inline picojson::value ToJSON(const std::string& t) {
  return picojson::value(t);
}

namespace json {

template <>
inline picojson::value makeEmptyObject() {
  return picojson::value(picojson::object_type, true);
}

template <>
inline picojson::value makeEmptyArray() {
  return picojson::value(picojson::array_type, true);
}

template <>
inline void putKV(picojson::value& object,
                  const std::string& key,
                  const picojson::value& val) {
  object.get<picojson::object>()[key] = val;
}

template <>
inline void putStringKV(picojson::value& object,
                        const std::string& key,
                        const std::string& value) {
  object.get<picojson::object>()[key] = picojson::value(value);
}

template <>
inline void putIntKV(picojson::value& object,
                     const std::string& key,
                     int64_t value) {
  object.get<picojson::object>()[key] = picojson::value(value);
}

template <>
inline void putNullKV(picojson::value& object, const std::string& key) {
  object.get<picojson::object>()[key] = picojson::value();
}

template <>
inline void arrayPushBack(picojson::value& array, const picojson::value& val) {
  array.get<picojson::array>().push_back(val);
}

template <>
inline void putBoolKV(picojson::value& object,
                      const std::string& key,
                      bool value) {
  object.get<picojson::object>()[key] = picojson::value(value);
}

}  // namespace json
}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_PICOJSON_HPP
