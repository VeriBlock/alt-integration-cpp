// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_OUTPUT_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_OUTPUT_HPP_

#include "veriblock/entities/address.hpp"
#include "veriblock/entities/coin.hpp"
#include "veriblock/json.hpp"

namespace altintegration {

struct Output {
  Address address{};
  Coin coin{};

  Output() = default;

  Output(Address _address, Coin _coin)
      : address(std::move(_address)), coin(_coin) {}

  /**
   * Compare two Outputs for equality
   * @param other Outputs
   * @result comparing result
   */
  bool operator==(const Output& other) const noexcept;

  /**
   * Read VBK data from the stream and convert it to Output
   * @param stream data stream to read from
   * @return Output
   */
  static Output fromVbkEncoding(ReadStream& stream);

  /**
   * Convert Output to data stream using Output VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;
};

template <typename JsonValue>
JsonValue ToJSON(const Output& o) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  json::putStringKV(obj, "address", o.address.toString());
  json::putIntKV(obj, "coin", o.coin.units);
  return obj;
}

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_OUTPUT_HPP_
