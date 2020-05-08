// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/atv.hpp"

#include "veriblock/hashutil.hpp"

using namespace altintegration;

ATV ATV::fromVbkEncoding(ReadStream& stream) {
  ATV atv{};
  atv.transaction = VbkTx::fromVbkEncoding(stream);
  atv.merklePath = VbkMerklePath::fromVbkEncoding(stream);
  atv.containingBlock = VbkBlock::fromVbkEncoding(stream);
  atv.context =
      readArrayOf<VbkBlock>(stream,
                            0,
                            MAX_CONTEXT_COUNT_ALT_PUBLICATION,
                            (VbkBlock(*)(ReadStream&))VbkBlock::fromVbkEncoding

      );

  return atv;
}

ATV ATV::fromVbkEncoding(Slice<const uint8_t> bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

void ATV::toVbkEncoding(WriteStream& stream) const {
  transaction.toVbkEncoding(stream);
  merklePath.toVbkEncoding(stream);
  containingBlock.toVbkEncoding(stream);
  writeSingleBEValue(stream, context.size());
  for (const auto& block : context) {
    block.toVbkEncoding(stream);
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
  WriteStream stream;
  auto left = transaction.getHash();
  containingBlock.toVbkEncoding(stream);

  return sha256(left, stream.data());
}
