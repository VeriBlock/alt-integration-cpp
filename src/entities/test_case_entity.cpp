// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/entities/test_case_entity.hpp>
#include <veriblock/hashutil.hpp>

namespace altintegration {

TestCase TestCase::fromRaw(const std::vector<uint8_t>& bytes) {
  ReadStream stream(bytes);
  return fromRaw(stream);
}

TestCase TestCase::fromRaw(ReadStream& stream) {
  uint256 expectedHashSum = stream.read(SHA256_HASH_SIZE);

  size_t savedPosition = stream.position();
  Slice<const uint8_t> data = stream.readSlice(stream.remaining());
  uint256 storedHashSum = sha256(data);
  if (storedHashSum != expectedHashSum) {
    throw std::invalid_argument("stream: hashSum is invalid");
  }
  stream.setPosition(savedPosition);

  TestCase result;
  result.alt_tree = readArrayOf<alt_block_with_payloads_t>(
      stream,
      0,
      MAX_CONTEXT_COUNT,
      [&](ReadStream& stream) -> alt_block_with_payloads_t {
        AltBlock block = AltBlock::fromVbkEncoding(stream);

        PopData popdata = PopData::fromVbkEncoding(stream);

        return std::make_pair(std::move(block), std::move(popdata));
      });

  result.config = Config::fromRaw(stream);

  return result;
}

std::vector<uint8_t> TestCase::toRaw() const {
  WriteStream stream;
  toRaw(stream);
  return stream.data();
}

void TestCase::toRaw(WriteStream& stream) const {
  WriteStream temp;
  writeSingleBEValue(temp, alt_tree.size());
  for (const auto& el : alt_tree) {
    el.first.toVbkEncoding(temp);
    el.second.toVbkEncoding(temp);
  }
  config.toRaw(temp);

  uint256 hashSum = sha256(temp.data());
  stream.write(hashSum.data(), SHA256_HASH_SIZE);
  stream.write(temp.data().data(), temp.data().size());
}

}  // namespace altintegration
