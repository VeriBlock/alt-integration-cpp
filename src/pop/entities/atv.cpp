// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/entities/atv.hpp>
#include <veriblock/pop/hashutil.hpp>

using namespace altintegration;

// NOLINTNEXTLINE(cert-err58-cpp)
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

size_t ATV::estimateSize() const {
  size_t size = 0;
  size += sizeof(version);
  if (version == 1) {
    size += transaction.estimateSize();
    size += merklePath.estimateSize();
    size += blockOfProof.estimateSize();
  } else {
    VBK_ASSERT_MSG(
        false, "ATV estimate size version=%d is not implemented", version);
  }
  return size;
}

ATV::id_t ATV::getId() const {
  auto left = transaction.getHash();
  auto right = blockOfProof.getHash();
  return sha256(left, right);
}

std::string ATV::toShortPrettyString() const {
  return format("ATV(id={}, blockOfProof={}:{})",
                getId().toHex(),
                blockOfProof.getHeight(),
                blockOfProof.getHash());
}

std::string ATV::toPrettyString() const {
  return format("ATV{{containingTx={}, containingBlock={}}}",
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
    return state.Invalid("atv-bad-version",
                         format("Expected version=1, got={}", atv.version));
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
