// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/entities/publication_data.hpp>

using namespace altintegration;

void PublicationData::toVbkEncoding(WriteStream& stream) const {
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
  return rawSize;
}

std::string PublicationData::toPrettyString() const {
  return format("PublicationData{{id={}, header={}, payoutInfo={}}}",
                identifier,
                HexStr(header),
                HexStr(payoutInfo));
}

bool altintegration::DeserializeFromVbkEncoding(ReadStream& stream,
                                                PublicationData& out,
                                                ValidationState& state) {
  if (!readSingleBEValue<int64_t>(stream, out.identifier, state)) {
    return state.Invalid("pub-identifier");
  }
  Slice<const uint8_t> header;
  if (!readVarLenValue(
          stream, header, state, 0, MAX_HEADER_SIZE_PUBLICATION_DATA)) {
    return state.Invalid("pub-header");
  }
  out.header = header.asVector();
  Slice<const uint8_t> contextInfo;
  if (!readVarLenValue(
          stream, contextInfo, state, 0, MAX_CONTEXT_SIZE_PUBLICATION_DATA)) {
    return state.Invalid("pub-context-info");
  }
  out.contextInfo = contextInfo.asVector();
  Slice<const uint8_t> payoutInfo;
  if (!readVarLenValue(stream, payoutInfo, state, 0, MAX_PAYOUT_INFO_SIZE)) {
    return state.Invalid("pub-payout-info");
  }
  out.payoutInfo = payoutInfo.asVector();
  return true;
}
