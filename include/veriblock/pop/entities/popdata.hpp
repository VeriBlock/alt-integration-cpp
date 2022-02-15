// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALT_POP_TRANSACTION_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALT_POP_TRANSACTION_HPP_

#include <stdint.h>

#include <vector>
#include <veriblock/pop/serde.hpp>
#include <veriblock/pop/slice.hpp>

#include "atv.hpp"
#include "vtb.hpp"

namespace altintegration {

/**
 * @struct PopData
 *
 * Represents ALT block body of POP-related info.
 *
 * @version 1
 */
struct PopData {
  using id_t = uint256;

  uint32_t version = 1;
  std::vector<VbkBlock> context;
  std::vector<VTB> vtbs{};
  std::vector<ATV> atvs{};

  uint256 getMerkleRoot() const;

  static uint256 getMerkleRoot(uint32_t version,
                               const std::vector<ATV::id_t>& atvs,
                               const std::vector<VTB::id_t>& vtbs,
                               const std::vector<VbkBlock::id_t>& vbks);

  void mergeFrom(const PopData& p) {
    context.insert(context.end(), p.context.begin(), p.context.end());
    vtbs.insert(vtbs.end(), p.vtbs.begin(), p.vtbs.end());
    atvs.insert(atvs.end(), p.atvs.begin(), p.atvs.end());
  }

  void clear() {
    context.clear();
    vtbs.clear();
    atvs.clear();
  }

  /**
   * Convert PopData to data stream using Vbk byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  /**
   * Convert PopData to raw bytes data using Vbk byte format
   * @return bytes data
   */
  std::vector<uint8_t> toVbkEncoding() const;

  size_t estimateSize() const;

  std::string toPrettyString() const;

  bool empty() const { return context.empty() && atvs.empty() && vtbs.empty(); }

  friend bool operator==(const PopData& a, const PopData& b) {
    // clang-format off
    return a.toVbkEncoding() == b.toVbkEncoding();
    // clang-format on
  }

  // if true, then this PopData and all its contents have been statelessly
  // checked already
  mutable bool checked = false;
};

namespace detail {

template <typename JsonValue, typename T>
inline void putArrayOfIds(JsonValue& obj,
                          std::string key,
                          const std::vector<T>& t) {
  JsonValue arr = json::makeEmptyArray<JsonValue>();
  for (const auto& b : t) {
    json::arrayPushBack(arr, ToJSON<JsonValue>(b.getId()));
  }

  json::putKV(obj, key, arr);
}

template <typename Value, typename Item>
void putArrayKV(Value& object,
                const std::string& key,
                const std::vector<Item>& val) {
  auto arr = json::makeEmptyArray<Value>();
  for (const auto& it : val) {
    json::arrayPushBack(arr, ToJSON<Value>(it));
  }
  json::putKV(object, key, arr);
}

}  // namespace detail

//! @overload
template <typename JsonValue>
JsonValue ToJSON(const PopData& p, bool verbose = false) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  json::putIntKV(obj, "version", p.version);
  if (verbose) {
    detail::putArrayKV(obj, "vbkblocks", p.context);
    detail::putArrayKV(obj, "vtbs", p.vtbs);
    detail::putArrayKV(obj, "atvs", p.atvs);
  } else {
    detail::putArrayOfIds(obj, "vbkblocks", p.context);
    detail::putArrayOfIds(obj, "vtbs", p.vtbs);
    detail::putArrayOfIds(obj, "atvs", p.atvs);
  }

  return obj;
}

//! @overload
bool DeserializeFromVbkEncoding(ReadStream& stream,
                                PopData& out,
                                ValidationState& state);

}  // namespace altintegration

#endif
