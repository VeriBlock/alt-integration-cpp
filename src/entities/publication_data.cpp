// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/publication_data.hpp"

using namespace altintegration;

void PublicationData::toRaw(WriteStream& stream) const {
  writeSingleBEValue(stream, identifier);
  writeVarLenValue(stream, header);
  writeVarLenValue(stream, contextInfo);
  writeVarLenValue(stream, payoutInfo);
}

size_t PublicationData::estimateSize() const {
  size_t rawSize = 0;
  rawSize += singleBEValueSize(identifier);
  rawSize += varLenValueSize(header);
  rawSize += varLenValueSize(contextInfo);
  rawSize += varLenValueSize(payoutInfo);

  size_t size = 0;
  size += varLenValueSize(rawSize);
  return size;
}

std::string PublicationData::toPrettyString() const {
	return fmt::sprintf("PublicationData{id=%lld, header=%s, payoutInfo=%s}",
						identifier,
						HexStr(header),
						HexStr(payoutInfo));
}

bool altintegration::DeserializeFromVbkEncoding(ReadStream& stream,
                                 PublicationData& out,
                                 ValidationState& state) {
  PublicationData pub;
  if (!readSingleBEValue<int64_t>(stream, pub.identifier, state)) {
    return state.Invalid("pub-identifier");
  }
  Slice<const uint8_t> header;
  if (!readVarLenValue(
          stream, header, state, 0, MAX_HEADER_SIZE_PUBLICATION_DATA)) {
    return state.Invalid("pub-header");
  }
  pub.header = header.asVector();
  Slice<const uint8_t> contextInfo;
  if (!readVarLenValue(
          stream, contextInfo, state, 0, MAX_CONTEXT_SIZE_PUBLICATION_DATA)) {
    return state.Invalid("pub-context-info");
  }
  pub.contextInfo = contextInfo.asVector();
  Slice<const uint8_t> payoutInfo;
  if (!readVarLenValue(
          stream, payoutInfo, state, 0, MAX_PAYOUT_SIZE_PUBLICATION_DATA)) {
    return state.Invalid("pub-payout-info");
  }
  pub.payoutInfo = payoutInfo.asVector();
  out = pub;
  return true;
}
