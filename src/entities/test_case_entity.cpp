// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/entities/test_case_entity.hpp>

namespace altintegration {

TestCase TestCase::fromRaw(const std::vector<uint8_t>& bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

TestCase TestCase::fromRaw(ReadStream& stream) {
  TestCase result;
  result.alt_tree = readArrayOf<alt_block_with_payloads_t>(
      stream,
      0,
      MAX_CONTEXT_COUNT,
      [&](ReadStream& stream) -> alt_block_with_payloads_t {
        AltBlock block = AltBlock::fromVbkEncoding(stream);

        std::vector<AltPayloads> payloads_vec = readArrayOf<AltPayloads>(
            stream,
            0,
            MAX_CONTEXT_COUNT,
            (AltPayloads(*)(ReadStream&))AltPayloads::fromVbkEncoding);

        return std::make_pair(std::move(block), std::move(payloads_vec));
      });

  return result;
}

std::vector<uint8_t> TestCase::toRaw() const {
  WriteStream stream;
  toRaw(stream);
  return stream.data();
}

void TestCase::toRaw(WriteStream& stream) const {
  writeSingleBEValue(stream, alt_tree.size());
  for (const auto& el : alt_tree) {
    el.first.toVbkEncoding(stream);
    writeSingleBEValue(stream, el.second.size());
    for (const auto& p : el.second) {
      p.toVbkEncoding(stream);
    }
  }
}

}  // namespace altintegration
