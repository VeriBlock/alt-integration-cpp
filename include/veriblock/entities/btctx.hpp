#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCTX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCTX_HPP_

#include <cstdint>
#include <vector>

#include "veriblock/serde.hpp"
#include "veriblock/slice.hpp"

namespace VeriBlock {

struct BtcTx {
  BtcTx(Slice<const uint8_t> slice) : tx(slice.begin(), slice.end()) {}

  static BtcTx fromVbkEncoding(ReadStream& stream) {
    return BtcTx(readVarLenValue(stream, 0, BTC_TX_MAX_RAW_SIZE));
  }

  bool operator==(const BtcTx& other) const noexcept { return tx == other.tx; }

 private:
  std::vector<uint8_t> tx;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_BTCTX_HPP_
