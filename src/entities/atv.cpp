// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/atv.hpp"

#include "veriblock/hashutil.hpp"

using namespace altintegration;

const std::string ATV::_name = "ATV";

ATV ATV::fromVbkEncoding(ReadStream& stream) {
  ATV atv{};
  atv.version = stream.readBE<uint32_t>();
  if (atv.version == 1) {
    atv.transaction = VbkTx::fromVbkEncoding(stream);
    atv.merklePath = VbkMerklePath::fromVbkEncoding(stream);
    atv.blockOfProof = VbkBlock::fromVbkEncoding(stream);
  } else {
    throw std::domain_error(fmt::format(
        "ATV deserialization version={} is not implemented", atv.version));
  }

  return atv;
}

ATV ATV::fromVbkEncoding(Slice<const uint8_t> bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

void ATV::toVbkEncoding(WriteStream& stream) const {
  stream.writeBE<uint32_t>(version);
  if (version == 1) {
    transaction.toVbkEncoding(stream);
    merklePath.toVbkEncoding(stream);
    blockOfProof.toVbkEncoding(stream);
  } else {
    VBK_ASSERT_MSG(
        false, "ATV serialization version=%d is not implemented", version);
  }
}

std::vector<uint8_t> ATV::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

ATV ATV::fromHex(const std::string& h) {
  auto data = ParseHex(h);
  ReadStream stream(data);
  return ATV::fromVbkEncoding(stream);
}

ATV::id_t ATV::getId() const {
  auto left = transaction.getHash();
  auto right = blockOfProof.getHash();
  return sha256(left, right);
}

bool altintegration::Deserialize(ReadStream& stream,
                                 ATV& out,
                                 ValidationState& state) {
  ATV atv{};
  if (!stream.readBE<uint32_t>(atv.version, state)) {
    return state.Invalid("atv-version");
  }
  if (atv.version != 1) {
    return state.Invalid("atv-bad-version");
  }

  if (!Deserialize(stream, atv.transaction, state)) {
    return state.Invalid("atv-transaction");
  }
  if (!Deserialize(stream, atv.merklePath, state)) {
    return state.Invalid("atv-merkle-path");
  }
  if (!Deserialize(stream, atv.blockOfProof, state)) {
    return state.Invalid("atv-containing-block");
  }

  out = atv;
  return true;
}
