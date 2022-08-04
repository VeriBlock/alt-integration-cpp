// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/entities/btctx.hpp>

#include "veriblock/pop/consts.hpp"
#include "veriblock/pop/crypto/vblake.hpp"
#include "veriblock/pop/hashutil.hpp"
#include "veriblock/pop/serde.hpp"
#include "veriblock/pop/slice.hpp"
#include "veriblock/pop/uint.hpp"
#include "veriblock/pop/validation_state.hpp"
#include "veriblock/pop/write_stream.hpp"

namespace altintegration {
struct ReadStream;
}  // namespace altintegration

using namespace altintegration;

void BtcTx::toVbkEncoding(WriteStream& stream) const {
  writeVarLenValue(stream, tx);
}

void BtcTx::toRaw(WriteStream& stream) const { stream.write(tx); }

size_t BtcTx::estimateSize() const {
  return varLenValueSize(tx);
}

uint256 BtcTx::getHash() const { return sha256twice(tx); }

bool altintegration::DeserializeFromVbkEncoding(ReadStream& stream,
                                                BtcTx& out,
                                                ValidationState& state) {
  Slice<const uint8_t> tx;
  if (!readVarLenValue(stream, tx, state, 0, BTC_TX_MAX_RAW_SIZE)) {
    return state.Invalid("btc-invalid-tx");
  }
  out = BtcTx(tx);
  return true;
}
