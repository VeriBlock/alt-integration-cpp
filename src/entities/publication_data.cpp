// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/publication_data.hpp"

using namespace altintegration;

PublicationData PublicationData::fromRaw(const std::vector<uint8_t>& bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

PublicationData PublicationData::fromRaw(ReadStream& stream) {
  PublicationData pub;
  pub.identifier = readSingleBEValue<int64_t>(stream);
  pub.header =
      readVarLenValue(stream, 0, MAX_HEADER_SIZE_PUBLICATION_DATA).asVector();
  pub.contextInfo =
      readVarLenValue(stream, 0, MAX_CONTEXT_SIZE_PUBLICATION_DATA).asVector();
  pub.payoutInfo =
      readVarLenValue(stream, 0, MAX_PAYOUT_SIZE_PUBLICATION_DATA).asVector();
  return pub;
}

void PublicationData::toRaw(WriteStream& stream) const {
  writeSingleBEValue(stream, identifier);
  writeVarLenValue(stream, header);
  writeVarLenValue(stream, contextInfo);
  writeVarLenValue(stream, payoutInfo);
}

bool altintegration::Deserialize(ReadStream& stream,
                                 PublicationData& out,
                                 ValidationState& state) {
  PublicationData pub;
  if (!readSingleBEValue<int64_t>(stream, pub.identifier, state)) {
    return state.Invalid("invalid-identifier");
  }
  Slice<const uint8_t> header;
  if (!readVarLenValue(
          stream, header, state, 0, MAX_HEADER_SIZE_PUBLICATION_DATA)) {
    return state.Invalid("invalid-header");
  }
  pub.header = header.asVector();
  Slice<const uint8_t> contextInfo;
  if (!readVarLenValue(
          stream, contextInfo, state, 0, MAX_CONTEXT_SIZE_PUBLICATION_DATA)) {
    return state.Invalid("invalid-context-info");
  }
  pub.contextInfo = contextInfo.asVector();
  Slice<const uint8_t> payoutInfo;
  if (!readVarLenValue(
          stream, payoutInfo, state, 0, MAX_PAYOUT_SIZE_PUBLICATION_DATA)) {
    return state.Invalid("invalid-payout-info");
  }
  pub.payoutInfo = payoutInfo.asVector();
  out = pub;
  return true;
}
