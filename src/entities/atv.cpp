// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/atv.hpp"

#include "veriblock/hashutil.hpp"

using namespace altintegration;

const std::string ATV::_name = "ATV";

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

ATV::id_t ATV::getId() const {
  auto left = transaction.getHash();
  auto right = blockOfProof.getHash();
  return sha256(left, right);
}

std::string ATV::toPrettyString() const {
  return fmt::sprintf("ATV{containingTx=%s, containingBlock=%s}",
                      transaction.getHash().toHex(),
                      blockOfProof.getHash().toHex());
}

bool altintegration::DeserializeFromVbkEncoding(ReadStream& stream,
                                                ATV& atv,
                                                ValidationState& state) {
  if (!stream.readBE<uint32_t>(atv.version, state)) {
    return state.Invalid("atv-version");
  }
  if (atv.version != 1) {
    return state.Invalid(
        "atv-bad-version",
        fmt::format("Expected version=1, got={}", atv.version));
  }

  if (!DeserializeFromVbkEncoding(stream, atv.transaction, state)) {
    return state.Invalid("atv-transaction");
  }
  if (!DeserializeFromVbkEncoding(stream, atv.merklePath, state)) {
    return state.Invalid("atv-merkle-path");
  }
  if (!DeserializeFromVbkEncoding(stream, atv.blockOfProof, state)) {
    return state.Invalid("atv-containing-block");
  }
  return true;
}
