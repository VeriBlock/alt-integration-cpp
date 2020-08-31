// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALT_POP_TRANSACTION_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_ALT_POP_TRANSACTION_HPP_

#include <stdint.h>

#include <vector>

#include "veriblock/entities/atv.hpp"
#include "veriblock/entities/vtb.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/slice.hpp"

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

  void mergeFrom(const PopData& p) {
    context.insert(context.end(), p.context.begin(), p.context.end());
    vtbs.insert(vtbs.end(), p.vtbs.begin(), p.vtbs.end());
    atvs.insert(atvs.end(), p.atvs.begin(), p.atvs.end());
  }

  size_t estimateSize() const { return toVbkEncoding().size(); }

  /**
   * Read VBK data from the stream and convert it to PopData
   * @param stream data stream to read from
   * @return PopData
   */
  static PopData fromVbkEncoding(ReadStream& stream);

  /**
   * Read VBK data from the raw byte representation and convert it to PopData
   * @param string data bytes to read from
   * @return PopData
   */
  static PopData fromVbkEncoding(Slice<const uint8_t> bytes);

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

  bool empty() const { return context.empty() && atvs.empty() && vtbs.empty(); }

  friend bool operator==(const PopData& a, const PopData& b) {
    // clang-format off
    return a.toVbkEncoding() == b.toVbkEncoding();
    // clang-format on
  }
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

}  // namespace detail

template <typename JsonValue>
JsonValue ToJSON(const PopData& p, bool verbose) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  if (verbose) {
    json::putArrayKV(obj, "vbkblocks", p.context);
    json::putArrayKV(obj, "vtbs", p.vtbs);
    json::putArrayKV(obj, "atv", p.atvs);
  } else {
    detail::putArrayOfIds(obj, "vbkblocks", p.context);
    detail::putArrayOfIds(obj, "vtbs", p.vtbs);
    detail::putArrayOfIds(obj, "atvs", p.atvs);
  }

  return obj;
}

bool Deserialize(ReadStream& stream, PopData& out, ValidationState& state);

}  // namespace altintegration

#endif