// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_PICOJSON_HPP
#define VERIBLOCK_POP_CPP_PICOJSON_HPP

#define PICOJSON_USE_INT64

#ifdef WIN32
#pragma warning(push, 0)
#endif

#include <veriblock/pop/third_party/picojson.h>

#ifdef WIN32
#pragma warning(pop)
#endif

#include <veriblock/pop/json.hpp>

namespace altintegration {

//! @private
template <>
inline picojson::value ToJSON(const std::string& t) {
  return picojson::value(t);
}

//! @private
template <>
inline picojson::value ToJSON(const int& t) {
  return picojson::value((int64_t)t);
}

//! @private
template <>
inline picojson::value ToJSON(const double& t) {
  return picojson::value(t);
}

//! @private
template <>
inline picojson::value ToJSON(const unsigned int& t) {
  return picojson::value((int64_t)t);
}

//! @private
template <>
inline picojson::value ToJSON(const picojson::value& t) {
  return t;
}

namespace json {

//! @private
template <>
inline picojson::value makeEmptyObject() {
  return picojson::value(picojson::object_type, true);
}

//! @private
template <>
inline picojson::value makeEmptyArray() {
  return picojson::value(picojson::array_type, true);
}

//! @private
template <>
inline void putKV(picojson::value& object,
                  const std::string& key,
                  const picojson::value& val) {
  object.get<picojson::object>()[key] = val;
}

//! @private
template <>
inline void putStringKV(picojson::value& object,
                        const std::string& key,
                        const std::string& value) {
  object.get<picojson::object>()[key] = picojson::value(value);
}

//! @private
template <>
inline void putIntKV(picojson::value& object,
                     const std::string& key,
                     int64_t value) {
  object.get<picojson::object>()[key] = picojson::value(value);
}

//! @private
template <>
inline void putNullKV(picojson::value& object, const std::string& key) {
  object.get<picojson::object>()[key] = picojson::value();
}

//! @private
template <>
inline void arrayPushBack(picojson::value& array, const picojson::value& val) {
  array.get<picojson::array>().push_back(val);
}

//! @private
template <>
inline void putBoolKV(picojson::value& object,
                      const std::string& key,
                      bool value) {
  object.get<picojson::object>()[key] = picojson::value(value);
}

}  // namespace json
}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_PICOJSON_HPP
