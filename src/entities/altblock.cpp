// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/altblock.hpp"
#include "veriblock/strutil.hpp"

namespace altintegration {

// NOLINTNEXTLINE(cert-err58-cpp)
const std::string AltBlock::_name = "ALT";

void AltBlock::toRaw(WriteStream& stream) const {
  writeSingleByteLenValue(stream, hash);
  writeSingleByteLenValue(stream, previousBlock);
  stream.writeBE<int32_t>(height);
  stream.writeBE<uint32_t>(timestamp);
}

std::vector<uint8_t> AltBlock::toRaw() const {
  WriteStream stream;
  toRaw(stream);
  return stream.data();
}

size_t AltBlock::estimateSize() const {
  size_t size = 0;
  size += singleByteLenValueSize(hash);
  size += singleByteLenValueSize(previousBlock);
  size += sizeof(height);
  size += sizeof(timestamp);
  return size;
}

uint32_t AltBlock::getBlockTime() const noexcept { return timestamp; }

AltBlock::hash_t AltBlock::getHash() const { return hash; }

bool operator==(const AltBlock& a, const AltBlock& b) {
  return a.hash == b.hash;
}
bool operator!=(const AltBlock& a, const AltBlock& b) { return !(a == b); }

std::string AltBlock::toPrettyString() const {
  return fmt::sprintf("AltBlock{height=%d, hash=%s}", height, HexStr(hash));
}
void AltBlock::toVbkEncoding(WriteStream& stream) const {
  return toRaw(stream);
}

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                AltBlock& out,
                                ValidationState& state,
                                const AltBlock::hash_t& /* ignore */) {
  return DeserializeFromRaw(stream, out, state);
}

bool DeserializeFromRaw(ReadStream& stream,
                        AltBlock& out,
                        ValidationState& state,
                        const AltBlock::hash_t& /* ignore */) {
  if (!readSingleByteLenValue(
          stream, out.hash, state, MIN_ALT_HASH_SIZE, MAX_ALT_HASH_SIZE)) {
    return state.Invalid("alt-block-hash");
  }
  if (!readSingleByteLenValue(stream,
                              out.previousBlock,
                              state,
                              MIN_ALT_HASH_SIZE,
                              MAX_ALT_HASH_SIZE)) {
    return state.Invalid("alt-block-prevhash");
  }
  if (!stream.readBE<int32_t>(out.height, state)) {
    return state.Invalid("alt-block-height");
  }
  if (!stream.readBE<uint32_t>(out.timestamp, state)) {
    return state.Invalid("alt-block-timestamp");
  }
  return true;
}

}  // namespace altintegration