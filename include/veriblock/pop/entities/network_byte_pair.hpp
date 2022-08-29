// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_NETWORK_BYTE_PAIR_HPP
#define VERIBLOCK_POP_CPP_NETWORK_BYTE_PAIR_HPP

#include <cstdint>
#include <string>
#include <veriblock/pop/json.hpp>

namespace altintegration {

//! works as optional<uint8_t>
struct VbkNetworkType {
  //! if hasValue == false, it is mainnet
  bool hasValue = false;
  //! otherwise, use value for network ID
  uint8_t value = 0;

  operator bool() const { return hasValue; }

  bool operator==(const VbkNetworkType& other) const {
    if (!hasValue && !other.hasValue) {
      return true;
    }

    return hasValue && other.hasValue && value == other.value;
  }

  std::string toPrettyString() const {
    if (hasValue) {
      return std::to_string(value);
    }

    return {"null"};
  }

  template <typename JsonValue>
  void putJson(JsonValue& obj) const {
    if (hasValue) {
      json::putIntKV(obj, "networkByte", value);
    } else {
      json::putNullKV(obj, "networkByte");
    }
  }
};

/**
 * Stores pair of TxType and VBK network byte.
 */
struct NetworkBytePair {
  ///< works as std::optional. if hasNetworkByte is true, networkByte is set
  VbkNetworkType networkType;
  uint8_t typeId = 0;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_NETWORK_BYTE_PAIR_HPP
