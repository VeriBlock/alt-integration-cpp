#include "veriblock/entities/btctx.hpp"

using namespace altintegration;

BtcTx BtcTx::fromVbkEncoding(ReadStream& stream) {
  return BtcTx(readVarLenValue(stream, 0, BTC_TX_MAX_RAW_SIZE));
}

void BtcTx::toVbkEncoding(WriteStream& stream) const {
  writeVarLenValue(stream, tx);
}

uint256 BtcTx::getHash() const { return sha256twice(tx); }

std::string BtcTx::toHex() const {
  return HexStr(tx);
}