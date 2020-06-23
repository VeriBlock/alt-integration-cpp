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

struct PopData {
  using id_t = uint256;

  int32_t version{};

  std::vector<VbkBlock> context;
  std::vector<ATV> atvs{};
  std::vector<VTB> vtbs{};

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

  /**
   * Calculate a Payloads id that is the sha256 hash of the payloads rawBytes
   * @return id sha256 hash
   */

  id_t getHash() const;

  id_t getId() const { return getHash(); }

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
  json::putIntKV(obj, "version", p.version);

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

}  // namespace altintegration

#endif
